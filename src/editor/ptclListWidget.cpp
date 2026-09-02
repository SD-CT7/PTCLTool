#include "editor/ptclListWidget.h"
#include "util/iconUtil.h"

#include <functional>


namespace PtclEditor {


// ========================================================================== //


PtclList::PtclList(QWidget* parent) :
    QWidget{parent} {
    // Toolbar
    mToolBar.setIconSize(QSize(24, 24));
    mToolBar.setToolButtonStyle(Qt::ToolButtonIconOnly);

    mAddEmitterSetAction = mToolBar.addAction(QIcon(":/res/icons/add_emitterset.png"), "Add Emitter Set");
    connect(mAddEmitterSetAction, &QAction::triggered, this, [this] { mListController.addEmitterSet(currentItem()); });

    mAddEmitterAction = mToolBar.addAction(QIcon(":/res/icons/add_emitter.png"), "Add Emitter");
    connect(mAddEmitterAction, &QAction::triggered, this, [this] { mListController.addEmitter(currentItem()); });

    mToolBar.addSeparator();

    mRemoveAction = mToolBar.addAction("Remove");
    connect(mRemoveAction, &QAction::triggered, this, [this] { mListController.removeItem(currentItem()); });

    mToolBar.addSeparator();

    mCopyAction = mToolBar.addAction(QIcon(":/res/icons/copy.png"), "Copy");
    connect(mCopyAction, &QAction::triggered, this, [this] { mListController.copyItem(currentItem()); });

    mPasteAction = mToolBar.addAction(QIcon(":/res/icons/paste.png"), "Paste");
    connect(mPasteAction, &QAction::triggered, this, [this] { mListController.pasteItem(currentItem()); });

    for (auto* act : { mAddEmitterSetAction, mAddEmitterAction, mRemoveAction, mCopyAction, mPasteAction }) {
        act->setEnabled(false);
    }

    // Proxy Model
    mProxyModel.setSourceModel(mListController.model());
    mProxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel.setRecursiveFilteringEnabled(false);

    // Tree View
    mTreeView.setModel(&mProxyModel);
    mTreeView.setHeaderHidden(true);
    mTreeView.setContextMenuPolicy(Qt::CustomContextMenu);
    connect(&mTreeView, &QTreeView::customContextMenuRequested, this, &PtclList::showContextMenu);
    connect(&mTreeView, &QTreeView::clicked, this, [this](const QModelIndex& proxyIndex) {
        const QModelIndex sourceIndex = mProxyModel.mapToSource(proxyIndex);
        QStandardItem* item = mListController.model()->itemFromIndex(sourceIndex);
        mContextItem = item;

        if (!item || !mSelection) {
            return;
        }

        updateToolbarForSelection(item);

        const auto type = static_cast<NodeType>(item->data(sRoleNodeType).toUInt());
        const s32 setIndex = item->data(sRoleSetIdx).toInt();
        const s32 emitterIndex = item->data(sRoleEmitterIdx).toInt();

        switch (type) {
        case NodeType::EmitterSet:
            mSelection->set(setIndex, 0, Ptcl::Selection::Type::EmitterSet);
            break;
        case NodeType::Emitter:
            mSelection->set(setIndex, emitterIndex, Ptcl::Selection::Type::Emitter);
            break;
        case NodeType::ChildData:
            mSelection->set(setIndex, emitterIndex, Ptcl::Selection::Type::EmitterChild);
            break;
        case NodeType::Fluctuation:
            mSelection->set(setIndex, emitterIndex, Ptcl::Selection::Type::EmitterFlux);
            break;
        case NodeType::Field:
            mSelection->set(setIndex, emitterIndex, Ptcl::Selection::Type::EmitterField);
            break;
        default:
            break;
        }
    });

    // Search Box
    mSearchBox.setPlaceholderText("Search");
    connect(&mSearchBox, &QLineEdit::textChanged, this, &PtclList::filterList);
    connect(&mSearchBox, &QLineEdit::textChanged, this, [this] {
        mTreeView.expandAll();
    });

    // Filter Button
    mFilterButton.setPopupMode(QToolButton::InstantPopup);
    mFilterButton.setMenu(&mFilterMenu);

    connect(&mFilterMenu, &EmitterFilterMenu::emitterFilterChanged, this, [this](const EmitterFilter& filter) {
        mProxyModel.setEmitterFilter(filter);
    });

    // Shortcuts
    connect(&mCopyShortcut, &QShortcut::activated, this, [this] { mListController.copyItem(currentItem()); });
    connect(&mPasteShortcut, &QShortcut::activated, this, [this] { mListController.pasteItem(currentItem()); });
    connect(&mDuplicateShortcut, &QShortcut::activated, this, [this] { mListController.duplicateItem(currentItem()); });

    connect(&mListController, &EmitterListController::contentChanged, this, [this] {
        applyIcons();
        updateToolbarForSelection(mContextItem);
    });

    mContextMenu.setListController(&mListController);

    // Search Layout
    auto* searchLayout = new QHBoxLayout;
    searchLayout->addWidget(&mSearchBox);
    searchLayout->addWidget(&mFilterButton);

    // Main layout
    mMainLayout.addWidget(&mToolBar);
    mMainLayout.addWidget(&mTreeView);
    mMainLayout.addLayout(searchLayout);

    setLayout(&mMainLayout);

    applyIcons();
    connect(&IconManager::instance(), &IconManager::iconsChanged, this, &PtclList::applyIcons);
}

QStandardItem* PtclList::currentItem() const {
    const QModelIndex proxyIndex = mTreeView.currentIndex();
    const QModelIndex sourceIndex = mProxyModel.mapToSource(proxyIndex);
    return mListController.model()->itemFromIndex(sourceIndex);
}

QIcon PtclList::nodeIcon(NodeType type) const {
    const char* name = nullptr;

    switch (type) {
    case NodeType::EmitterSet:
        name = "emitterset";
        break;
    case NodeType::Emitter:
        name = "emitter";
        break;
    default:
        return {};
    }

    return IconManager::instance().icon(
        QStringLiteral(":/res/icons/%1.svg").arg(QLatin1String(name)),
        QPalette::Text,
        this,
        {24, 24},
        IconRotation::None
    );
}

void PtclList::applyIcons() {
    constexpr QSize iconSize{24, 24};

    IconUtil::setIcon(mAddEmitterSetAction, "add_emitterset", this, iconSize);
    IconUtil::setIcon(mAddEmitterAction, "add_emitter", this, iconSize);
    IconUtil::setIcon(mRemoveAction, "remove", this, iconSize);
    IconUtil::setIcon(mCopyAction, "copy", this, iconSize);
    IconUtil::setIcon(mPasteAction, "paste", this, iconSize);

    IconUtil::setIcon(&mFilterButton, "filter", this, iconSize);

    // Tree Nodes
    const auto applyNodeIcon = [this](QStandardItem* item) {
        const auto type = static_cast<NodeType>(
            item->data(sRoleNodeType).toInt()
            );

        item->setIcon(nodeIcon(type));
    };

    std::function<void(QStandardItem*)> visit = [&](QStandardItem* item) {
        applyNodeIcon(item);

        for (s32 i = 0; i < item->rowCount(); ++i) {
            visit(item->child(i));
        }
    };

    auto* listModel = mListController.model();
    for (s32 i = 0; i < listModel->rowCount(); ++i) {
        visit(listModel->item(i));
    }
}

void PtclList::showContextMenu(const QPoint& pos) {
    if (!mDocument) {
        return;
    }

    QModelIndex proxyIndex = mTreeView.indexAt(pos);
    QModelIndex sourceIndex = mProxyModel.mapToSource(proxyIndex);
    QStandardItem* item = mListController.model()->itemFromIndex(sourceIndex);
    if (!item) {
        return;
    }

    mContextMenu.showForItem(
        mTreeView.viewport()->mapToGlobal(pos),
        item->data(sRoleSetIdx).toInt(),
        item->data(sRoleEmitterIdx).toInt(),
        static_cast<NodeType>(item->data(sRoleNodeType).toUInt()),
        item
    );
}

void PtclList::setDocument(Ptcl::Document* document) {
    mDocument = document;
    mListController.setDocument(document);
    mContextMenu.setDocument(document);

    if (!document) {
        mSearchBox.clear();
        mContextItem = nullptr;
        setEnabled(false);
        return;
    }

    mTreeView.expandAll();
    filterList(mSearchBox.text());
    applyIcons();
    setEnabled(true);
}

void PtclList::setSelection(Ptcl::Selection* selection) {
    mSelection = selection;
    mListController.setSelection(selection);

    connect(selection, &Ptcl::Selection::selectionChanged, this, [this](s32 setIndex, s32 emitterIndex, Ptcl::Selection::Type type) {
        if (!mDocument) {
            return;
        }

        QSignalBlocker b(mTreeView.selectionModel());

        const QStandardItem* item = mListController.findItem(setIndex, emitterIndex, type);
        if (!item) {
            mTreeView.clearSelection();
            return;
        }

        for (const QStandardItem* ancestor = item; ancestor; ancestor = ancestor->parent()) {
            const QModelIndex ancestorSource = mListController.model()->indexFromItem(ancestor);
            const QModelIndex ancestorProxy = mProxyModel.mapFromSource(ancestorSource);
            if (ancestorProxy.isValid()) {
                mTreeView.expand(ancestorProxy);
            }
        }

        const QModelIndex sourceIndex = mListController.model()->indexFromItem(item);
        const QModelIndex proxyIndex = mProxyModel.mapFromSource(sourceIndex);

        if (!proxyIndex.isValid()) {
            mTreeView.clearSelection();
            return;
        }

        auto* selectionModel = mTreeView.selectionModel();
        selectionModel->setCurrentIndex(proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        mTreeView.scrollTo(proxyIndex);
    });
}

void PtclList::filterList(const QString& text) {
    mProxyModel.setFilterFixedString(text);
}

void PtclList::updateToolbarForSelection(const QStandardItem* item) {
    mAddEmitterSetAction->setEnabled(true);
    mAddEmitterAction->setEnabled(false);
    mRemoveAction->setEnabled(false);
    mCopyAction->setEnabled(false);
    mPasteAction->setEnabled(false);

    if (!item) {
        return;
    }

    const auto type = static_cast<NodeType>(item->data(sRoleNodeType).toUInt());

    mPasteAction->setEnabled(mListController.canPaste());

    switch (type) {
    case NodeType::EmitterSet: {
        mAddEmitterAction->setEnabled(true);

        const s32 emitterSetCount = mListController.model()->rowCount();
        mRemoveAction->setEnabled(emitterSetCount > 1);
        mCopyAction->setEnabled(true);
        break;
    }

    case NodeType::Emitter: {
        mAddEmitterAction->setEnabled(true);

        const auto* setItem = item->parent();
        if (!setItem) {
            break;
        }

        const s32 emitterCount = setItem->rowCount();
        mRemoveAction->setEnabled(emitterCount > 1);
        mCopyAction->setEnabled(true);
        break;
    }
    case NodeType::ChildData:
    case NodeType::Fluctuation:
    case NodeType::Field:
        mAddEmitterAction->setEnabled(false);
        break;
    }
}


// ========================================================================== //


} // namespace PtclEditor