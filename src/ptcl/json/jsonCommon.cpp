#include "ptcl/json/jsonCommon.h"

#include <QSaveFile>
#include <QIODevice>
#include <QFile>


namespace Ptcl::Json {


// ========================================================================== //


QJsonObject createMetaInfo(FileKind kind, s32 version) {
    QJsonObject metaInfo{};
    metaInfo["fileType"] = std::to_underlying(kind);
    metaInfo["version"]  = version;
    return metaInfo;
}

bool validateMetaInfo(const QJsonObject& metaInfo, FileKind kind, s32 version) {
    return (metaInfo["fileType"].toInt() == std::to_underlying(kind) && metaInfo["version"].toInt() == version);
}

std::optional<FileKind> classifyJson(const QJsonObject& json) {
    if (!json.contains("metaInfo")) {
        return std::nullopt;
    }

    const auto metaInfo = json.value("metaInfo").toObject();
    if (metaInfo.isEmpty() || !metaInfo.contains("fileType")) {
        return std::nullopt;
    }

    const auto value = metaInfo.value("fileType");
    if (!value.isDouble()) {
        return std::nullopt;
    }

    const auto type = value.toInt();
    if (type < 0) {
        return std::nullopt;
    }

    switch (static_cast<FileKind>(type)) {
    case FileKind::Project:
    case FileKind::Texture:
    case FileKind::EmitterSet:
    case FileKind::Emitter:
        return static_cast<FileKind>(type);
    case FileKind::Binary:
    case FileKind::Image:
    case FileKind::Unknown:
        break;
    }

    return std::nullopt;
}

QJsonValue floatToJson(f32 value) {
    if (value == 0.0f && std::signbit(value)) {
        return {
            QString{"-0"}
        };
    }
    return {
        static_cast<f64>(value)
    };
}

f32 jsonToFloat(const QJsonValue& json) {
    if (json.isString() && json.toString() == "-0") {
        return -0.0f;
    }
    return static_cast<f32>(json.toDouble());
}

bool isNumberValue(const QJsonValue& value) {
    return value.isDouble() || (value.isString() && value.toString() == "-0");
}

QJsonObject vec3fToJson(const Math::Vector3f& vector) {
    QJsonObject vectorJson{};
    vectorJson["x"] = floatToJson(vector.getX());
    vectorJson["y"] = floatToJson(vector.getY());
    vectorJson["z"] = floatToJson(vector.getZ());
    return vectorJson;
}

Math::Vector3f jsonToVec3f(const QJsonObject& json) {
    return Math::Vector3f{
        jsonToFloat(json["x"]),
        jsonToFloat(json["y"]),
        jsonToFloat(json["z"])
    };
}

QJsonObject vec3iToJson(const Math::Vector3i& vector) {
    QJsonObject vectorJson{};
    vectorJson["x"] = vector.getX();
    vectorJson["y"] = vector.getY();
    vectorJson["z"] = vector.getZ();
    return vectorJson;
}

Math::Vector3i jsonToVec3i(const QJsonObject& json) {
    return Math::Vector3i{
        json["x"].toInt(),
        json["y"].toInt(),
        json["z"].toInt()
    };
}

QJsonObject vec2fToJson(const Math::Vector2f& vector) {
    QJsonObject vectorJson{};
    vectorJson["x"] = floatToJson(vector.getX());
    vectorJson["y"] = floatToJson(vector.getY());
    return vectorJson;
}

Math::Vector2f jsonToVec2f(const QJsonObject& json) {
    return Math::Vector2f{
        jsonToFloat(json["x"]),
        jsonToFloat(json["y"])
    };
}

QJsonObject colorToJson(const Gfx::Color& color) {
    QJsonObject colorJson{};
    colorJson["r"] = floatToJson(color.r());
    colorJson["g"] = floatToJson(color.g());
    colorJson["b"] = floatToJson(color.b());
    colorJson["a"] = floatToJson(color.a());
    return colorJson;
}

Gfx::Color jsonToColor(const QJsonObject& json) {
    return Gfx::Color{
        jsonToFloat(json["r"]),
        jsonToFloat(json["g"]),
        jsonToFloat(json["b"]),
        jsonToFloat(json["a"])
    };
}

QJsonArray matrixToJson(const Math::Matrix34f& matrix) {
    QJsonArray matrixJson{};

    for (s32 row = 0; row < 3; ++row) {
        QJsonArray rowJson{};

        for (s32 col = 0; col < 4; ++col) {
            rowJson.append(floatToJson(matrix(row, col)));
        }

        matrixJson.append(rowJson);
    }

    return matrixJson;
}

Math::Matrix34f jsonToMatrix(const QJsonArray& json) {
    Math::Matrix34f matrix{};
    for (s32 row = 0; row < 3; ++row) {
        const QJsonArray rowJson = json[row].toArray();
        for (s32 col = 0; col < 4; ++col) {
            matrix(row, col) = jsonToFloat(rowJson[col]);
        }
    }
    return matrix;
}

QJsonObject scaleAnimToJson(const Emitter::ScaleAnim& anim) {
    QJsonObject animJson{};
    animJson["initScale"]     = vec2fToJson(anim.initScale);
    animJson["diffScale21"]   = vec2fToJson(anim.diffScale21);
    animJson["diffScale32"]   = vec2fToJson(anim.diffScale32);
    animJson["scaleSection1"] = anim.scaleSection1;
    animJson["scaleSection2"] = anim.scaleSection2;
    animJson["isFlatStart"]   = anim.isFlatStart;
    return animJson;
}

Emitter::ScaleAnim scaleAnimFromJson(const QJsonObject& json) {
    return Emitter::ScaleAnim{
        .initScale = jsonToVec2f(json["initScale"].toObject()),
        .diffScale21 = jsonToVec2f(json["diffScale21"].toObject()),
        .diffScale32 = jsonToVec2f(json["diffScale32"].toObject()),
        .scaleSection1 = json["scaleSection1"].toInt(),
        .scaleSection2 = json["scaleSection2"].toInt(),
        .isFlatStart = json["isFlatStart"].toBool()
    };
}

QJsonObject alphaAnimToJson(const Emitter::AlphaAnim& anim) {
    QJsonObject animJson{};
    animJson["initAlpha"]     = floatToJson(anim.initAlpha);
    animJson["diffAlpha21"]   = floatToJson(anim.diffAlpha21);
    animJson["diffAlpha32"]   = floatToJson(anim.diffAlpha32);
    animJson["alphaSection1"] = anim.alphaSection1;
    animJson["alphaSection2"] = anim.alphaSection2;
    animJson["isFlatStart"]   = anim.isFlatStart;
    return animJson;
}

Emitter::AlphaAnim alphaAnimFromJson(const QJsonObject& json) {
    return Emitter::AlphaAnim{
        .initAlpha = jsonToFloat(json["initAlpha"]),
        .diffAlpha21 = jsonToFloat(json["diffAlpha21"]),
        .diffAlpha32 = jsonToFloat(json["diffAlpha32"]),
        .alphaSection1 = json["alphaSection1"].toInt(),
        .alphaSection2 = json["alphaSection2"].toInt(),
        .isFlatStart = json["isFlatStart"].toBool()
    };
}

bool writeJsonFile(const QJsonObject& root, const QString& filePath) {
    QSaveFile file{filePath};
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    const auto data = QJsonDocument(root).toJson();

    if (file.write(data) != data.size()) {
        return false;
    }

    return file.commit();
}

std::optional<QJsonObject> readJsonFile(const QString& filePath) {
    QFile file{filePath};
    if (!file.open(QIODevice::ReadOnly)) {
        return std::nullopt;
    }

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);

    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        return std::nullopt;
    }

    return document.object();
}


// ========================================================================== //


} // namespace Ptcl::Json
