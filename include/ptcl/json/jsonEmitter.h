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

bool exportEmitter(const Emitter& emitter, const QString& filePath);

std::optional<QString> exportEmitter(const Emitter& emitter, s32 idx, const QDir& dir, const TextureIndexMap& textureMap);

struct ImportEmitterResult {
    std::unique_ptr<Emitter> emitter;
    TextureList textures;
};

std::optional<ImportEmitterResult> importEmitter(const QString& filePath, const QString& projectDir = {});

void reIdEmitterTextures(Emitter& emitter, TextureList& resultTextures, TextureRemap& remap);

QJsonObject exportEmitters(const EmitterList& emitters, const QDir& dir, const TextureIndexMap& textureMap);
std::optional<EmitterList> importEmitters(const QJsonObject& emittersJson, const QDir& dir, const TextureList& textures);


// ========================================================================== //


} // namespace Ptcl::Json
