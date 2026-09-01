#include "ptcl/json/jsonEmitterSet.h"

#include "ptcl/json/jsonEmitter.h"
#include "ptcl/json/jsonTexture.h"
#include "util/fileUtil.h"


namespace Ptcl::Json {


// ========================================================================== //


QJsonObject emitterSetToJson(const EmitterSet& emitterSet) {
    QJsonObject json{};
    json.insert("metaInfo",       createMetaInfo(FileKind::EmitterSet, 1));
    json.insert("name",           emitterSet.name());
    json.insert("userData",       static_cast<s64>(emitterSet.userData()));
    json.insert("lastUpdateDate", static_cast<s64>(emitterSet.lastUpdateDate()));
    return json;
}

std::optional<EmitterSet> emitterSetFromJson(const QJsonObject& json, const QString& filePath, const TextureList& textures) {
    if (!validateMetaInfo(json["metaInfo"].toObject(), FileKind::EmitterSet, 1)) {
        return std::nullopt;
    }

    const auto name           = json["name"];
    const auto userData       = json["userData"];
    const auto lastUpdateDate = json["lastUpdateDate"];

    if (!name.isString())           { return std::nullopt; }
    if (!userData.isDouble())       { return std::nullopt; }
    if (!lastUpdateDate.isDouble()) { return std::nullopt; }

    EmitterSet emitterSet{};

    emitterSet.setName(name.toString());
    emitterSet.setUserData(static_cast<u32>(userData.toInteger()));
    emitterSet.setLastUpdateDate(static_cast<u32>(lastUpdateDate.toInteger()));

    const QDir emitterSetDir{QFileInfo(filePath).absolutePath()};

    auto emitters = importEmitters(json["emitters"].toObject(), emitterSetDir, textures);
    if (!emitters) {
        return std::nullopt;
    }

    emitterSet.emitters() = std::move(*emitters);

    return emitterSet;
}

bool exportEmitterSet(const EmitterSet& emitterSet, const QString& filePath) {
    std::vector<const Texture*> uniqueTextures{};
    TextureIndexMap textureToIndex{};

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

    return writeJsonFile(rootJson, filePath);
}

std::optional<QString> exportEmitterSet(const EmitterSet& emitterSet, s32 idx, const QDir& dir, const TextureIndexMap& textureMap) {
    const auto emitterSetName = QStringLiteral("set_%1_%2").arg(idx).arg(emitterSet.name());

    auto json = emitterSetToJson(emitterSet);

    dir.mkdir(emitterSetName);
    QDir emitterSetDir{dir.filePath(emitterSetName)};
    json.insert("emitters", exportEmitters(emitterSet.emitters(), emitterSetDir, textureMap));

    auto emitterSetFileName = FileUtil::ensureExtention(emitterSetName, FileKind::EmitterSet);

    if (!writeJsonFile(json, dir.filePath(emitterSetFileName))) {
        return std::nullopt;
    }

    return emitterSetFileName;
}

static std::optional<EmitterSet> importEmitterSetFile(const QString& filePath, const TextureList& textures) {
    const auto readResult = readJsonFile(filePath);
    if (!readResult) {
        return std::nullopt;
    }
    return emitterSetFromJson(*readResult, filePath, textures);
}

static std::optional<ImportEmitterSetResult> importStandaloneEmitterSet(const QJsonObject& setJson) {
    TextureList textures{};

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

    auto emitterSet = std::make_unique<EmitterSet>();
    emitterSet->setName(setJson["name"].toString());
    emitterSet->setUserData(static_cast<u32>(setJson["userData"].toInteger()));
    emitterSet->setLastUpdateDate(static_cast<u32>(setJson["lastUpdateDate"].toInteger()));

    const QJsonObject emittersJson = setJson["emitters"].toObject();
    for (auto it = emittersJson.constBegin(); it != emittersJson.constEnd(); ++it) {
        bool ok{false};
        const s32 idx = it.key().toInt(&ok);
        if (!ok) {
            return std::nullopt;
        }

        const QJsonObject emitterJson = it.value().toObject();

        auto emitter = emitterFromJson(emitterJson, textures);
        if (!emitter) {
            return std::nullopt;
        }

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

    return ImportEmitterSetResult{std::move(emitterSet), std::move(textures)};
}

static std::optional<ImportEmitterSetResult> importLinkedEmitterSet(const QJsonObject& setJson, const QString& filePath, const QString& projectDir) {
    const QDir sourceProjectDir = sourceProjectDirFor(filePath, projectDir, 1);

    auto sourceTextures = importProjectTextures(sourceProjectDir);
    if (!sourceTextures) {
        return std::nullopt;
    }

    auto emitterSet = emitterSetFromJson(setJson, filePath, *sourceTextures);
    if (!emitterSet) {
        return std::nullopt;
    }

    TextureList resultTextures{};
    TextureRemap remap{};
    for (s32 i = 0; i < emitterSet->emitterCount(); ++i) {
        reIdEmitterTextures(*emitterSet->emitters().at(i), resultTextures, remap);
    }

    return ImportEmitterSetResult{std::make_unique<EmitterSet>(std::move(*emitterSet)), std::move(resultTextures)};
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

    if (setJson.contains("textures")) {
        return importStandaloneEmitterSet(setJson);
    }

    return importLinkedEmitterSet(setJson, filePath, projectDir);
}

QJsonObject exportEmitterSets(const EmitterSetList& emitterSets, const QDir& dir, const TextureIndexMap& textureMap) {
    QJsonObject emitterSetListJson{};
    for (s32 idx = 0; idx < static_cast<s32>(emitterSets.size()); ++idx) {
        const auto emitterSetName = exportEmitterSet(*emitterSets.at(idx), idx, dir, textureMap);

        if (emitterSetName) {
            emitterSetListJson[QString::number(idx)] = dir.dirName() + "/" + *emitterSetName;
        }
    }
    return emitterSetListJson;
}

std::optional<EmitterSetList> importEmitterSets(const QJsonObject& emitterSetsJson, const QDir& projectDir, TextureList& textures) {
    EmitterSetList emitterSets{};
    emitterSets.resize(emitterSetsJson.size());

    for (auto it = emitterSetsJson.constBegin(); it != emitterSetsJson.constEnd(); ++it) {
        bool ok{false};

        const size_t idx = it.key().toInt(&ok);

        if (!ok || idx >= emitterSets.size()) {
            return std::nullopt;
        }

        const QString emitterSetPath = projectDir.filePath(it.value().toString());
        auto emitterSet = importEmitterSetFile(emitterSetPath, textures);

        if (!emitterSet) {
            return std::nullopt;
        }

        emitterSets[idx] = (std::make_unique<EmitterSet>(std::move(*emitterSet)));
    }

    for (const auto& emitterSet : emitterSets) {
        if (!emitterSet) {
            return std::nullopt;
        }
    }

    return emitterSets;
}


// ========================================================================== //


} // namespace Ptcl::Json
