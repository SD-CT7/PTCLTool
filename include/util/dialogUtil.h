#pragma once

#include "util/fileUtil.h"
#include "util/settingsUtil.h"

#include <QFileDialog>

#include <initializer_list>


namespace DialogUtil {


// ========================================================================== //


inline QString getOpenFileName(QWidget* parent, const QString& title, SettingsUtil::PathType type,
                               std::initializer_list<FileKind> kinds) {
    const auto filePath = QFileDialog::getOpenFileName(
        parent,
        title,
        SettingsUtil::dialogPath(type),
        FileUtil::fileFilter(kinds)
    );

    if (filePath.isEmpty()) {
        return {};
    }

    SettingsUtil::setDialogPath(type, filePath);

    return filePath;
}

inline QString getSaveFileName(QWidget* parent, const QString& title, SettingsUtil::PathType type,
                               std::initializer_list<FileKind> kinds, const QString& defaultName = {}) {
    const auto filter = FileUtil::fileFilter(kinds);
    QString selectedFilter{};

    auto filePath = QFileDialog::getSaveFileName(
        parent,
        title,
        defaultName.isEmpty() ? SettingsUtil::dialogPath(type) : defaultName,
        filter,
        &selectedFilter
    );

    if (filePath.isEmpty()) {
        return {};
    }

    filePath = (*kinds.begin() == FileKind::Image)
        ? FileUtil::ensureImageExtention(filePath, selectedFilter)
        : FileUtil::ensureExtention(filePath, *kinds.begin());

    SettingsUtil::setDialogPath(type, filePath);

    return filePath;
}

inline QString getExistingDirectory(QWidget* parent, const QString& title, SettingsUtil::PathType type) {
    const auto dirPath = QFileDialog::getExistingDirectory(
        parent,
        title,
        SettingsUtil::dialogPath(type),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (dirPath.isEmpty()) {
        return {};
    }

    SettingsUtil::setDialogPath(type, dirPath);

    return dirPath;
}


// ========================================================================== //


} // namespace DialogUtil
