#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>

#include <expected>
#include <string_view>

namespace subedit::core {

/// Turns the text of a file into subtitles.
///
/// **A reader is handed already decoded content, never a path.** Reaching the
/// disk goes through an injected file system, so that a test reads a string
/// and needs neither a temporary file nor a directory it has to clean up.
///
/// One implementation per format. This is one of the five places where the
/// project knows the variation is real: two formats in phase 1, nine in the
/// end.
class SubtitleReader {

public:
    virtual ~SubtitleReader() = default;

    /// Reads what can be read, and says what was wrong with the rest.
    [[nodiscard]] virtual std::expected<ReadResult, ReadError>
    read(std::string_view content) const = 0;

protected:
    SubtitleReader() = default;
    SubtitleReader(const SubtitleReader&) = default;
    SubtitleReader(SubtitleReader&&) = default;
    SubtitleReader& operator=(const SubtitleReader&) = default;
    SubtitleReader& operator=(SubtitleReader&&) = default;
};

} // namespace subedit::core
