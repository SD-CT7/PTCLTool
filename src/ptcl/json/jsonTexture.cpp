#include "ptcl/json/jsonTexture.h"

#include "ptcl/json/jsonCommon.h"
#include "util/fileUtil.h"


namespace Ptcl::Json {


// ========================================================================== //


QJsonObject textureToJson(const Texture& texture) {
    const auto& data = texture.textureData();
    const auto& raw = texture.textureDataRaw();

    QJsonObject json{};
    json.insert("format", static_cast<s32>(texture.textureFormat()));
    json.insert("width",  data.width());
    json.insert("height", data.height());
    json.insert("data",   QString::fromLatin1(QByteArray(raw).toBase64()));
    return json;
}

std::optional<Texture> textureFromJson(const QJsonObject& json) {
    const auto formatValue = json["format"];
    const auto widthValue  = json["width"];
    const auto heightValue = json["height"];
    const auto dataValue   = json["data"];

    if (!formatValue.isDouble() || !widthValue.isDouble() ||
        !heightValue.isDouble() || !dataValue.isString()) {
        return std::nullopt;
    }

    const auto format = static_cast<TextureFormat>(formatValue.toInt());
    const s32 width = widthValue.toInt();
    const s32 height = heightValue.toInt();

    if (width <= 0 || height <= 0) {
        return std::nullopt;
    }

    const auto data = QByteArray::fromBase64(dataValue.toString().toLatin1());

    if (data.isEmpty()) {
        return std::nullopt;
    }

    std::vector<u8> dataVec(data.begin(), data.end());

    return Texture{
        &dataVec,
        width,
        height,
        format
    };
}

std::optional<QString> exportTexture(const Texture& texture, s32 idx, const QDir& dir) {
    QJsonObject json = textureToJson(texture);
    json.insert("metaInfo", createMetaInfo(FileKind::Texture, 1));

    auto textureName = QStringLiteral("tex_%1").arg(idx);
    textureName = FileUtil::ensureExtention(textureName, FileKind::Texture);

    if (!writeJsonFile(json, dir.filePath(textureName))) {
        return std::nullopt;
    }

    return textureName;
}

std::optional<Texture> importTexture(const QString& filePath) {
    const auto readResult = readJsonFile(filePath);
    if (!readResult) {
        return std::nullopt;
    }
    const auto& textureJson = *readResult;

    if (!validateMetaInfo(textureJson["metaInfo"].toObject(), FileKind::Texture, 1)) {
        return std::nullopt;
    }

    return textureFromJson(textureJson);
}

QJsonObject exportTextures(const TextureList& textures, const QDir& dir) {
    QJsonObject texturesListJson{};
    for (s32 idx = 0; idx < static_cast<s32>(textures.size()); ++idx) {
        const auto textureName = exportTexture(*textures.at(idx), idx, dir);

        if (textureName) {
            texturesListJson[QString::number(idx)] = dir.dirName() + "/" + *textureName;
        }
    }
    return texturesListJson;
}

std::optional<TextureList> importTextures(const QJsonObject& texturesJson, const QDir& projectDir) {
    TextureList textures{};
    textures.resize(texturesJson.size());

    for (auto it = texturesJson.constBegin(); it != texturesJson.constEnd(); ++it) {
        bool ok{false};

        const size_t idx = it.key().toInt(&ok);

        if (!ok || idx >= textures.size()) {
            return std::nullopt;
        }

        const QString texturePath = projectDir.filePath(it.value().toString());
        auto texture = importTexture(texturePath);

        if (!texture) {
            return std::nullopt;
        }

        textures[idx] = (std::make_unique<Texture>(std::move(*texture)));
    }
    return textures;
}


// ========================================================================== //


} // namespace Ptcl::Json
