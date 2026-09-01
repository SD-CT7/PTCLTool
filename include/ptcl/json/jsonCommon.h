#pragma once

#include "typedefs.h"
#include "util/fileKind.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "gfx/color.h"

#include "ptcl/ptclEmitter.h"

#include <QDir>
#include <QJsonObject>
#include <QJsonArray>

#include <map>
#include <unordered_map>


namespace Ptcl::Json {


// ========================================================================== //


using TextureIndexMap = std::unordered_map<const Texture*, s32>;
using TextureRemap = std::map<Texture*, Texture*>;

QJsonObject createMetaInfo(FileKind kind, s32 version);
bool validateMetaInfo(const QJsonObject& metaInfo, FileKind kind, s32 version);

std::optional<FileKind> classifyJson(const QJsonObject& json);

QJsonValue floatToJson(f32 value);
f32 jsonToFloat(const QJsonValue& json);

bool isNumberValue(const QJsonValue& value);

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

QDir sourceProjectDirFor(const QString& filePath, const QString& hintDir, s32 levelsUp);


// ========================================================================== //


} // namespace Ptcl::Json
