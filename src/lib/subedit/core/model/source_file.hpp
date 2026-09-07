#pragma once

#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/file_extras.hpp>
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

    /// The encoding the file was read in, byte order mark included.
    ///
    /// **Since phase 8**, where the boolean that stood here — `hadUtf8Bom` —
    /// stopped being enough: a UTF-16 mark answers a question a UTF-8 one does
    /// not, and both answers have to survive being written back.
    Encoding encoding = Encoding::utf8(ByteOrderMark::Absent);

    /// The free header, empty for the formats that have none.
    ///
    /// WebVTT until phase 9, and since then SubViewer 2's `[INFORMATION]`
    /// block, Sub Station Alpha's `[Script Info]` and styles, LRC's opening
    /// lines and MicroDVD's `{DEFAULT}` line. A string suffices to all of them:
    /// nothing has to understand it, only put it back.
    std::string header{};

    /// What the file declares about itself that no subtitle carries.
    ///
    /// **Since ADR 0030.** The header above is the free text; this is the rest,
    /// which a writer has to read rather than repeat — the order of Sub Station
    /// Alpha's event fields, the shape of TMPlayer's hour, the rate a MicroDVD
    /// was read at.
    FileExtras extras{};

    friend bool operator==(const SourceFile&, const SourceFile&) = default;
};

} // namespace subedit::core
