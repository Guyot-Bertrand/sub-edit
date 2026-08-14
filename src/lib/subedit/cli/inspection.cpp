#include <subedit/cli/batch.hpp>
#include <subedit/cli/inspection.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/file_system.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <algorithm>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace subedit::cli {

namespace {

using core::DiagnosticKind;
using core::Newline;
using core::ReadErrorKind;
using core::SubtitleFormat;

std::string_view nameOf(SubtitleFormat format) {
    switch (format) {
    case SubtitleFormat::SubRip:
        return "SubRip";
    case SubtitleFormat::WebVtt:
        return "WebVTT";
    }
    std::unreachable();
}

std::string_view nameOf(Newline newline) {
    switch (newline) {
    case Newline::Lf:
        return "LF";
    case Newline::CrLf:
        return "CRLF";
    case Newline::Cr:
        return "CR";
    }
    std::unreachable();
}

std::string_view reasonOf(ReadErrorKind kind) {
    switch (kind) {
    case ReadErrorKind::InvalidUtf8:
        return "is not valid UTF-8";
    case ReadErrorKind::NoSubtitleFound:
        return "holds nothing recognisable as a subtitle";
    case ReadErrorKind::UnknownFormat:
        return "is in no format this tool knows";
    }
    std::unreachable();
}

std::string_view reasonOf(core::FileErrorKind kind) {
    switch (kind) {
    case core::FileErrorKind::NotFound:
        return "does not exist";
    case core::FileErrorKind::PermissionDenied:
        return "cannot be opened: permission denied";
    case core::FileErrorKind::Io:
        return "cannot be read";
    }
    std::unreachable();
}

/// The line ending, plus where a second kind first appears when the file mixes
/// them. A file assembled out of two others does that more often than it
/// should, and it is worth saying rather than silently normalising.
std::string lineEndings(const core::ReadResult& read) {
    std::string text{nameOf(read.newline)};

    const auto mixed = std::ranges::find_if(read.diagnostics, [](const core::Diagnostic& d) {
        return d.kind == DiagnosticKind::MixedNewlines;
    });
    if (mixed != read.diagnostics.end()) {
        text += ", mixed from line " + std::to_string(mixed->line);
    }
    return text;
}

/// The whole stretch the subtitles cover: the earliest start and the latest
/// end, and not the first and last of the file — which would say something
/// false about a file whose order is broken.
std::string span(const std::vector<core::Subtitle>& subtitles) {
    // Never empty: `readSubtitles` refuses a file that holds no subtitle rather
    // than returning an empty list — see ReadErrorKind::NoSubtitleFound.
    core::Timestamp first = subtitles.front().start;
    core::Timestamp last = subtitles.front().end;
    for (const core::Subtitle& subtitle : subtitles) {
        first = std::min(first, subtitle.start);
        last = std::max(last, subtitle.end);
    }

    const auto write = [](core::Timestamp point) {
        return point.format(core::DecimalMark::Period, core::HourField::Always);
    };
    return write(first) + " -> " + write(last);
}

/// Which lines break the order, counted from one as they are shown.
std::string order(const core::Project& project) {
    const std::vector<core::SubtitleIndex> broken = project.outOfOrder();
    if (broken.empty()) {
        return "in order";
    }

    std::string text = broken.size() == 1 ? "line " : "lines ";
    for (std::size_t i = 0; i < broken.size(); ++i) {
        text += (i == 0 ? "" : ", ") + std::to_string(broken[i].number());
    }
    text += broken.size() == 1 ? " breaks the order" : " break the order";
    return text;
}

} // namespace

bool inspectFile(const core::FileSystem& files,
                 const std::string& path,
                 std::ostream& out,
                 const Reporter& reporter) {
    const std::expected<std::string, core::FileError> content = files.readFile(path);
    if (!content) {
        reporter.failed(path + ": " + std::string{reasonOf(content.error().kind)});
        return false;
    }

    reporter.say(3, path + ": " + std::to_string(content->size()) + " bytes read");

    const std::expected<core::ReadResult, core::ReadError> read = core::readSubtitles(*content);
    if (!read) {
        reporter.failed(path + ": " + std::string{reasonOf(read.error().kind)});
        return false;
    }

    // Most detailed first, least detailed last: that order is what makes each
    // level contain the one below it line for line.
    reporter.say(
        3, path + ": " + std::to_string(read->diagnostics.size()) + " diagnostic(s) while reading");
    reporter.say(2,
                 path + ": " + std::string{nameOf(read->format)} + ", UTF-8, " +
                     (read->hadUtf8Bom ? "BOM" : "no BOM") + ", " +
                     std::string{nameOf(read->newline)} + " line endings");
    reporter.say(1, path + ": " + std::to_string(read->subtitles.size()) + " subtitles");

    core::Project project;
    project.setSubtitles(read->subtitles);

    out << path << '\n';
    out << "  format: " << nameOf(read->format) << '\n';
    // The core refuses anything else outright, so there is only one answer to
    // give today. It is written all the same: a report that omits what it
    // cannot vary is a report that will quietly mean something else in phase 9.
    out << "  encoding: UTF-8\n";
    out << "  byte order mark: " << (read->hadUtf8Bom ? "present" : "absent") << '\n';
    out << "  line endings: " << lineEndings(*read) << '\n';
    out << "  subtitles: " << read->subtitles.size() << '\n';
    out << "  span: " << span(read->subtitles) << '\n';
    out << "  order: " << order(project) << '\n';

    return true;
}

ExitCode inspectAll(const core::FileSystem& files,
                    const std::vector<std::string>& paths,
                    std::ostream& out,
                    const Reporter& reporter) {
    std::size_t done = 0;
    for (const std::string& path : paths) {
        if (inspectFile(files, path, out, reporter)) {
            ++done;
        }
    }
    return tally(reporter, "inspected", done, paths.size());
}

} // namespace subedit::cli
