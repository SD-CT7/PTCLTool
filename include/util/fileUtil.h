#pragma once

#include "util/fileKind.h"
#include "ptcl/json/jsonCommon.h"

#include <QFile>
#include <QImageReader>
#include <QStringList>

#include <utility>


// ========================================================================== //


namespace FileUtil {


inline QString fileExtention(FileKind kind) {
    switch (kind) {
    case FileKind::Binary:     return ".ptcl";
    case FileKind::Project:    return ".ptclproj";
    case FileKind::Texture:    return ".ptex";
    case FileKind::EmitterSet: return ".pset";
    case FileKind::Emitter:    return ".pemt";
    case FileKind::Image:
    case FileKind::Unknown:
        break;
    }
    std::unreachable();
}

inline QString fileDescription(FileKind kind) {
    switch (kind) {
    case FileKind::Binary:     return "PTCL Binary";
    case FileKind::Project:    return "PTCL Project";
    case FileKind::Texture:    return "PTCL Texture";
    case FileKind::EmitterSet: return "EmitterSet";
    case FileKind::Emitter:    return "Emitter";
    case FileKind::Image:
    case FileKind::Unknown:
        break;
    }
    std::unreachable();
}

inline QString fileFilter(std::initializer_list<FileKind> kinds) {
    QStringList filters{};

    for (FileKind kind : kinds) {
        filters.append(QStringLiteral("%1 (*%2)").arg(fileDescription(kind), fileExtention(kind)));
    }

    return filters.join(QStringLiteral(";;"));
}

inline bool hasExtention(const QString& path, FileKind kind) {
    const auto extension = fileExtention(kind);
    return path.endsWith(extension);
}

inline QString ensureExtention(const QString& path, FileKind kind) {
    const auto extension = fileExtention(kind);

    if (path.endsWith(extension)) {
        return path;
    }

    return path + extension;
}

inline bool isPtclBinary(const QString& path) {
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray magic = file.read(4);
    return magic == "SPBD";
}

inline FileKind classifyFile(const QString& path) {
    if (QImageReader(path).canRead()) {
        return FileKind::Image;
    }

    if (isPtclBinary(path)) {
        return FileKind::Binary;
    }

    if (const auto json = Ptcl::Json::readJsonFile(path)) {
        if (const auto kind = Ptcl::Json::classifyJson(*json)) {
            return *kind;
        }
    }

    return FileKind::Unknown;
}


// ========================================================================== //


} // namespace FileUtil
