#pragma once

#include <utility>


// ========================================================================== //


enum class FileKind {
    // Json PTCL Formats
    Project,
    Texture,
    EmitterSet,
    Emitter,
    // Binary PTCL Formats
    Binary,
    // Other Formats
    Image,
    Unknown,
};

static_assert(std::to_underlying(FileKind::Project)    == 0, "fileType values are persisted in JSON metaInfo and must stay stable");
static_assert(std::to_underlying(FileKind::Texture)    == 1, "fileType values are persisted in JSON metaInfo and must stay stable");
static_assert(std::to_underlying(FileKind::EmitterSet) == 2, "fileType values are persisted in JSON metaInfo and must stay stable");
static_assert(std::to_underlying(FileKind::Emitter)    == 3, "fileType values are persisted in JSON metaInfo and must stay stable");


// ========================================================================== //
