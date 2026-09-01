#pragma once

#include "typedefs.h"
#include "ptcl/ptcl.h"
#include "ptcl/json/jsonCommon.h"

#include <QJsonObject>
#include <QDir>


namespace Ptcl::Json {


// ========================================================================== //


QJsonObject emitterToJson(const Emitter& emitter, bool embedTextures, const TextureIndexMap* textureMap);
std::optional<Emitter> emitterFromJson(const QJsonObject& emitterJson, const TextureList& textures);

std::optional<QString> exportEmitter(const Emitter& emitter, s32 idx, const QDir& dir, const TextureIndexMap& textureMap);
std::optional<Emitter> importEmitter(const QString& filePath, const TextureList& textures);

QJsonObject exportEmitters(const EmitterList& emitters, const QDir& dir, const TextureIndexMap& textureMap);
std::optional<EmitterList> importEmitters(const QJsonObject& emittersJson, const QDir& projectDir);


// ========================================================================== //


} // namespace Ptcl::Json
