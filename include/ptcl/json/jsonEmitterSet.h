#pragma once

#include "typedefs.h"
#include "ptcl/ptcl.h"
#include "ptcl/json/jsonCommon.h"

#include <QJsonObject>
#include <QDir>


namespace Ptcl::Json {


// ========================================================================== //


QJsonObject emitterSetToJson(const EmitterSet& emitterSet);
std::optional<EmitterSet> emitterSetFromJson(const QJsonObject& emitterSetJson, const QString& filePath, TextureList& textures);

std::optional<QString> exportEmitterSet(const EmitterSet& emitterSet, s32 idx, const QDir& dir, const TextureIndexMap& textureMap);
std::optional<EmitterSet> importEmitterSet(const QString& filePath, const TextureList& textures);

QJsonObject exportEmitterSets(const EmitterSetList& emitterSets, const QDir& dir, const TextureIndexMap& textureMap);
std::optional<EmitterSetList> importEmitterSets(const QJsonObject& emitterSetsJson, const QDir& projectDir, TextureList& textures);


// ========================================================================== //


} // namespace Ptcl::Json
