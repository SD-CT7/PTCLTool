#pragma once

#include "editor/emitterList/emitterListTypes.h"

#include <QMenu>


namespace PtclEditor {


// ========================================================================== //

class EmitterFilterMenu : public QMenu {
    Q_OBJECT
public:
    explicit EmitterFilterMenu(QWidget* parent = nullptr);

signals:
    void emitterFilterChanged(const PtclEditor::EmitterFilter& filter);
};


// ========================================================================== //


} // namespace PtclEditor
