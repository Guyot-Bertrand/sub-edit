#pragma once

#include <filesystem>

namespace subedit::core {

/// Where the path of the associated video came from.
///
/// The distinction is not bookkeeping: decision D5 of the phase-6 spec rests
/// on it. The naming convention **proposes**, and a proposal is allowed to be
/// replaced by a later one; a user's choice **decides**, and nothing guessed
/// ever overwrites it — not opening another file, not saving under a new name.
enum class VideoOrigin {
    Guessed, ///< found by the naming convention, beside the subtitle file
    Chosen,  ///< named by the user
};

/// The video a document is watched against.
struct AssociatedVideo {
    std::filesystem::path path;
    VideoOrigin origin = VideoOrigin::Chosen;

    friend bool operator==(const AssociatedVideo&, const AssociatedVideo&) = default;
};

} // namespace subedit::core
