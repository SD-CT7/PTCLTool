#include "ptcl/json/json.h"

#include "ptcl/json/jsonCommon.h"
#include "ptcl/json/jsonTexture.h"


#include <QDataStream>

#include <cstring>
#include <map>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

namespace Ptcl::Json {


// ========================================================================== //


using TextureMap = std::unordered_map<const Texture*, s32>;

namespace Internal {

QJsonObject buildEmitterJson(const Emitter& emitter, bool embedTextures, const TextureMap* textureMap) {
    QJsonObject emitterJson{};
    emitterJson["metaInfo"] = createMetaInfo(JsonFileType::EmitterFile, 1);

    emitterJson["type"] = static_cast<s64>(emitter.type());
    emitterJson["flag"] = static_cast<s64>(emitter.flags().value());
    emitterJson["followType"] = static_cast<s64>(emitter.followType());
    emitterJson["name"] = emitter.name();
    emitterJson["randomSeed"] = static_cast<s64>(emitter.randomSeed().raw());
    emitterJson["billboardType"] = static_cast<s64>(emitter.billboardType());
    emitterJson["isPolygon"] = emitter.isPolygon();
    emitterJson["isVelLook"] = emitter.isVelLook();
    emitterJson["isEmitterBillboardMtx"] = emitter.isEmitterBillboardMtx();
    emitterJson["isFollow"] = emitter.isFollow();

    emitterJson["isDirectional"] = emitter.isDirectional();
    emitterJson["gravity"] = vec3fToJson(emitter.gravity());

    emitterJson["ptclLife"] = emitter.ptclLife();
    emitterJson["ptclLifeRandom"] = emitter.ptclLifeRandom();

    emitterJson["isStopEmitInFade"] = emitter.isStopEmitInFade();
    emitterJson["alphaAddInFade"] = floatToJson(emitter.alphaAddInFade());

    emitterJson["transformRT"] = matrixToJson(emitter.transformRT());
    emitterJson["transformSRT"] = matrixToJson(emitter.transformSRT());

    emitterJson["scaleAnim"] = scaleAnimToJson(emitter.scaleAnim());
    emitterJson["scaleRand"] = floatToJson(emitter.scaleRand());

    emitterJson["emitStartFrame"] = emitter.emitStartFrame();
    emitterJson["emitEndFrame"] = emitter.emitEndFrame();
    emitterJson["lifeStep"] = emitter.lifeStep();
    emitterJson["lifeStepRandom"] = emitter.lifeStepRandom();
    emitterJson["emitRate"] = emitter.emitRate();

    emitterJson["figureVelocity"] = floatToJson(emitter.figureVelocity());
    emitterJson["velocityDir"] = vec3fToJson(emitter.velocityDirection());
    emitterJson["initVelocity"] = floatToJson(emitter.initialVelocity());
    emitterJson["initVelocityRandom"] = floatToJson(emitter.initialVelocityRandom());
    emitterJson["spreadVec"] = vec3fToJson(emitter.spreadVector());
    emitterJson["airResist"] = floatToJson(emitter.airResistance());

    emitterJson["volumeTblIndex"] = emitter.volumeTblIndex();
    emitterJson["volumeType"] = static_cast<s64>(emitter.volumeType());
    emitterJson["volumeRadius"] = vec3fToJson(emitter.volumeRadius());
    emitterJson["volumeArcWidth"] = emitter.volumeArcWidth();
    emitterJson["volumeArcStart"] = emitter.volumeArcStart();

    emitterJson["rotType"] = static_cast<s64>(emitter.rotationType());
    emitterJson["initRot"] = vec3iToJson(emitter.initialRotation());
    emitterJson["initRotRandom"] = vec3iToJson(emitter.initialRotationRandom());
    emitterJson["rotVel"] = vec3iToJson(emitter.rotationVelocity());
    emitterJson["rotVelRandom"] = vec3iToJson(emitter.rotationVelocityRandom());
    emitterJson["rotBasis"] = vec2fToJson(emitter.rotationBasis());

    emitterJson["alphaAnim"] = alphaAnimToJson(emitter.alphaAnim());

    emitterJson["blendFunc"] = static_cast<s64>(emitter.blendFunction());
    emitterJson["depthFunc"] = static_cast<s64>(emitter.depthFunction());
    emitterJson["combinerFunc"] = static_cast<s64>(emitter.combinerFunction());

    {
        QJsonArray color0Json{};
        for (auto& color : emitter.color0()) {
            color0Json.append(colorToJson(color));
        }
        emitterJson["color0"] = color0Json;
    }

    emitterJson["colorSection1"] = emitter.colorSection1();
    emitterJson["colorSection2"] = emitter.colorSection2();
    emitterJson["colorSection3"] = emitter.colorSection3();
    emitterJson["colorNumRepeat"] = emitter.colorNumRepeat();
    emitterJson["colorCalcType"] = static_cast<s64>(emitter.colorCalcType());

    emitterJson["color1"] = colorToJson(emitter.primaryColor());

    emitterJson["textureWrapT"] = static_cast<s64>(emitter.textureWrapT());
    emitterJson["textureWrapS"] = static_cast<s64>(emitter.textureWrapS());
    emitterJson["textureLodLevel"] = emitter.textureLodLevel();
    emitterJson["textureFilter"] = static_cast<s64>(emitter.textureFilter());
    emitterJson["numTexturePattern"] = emitter.numTexturePattern();
    emitterJson["numTextureDivisionX"] = emitter.numTextureDivisionX();
    emitterJson["numTextureDivisionY"] = emitter.numTextureDivisionY();
    emitterJson["textureUVScale"] = vec2fToJson(emitter.textureUVScale());
    {
        QJsonArray patternTableJson{};
        for (auto& val : emitter.texturePatternTable()) {
            patternTableJson.append(val);
        }
        emitterJson["texturePatternTable"] = patternTableJson;
    }
    emitterJson["texturePatFreq"] = emitter.texturePatternFrequency();
    emitterJson["texturePatFrameCount"] = emitter.texturePatternFrameCount();
    emitterJson["isTexPatAnim"] = emitter.isTexturePatternAnim();

    if (embedTextures && emitter.textureHandle().isValid()) {
        emitterJson["texture"] = textureToJson(*emitter.texture());
    } else if (emitter.textureHandle().isValid()) {
        emitterJson["texture"] = static_cast<s64>(textureMap->at(emitter.textureHandle().get()));
    } else {
        emitterJson["texture"] = -1;
    }

    {
        QJsonObject complexJson{};
        complexJson["fluctuationScale"] = floatToJson(emitter.fluctuationScale());
        complexJson["fluctuationFreq"] = emitter.fluctuationFrequency();
        complexJson["fluctuationPhaseRand"] = emitter.isFluctuationPhaseRandom();
        complexJson["fluctuationFlags"] = emitter.fluctuationFlags().value();

        complexJson["stripeType"] = static_cast<s64>(emitter.stripeType());
        complexJson["stripeNumHistory"] = emitter.stripeNumHistory();
        complexJson["stripeStartAlpha"] = floatToJson(emitter.stripeStartAlpha());
        complexJson["stripeEndAlpha"] = floatToJson(emitter.stripeEndAlpha());
        complexJson["stripeUVScrollSpeed"] = vec2fToJson(emitter.stripeUVScrollSpeed());
        complexJson["stripeHistoryStep"] = emitter.stripeHistoryStep();
        complexJson["stripeDirInterpolate"] = floatToJson(emitter.stripeDirInterpolate());
        complexJson["stripeFlags"] = emitter.stripeFlags().value();

        complexJson["fieldRandomBlank"] = emitter.fieldRandomBlank();
        complexJson["fieldRandomVelAdd"] = vec3fToJson(emitter.fieldRandomVelAdd());

        complexJson["fieldMagnetPower"] = floatToJson(emitter.fieldMagnetPower());
        complexJson["fieldMagnetPos"] = vec3fToJson(emitter.fieldMagnetPos());
        complexJson["fieldMagnetFlag"] = static_cast<s64>(emitter.fieldMagnetFlag().value());

        complexJson["fieldSpinRotate"] = emitter.fieldSpinRotate();
        complexJson["fieldSpinAxis"] = static_cast<s64>(emitter.fieldSpinAxis());

        complexJson["fieldCollisionType"] = static_cast<s64>(emitter.fieldCollisionType());
        complexJson["fieldCollisionIsWorld"] = emitter.fieldCollisionIsWorld();
        complexJson["fieldCollisionCoord"] = floatToJson(emitter.fieldCollisionCoord());
        complexJson["fieldCollisionCoef"] = floatToJson(emitter.fieldCollisionCoef());

        complexJson["fieldConvergenceType"] = static_cast<s64>(emitter.fieldConvergenceType());
        complexJson["fieldConvergencePos"] = vec3fToJson(emitter.fieldConvergencePos());

        complexJson["fieldPosAddPosition"] = vec3fToJson(emitter.fieldPosAddPosition());

        complexJson["fieldFlags"] = emitter.fieldFlags().value();

        {
            QJsonObject childJson{};
            childJson["billboardType"] = static_cast<s64>(emitter.childBillboardType());

            childJson["emitRate"] = emitter.childEmitRate();
            childJson["emitTiming"] = emitter.childEmitTiming();
            childJson["life"] = emitter.childLife();
            childJson["emitStep"] = emitter.childEmitStep();

            childJson["randVelocity"] = vec3fToJson(emitter.childRandVelocity());
            childJson["gravity"] = vec3fToJson(emitter.childGravity());
            childJson["velocityInheritRate"] = floatToJson(emitter.childVelocityInheritRate());
            childJson["initialPositionRand"] = floatToJson(emitter.childInitalPositionRand());
            childJson["figureVelocity"] = floatToJson(emitter.childFigureVelocity());
            childJson["airResist"] = floatToJson(emitter.childAirResistance());

            childJson["rotationType"] = static_cast<s64>(emitter.childRotationType());
            childJson["initialRotation"] = vec3iToJson(emitter.childInitialRotation());
            childJson["initialRotationRandom"] = vec3iToJson(emitter.childInitialRotationRandom());
            childJson["rotationVelocity"] = vec3iToJson(emitter.childRotationVelocity());
            childJson["rotationVelocityRandom"] = vec3iToJson(emitter.childRotationVelocityRandom());
            childJson["rotationBasis"] = vec2fToJson(emitter.childRotationBasis());

            childJson["scale"] = vec2fToJson(emitter.childScale());
            childJson["scaleTarget"] = vec2fToJson(emitter.childScaleTarget());
            childJson["scaleInheritRate"] = floatToJson(emitter.childScaleInheritRate());
            childJson["scaleStartFrame"] = emitter.childScaleStartFrame();

            childJson["textureWrapT"] = static_cast<s64>(emitter.childTextureWrapT());
            childJson["textureWrapS"] = static_cast<s64>(emitter.childTextureWrapS());
            childJson["textureLodLevel"] = emitter.childTextureLodLevel();
            childJson["textureFilter"] = static_cast<s64>(emitter.childTextureFilter());
            childJson["textureUVScale"] = vec2fToJson(emitter.childTextureUVScale());

            if (embedTextures && emitter.childTextureHandle().isValid()) {
                childJson["texture"] = textureToJson(*emitter.childTexture());
            } else if (emitter.childTextureHandle().isValid()) {
                childJson["texture"] = static_cast<s64>(textureMap->at(emitter.childTextureHandle().get()));
            } else {
                childJson["texture"] = -1;
            }

            childJson["color0"] = colorToJson(emitter.childSecondaryColor());
            childJson["color1"] = colorToJson(emitter.childPrimaryColor());

            childJson["alpha"] = floatToJson(emitter.childAlpha());
            childJson["alphaTarget"] = floatToJson(emitter.childAlphaTarget());
            childJson["alphaInit"] = floatToJson(emitter.childAlphaInit());
            childJson["alphaStartFrame"] = emitter.childAlphaStartFrame();
            childJson["alphaBaseFrame"] = emitter.childAlphaBaseFrame();

            childJson["blendFunc"] = static_cast<s64>(emitter.childBlendFunc());
            childJson["depthFunc"] = static_cast<s64>(emitter.childDepthFunc());
            childJson["combinerFunc"] = static_cast<s64>(emitter.childCombinerFunc());

            complexJson["child"] = childJson;
        }

        complexJson["childFlags"] = emitter.childFlags().value();

        emitterJson["complex"] = complexJson;
    }

    return emitterJson;
}

std::optional<QString> exportEmitter(const Emitter& emitter, s32 idx, const QDir& dir, const TextureMap& textureMap) {
    auto emitterJson = buildEmitterJson(emitter, false, &textureMap);

    auto emitterName = QString("emitter_%1_%2.pemt").arg(idx).arg(emitter.name());

    if (!writeJsonFile(emitterJson, dir.filePath(emitterName))) {
        return std::nullopt;
    }

    return emitterName;
}

QJsonObject exportEmitters(const EmitterList& emitters, const QDir& dir, const TextureMap& textureMap) {
    QJsonObject emitterSetListJson{};
    for (s32 idx = 0; idx < static_cast<s32>(emitters.size()); ++idx) {
        const auto emitterName = exportEmitter(*emitters.at(idx), idx, dir, textureMap);

        if (emitterName) {
            emitterSetListJson[QString::number(idx)] = dir.dirName() + "/" + *emitterName;
        }
    }
    return emitterSetListJson;
}

QJsonObject buildEmitterSetJson(const EmitterSet& emitterSet) {
    QJsonObject emitterSetJson{};
    emitterSetJson["metaInfo"] = createMetaInfo(JsonFileType::EmitterSetFile, 1);
    emitterSetJson["name"]           = emitterSet.name();
    emitterSetJson["userData"]       = static_cast<s64>(emitterSet.userData());
    emitterSetJson["lastUpdateDate"] = static_cast<s64>(emitterSet.lastUpdateDate());
    return emitterSetJson;
}

std::optional<QString> exportEmitterSet(const EmitterSet& emitterSet, s32 idx, const QDir& dir, const TextureMap& textureMap) {
    const auto emitterSetName = QString("set_%1_%2").arg(idx).arg(emitterSet.name());

    auto emitterSetJson = buildEmitterSetJson(emitterSet);

    dir.mkdir(emitterSetName);
    QDir emitterSetDir{dir.filePath(emitterSetName)};
    emitterSetJson["emitters"] = exportEmitters(emitterSet.emitters(), emitterSetDir, textureMap);

    auto emitterSetFileName = QString("%1.pset").arg(emitterSetName);

    if (!writeJsonFile(emitterSetJson, dir.filePath(emitterSetFileName))) {
        return std::nullopt;
    }

    return emitterSetFileName;
}

QJsonObject exportEmitterSets(const EmitterSetList& emitterSets, const QDir& dir, const TextureMap& textureMap) {
    QJsonObject emitterSetListJson{};
    for (s32 idx = 0; idx < static_cast<s32>(emitterSets.size()); ++idx) {
        const auto emitterSetName = exportEmitterSet(*emitterSets.at(idx), idx, dir, textureMap);

        if (emitterSetName) {
            emitterSetListJson[QString::number(idx)] = dir.dirName() + "/" + *emitterSetName;
        }
    }
    return emitterSetListJson;
}

// ========================================================================== //


std::unique_ptr<Emitter> importEmitterFromJson(const QJsonObject& emitterJson, const TextureList& textures) {
    if (validateMetaInfo(emitterJson["metaInfo"].toObject(), JsonFileType::EmitterFile, 1)) {
        return nullptr;
    }

    auto emitter = std::make_unique<Emitter>();

    // Basic Properties
    emitter->setType(static_cast<EmitterType>(emitterJson["type"].toInteger()));
    emitter->flags() = BitFlag<EmitterFlag>(static_cast<u32>(emitterJson["flag"].toInteger()));
    emitter->setFollowType(static_cast<FollowType>(emitterJson["followType"].toInteger()));
    emitter->setName(emitterJson["name"].toString());
    emitter->setRandomSeed(PtclSeed{static_cast<u32>(emitterJson["randomSeed"].toInteger())});
    emitter->setBillboardType(static_cast<BillboardType>(emitterJson["billboardType"].toInteger()));

    emitter->setDirectional(emitterJson["isDirectional"].toBool());
    emitter->setGravity(jsonToVec3f(emitterJson["gravity"].toObject()));

    emitter->setPtclLife(emitterJson["ptclLife"].toInt());
    emitter->setPtclLifeRandom(emitterJson["ptclLifeRandom"].toInt());

    emitter->setIsStopEmitInFade(emitterJson["isStopEmitInFade"].toBool());
    emitter->setAlphaAddInFade(jsonToFloat(emitterJson["alphaAddInFade"]));

    {
        const auto rt = jsonToMatrix(emitterJson["transformRT"].toArray());
        const auto srt = jsonToMatrix(emitterJson["transformSRT"].toArray());
        emitter->setTransformFromMatrices(rt, srt);
    }

    emitter->setScaleAnim(scaleAnimFromJson(emitterJson["scaleAnim"].toObject()));
    emitter->setScaleRand(jsonToFloat(emitterJson["scaleRand"]));

    emitter->setEmitStartFrame(emitterJson["emitStartFrame"].toInt());
    emitter->setEmitEndFrame(emitterJson["emitEndFrame"].toInt());
    emitter->setLifeStep(emitterJson["lifeStep"].toInt());
    emitter->setLifeStepRandom(emitterJson["lifeStepRandom"].toInt());
    emitter->setEmitRate(emitterJson["emitRate"].toInt());

    emitter->setFigureVelocity(jsonToFloat(emitterJson["figureVelocity"]));
    emitter->setVelocityDirection(jsonToVec3f(emitterJson["velocityDir"].toObject()));
    emitter->setInitialVelocity(jsonToFloat(emitterJson["initVelocity"]));
    emitter->setInitialVelocityRandom(jsonToFloat(emitterJson["initVelocityRandom"]));
    emitter->setSpreadVector(jsonToVec3f(emitterJson["spreadVec"].toObject()));
    emitter->setAirResistance(jsonToFloat(emitterJson["airResist"]));

    emitter->setVolumeTblIndex(static_cast<u8>(emitterJson["volumeTblIndex"].toInt()));
    emitter->setVolumeType(static_cast<VolumeType>(emitterJson["volumeType"].toInteger()));
    emitter->setVolumeRadius(jsonToVec3f(emitterJson["volumeRadius"].toObject()));
    emitter->setVolumeArcWidth(emitterJson["volumeArcWidth"].toInt());
    emitter->setVolumeArcStart(emitterJson["volumeArcStart"].toInt());

    emitter->setRotationType(static_cast<RotType>(emitterJson["rotType"].toInteger()));
    emitter->setInitialRotation(jsonToVec3i(emitterJson["initRot"].toObject()));
    emitter->setInitialRotationRandom(jsonToVec3i(emitterJson["initRotRandom"].toObject()));
    emitter->setRotationVelocity(jsonToVec3i(emitterJson["rotVel"].toObject()));
    emitter->setRotationVelocityRandom(jsonToVec3i(emitterJson["rotVelRandom"].toObject()));
    emitter->setRotationBasis(jsonToVec2f(emitterJson["rotBasis"].toObject()));

    emitter->setAlphaAnim(alphaAnimFromJson(emitterJson["alphaAnim"].toObject()));

    emitter->setBlendFunction(static_cast<BlendFuncType>(emitterJson["blendFunc"].toInteger()));
    emitter->setDepthFunction(static_cast<DepthFuncType>(emitterJson["depthFunc"].toInteger()));
    emitter->setCombinerFunction(static_cast<ColorCombinerFuncType>(emitterJson["combinerFunc"].toInteger()));

    {
        const QJsonArray color0Json = emitterJson["color0"].toArray();
        for (s32 i = 0; i < 3 && i < color0Json.size(); ++i) {
            auto color = jsonToColor(color0Json[i].toObject());
            switch (i) {
            case 0: emitter->setStartColor(color); break;
            case 1: emitter->setMidColor(color); break;
            case 2: emitter->setEndColor(color); break;
            }
        }
    }

    emitter->setColorSection1(emitterJson["colorSection1"].toInt());
    emitter->setColorSection2(emitterJson["colorSection2"].toInt());
    emitter->setColorSection3(emitterJson["colorSection3"].toInt());
    emitter->setColorNumRepeat(emitterJson["colorNumRepeat"].toInt());
    emitter->setColorCalcType(static_cast<ColorCalcType>(emitterJson["colorCalcType"].toInteger()));
    emitter->setPrimaryColor(jsonToColor(emitterJson["color1"].toObject()));

    emitter->setTextureWrapT(static_cast<TextureWrap>(emitterJson["textureWrapT"].toInteger()));
    emitter->setTextureWrapS(static_cast<TextureWrap>(emitterJson["textureWrapS"].toInteger()));
    emitter->setTextureLodLevel(static_cast<u8>(emitterJson["textureLodLevel"].toInt()));
    emitter->setTextureFilter(static_cast<TextureFilter>(emitterJson["textureFilter"].toInteger()));
    emitter->setNumTexturePattern(static_cast<u16>(emitterJson["numTexturePattern"].toInt()));
    emitter->setNumTextureDivisionX(static_cast<u8>(emitterJson["numTextureDivisionX"].toInt()));
    emitter->setNumTextureDivisionY(static_cast<u8>(emitterJson["numTextureDivisionY"].toInt()));
    emitter->setTextureUVScale(jsonToVec2f(emitterJson["textureUVScale"].toObject()));
    {
        const QJsonArray patternTableJson = emitterJson["texturePatternTable"].toArray();
        std::array<u8, 16> patternTable{};
        for (s32 i = 0; i < 16 && i < patternTableJson.size(); ++i) {
            patternTable[i] = static_cast<u8>(patternTableJson[i].toInt());
        }
        emitter->setTexturePatternTable(patternTable);
    }
    emitter->setTexturePatternFrequency(static_cast<u16>(emitterJson["texturePatFreq"].toInt()));
    emitter->setTexturePatternFrameCount(static_cast<u16>(emitterJson["texturePatFrameCount"].toInt()));
    emitter->setIsTexturePatternAnim(emitterJson["isTexPatAnim"].toBool());

    // Resolve texture handle
    {
        const s32 texIdx = emitterJson["texture"].toInt();
        if (texIdx >= 0 && texIdx < static_cast<s32>(textures.size()) && textures[texIdx]) {
            emitter->setTexture(textures[texIdx].get());
        }
    }

    // Complex Properties
    {
        const QJsonObject complexJson = emitterJson["complex"].toObject();

        emitter->setFluctuationScale(jsonToFloat(complexJson["fluctuationScale"]));
        emitter->setFluctuationFrequency(jsonToFloat(complexJson["fluctuationFreq"]));
        emitter->setFluctuationPhaseRandom(complexJson["fluctuationPhaseRand"].toBool());
        emitter->setFluctuationFlags(BitFlag<FluctuationFlag>(static_cast<u16>(complexJson["fluctuationFlags"].toInt())));

        emitter->setStripeType(static_cast<StripeType>(complexJson["stripeType"].toInteger()));
        emitter->setStripeNumHistory(complexJson["stripeNumHistory"].toInt());
        emitter->setStripeStartAlpha(jsonToFloat(complexJson["stripeStartAlpha"]));
        emitter->setStripeEndAlpha(jsonToFloat(complexJson["stripeEndAlpha"]));
        emitter->setStripeUVScrollSpeed(jsonToVec2f(complexJson["stripeUVScrollSpeed"].toObject()));
        emitter->setStripeHistoryStep(complexJson["stripeHistoryStep"].toInt());
        emitter->setStripeDirInterpolate(jsonToFloat(complexJson["stripeDirInterpolate"]));
        emitter->setStripeFlags(BitFlag<StripeFlag>(static_cast<u16>(complexJson["stripeFlags"].toInt())));

        emitter->setFieldRandomBlank(complexJson["fieldRandomBlank"].toInt());
        emitter->setFieldRandomVelAdd(jsonToVec3f(complexJson["fieldRandomVelAdd"].toObject()));

        emitter->setFieldMagnetPower(jsonToFloat(complexJson["fieldMagnetPower"]));
        emitter->setFieldMagnetPos(jsonToVec3f(complexJson["fieldMagnetPos"].toObject()));
        {
            const BitFlag<FieldMagnetFlag> magnetFlags{static_cast<u32>(complexJson["fieldMagnetFlag"].toInteger())};
            emitter->setFieldMagnetAxisTargetX(magnetFlags.isSet(FieldMagnetFlag::AxisTargetX));
            emitter->setFieldMagnetAxisTargetY(magnetFlags.isSet(FieldMagnetFlag::AxisTargetY));
            emitter->setFieldMagnetAxisTargetZ(magnetFlags.isSet(FieldMagnetFlag::AxisTargetZ));
        }

        emitter->setFieldSpinRotate(complexJson["fieldSpinRotate"].toInt());
        emitter->setFieldSpinAxis(static_cast<FieldSpinAxis>(complexJson["fieldSpinAxis"].toInteger()));

        emitter->setFieldCollisionType(static_cast<FieldCollisionType>(complexJson["fieldCollisionType"].toInteger()));
        emitter->setFieldCollisionIsWorld(complexJson["fieldCollisionIsWorld"].toBool());
        emitter->setFieldCollisionCoord(jsonToFloat(complexJson["fieldCollisionCoord"]));
        emitter->setFieldCollisionCoef(jsonToFloat(complexJson["fieldCollisionCoef"]));

        emitter->setFieldConvergenceType(static_cast<FieldConvergenceType>(complexJson["fieldConvergenceType"].toInteger()));
        emitter->setFieldConvergencePos(jsonToVec3f(complexJson["fieldConvergencePos"].toObject()));

        emitter->setFieldPosAddPosition(jsonToVec3f(complexJson["fieldPosAddPosition"].toObject()));

        emitter->setFieldFlags(BitFlag<FieldFlag>(static_cast<u16>(complexJson["fieldFlags"].toInt())));

        // Child Properties
        {
            const QJsonObject childJson = complexJson["child"].toObject();

            emitter->setChildBillboardType(static_cast<BillboardType>(childJson["billboardType"].toInteger()));

            emitter->setChildEmitRate(childJson["emitRate"].toInt());
            emitter->setChildEmitTiming(childJson["emitTiming"].toInt());
            emitter->setChildLife(childJson["life"].toInt());
            emitter->setChildEmitStep(childJson["emitStep"].toInt());

            emitter->setChildRandVelocity(jsonToVec3f(childJson["randVelocity"].toObject()));
            emitter->setChildGravity(jsonToVec3f(childJson["gravity"].toObject()));
            emitter->setChildVelocityInheritRate(jsonToFloat(childJson["velocityInheritRate"]));
            emitter->setChildInitialPositionRand(jsonToFloat(childJson["initialPositionRand"]));
            emitter->setChildFigureVelocity(jsonToFloat(childJson["figureVelocity"]));
            emitter->setChildAirResistance(jsonToFloat(childJson["airResist"]));

            emitter->setChildRotationType(static_cast<RotType>(childJson["rotationType"].toInteger()));
            emitter->setChildInitialRotation(jsonToVec3i(childJson["initialRotation"].toObject()));
            emitter->setChildInitialRotationRandom(jsonToVec3i(childJson["initialRotationRandom"].toObject()));
            emitter->setChildRotationVelocity(jsonToVec3i(childJson["rotationVelocity"].toObject()));
            emitter->setChildRotationVelocityRandom(jsonToVec3i(childJson["rotationVelocityRandom"].toObject()));
            emitter->setChildRotationBasis(jsonToVec2f(childJson["rotationBasis"].toObject()));

            emitter->setChildScale(jsonToVec2f(childJson["scale"].toObject()));
            emitter->setChildScaleTarget(jsonToVec2f(childJson["scaleTarget"].toObject()));
            emitter->setChildScaleInheritRate(jsonToFloat(childJson["scaleInheritRate"]));
            emitter->setChildScaleStartFrame(childJson["scaleStartFrame"].toInt());

            emitter->setChildTextureWrapT(static_cast<TextureWrap>(childJson["textureWrapT"].toInteger()));
            emitter->setChildTextureWrapS(static_cast<TextureWrap>(childJson["textureWrapS"].toInteger()));
            emitter->setChildTextureLodLevel(static_cast<u8>(childJson["textureLodLevel"].toInt()));
            emitter->setChildTextureFilter(static_cast<TextureFilter>(childJson["textureFilter"].toInteger()));
            emitter->setChildTextureUVScale(jsonToVec2f(childJson["textureUVScale"].toObject()));

            // Resolve child texture handle
            {
                const s32 texIdx = childJson["texture"].toInt();
                if (texIdx >= 0 && texIdx < static_cast<s32>(textures.size()) && textures[texIdx]) {
                    emitter->setChildTexture(textures[texIdx].get());
                }
            }

            emitter->setChildSecondaryColor(jsonToColor(childJson["color0"].toObject()));
            emitter->setChildPrimaryColor(jsonToColor(childJson["color1"].toObject()));

            emitter->setChildAlpha(jsonToFloat(childJson["alpha"]));
            emitter->setChildAlphaTarget(jsonToFloat(childJson["alphaTarget"]));
            emitter->setChildAlphaInit(jsonToFloat(childJson["alphaInit"]));
            emitter->setChildAlphaStartFrame(childJson["alphaStartFrame"].toInt());
            emitter->setChildAlphaBaseFrame(childJson["alphaBaseFrame"].toInt());

            emitter->setChildBlendFunc(static_cast<BlendFuncType>(childJson["blendFunc"].toInteger()));
            emitter->setChildDepthFunc(static_cast<DepthFuncType>(childJson["depthFunc"].toInteger()));
            emitter->setChildCombinerFunc(static_cast<ColorCombinerFuncType>(childJson["combinerFunc"].toInteger()));
        }

        emitter->setChildFlags(BitFlag<ChildFlag>(static_cast<u16>(complexJson["childFlags"].toInt())));
    }

    return emitter;
}

std::unique_ptr<Emitter> importEmitter(const QString& filePath, const TextureList& textures) {
    const auto readResult = readJsonFile(filePath);
    if (!readResult) {
        return nullptr;
    }
    return importEmitterFromJson(readResult.value(), textures);
}

std::optional<EmitterSet> importEmitterSet(const QString& filePath, TextureList& textures) {
    const auto readResult = readJsonFile(filePath);
    if (!readResult) {
        return std::nullopt;
    }
    const auto& emitterSetJson = *readResult;

    if (validateMetaInfo(emitterSetJson["metaInfo"].toObject(), JsonFileType::EmitterSetFile, 1)) {
        return std::nullopt;
    }

    const auto name = emitterSetJson["name"].toString();
    const auto userData = static_cast<u32>(emitterSetJson["userData"].toInteger());
    const auto lastUpdateDate = static_cast<u32>(emitterSetJson["lastUpdateDate"].toInteger());

    EmitterSet emitterSet{};

    emitterSet.setName(name);
    emitterSet.setUserData(userData);
    emitterSet.setLastUpdateDate(lastUpdateDate);

    const QDir emitterSetDir{QFileInfo(filePath).absolutePath()};

    const QJsonObject emittersJson = emitterSetJson["emitters"].toObject();
    for (auto it = emittersJson.constBegin(); it != emittersJson.constEnd(); ++it) {
        bool ok{false};
        const s32 idx = it.key().toInt(&ok);
        if (!ok) {
            return std::nullopt;
        }

        const QString emitterPath = emitterSetDir.filePath(it.value().toString());
        auto emitter = importEmitter(emitterPath, textures);
        if (!emitter) {
            return std::nullopt;
        }

        emitterSet.insertEmitter(idx, std::move(emitter));
    }

    return emitterSet;
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
    return emitterSets;
}


} // namespace Internal


// ========================================================================== //


bool exportProject(const PtclRes& res, const QString& dirPath) {
    QString nativePath = QDir::toNativeSeparators(dirPath);

    QDir projectDir{nativePath};

    projectDir.mkdir("textures");
    projectDir.mkdir("emitterSets");

    QDir texturesDir{projectDir.filePath("textures")};
    QDir emitterSetsDir{projectDir.filePath("emitterSets")};

    TextureMap textureMap{};
    textureMap.reserve(res.textures().size());

    for (size_t idx = 0; idx < res.textures().size(); ++idx) {
        textureMap.emplace(res.textures()[idx].get(), idx);
    }

    QJsonObject projectJson{};
    projectJson["metaInfo"]    = createMetaInfo(JsonFileType::ProjectFile, 1);
    projectJson["name"]        = res.name();
    projectJson["textures"]    = exportTextures(res.textures(), texturesDir);
    projectJson["emitterSets"] = Internal::exportEmitterSets(res.getEmitterSets(), emitterSetsDir, textureMap);

    auto projectName = QString("%1.ptclproj").arg(res.name());

    if (!writeJsonFile(projectJson, projectDir.filePath(projectName))) {
        return false;
    }

    return true;
}

bool exportEmitter(const Emitter& emitter, const QString& filePath) {
    auto emitterJson = Internal::buildEmitterJson(emitter, true, nullptr);

    if (!writeJsonFile(emitterJson, filePath)) {
        return false;
    }

    return true;
}

bool exportEmitterSet(const EmitterSet& emitterSet, const QString& filePath) {
    std::vector<const Texture*> uniqueTextures{};
    std::unordered_map<const Texture*, s32> textureToIndex{};

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
        emittersJson[QString::number(idx)] = Internal::buildEmitterJson(*emitterSet.emitters().at(idx), false, &textureToIndex);
    }

    auto rootJson = Internal::buildEmitterSetJson(emitterSet);
    rootJson["textures"] = texturesJson;
    rootJson["emitters"] = emittersJson;

    if (!writeJsonFile(rootJson, filePath)) {
        return false;
    }

    return true;
}

std::optional<ImportEmitterResult> importEmitter(const QString& filePath, const QString& projectDir) {
    const auto emitterReadResult = readJsonFile(filePath);
    if (!emitterReadResult) {
        return std::nullopt;
    }
    const auto& emitterJson = *emitterReadResult;

    if (validateMetaInfo(emitterJson["metaInfo"].toObject(), JsonFileType::EmitterFile, 1)) {
        return std::nullopt;
    }

    const QJsonValue texVal = emitterJson["texture"];
    const QJsonObject complexJson = emitterJson["complex"].toObject();
    const QJsonObject childJson = complexJson["child"].toObject();
    const QJsonValue childTexVal = childJson["texture"];

    const bool isStandalone = texVal.isObject() || childTexVal.isObject();

    if (isStandalone) {
        TextureList textures{};
        if (texVal.isObject()) {
            auto tex = textureFromJson(texVal.toObject());
            if (tex) {
                textures.push_back(std::make_unique<Texture>(std::move(*tex)));
            }
        }
        if (childTexVal.isObject()) {
            auto tex = textureFromJson(childTexVal.toObject());
            if (tex) {
                textures.push_back(std::make_unique<Texture>(std::move(*tex)));
            }
        }

        auto emitter = Internal::importEmitter(filePath, textures);

        return ImportEmitterResult{std::move(emitter), std::move(textures)};
    }

    QDir sourceProjectDir{};
    if (!projectDir.isEmpty()) {
        sourceProjectDir = QDir{projectDir};
    } else {
        const QFileInfo emitterFileInfo{filePath};
        sourceProjectDir = QDir{emitterFileInfo.absolutePath()};
        sourceProjectDir.cdUp(); // emitterSets
        sourceProjectDir.cdUp(); // project root
    }

    const QDir texDir{sourceProjectDir.filePath("textures")};
    if (!texDir.exists()) {
        return std::nullopt;
    }

    const auto projFiles = sourceProjectDir.entryList({"*.ptclproj"}, QDir::Files);
    if (projFiles.isEmpty()) {
        return std::nullopt;
    }

    const auto projReadResult = readJsonFile(sourceProjectDir.filePath(projFiles.first()));
    if (!projReadResult) {
        return std::nullopt;
    }
    const auto& projJson = *projReadResult;

    auto sourceTextures = importTextures(projJson["textures"].toObject(), sourceProjectDir);
    if (!sourceTextures) {
        return std::nullopt;
    }

    auto emitter = Internal::importEmitter(filePath, *sourceTextures);
    if (!emitter) {
        return std::nullopt;
    }

    TextureList resultTextures{};
    std::map<Texture*, Texture*> sourceToReIded{};

    const auto getOrCreateReIded = [&](Texture* sourceTex) -> Texture* {
        if (!sourceTex || sourceTex->isPlaceholder()) {
            return nullptr;
        }

        auto it = sourceToReIded.find(sourceTex);
        if (it != sourceToReIded.end()) {
            return it->second;
        }

        std::vector<u8> data{sourceTex->textureDataRaw().begin(), sourceTex->textureDataRaw().end()};
        auto newTex = std::make_unique<Texture>(
            &data,
            sourceTex->textureData().width(),
            sourceTex->textureData().height(),
            sourceTex->textureFormat()
        );
        auto* ptr = newTex.get();
        sourceToReIded[sourceTex] = ptr;
        resultTextures.push_back(std::move(newTex));
        return ptr;
    };

    if (emitter->textureHandle().isValid()) {
        if (auto* newTex = getOrCreateReIded(emitter->texture())) {
            emitter->setTexture(newTex);
        }
    }
    if (emitter->childTextureHandle().isValid()) {
        if (auto* newTex = getOrCreateReIded(emitter->childTexture())) {
            emitter->setChildTexture(newTex);
        }
    }

    return ImportEmitterResult{std::move(emitter), std::move(resultTextures)};
}

std::optional<ImportEmitterSetResult> importEmitterSet(const QString& filePath, const QString& projectDir) {    
    const auto readResult = readJsonFile(filePath);
    if (!readResult) {
        return std::nullopt;
    }
    const auto& setJson = *readResult;

    if (validateMetaInfo(setJson["metaInfo"].toObject(), JsonFileType::EmitterSetFile, 1)) {
        return std::nullopt;
    }

    const bool isStandalone = setJson.contains("textures");

    auto emitterSet = std::make_unique<EmitterSet>();
    emitterSet->setName(setJson["name"].toString());
    emitterSet->setUserData(static_cast<u32>(setJson["userData"].toInteger()));
    emitterSet->setLastUpdateDate(static_cast<u32>(setJson["lastUpdateDate"].toInteger()));

    TextureList textures{};

    if (isStandalone) {
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

        const QJsonObject emittersJson = setJson["emitters"].toObject();
        for (auto it = emittersJson.constBegin(); it != emittersJson.constEnd(); ++it) {
            bool ok{false};
            const s32 idx = it.key().toInt(&ok);
            if (!ok) {
                return std::nullopt;
            }

            auto emitter = Internal::importEmitterFromJson(it.value().toObject(), textures);
            if (!emitter) {
                return std::nullopt;
            }

            const QJsonObject emitterJson = it.value().toObject();
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

            emitterSet->insertEmitter(idx, std::move(emitter));
        }
    } else {
        QDir sourceProjectDir{};
        if (!projectDir.isEmpty()) {
            sourceProjectDir = QDir{projectDir};
        } else {
            const QFileInfo setFileInfo{filePath};
            sourceProjectDir = QDir{setFileInfo.absolutePath()};
            sourceProjectDir.cdUp(); // project root
        }

        const QDir texDir{sourceProjectDir.filePath("textures")};
        if (!texDir.exists()) {
            return std::nullopt;
        }

        const auto projFiles = sourceProjectDir.entryList({"*.ptclproj"}, QDir::Files);
        if (projFiles.isEmpty()) {
            return std::nullopt;
        }

        const auto readResult = readJsonFile(sourceProjectDir.filePath(projFiles.first()));
        if (!readResult) {
            return std::nullopt;
        }
        const auto& projJson = *readResult;

        auto sourceTextures = importTextures(projJson["textures"].toObject(), sourceProjectDir);
        if (!sourceTextures) {
            return std::nullopt;
        }

        auto importedSet = Internal::importEmitterSet(filePath, *sourceTextures);
        if (!importedSet) {
            return std::nullopt;
        }

        emitterSet = std::make_unique<EmitterSet>(std::move(*importedSet));

        std::map<Texture*, Texture*> sourceToReIded{};

        const auto getOrCreateReIded = [&](Texture* sourceTex) -> Texture* {
            if (!sourceTex || sourceTex->isPlaceholder()) {
                return nullptr;
            }

            auto it = sourceToReIded.find(sourceTex);
            if (it != sourceToReIded.end()) {
                return it->second;
            }

            std::vector<u8> data{sourceTex->textureDataRaw().begin(), sourceTex->textureDataRaw().end()};
            auto newTex = std::make_unique<Texture>(
                &data,
                sourceTex->textureData().width(),
                sourceTex->textureData().height(),
                sourceTex->textureFormat()
            );
            auto* ptr = newTex.get();
            sourceToReIded[sourceTex] = ptr;
            textures.push_back(std::move(newTex));
            return ptr;
        };

        for (s32 i = 0; i < emitterSet->emitterCount(); ++i) {
            auto* emitter = emitterSet->emitters().at(i).get();
            if (emitter->textureHandle().isValid()) {
                if (auto* newTex = getOrCreateReIded(emitter->texture())) {
                    emitter->setTexture(newTex);
                }
            }
            if (emitter->childTextureHandle().isValid()) {
                if (auto* newTex = getOrCreateReIded(emitter->childTexture())) {
                    emitter->setChildTexture(newTex);
                }
            }
        }
    }

    return ImportEmitterSetResult{std::move(emitterSet), std::move(textures)};
}

bool importProject(const QString& projPath, PtclRes& res, [[maybe_unused]] PtclSanitizeReport& report) {
    const auto readResult = readJsonFile(projPath);
    if (!readResult) {
        return false;
    }
    const auto& projectJson = *readResult;

    if (validateMetaInfo(projectJson["metaInfo"].toObject(), JsonFileType::ProjectFile, 1)) {
        return false;
    }

    res.setName(projectJson["name"].toString());

    const QDir projectDir{QFileInfo(projPath).absolutePath()};

    auto textures = importTextures(projectJson["textures"].toObject(), projectDir);
    if (!textures) {
        return false;
    }

    res.textures() = std::move(*textures);

    auto emitterSets = Internal::importEmitterSets(projectJson["emitterSets"].toObject(), projectDir, res.textures());
    if (!emitterSets) {
        return false;
    }

    res.getEmitterSets() = std::move(*emitterSets);

    // TODO: Validate stuff

    return true;
}


// ========================================================================== //


} // namespace Ptcl::Json
