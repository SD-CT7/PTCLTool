#include "ptcl/json/json.h"

#include "ptcl/json/jsonCommon.h"
#include "ptcl/json/jsonTexture.h"
#include "ptcl/json/jsonEmitter.h"
#include "ptcl/json/jsonEmitterSet.h"
#include "util/fileUtil.h"


#include <QDataStream>

#include <cstring>
#include <map>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

namespace Ptcl::Json {


// ========================================================================== //


bool exportProject(const PtclRes& res, const QString& dirPath) {
    QString nativePath = QDir::toNativeSeparators(dirPath);

    QDir projectDir{nativePath};

    projectDir.mkdir("textures");
    projectDir.mkdir("emitterSets");

    QDir texturesDir{projectDir.filePath("textures")};
    QDir emitterSetsDir{projectDir.filePath("emitterSets")};

    TextureIndexMap textureMap{};
    textureMap.reserve(res.textures().size());

    for (size_t idx = 0; idx < res.textures().size(); ++idx) {
        textureMap.emplace(res.textures()[idx].get(), idx);
    }

    QJsonObject projectJson{};
    projectJson["metaInfo"]    = createMetaInfo(FileKind::Project, 1);
    projectJson["name"]        = res.name();
    projectJson["textures"]    = exportTextures(res.textures(), texturesDir);
    projectJson["emitterSets"] = exportEmitterSets(res.getEmitterSets(), emitterSetsDir, textureMap);

    auto projectName = FileUtil::ensureExtention(res.name(), FileKind::Project);

    if (!writeJsonFile(projectJson, projectDir.filePath(projectName))) {
        return false;
    }

    return true;
}

bool exportEmitter(const Emitter& emitter, const QString& filePath) {
    auto emitterJson = emitterToJson(emitter, true, nullptr);

    if (!writeJsonFile(emitterJson, filePath)) {
        return false;
    }

    return true;
}

bool exportEmitterSet(const EmitterSet& emitterSet, const QString& filePath) {
    std::vector<const Texture*> uniqueTextures{};
    std::unordered_map<const Texture*, s32> textureToIndex{};

    const auto addTexture = [&](Texture* tex) {
        if (!tex || tex->isPlaceholder()) {
            return;
        }
        if (textureToIndex.find(tex) == textureToIndex.end()) {
            textureToIndex[tex] = static_cast<s32>(uniqueTextures.size());
            uniqueTextures.push_back(tex);
        }
    };

    for (const auto& emitter : emitterSet.emitters()) {
        addTexture(emitter->texture());
        addTexture(emitter->childTexture());
    }

    QJsonObject texturesJson{};
    for (const auto& [tex, idx] : textureToIndex) {
        texturesJson[QString::number(idx)] = textureToJson(*tex);
    }

    QJsonObject emittersJson{};
    for (s32 idx = 0; idx < emitterSet.emitterCount(); ++idx) {
        emittersJson[QString::number(idx)] = emitterToJson(*emitterSet.emitters().at(idx), false, &textureToIndex);
    }

    auto rootJson = emitterSetToJson(emitterSet);
    rootJson["textures"] = texturesJson;
    rootJson["emitters"] = emittersJson;

    if (!writeJsonFile(rootJson, filePath)) {
        return false;
    }

    return true;
}

std::optional<ImportEmitterResult> importEmitter(const QString& filePath, const QString& projectDir) {
    const auto emitterReadResult = readJsonFile(filePath);
    if (!emitterReadResult) {
        return std::nullopt;
    }
    const auto& emitterJson = *emitterReadResult;

    if (!validateMetaInfo(emitterJson["metaInfo"].toObject(), FileKind::Emitter, 1)) {
        return std::nullopt;
    }

    const QJsonValue texVal = emitterJson["texture"];
    const QJsonObject complexJson = emitterJson["complex"].toObject();
    const QJsonObject childJson = complexJson["child"].toObject();
    const QJsonValue childTexVal = childJson["texture"];

    const bool isStandalone = texVal.isObject() || childTexVal.isObject();

    if (isStandalone) {
        TextureList textures{};
        if (texVal.isObject()) {
            auto tex = textureFromJson(texVal.toObject());
            if (tex) {
                textures.push_back(std::make_unique<Texture>(std::move(*tex)));
            }
        }
        if (childTexVal.isObject()) {
            auto tex = textureFromJson(childTexVal.toObject());
            if (tex) {
                textures.push_back(std::make_unique<Texture>(std::move(*tex)));
            }
        }

        auto emitter = importEmitter(filePath, textures);

        if (!emitter) {
            return std::nullopt;
        }

        return ImportEmitterResult{std::make_unique<Emitter>(std::move(*emitter)), std::move(textures)};
    }

    QDir sourceProjectDir{};
    if (!projectDir.isEmpty()) {
        sourceProjectDir = QDir{projectDir};
    } else {
        const QFileInfo emitterFileInfo{filePath};
        sourceProjectDir = QDir{emitterFileInfo.absolutePath()};
        sourceProjectDir.cdUp(); // emitterSets
        sourceProjectDir.cdUp(); // project root
    }

    const QDir texDir{sourceProjectDir.filePath("textures")};
    if (!texDir.exists()) {
        return std::nullopt;
    }

    const auto projFileExt = FileUtil::fileExtention(FileKind::Project);
    const auto projFiles = sourceProjectDir.entryList({projFileExt}, QDir::Files);
    if (projFiles.isEmpty()) {
        return std::nullopt;
    }

    const auto projReadResult = readJsonFile(sourceProjectDir.filePath(projFiles.first()));
    if (!projReadResult) {
        return std::nullopt;
    }
    const auto& projJson = *projReadResult;

    auto sourceTextures = importTextures(projJson["textures"].toObject(), sourceProjectDir);
    if (!sourceTextures) {
        return std::nullopt;
    }

    auto emitter = importEmitter(filePath, *sourceTextures);
    if (!emitter) {
        return std::nullopt;
    }

    TextureList resultTextures{};
    std::map<Texture*, Texture*> sourceToReIded{};

    const auto getOrCreateReIded = [&](Texture* sourceTex) -> Texture* {
        if (!sourceTex || sourceTex->isPlaceholder()) {
            return nullptr;
        }

        auto it = sourceToReIded.find(sourceTex);
        if (it != sourceToReIded.end()) {
            return it->second;
        }

        std::vector<u8> data{sourceTex->textureDataRaw().begin(), sourceTex->textureDataRaw().end()};
        auto newTex = std::make_unique<Texture>(
            &data,
            sourceTex->textureData().width(),
            sourceTex->textureData().height(),
            sourceTex->textureFormat()
        );
        auto* ptr = newTex.get();
        sourceToReIded[sourceTex] = ptr;
        resultTextures.push_back(std::move(newTex));
        return ptr;
    };

    if (emitter->textureHandle().isValid()) {
        if (auto* newTex = getOrCreateReIded(emitter->texture())) {
            emitter->setTexture(newTex);
        }
    }
    if (emitter->childTextureHandle().isValid()) {
        if (auto* newTex = getOrCreateReIded(emitter->childTexture())) {
            emitter->setChildTexture(newTex);
        }
    }

    return ImportEmitterResult{std::make_unique<Emitter>(std::move(*emitter)), std::move(resultTextures)};
}

std::optional<ImportEmitterSetResult> importEmitterSet(const QString& filePath, const QString& projectDir) {    
    const auto readResult = readJsonFile(filePath);
    if (!readResult) {
        return std::nullopt;
    }
    const auto& setJson = *readResult;

    if (!validateMetaInfo(setJson["metaInfo"].toObject(), FileKind::EmitterSet, 1)) {
        return std::nullopt;
    }

    const bool isStandalone = setJson.contains("textures");

    auto emitterSet = std::make_unique<EmitterSet>();
    emitterSet->setName(setJson["name"].toString());
    emitterSet->setUserData(static_cast<u32>(setJson["userData"].toInteger()));
    emitterSet->setLastUpdateDate(static_cast<u32>(setJson["lastUpdateDate"].toInteger()));

    TextureList textures{};

    if (isStandalone) {
        // Standalone: textures embedded in the file, emitters inline JSON.
        const QJsonObject texturesJson = setJson["textures"].toObject();
        for (auto it = texturesJson.constBegin(); it != texturesJson.constEnd(); ++it) {
            bool ok{false};
            const s32 idx = it.key().toInt(&ok);
            if (!ok) {
                return std::nullopt;
            }

            auto tex = textureFromJson(it.value().toObject());
            if (tex) {
                if (idx >= static_cast<s32>(textures.size())) {
                    textures.resize(idx + 1);
                }
                textures[idx] = std::make_unique<Texture>(std::move(*tex));
            }
        }

        const QJsonObject emittersJson = setJson["emitters"].toObject();
        for (auto it = emittersJson.constBegin(); it != emittersJson.constEnd(); ++it) {
            bool ok{false};
            const s32 idx = it.key().toInt(&ok);
            if (!ok) {
                return std::nullopt;
            }

            auto emitter = emitterFromJson(it.value().toObject(), textures);
            if (!emitter) {
                return std::nullopt;
            }

            const QJsonObject emitterJson = it.value().toObject();
            const s32 texId = emitterJson["texture"].toInt();
            if (texId >= 0 && texId < static_cast<s32>(textures.size()) && textures[texId]) {
                emitter->setTexture(textures[texId].get());
            }
            const QJsonObject complexJson = emitterJson["complex"].toObject();
            const QJsonObject childJson = complexJson["child"].toObject();
            const s32 childTexId = childJson["texture"].toInt();
            if (childTexId >= 0 && childTexId < static_cast<s32>(textures.size()) && textures[childTexId]) {
                emitter->setChildTexture(textures[childTexId].get());
            }

            emitterSet->insertEmitter(idx, std::make_unique<Emitter>(std::move(*emitter)));
        }
    } else {
        QDir sourceProjectDir{};
        if (!projectDir.isEmpty()) {
            sourceProjectDir = QDir{projectDir};
        } else {
            const QFileInfo setFileInfo{filePath};
            sourceProjectDir = QDir{setFileInfo.absolutePath()};
            sourceProjectDir.cdUp(); // project root
        }

        const QDir texDir{sourceProjectDir.filePath("textures")};
        if (!texDir.exists()) {
            return std::nullopt;
        }

        const auto projFileExt = FileUtil::fileExtention(FileKind::Project);
        const auto projFiles = sourceProjectDir.entryList({projFileExt}, QDir::Files);
        if (projFiles.isEmpty()) {
            return std::nullopt;
        }

        const auto readResult = readJsonFile(sourceProjectDir.filePath(projFiles.first()));
        if (!readResult) {
            return std::nullopt;
        }
        const auto& projJson = *readResult;

        auto sourceTextures = importTextures(projJson["textures"].toObject(), sourceProjectDir);
        if (!sourceTextures) {
            return std::nullopt;
        }

        auto importedSet = importEmitterSet(filePath, *sourceTextures);
        if (!importedSet) {
            return std::nullopt;
        }

        emitterSet = std::make_unique<EmitterSet>(std::move(*importedSet));

        std::map<Texture*, Texture*> sourceToReIded{};

        const auto getOrCreateReIded = [&](Texture* sourceTex) -> Texture* {
            if (!sourceTex || sourceTex->isPlaceholder()) {
                return nullptr;
            }

            auto it = sourceToReIded.find(sourceTex);
            if (it != sourceToReIded.end()) {
                return it->second;
            }

            std::vector<u8> data{sourceTex->textureDataRaw().begin(), sourceTex->textureDataRaw().end()};
            auto newTex = std::make_unique<Texture>(
                &data,
                sourceTex->textureData().width(),
                sourceTex->textureData().height(),
                sourceTex->textureFormat()
            );
            auto* ptr = newTex.get();
            sourceToReIded[sourceTex] = ptr;
            textures.push_back(std::move(newTex));
            return ptr;
        };

        for (s32 i = 0; i < emitterSet->emitterCount(); ++i) {
            auto* emitter = emitterSet->emitters().at(i).get();
            if (emitter->textureHandle().isValid()) {
                if (auto* newTex = getOrCreateReIded(emitter->texture())) {
                    emitter->setTexture(newTex);
                }
            }
            if (emitter->childTextureHandle().isValid()) {
                if (auto* newTex = getOrCreateReIded(emitter->childTexture())) {
                    emitter->setChildTexture(newTex);
                }
            }
        }
    }

    return ImportEmitterSetResult{std::move(emitterSet), std::move(textures)};
}

bool importProject(const QString& projPath, PtclRes& res, [[maybe_unused]] PtclSanitizeReport& report) {
    const auto readResult = readJsonFile(projPath);
    if (!readResult) {
        return false;
    }
    const auto& projectJson = *readResult;

    if (!validateMetaInfo(projectJson["metaInfo"].toObject(), FileKind::Project, 1)) {
        return false;
    }

    res.setName(projectJson["name"].toString());

    const QDir projectDir{QFileInfo(projPath).absolutePath()};

    auto textures = importTextures(projectJson["textures"].toObject(), projectDir);
    if (!textures) {
        return false;
    }

    res.textures() = std::move(*textures);

    auto emitterSets = importEmitterSets(projectJson["emitterSets"].toObject(), projectDir, res.textures());
    if (!emitterSets) {
        return false;
    }

    res.getEmitterSets() = std::move(*emitterSets);

    // TODO: Validate stuff

    return true;
}


// ========================================================================== //


} // namespace Ptcl::Json
