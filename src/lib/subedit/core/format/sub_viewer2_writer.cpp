#include <subedit/core/format/sub_viewer2_syntax.hpp>
#include <subedit/core/format/sub_viewer2_writer.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

namespace {

/// What a file that never had a header gets, so that it can be read back.
///
/// Gaupol's template, verbatim. The values are the empty ones a new file
/// carries; what matters is the shape, since the bracketed lines are what the
/// reader and the detection recognise.
constexpr std::string_view kTemplateHeader = "[INFORMATION]\n"
                                             "[TITLE]\n"
                                             "[AUTHOR]\n"
                                             "[SOURCE]\n"
                                             "[PRG]\n"
                                             "[FILEPATH]\n"
                                             "[DELAY]0\n"
                                             "[CD TRACK]0\n"
                                             "[COMMENT]\n"
                                             "[END INFORMATION]\n"
                                             "[SUBTITLE]\n"
                                             "[COLF]&HFFFFFF,[STYLE]bd,[SIZE]18,[FONT]Sans";

} // namespace

std::string SubViewer2Writer::write(const WriteRequest& request) const {
    const std::string_view ending = charactersOf(request.newline);

    std::string out;
    appendWithEnding(out, request.header.empty() ? kTemplateHeader : request.header, ending);
    out += ending;

    for (const Subtitle& subtitle : request.subtitles) {
        // The blank line opens the block rather than closing it, which is what
        // leaves the file without a trailing one.
        out += ending;

        out +=
            subtitle.start.format(DecimalMark::Period, HourField::Always, Decimals::Centiseconds);
        out += ',';
        out += subtitle.end.format(DecimalMark::Period, HourField::Always, Decimals::Centiseconds);
        out += ending;

        out += textToSubViewer2(subtitle.text(request.document));
        out += ending;
    }
    return out;
}

} // namespace subedit::core
