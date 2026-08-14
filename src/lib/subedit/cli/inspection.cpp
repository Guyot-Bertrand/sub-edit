#include <subedit/cli/batch.hpp>
#include <subedit/cli/inspection.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/wording.hpp>
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

/// Which lines are out of order, counted from one as they are shown.
///
/// The wording differs between the two readings, and that is what tells a
/// reader which one they are looking at: a bare list of indices would be
/// ambiguous between "breaks the order" and "starts late", which name different
/// lines on the same file.
std::string order(const core::Project& project, core::OrderReport reading) {
    const std::vector<core::SubtitleIndex> named = project.outOfOrder(reading);
    if (named.empty()) {
        return "in order";
    }

    const bool one = named.size() == 1;
    std::string text = one ? "line " : "lines ";
    for (std::size_t i = 0; i < named.size(); ++i) {
        text += (i == 0 ? "" : ", ") + std::to_string(named[i].number());
    }

    switch (reading) {
    case core::OrderReport::Breaks:
        text += one ? " breaks the order" : " break the order";
        break;
    case core::OrderReport::Late:
        text += one ? " starts late" : " start late";
        break;
    }
    return text;
}

} // namespace

bool inspectFile(const core::FileSystem& files,
                 const std::string& path,
                 std::ostream& out,
                 const Reporter& reporter,
                 core::OrderReport reading) {
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
    reporter.say(3,
                 path + ": " + countOf(read->diagnostics.size(), "diagnostic") + " while reading");
    reporter.say(2,
                 path + ": " + std::string{nameOf(read->format)} + ", UTF-8, " +
                     (read->hadUtf8Bom ? "BOM" : "no BOM") + ", " +
                     std::string{nameOf(read->newline)} + " line endings");
    reporter.say(1, path + ": " + countOf(read->subtitles.size(), "subtitle"));

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
    out << "  order: " << order(project, reading) << '\n';

    return true;
}

ExitCode inspectAll(const core::FileSystem& files,
                    const std::vector<std::string>& paths,
                    std::ostream& out,
                    const Reporter& reporter,
                    core::OrderReport reading) {
    std::size_t done = 0;
    for (const std::string& path : paths) {
        if (inspectFile(files, path, out, reporter, reading)) {
            ++done;
        }
    }
    return tally(reporter, "inspected", done, paths.size());
}

} // namespace subedit::cli
