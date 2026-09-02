#pragma once

#include "util/bitflagUtil.h"

#include "typedefs.h"


namespace PtclEditor {


// ========================================================================== //


enum class NodeType {
    EmitterSet,
    Emitter,
    ChildData,
    Fluctuation,
    Field
};

enum class EmitterFilterFlag {
    Simple  = 1 << 0,
    Complex = 1 << 1,
    Compact = 1 << 2,
};

using EmitterFilter = BitFlag<EmitterFilterFlag>;

static constexpr s32 sRoleNodeType      = Qt::UserRole;
static constexpr s32 sRoleSetIdx        = Qt::UserRole + 1;
static constexpr s32 sRoleEmitterIdx    = Qt::UserRole + 2;
static constexpr s32 sRoleEnabled       = Qt::UserRole + 3;
static constexpr s32 sRoleEmitterType   = Qt::UserRole + 4;


// ========================================================================== //


} //namespace PtclEditor

