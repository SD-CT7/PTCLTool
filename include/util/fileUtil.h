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

inline const QStringList& imageFormats() {
    static const QStringList sFormats{
        QStringLiteral("png"),  QStringLiteral("jpg"), QStringLiteral("jpeg"),
        QStringLiteral("jfif"), QStringLiteral("bmp"), QStringLiteral("webp"),
        QStringLiteral("tga"),  QStringLiteral("tif"), QStringLiteral("tiff")
    };
    return sFormats;
}

inline bool fileHasImageExtention(const QString& path) {
    for (const auto& format : imageFormats()) {
        if (path.endsWith(QStringLiteral(".") + format, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

inline QStringList formatPatterns(const QStringList& extensions) {
    QStringList patterns{};
    for (const auto& extension : extensions) {
        patterns.append(QStringLiteral("*.") + extension);
    }
    return patterns;
}

inline QString fileFilter(std::initializer_list<FileKind> kinds) {
    QStringList filters{};
    QStringList allExtensions{};

    for (FileKind kind : kinds) {
        if (kind == FileKind::Image) {
            const auto& imageExtensions = imageFormats();

            filters.append(QStringLiteral("Image (%1)").arg(formatPatterns(imageExtensions).join(QStringLiteral(" "))));
            allExtensions.append(imageExtensions);

            for (const auto& format : imageExtensions) {
                filters.append(QStringLiteral("%1 (%2)").arg(format, QStringLiteral("*.") + format));
            }
            continue;
        }

        filters.append(QStringLiteral("%1 (*%2)").arg(fileDescription(kind), fileExtention(kind)));
        allExtensions.append(fileExtention(kind).mid(1));
    }

    if (kinds.size() > 1) {
        filters.prepend(QStringLiteral("All Supported Files (%1)").arg(formatPatterns(allExtensions).join(QStringLiteral(" "))));
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

inline QString ensureImageExtention(const QString& path, const QString& selectedFilter) {
    const auto& allowed = imageFormats();

    for (const auto& format : allowed) {
        if (path.endsWith(QStringLiteral(".") + format, Qt::CaseInsensitive)) {
            return path;
        }
    }

    for (const auto& format : allowed) {
        if (selectedFilter.indexOf(QStringLiteral("*.") + format) != -1) {
            return path + QStringLiteral(".") + format;
        }
    }

    return path + QStringLiteral(".") + allowed.first();
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
    if (QImageReader(path).canRead() && fileHasImageExtention(path)) {
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
