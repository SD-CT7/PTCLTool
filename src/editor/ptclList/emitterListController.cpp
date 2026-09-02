#include "editor/ptclList/emitterListController.h"

#include <QMessageBox>

#include <algorithm>


namespace PtclEditor {


// ========================================================================== //


EmitterListController::EmitterListController(QObject* parent) :
    QObject{parent} {}

void EmitterListController::setDocument(Ptcl::Document* document) {
    if (mDocument) {
        mDocument->disconnect(this);
    }

    mDocument = document;

    if (!mDocument) {
        mListModel.clear();
        mClipboardSet.reset();
        mClipboardEmitter.reset();
        return;
    }

    connect(mDocument, &Ptcl::Document::emitterAdded, this, [this](s32 setIndex, s32 emitterIndex) {
        QStandardItem* setItem = mListModel.item(setIndex);
        if (!setItem) {
            return;
        }

        insertEmitterNode(setItem, setIndex, emitterIndex);
        reindexEmitters(setItem, setIndex);
    });

    connect(mDocument, &Ptcl::Document::emitterRemoved, this, [this](s32 setIndex, s32 emitterIndex) {
        QStandardItem* setItem = mListModel.item(setIndex);
        if (!setItem) {
            return;
        }

        setItem->removeRow(emitterIndex);
        reindexEmitters(setItem, setIndex);
    });

    connect(mDocument, &Ptcl::Document::emitterSetAdded, this, [this](s32 setIndex) {
        insertEmitterSetNode(setIndex);
        reindexEmitterSets();
    });

    connect(mDocument, &Ptcl::Document::emitterSetRemoved, this, [this](s32 setIndex) {
        mListModel.removeRow(setIndex);
        reindexEmitterSets();
    });

    connect(mDocument, &Ptcl::Document::emitterChanged, this, [this](s32 setIndex, s32 emitterIndex) {
        updateEmitterName(setIndex, emitterIndex);
        updateEmitter(setIndex, emitterIndex);
    });

    connect(mDocument, &Ptcl::Document::emitterSetChanged, this, [this](s32 setIndex) {
        updateEmitterSetName(setIndex);
    });

    mListModel.clear();
    populate();
}

void EmitterListController::setSelection(Ptcl::Selection* selection) {
    mSelection = selection;
}

QStandardItemModel* EmitterListController::model() {
    return &mListModel;
}

const QStandardItemModel* EmitterListController::model() const {
    return &mListModel;
}

void EmitterListController::populate() {
    if (!mDocument) {
        return;
    }

    mListModel.clear();
    const auto& sets = mDocument->emitterSets();
    for (size_t setIndex = 0; setIndex < sets.size(); ++setIndex) {
        insertEmitterSetNode(static_cast<s32>(setIndex));
    }
}

QStandardItem* EmitterListController::findItem(s32 setIndex, s32 emitterIndex, Ptcl::Selection::Type type) const {
    auto* setItem = mListModel.item(setIndex);
    if (!setItem) {
        return nullptr;
    }

    if (type == Ptcl::Selection::Type::EmitterSet) {
        return setItem;
    }

    auto* emitterItem = setItem->child(emitterIndex);
    if (!emitterItem) {
        return nullptr;
    }

    switch (type) {
    case Ptcl::Selection::Type::Emitter:
        return emitterItem;
    case Ptcl::Selection::Type::EmitterChild:
        return findChildByType(emitterItem, NodeType::ChildData);
    case Ptcl::Selection::Type::EmitterFlux:
        return findChildByType(emitterItem, NodeType::Fluctuation);
    case Ptcl::Selection::Type::EmitterField:
        return findChildByType(emitterItem, NodeType::Field);
    default:
        return nullptr;
    }
}

bool EmitterListController::canPaste() const {
    return mClipboardSet || mClipboardEmitter;
}

void EmitterListController::insertEmitterSetNode(s32 setIndex) {
    const auto* set = mDocument->emitterSet(setIndex);
    if (!set) {
        return;
    }

    QString setName = QString("%1: %2").arg(setIndex).arg(set->name());
    auto* setItem = new QStandardItem(setName);
    setItem->setEditable(false);
    setItem->setData(static_cast<s32>(NodeType::EmitterSet), sRoleNodeType);
    setItem->setData(setIndex, sRoleSetIdx);

    for (s32 emitterIndex = 0; emitterIndex < mDocument->emitterCount(setIndex); ++emitterIndex) {
        insertEmitterNode(setItem, setIndex, emitterIndex);
    }
    mListModel.insertRow(setIndex, setItem);
}

void EmitterListController::insertEmitterNode(QStandardItem* setItem, s32 setIndex, s32 emitterIndex) {
    const auto* emitter = mDocument->emitter(setIndex, emitterIndex);
    if (!emitter) {
        return;
    }

    QString emitterName = QString("%1: %2").arg(emitterIndex).arg(emitter->name());
    auto* emitterItem = new QStandardItem(emitterName);
    emitterItem->setEditable(false);
    emitterItem->setData(static_cast<s32>(NodeType::Emitter), sRoleNodeType);
    emitterItem->setData(setIndex, sRoleSetIdx);
    emitterItem->setData(emitterIndex, sRoleEmitterIdx);
    emitterItem->setData(static_cast<u32>(emitter->type()), sRoleEmitterType);

    if (emitter->type() == Ptcl::EmitterType::Complex || emitter->type() == Ptcl::EmitterType::Compact) {
        addComplexNodes(emitterItem, setIndex, emitterIndex);
    }
    setItem->insertRow(emitterIndex, emitterItem);
}

QStandardItem* EmitterListController::findChildByType(QStandardItem* parent, NodeType type) {
    if (!parent) {
        return nullptr;
    }

    for (s32 i = 0; i < parent->rowCount(); ++i) {
        auto* child = parent->child(i);
        if (!child) {
            continue;
        }

        if (child->data(sRoleNodeType).toUInt() == static_cast<u32>(type)) {
            return child;
        }
    }
    return nullptr;
}

void EmitterListController::addComplexNodes(QStandardItem* emitterItem, s32 setIndex, s32 emitterIndex) {
    const auto& emitter = mDocument->emitter(setIndex, emitterIndex);

    ensureComplexNode(
        emitterItem,
        NodeType::ChildData,
        "ChildData",
        setIndex,
        emitterIndex,
        emitter->isChildEnabled()
    );

    ensureComplexNode(
        emitterItem,
        NodeType::Fluctuation,
        "Fluctuation",
        setIndex,
        emitterIndex,
        emitter->isFluctuationEnabled()
    );

    ensureComplexNode(
        emitterItem,
        NodeType::Field,
        "Field",
        setIndex,
        emitterIndex,
        emitter->isFieldEnabled()
    );
}

void EmitterListController::ensureComplexNode(QStandardItem* emitterItem, NodeType type, const QString& label, s32 setIndex, s32 emitterIndex, bool enabled) {
    auto* item = findChildByType(emitterItem, type);

    if (!item) {
        item = new QStandardItem(label);
        item->setEditable(false);
        item->setData(static_cast<s32>(type), sRoleNodeType);
        item->setData(setIndex, sRoleSetIdx);
        item->setData(emitterIndex, sRoleEmitterIdx);
        emitterItem->appendRow(item);
    }

    item->setData(enabled, sRoleEnabled);
}

void EmitterListController::selectNearestValidEmitter(s32 setIndex, s32 preferredEmitter) {
    if (!mDocument || !mSelection) {
        return;
    }

    const auto& set = mDocument->emitterSet(setIndex);
    const s32 count = set ? set->emitterCount() : 0;

    if (count <= 0) {
        mSelection->set(setIndex, 0, Ptcl::Selection::Type::EmitterSet);
        return;
    }

    const s32 clamped = std::clamp(preferredEmitter, 0, count - 1);
    mSelection->set(setIndex, clamped, Ptcl::Selection::Type::Emitter);
}

void EmitterListController::selectNearestValidEmitterSet(s32 preferredSet) {
    if (!mDocument || !mSelection) {
        return;
    }

    const s32 count = mDocument->emitterSetCount();

    if (count <= 0) {
        return;
    }

    const s32 clamped = std::clamp(preferredSet, 0, count - 1);
    mSelection->set(clamped, 0, Ptcl::Selection::Type::EmitterSet);
}

void EmitterListController::reindexEmitters(QStandardItem* setItem, s32 setIndex) {
    if (!setItem) {
        return;
    }

    for (s32 i = 0; i < setItem->rowCount(); ++i) {
        QStandardItem* emitterItem = setItem->child(i);
        if (!emitterItem) {
            continue;
        }

        emitterItem->setData(i, sRoleEmitterIdx);
        const auto* emitter = mDocument->emitter(setIndex, i);
        if (!emitter) {
            continue;
        }
        emitterItem->setText(QString("%1: %2").arg(i).arg(emitter->name()));

        for (s32 c = 0; c < emitterItem->rowCount(); ++c) {
            QStandardItem* child = emitterItem->child(c);
            if (child) {
                child->setData(i, sRoleEmitterIdx);
            }
        }
    }
}

void EmitterListController::reindexEmitterSets() {
    for (s32 i = 0; i < mListModel.rowCount(); ++i) {
        QStandardItem* setItem = mListModel.item(i);
        if (!setItem) {
            continue;
        }

        setItem->setData(i, sRoleSetIdx);
        const auto* set = mDocument->emitterSet(i);
        if (!set) {
            continue;
        }
        setItem->setText(QString("%1: %2").arg(i).arg(set->name()));
        reindexEmitters(setItem, i);
    }
}

void EmitterListController::updateEmitter(s32 setIndex, s32 emitterIndex) {
    const QStandardItem* setItem = mListModel.item(setIndex);
    if (!setItem) {
        return;
    }

    QStandardItem* emitterItem = setItem->child(emitterIndex);
    if (!emitterItem) {
        return;
    }

    const auto* emitter = mDocument->emitter(setIndex, emitterIndex);
    if (!emitter) {
        return;
    }
    emitterItem->setData(static_cast<u32>(emitter->type()), sRoleEmitterType);

    if (emitter->type() == Ptcl::EmitterType::Simple) {
        emitterItem->removeRows(0, emitterItem->rowCount());
        return;
    }

    addComplexNodes(emitterItem, setIndex, emitterIndex);
}

void EmitterListController::updateEmitterName(s32 setIndex, s32 emitterIndex) {
    const QStandardItem* setItem = mListModel.item(setIndex);
    if (!setItem) {
        return;
    }

    QStandardItem* emitterItem = setItem->child(emitterIndex);
    if (!emitterItem) {
        return;
    }

    const auto* emitter = mDocument->emitter(setIndex, emitterIndex);
    if (!emitter) {
        return;
    }

    QString emitterName = QString("%1: %2").arg(emitterIndex).arg(emitter->name());
    emitterItem->setText(emitterName);
}

void EmitterListController::updateEmitterSetName(s32 setIndex) {
    QStandardItem* setItem = mListModel.item(setIndex);
    if (!setItem) {
        return;
    }

    const auto* set = mDocument->emitterSet(setIndex);
    if (!set) {
        return;
    }

    QString setName = QString("%1: %2").arg(setIndex).arg(set->name());
    setItem->setText(setName);
}

void EmitterListController::addEmitterSet(QStandardItem* contextItem) {
    if (!contextItem || !mDocument || !mSelection) {
        return;
    }

    const s32 setIndex = mDocument->emitterSetCount();
    mDocument->addEmitterSet("Add New EmitterSet");

    mSelection->set(setIndex, 0, Ptcl::Selection::Type::EmitterSet);
    emit contentChanged();
}

void EmitterListController::addEmitter(QStandardItem* contextItem) {
    if (!contextItem || !mDocument || !mSelection) {
        return;
    }

    auto type = static_cast<NodeType>(contextItem->data(sRoleNodeType).toUInt());
    QStandardItem* setItem = contextItem;
    if (type == NodeType::Emitter) {
        setItem = contextItem->parent();
    }

    s32 setIndex = setItem->data(sRoleSetIdx).toInt();
    const auto& emitterSet = mDocument->emitterSet(setIndex);
    const s32 emitterIndex = emitterSet->emitterCount();

    mDocument->addEmitter("Add New Emitter", setIndex);
    mSelection->set(setIndex, emitterIndex, Ptcl::Selection::Type::Emitter);
    emit contentChanged();
}

void EmitterListController::removeItem(QStandardItem* contextItem) {
    if (!contextItem) {
        return;
    }

    auto type = static_cast<NodeType>(contextItem->data(sRoleNodeType).toUInt());

    if (type == NodeType::Emitter) {
        removeEmitter(contextItem->parent(), contextItem);
    } else if (type == NodeType::EmitterSet) {
        removeEmitterSet(contextItem);
    }
}

void EmitterListController::removeEmitter(QStandardItem* setItem, QStandardItem* emitterItem) {
    if (!setItem || !mDocument || !mSelection) {
        return;
    }

    const s32 setIndex = setItem->data(sRoleSetIdx).toInt();
    const s32 emitterIndex = emitterItem->data(sRoleEmitterIdx).toInt();

    const auto& emitter = mDocument->emitter(setIndex, emitterIndex);

    const auto confirmationMessage = QString("Are you sure you want to remove the Emitter '%1'?").arg(emitter->name());
    if (QMessageBox::question(nullptr, "Remove Emitter", confirmationMessage) != QMessageBox::Yes) {
        return;
    }

    const s32 nextPreferred = emitterIndex;
    mDocument->removeEmitter(setIndex, emitterIndex);
    selectNearestValidEmitter(setIndex, nextPreferred);
    emit contentChanged();
}

void EmitterListController::removeEmitterSet(QStandardItem* setItem) {
    if (!setItem || !mDocument || !mSelection) {
        return;
    }

    const s32 setIndex = setItem->data(sRoleSetIdx).toInt();
    const auto& emitterSet = mDocument->emitterSet(setIndex);

    const auto confirmationMessage = QString("Are you sure you want to remove the EmitterSet '%1'?").arg(emitterSet->name());
    if (QMessageBox::question(nullptr, "Remove EmitterSet", confirmationMessage) != QMessageBox::Yes) {
        return;
    }

    const s32 nextPreferred = setIndex;
    mDocument->removeEmitterSet(setIndex);
    selectNearestValidEmitterSet(nextPreferred);
    emit contentChanged();
}

void EmitterListController::copyItem(QStandardItem* contextItem) {
    if (!contextItem || !mDocument) {
        return;
    }

    const auto type = static_cast<NodeType>(contextItem->data(sRoleNodeType).toUInt());
    mClipboardSet.reset();
    mClipboardEmitter.reset();

    if (type == NodeType::EmitterSet) {
        const s32 setIndex = contextItem->data(sRoleSetIdx).toInt();
        mClipboardSet = mDocument->emitterSet(setIndex)->clone();
    } else if (type == NodeType::Emitter) {
        const s32 setIndex = contextItem->parent()->data(sRoleSetIdx).toInt();
        const s32 emitterIndex = contextItem->data(sRoleEmitterIdx).toInt();
        mClipboardEmitter = mDocument->emitter(setIndex, emitterIndex)->clone();
    }

    emit contentChanged();
}

void EmitterListController::pasteItem(QStandardItem* contextItem) {
    if (!contextItem || !mDocument || !mSelection) {
        return;
    }

    if (mClipboardSet) {
        mDocument->addEmitterSet("Paste EmitterSet", mClipboardSet->clone());

        const s32 setIndex = mDocument->emitterSetCount() - 1;
        mSelection->set(setIndex, 0, Ptcl::Selection::Type::EmitterSet);
    } else if (mClipboardEmitter) {
        auto type = static_cast<NodeType>(contextItem->data(sRoleNodeType).toUInt());

        s32 setIndex;
        if (type == NodeType::Emitter) {
            setIndex = contextItem->parent()->data(sRoleSetIdx).toInt();
        } else {
            setIndex = contextItem->data(sRoleSetIdx).toInt();
        }

        auto set = mDocument->emitterSet(setIndex);
        mDocument->addEmitter("Paste Emitter", setIndex, mClipboardEmitter->clone());

        const s32 emitterIndex = set->emitterCount() - 1;
        mSelection->set(setIndex, emitterIndex, Ptcl::Selection::Type::Emitter);
    }

    emit contentChanged();
}

void EmitterListController::duplicateItem(QStandardItem* contextItem) {
    if (!contextItem) {
        return;
    }

    auto type = static_cast<NodeType>(contextItem->data(sRoleNodeType).toUInt());

    if (type == NodeType::EmitterSet) {
        duplicateEmitterSet(contextItem);
    } else if (type == NodeType::Emitter) {
        duplicateEmitter(contextItem);
    }
}

void EmitterListController::duplicateEmitterSet(QStandardItem* contextItem) {
    if (!contextItem || !mDocument || !mSelection) {
        return;
    }

    const auto type = static_cast<NodeType>(contextItem->data(sRoleNodeType).toUInt());
    if (type != NodeType::EmitterSet) {
        return;
    }

    const s32 setIndex = contextItem->data(sRoleSetIdx).toInt();
    mDocument->addEmitterSet("Duplicate EmitterSet", mDocument->emitterSet(setIndex)->clone());

    const s32 newSetIndex = mDocument->emitterSetCount() - 1;
    mSelection->set(newSetIndex, 0, Ptcl::Selection::Type::EmitterSet);
    emit contentChanged();
}

void EmitterListController::duplicateEmitter(QStandardItem* contextItem) {
    if (!contextItem || !mDocument || !mSelection) {
        return;
    }

    const auto type = static_cast<NodeType>(contextItem->data(sRoleNodeType).toUInt());
    if (type != NodeType::Emitter) {
        return;
    }

    const s32 setIndex = contextItem->parent()->data(sRoleSetIdx).toInt();
    const s32 emitterIndex = contextItem->data(sRoleEmitterIdx).toInt();
    auto set = mDocument->emitterSet(setIndex);

    mDocument->addEmitter("Duplicate Emitter", setIndex, mDocument->emitter(setIndex, emitterIndex)->clone());

    const s32 newEmitterIndex = set->emitterCount() - 1;
    mSelection->set(setIndex, newEmitterIndex, Ptcl::Selection::Type::Emitter);
    emit contentChanged();
}


bool EmitterListController::importEmitterSet(const QString& filePath) {
    if (!mDocument || filePath.isEmpty()) {
        return false;
    }

    if (!mDocument->importEmitterSet(filePath)) {
        return false;
    }

    emit contentChanged();
    return true;
}


bool EmitterListController::importEmitter(s32 setIndex, const QString& filePath) {
    if (!mDocument || filePath.isEmpty()) {
        return false;
    }

    if (!mDocument->importEmitter(setIndex, filePath)) {
        return false;
    }

    emit contentChanged();
    return true;
}


bool EmitterListController::exportEmitter(s32 setIndex, s32 emitterIndex, const QString& filePath) {
    if (!mDocument || filePath.isEmpty()) {
        return false;
    }

    return mDocument->exportEmitter(setIndex, emitterIndex, filePath);
}


bool EmitterListController::exportEmitterSet(s32 setIndex, const QString& filePath) {
    if (!mDocument || filePath.isEmpty()) {
        return false;
    }

    return mDocument->exportEmitterSet(setIndex, filePath);
}


// ========================================================================== //


} // namespace PtclEditor