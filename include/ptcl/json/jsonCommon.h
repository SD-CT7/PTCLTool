#pragma once

#include "typedefs.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "gfx/color.h"

#include "ptcl/ptclEmitter.h"

#include <QJsonObject>
#include <QJsonArray>


namespace Ptcl::Json {


// ========================================================================== //


enum class JsonFileType {
    ProjectFile    = 0,
    TextureFile    = 1,
    EmitterSetFile = 2,
    EmitterFile    = 3,
};

QJsonObject createMetaInfo(JsonFileType type, s32 version);
bool validateMetaInfo(const QJsonObject& metaInfo, JsonFileType type, s32 version);

QJsonValue floatToJson(f32 value);
f32 jsonToFloat(const QJsonValue& json);

QJsonObject vec3fToJson(const Math::Vector3f& vector);
Math::Vector3f jsonToVec3f(const QJsonObject& json);

QJsonObject vec3iToJson(const Math::Vector3i& vector);
Math::Vector3i jsonToVec3i(const QJsonObject& json);

QJsonObject vec2fToJson(const Math::Vector2f& vector);
Math::Vector2f jsonToVec2f(const QJsonObject& json);

QJsonObject colorToJson(const Gfx::Color& color);
Gfx::Color jsonToColor(const QJsonObject& json);

QJsonArray matrixToJson(const Math::Matrix34f& matrix);
Math::Matrix34f jsonToMatrix(const QJsonArray& json);

QJsonObject scaleAnimToJson(const Emitter::ScaleAnim& anim);
Emitter::ScaleAnim scaleAnimFromJson(const QJsonObject& json);

QJsonObject alphaAnimToJson(const Emitter::AlphaAnim& anim);
Emitter::AlphaAnim alphaAnimFromJson(const QJsonObject& json);

bool writeJsonFile(const QJsonObject& root, const QString& filePath);
std::optional<QJsonObject> readJsonFile(const QString& filePath);


// ========================================================================== //


} // namespace Ptcl::Json
