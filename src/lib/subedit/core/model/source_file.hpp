#pragma once

#include <subedit/core/model/subtitle_format.hpp>

#include <filesystem>
#include <optional>
#include <string>

namespace subedit::core {

/// The line ending a file uses.
enum class Newline {
    Lf,   ///< `\n`, Unix
    CrLf, ///< `\r\n`, Windows
    Cr,   ///< `\r`, classic Mac OS
};

/// What the file a project came from looked like.
///
/// None of it is decoration. Writing back a file that arrived with a BOM and
/// CRLF endings without them would rewrite every one of its lines, and the
/// user would find a diff where they expected a single corrected subtitle.
///
/// Only the shape of the file is recorded here — the path is a value, not a
/// handle. Reaching the disk goes through an injected abstraction, so that a
/// test reads a string and never a file.
struct SourceFile {
    /// Absent for a project that has never been opened or saved.
    std::optional<std::filesystem::path> path{};

    /// The format the file was read as, and the one saving writes back.
    ///
    /// **Here since ADR 0018**, where it was the one thing this struct did not
    /// carry: a window that opens a file then saves it has to remember what it
    /// opened, and passing the format alongside the project would have been a
    /// second source of truth for one answer.
    SubtitleFormat format = SubtitleFormat::SubRip;

    Newline newline = Newline::Lf;
    bool hadUtf8Bom = false;

    /// The WebVTT header, empty for the formats that have none.
    std::string header{};

    friend bool operator==(const SourceFile&, const SourceFile&) = default;
};

} // namespace subedit::core
