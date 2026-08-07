#pragma once

#include <subedit/core/model/document.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

/// What to write, and how.
struct WriteRequest {
    std::span<const Subtitle> subtitles{};

    /// Which of the two texts goes into the file.
    Document document = Document::Main;

    /// The ending to put back.
    ///
    /// LF by default, which is what the project writes unless told otherwise.
    /// The choice belongs to the caller, and the information needed to put
    /// back what a file arrived with is kept in `SourceFile`.
    Newline newline = Newline::Lf;

    /// The header the file came with, for the formats that have one.
    ///
    /// Empty for the others, and for a project that came from nowhere; a
    /// writer that needs a header then produces the one its format requires.
    std::string_view header{};
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

/// Appends `text`, rewriting the line feeds it holds as `ending`.
///
/// A text is held with line feeds whatever the file it came from used; the
/// ending is a property of the file, decided at writing time. Every writer has
/// the same rewriting to do, so it is written once.
inline void appendWithEnding(std::string& out, std::string_view text, std::string_view ending) {
    std::size_t start = 0;
    while (true) {
        const std::size_t lineFeed = text.find('\n', start);
        if (lineFeed == std::string_view::npos) {
            out += text.substr(start);
            return;
        }
        out += text.substr(start, lineFeed - start);
        out += ending;
        start = lineFeed + 1;
    }
}

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
