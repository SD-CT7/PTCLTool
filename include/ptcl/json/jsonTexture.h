#pragma once

#include "typedefs.h"
#include "ptcl/ptcl.h"

#include <QJsonObject>
#include <QDir>


namespace Ptcl::Json {


// ========================================================================== //


QJsonObject textureToJson(const Texture& texture);
std::optional<Texture> textureFromJson(const QJsonObject& json);

std::optional<QString> exportTexture(const Texture& texture, s32 idx, const QDir& dir);
std::optional<Texture> importTexture(const QString& filePath);

QJsonObject exportTextures(const TextureList& textures, const QDir& dir);
std::optional<TextureList> importTextures(const QJsonObject& texturesJson, const QDir& projectDir);


// ========================================================================== //


} // namespace Ptcl::Json
