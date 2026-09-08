#pragma once

#include <subedit/core/model/document.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>

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

    /// The encoding to write in, byte order mark included.
    ///
    /// UTF-8 without a mark by default, which is what the project writes unless
    /// told otherwise. Here rather than an argument of its own since phase 8:
    /// the encoding is a property of the file being written, like its endings,
    /// and a third positional argument said nothing at the call site.
    Encoding encoding = Encoding::utf8(ByteOrderMark::Absent);

    /// The header the file came with, for the formats that have one.
    ///
    /// Empty for the others, and for a project that came from nowhere; a
    /// writer that needs a header then produces the one its format requires.
    std::string_view header{};

    /// What the file declared about itself that no subtitle carries — ADR 0030.
    ///
    /// `std::monostate` for a document that came from nowhere, and for the
    /// formats that declare nothing. A writer that needs something here and
    /// finds nothing produces its own default rather than refusing — except
    /// where the default would move every position, which is MicroDVD.
    FileExtras extras{};
};

/// The header a document's file gives to a file of `target`.
///
/// **A header does not cross a format boundary**, and this is where that is
/// enforced rather than remembered. A WebVTT header at the top of a SubViewer 2
/// file is not a header at all: the reader will not take it back, and the round
/// trip is lost on the first save. Written in its own format, a document keeps
/// everything — which is what makes opening and saving change nothing.
///
/// Found by measuring, not by reasoning: `docs/mesures/conversion.md` turned
/// red the day a third format could hold a header, on a pair that had been
/// converting cleanly for six phases.
[[nodiscard]] inline std::string_view headerFor(const SourceFile& source, SubtitleFormat target) {
    return source.format == target ? std::string_view{source.header} : std::string_view{};
}

/// What a document's file declared about itself, for a file of `target`.
///
/// The same rule as the header above, on what ADR 0030 puts beside it: the
/// order of an event field, the shape of an hour, the rate frames were read
/// at. Each of them is read back by the writer of **that** format, and means
/// nothing to any other.
[[nodiscard]] inline FileExtras extrasFor(const SourceFile& source, SubtitleFormat target) {
    return source.format == target ? source.extras : FileExtras{};
}

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
