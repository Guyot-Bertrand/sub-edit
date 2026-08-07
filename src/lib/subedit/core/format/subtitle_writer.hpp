#pragma once

#include <subedit/core/model/document.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

/// What to write, and how.
struct WriteRequest {
    std::span<const Subtitle> subtitles;

    /// Which of the two texts goes into the file.
    Document document = Document::Main;

    /// The ending to put back.
    ///
    /// LF by default, which is what the project writes unless told otherwise.
    /// The choice belongs to the caller, and the information needed to put
    /// back what a file arrived with is kept in `SourceFile`.
    Newline newline = Newline::Lf;
};

/// Turns subtitles into the text of a file.
///
/// **A writer returns a string, it does not touch the disk.** Putting it there
/// is the business of `writeAtomically`, which knows how not to destroy the
/// previous version — a concern no format has to repeat.
class SubtitleWriter {

public:
    virtual ~SubtitleWriter() = default;

    [[nodiscard]] virtual std::string write(const WriteRequest& request) const = 0;

protected:
    SubtitleWriter() = default;
    SubtitleWriter(const SubtitleWriter&) = default;
    SubtitleWriter(SubtitleWriter&&) = default;
    SubtitleWriter& operator=(const SubtitleWriter&) = default;
    SubtitleWriter& operator=(SubtitleWriter&&) = default;
};

/// Returns the characters `newline` stands for.
[[nodiscard]] constexpr std::string_view charactersOf(Newline newline) {
    switch (newline) {
    case Newline::Lf:
        return "\n";
    case Newline::CrLf:
        return "\r\n";
    case Newline::Cr:
        return "\r";
    }
    std::unreachable();
}

} // namespace subedit::core
