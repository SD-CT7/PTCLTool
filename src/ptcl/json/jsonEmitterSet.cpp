#include "ptcl/json/jsonEmitterSet.h"

#include "ptcl/json/jsonEmitter.h"
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

std::optional<EmitterSet> importEmitterSet(const QString& filePath, const TextureList& textures) {
    const auto readResult = readJsonFile(filePath);
    if (!readResult) {
        return std::nullopt;
    }
    return emitterSetFromJson(readResult.value(), filePath, textures);
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
        auto emitterSet = importEmitterSet(emitterSetPath, textures);

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
