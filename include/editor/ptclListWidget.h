#pragma once

#include "util/bitflagUtil.h"
#include "ptcl/ptclDocument.h"
#include "editor/ptclList/ptclListRoles.h"
#include "editor/ptclList/emitterFilterProxyModel.h"

#include <QLineEdit>
#include <QMenu>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QToolBar>

#include <memory>


namespace PtclEditor {


// ========================================================================== //


class PtclList : public QWidget {
    Q_OBJECT
public:
    explicit PtclList(QWidget* parent = nullptr);

    void setDocument(Ptcl::Document* document);
    void setSelection(Ptcl::Selection* selection);

    void updateEmitter(s32 setIndex, s32 emitterIndex);
    void updateEmitterName(s32 setIndex, s32 emitterIndex);
    void updateEmitterSetName(s32 setIndex);

private slots:
    void filterList(const QString& text);

private:
    void populateList();
    void setupFilterMenu();
    void setupContextMenu();
    void applyIcons();

    QIcon nodeIcon(NodeType type) const;

    void addComplexNodes(QStandardItem* emitterItem, s32 setIndex, s32 emitterIndex);
    void ensureComplexNode(QStandardItem* emitterItem, NodeType type, const QString& label, s32 setIndex, s32 emitterIndex, bool enabled);

    void updateToolbarForSelection(const QStandardItem* item);

    QStandardItem* findItem(s32 setIndex, s32 emitterIndex, Ptcl::Selection::Type type) const;
    static QStandardItem* findChildByType(QStandardItem* parent, NodeType type);

    void insertEmitterNode(QStandardItem* setItem, s32 setIndex, s32 emitterIndex);
    void insertEmitterSetNode(s32 setIndex);

    void addEmitter(QStandardItem* contextItem = nullptr);
    void addEmitterSet(QStandardItem* contextItem = nullptr);

    void removeItem(QStandardItem* contextItem = nullptr);
    void removeEmitter(QStandardItem* setItem, QStandardItem* emitterItem);
    void removeEmitterSet(QStandardItem* setItem);

    void reindexEmitters(QStandardItem* setItem, s32 setIndex);
    void reindexEmitterSets();

    void selectNearestValidEmitter(s32 setIndex, s32 preferredEmitter);
    void selectNearestValidEmitterSet(s32 preferredSet);

    void expandSourceIndex(const QModelIndex& sourceIndex);

    void copyItem(QStandardItem* contextItem = nullptr);
    void pasteItem(QStandardItem* contextItem = nullptr);

    void duplicateItem(QStandardItem* contextItem = nullptr);
    void duplicateEmitterSet(QStandardItem* contextItem = nullptr);
    void duplicateEmitter(QStandardItem* contextItem = nullptr);

private:
    Ptcl::Document* mDocument{nullptr};
    Ptcl::Selection* mSelection{nullptr};

    QStandardItemModel mListModel{};
    QTreeView  mTreeView{};
    QLineEdit mSearchBox{};
    QToolButton mFilterButton{};
    QMenu mFilterMenu{};

    QToolBar mToolBar{};
    QAction* mAddEmitterSetAction{nullptr};
    QAction* mAddEmitterAction{nullptr};
    QAction* mRemoveAction{nullptr};
    QAction* mCopyAction{nullptr};
    QAction* mPasteAction{nullptr};

    QVBoxLayout mMainLayout{};

    EmitterFilterProxyModel mProxyModel{};

    std::unique_ptr<Ptcl::EmitterSet> mClipboardSet;
    std::unique_ptr<Ptcl::Emitter> mClipboardEmitter;

    QShortcut mCopyShortcut{QKeySequence::Copy, this};
    QShortcut mPasteShortcut{QKeySequence::Paste, this};
    QShortcut mDuplicateShortcut{QKeySequence(Qt::CTRL | Qt::Key_D), this};
};


// ========================================================================== //


} //namespace PtclEditor

