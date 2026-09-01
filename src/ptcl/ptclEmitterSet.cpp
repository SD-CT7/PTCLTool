#include "ptcl/ptclEmitterSet.h"


namespace Ptcl {


// ========================================================================== //

EmitterSet::EmitterSet(QString name) :
    mName{std::move(name)} {
    auto emitter = std::make_unique<Emitter>("New_Emitter_0");
    insertEmitter(0, std::move(emitter));
}


EmitterSet::EmitterSet(const BinEmitterSet& /*binEmitterSet*/) {}

EmitterList& EmitterSet::emitters() {
    return mEmitters;
}

const EmitterList& EmitterSet::emitters() const {
    return mEmitters;
}

const QString& EmitterSet::name() const {
    return mName;
}

void EmitterSet::setName(const QString& name) {
    mName = name;
}

s32 EmitterSet::emitterCount() const {
    return static_cast<s32>(mEmitters.size());
}

u32 EmitterSet::lastUpdateDate() const {
    return mLastUpdateDate;
}

void EmitterSet::setLastUpdateDate(u32 lastUpdateDate) {
    mLastUpdateDate = lastUpdateDate;
}

u32 EmitterSet::userData() const {
    return mUserData;
}

void EmitterSet::setUserData(u32 data) {
    mUserData = data;
}

void EmitterSet::insertEmitter(s32 emitterIndex, std::unique_ptr<Emitter> emitter) {
    mEmitters.insert(mEmitters.begin() + emitterIndex, std::move(emitter));
}

std::unique_ptr<Emitter> EmitterSet::removeEmitter(s32 emitterIndex) {
    auto it = mEmitters.begin() + emitterIndex;

    std::unique_ptr<Emitter> removed = std::move(*it);
    mEmitters.erase(it);
    return removed;
}

std::unique_ptr<EmitterSet> EmitterSet::clone() const {
    auto newSet = std::make_unique<EmitterSet>();

    newSet->mName = mName;
    newSet->mUserData = mUserData;
    newSet->mLastUpdateDate = mLastUpdateDate;

    for (const auto& emitter : mEmitters) {
        if (emitter) {
            newSet->mEmitters.push_back(emitter->clone());
        }
    }
    return newSet;
}

void EmitterSet::validate(PtclSanitizeReport& report) {
    const QString baseContext = report.context();

    for (s32 i = 0; i < static_cast<s32>(mEmitters.size()); ++i) {
        if (!mEmitters[i]) {
            continue;
        }

        report.setContext(baseContext.isEmpty() ?
            QStringLiteral("Emitter %1").arg(i) :
            QStringLiteral("%1 / Emitter %2").arg(baseContext).arg(i));
        mEmitters[i]->validate(report);
    }

    report.setContext(baseContext);
}


// ========================================================================== //


} // namespace Ptcl
