#pragma once

// Compatibility helpers so the code base builds with both Qt 5.15 (Windows 7/8/8.1)
// and Qt 6.x.

#include <QtGlobal>
#include <QPointF>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFormLayout>
#include <QWidget>
#include <QImage>
#include <QCheckBox>
#include <QVariant>
#include <type_traits>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define PTCL_QT5 1
#endif

// QSortFilterProxyModel::beginFilterChange/endFilterChange only exist in Qt >= 6.10.
// Call them as PTCL_BEGIN_FILTER_CHANGE() / PTCL_END_FILTER_CHANGE() inside the proxy model.
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
#define PTCL_BEGIN_FILTER_CHANGE() beginFilterChange()
#define PTCL_END_FILTER_CHANGE() endFilterChange()
#else
#define PTCL_BEGIN_FILTER_CHANGE() ((void)0)
#define PTCL_END_FILTER_CHANGE() invalidateFilter()
#endif

// QCheckBox::checkStateChanged exists in Qt >= 6.7; older versions use stateChanged(int).
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
#define PTCL_CHECKBOX_CHANGED &QCheckBox::checkStateChanged
#else
#define PTCL_CHECKBOX_CHANGED &QCheckBox::stateChanged
#endif

namespace QtCompat {

// QImage::flipped exists in Qt >= 6.9; older versions use mirrored().
inline QImage flipped(const QImage& image, Qt::Orientations orient) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    return image.flipped(orient);
#else
    return image.mirrored(orient.testFlag(Qt::Horizontal), orient.testFlag(Qt::Vertical));
#endif
}


inline QPointF eventPos(const QMouseEvent* e) {
#ifdef PTCL_QT5
    return e->localPos();
#else
    return e->position();
#endif
}

inline QPointF eventPos(const QWheelEvent* e) {
#ifdef PTCL_QT5
    return e->posF();
#else
    return e->position();
#endif
}

// QFormLayout::setRowVisible only exists in Qt >= 6.4
inline void setRowVisible(QFormLayout* layout, QWidget* field, bool visible) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    layout->setRowVisible(field, visible);
#else
    int row = -1;
    QFormLayout::ItemRole role;
    layout->getWidgetPosition(field, &row, &role);
    if (row < 0) {
        return;
    }
    if (auto* label = layout->itemAt(row, QFormLayout::LabelRole)) {
        if (auto* w = label->widget()) { w->setVisible(visible); }
    }
    if (auto* f = layout->itemAt(row, QFormLayout::FieldRole)) {
        if (auto* w = f->widget()) { w->setVisible(visible); }
        else if (auto* l = f->layout()) {
            for (int i = 0; i < l->count(); ++i) {
                if (auto* cw = l->itemAt(i)->widget()) { cw->setVisible(visible); }
            }
        }
    }
    if (auto* span = layout->itemAt(row, QFormLayout::SpanningRole)) {
        if (auto* w = span->widget()) { w->setVisible(visible); }
    }
#endif
}

// Enums stored in QVariant: Qt5 requires Q_DECLARE_METATYPE for each enum, so store as int.
template <typename E>
inline QVariant enumToVariant(E value) {
    static_assert(std::is_enum_v<E>);
    return QVariant(static_cast<int>(value));
}

template <typename E>
inline E enumFromVariant(const QVariant& v) {
    static_assert(std::is_enum_v<E>);
    return static_cast<E>(v.toInt());
}

} // namespace QtCompat
