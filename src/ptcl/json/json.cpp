#include "ptcl/json/json.h"

#include "ptcl/json/jsonCommon.h"
#include "ptcl/json/jsonTexture.h"
#include "ptcl/json/jsonEmitterSet.h"
#include "util/fileUtil.h"


#include <QFileInfo>
#include <QJsonObject>
#include <QDir>

namespace Ptcl::Json {


// ========================================================================== //


bool exportProject(const PtclRes& res, const QString& dirPath) {
    QString nativePath = QDir::toNativeSeparators(dirPath);

    QDir projectDir{nativePath};

    projectDir.mkdir("textures");
    projectDir.mkdir("emitterSets");

    QDir texturesDir{projectDir.filePath("textures")};
    QDir emitterSetsDir{projectDir.filePath("emitterSets")};

    TextureIndexMap textureMap{};
    textureMap.reserve(res.textures().size());

    for (size_t idx = 0; idx < res.textures().size(); ++idx) {
        textureMap.emplace(res.textures()[idx].get(), idx);
    }

    QJsonObject json{};
    json.insert("metaInfo",    createMetaInfo(FileKind::Project, 1));
    json.insert("name",        res.name());
    json.insert("textures",    exportTextures(res.textures(), texturesDir));
    json.insert("emitterSets", exportEmitterSets(res.getEmitterSets(), emitterSetsDir, textureMap));

    auto projectName = FileUtil::ensureExtention(res.name(), FileKind::Project);

    return writeJsonFile(json, projectDir.filePath(projectName));
}

bool importProject(const QString& projPath, PtclRes& res, [[maybe_unused]] PtclSanitizeReport& report) {
    const auto readResult = readJsonFile(projPath);
    if (!readResult) {
        return false;
    }
    const auto& json = *readResult;

    if (!validateMetaInfo(json["metaInfo"].toObject(), FileKind::Project, 1)) {
        return false;
    }

    res.setName(json["name"].toString());

    const QDir projectDir{QFileInfo(projPath).absolutePath()};

    auto textures = importTextures(json["textures"].toObject(), projectDir);
    if (!textures) {
        return false;
    }

    res.textures() = std::move(*textures);

    auto emitterSets = importEmitterSets(json["emitterSets"].toObject(), projectDir, res.textures());
    if (!emitterSets) {
        return false;
    }

    res.getEmitterSets() = std::move(*emitterSets);

    // TODO: Validate stuff

    return true;
}


// ========================================================================== //


} // namespace Ptcl::Json