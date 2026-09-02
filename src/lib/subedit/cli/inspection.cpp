#include <subedit/cli/batch.hpp>
#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/inspection.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/wording.hpp>

#include <algorithm>
#include <ostream>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace subedit::cli {

namespace {

using core::DiagnosticKind;

/// The line ending, plus where a second kind first appears when the file mixes
/// them. A file assembled out of two others does that more often than it
/// should, and it is worth saying rather than silently normalising.
std::string lineEndings(core::Newline newline, const std::vector<core::Diagnostic>& diagnostics) {
    std::string text{nameOf(newline)};

    const auto mixed = std::ranges::find_if(diagnostics, [](const core::Diagnostic& d) {
        return d.kind == DiagnosticKind::MixedNewlines;
    });
    if (mixed != diagnostics.end()) {
        text += ", mixed from line " + std::to_string(mixed->line);
    }
    return text;
}

/// The whole stretch the subtitles cover: the earliest start and the latest
/// end, and not the first and last of the file — which would say something
/// false about a file whose order is broken.
std::string span(std::span<const core::Subtitle> subtitles) {
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

/// What the positions say about the grid they were written on.
///
/// A subtitle file does not declare the frame rate its positions were computed
/// against — SubRip has no header, WebVTT's is free text — so this is deduced
/// rather than read. The lines below the first appear only when they say
/// something: a file on a clean grid with no ambiguity gets one line, which is
/// all there is to know about it.
void sayGrid(std::ostream& out, const core::Project& project) {
    const core::FrameRateDeduction grid = core::deduceFrameRate(project);

    if (!grid.enoughStarts) {
        out << "  frame rate grid: " << nameOf(grid.verdict) << " (too few subtitles to tell)\n";
        return;
    }

    if (grid.verdict == core::GridVerdict::Silent) {
        // **No rate is named**, and that is the point of a closed set of
        // candidates: a file regular on a grid that is none of the eight would
        // otherwise be reported as the least wrong of them. « I do not know »
        // is the right answer, and it is what makes the deduction usable.
        out << "  frame rate grid: " << nameOf(grid.verdict) << " (best candidate at "
            << core::percentOf(grid.ranked.front().concentration) << ")\n";
        return;
    }

    out << "  frame rate grid: " << core::nameOf(grid.retained.rate) << " fps, "
        << nameOf(grid.verdict) << " (" << core::percentOf(grid.retained.concentration) << ")\n";

    if (grid.retained.phase != core::Duration::zero())
        out << "  grid offset: " << core::secondsOf(grid.retained.phase) << "\n";

    if (grid.harmonic.has_value())
        out << "  also fits: " << core::nameOf(*grid.harmonic)
            << " fps, of which this rate is a whole divisor\n";

    if (!grid.notSeparated.empty()) {
        out << "  too short a span to separate:";
        for (const core::FrameRate other : grid.notSeparated)
            out << " " << core::nameOf(other) << " fps";
        out << "\n";
    }

    if (!grid.strays.empty())
        out << "  off the grid: " << grid.strays.size() << " of " << grid.starts << " starts, in "
            << core::countOf(core::runsOfStrays(grid), "run") << "\n";
}

/// What is wrong with the document, subtitle by subtitle.
///
/// **By subtitle number and not by line**, which is the distinction ADR 0018
/// draws: a line only exists while a file is being read, and this report
/// describes what the document holds. One subtitle may appear twice — starting
/// before the previous one ends and before it starts are two statements, fixed
/// two different ways.
std::string anomalies(const core::Project& project) {
    const std::vector<core::Anomaly> found = core::scanAnomalies(project);
    if (found.empty())
        return "none";

    std::string text;
    for (const core::Anomaly& anomaly : found) {
        if (!text.empty())
            text += ", ";
        text += "subtitle " + std::to_string(anomaly.index.number()) + " " +
                std::string{nameOf(anomaly.kind)};
    }

    return text;
}

} // namespace

bool inspectFile(const core::FileSystem& files,
                 const std::string& path,
                 std::ostream& out,
                 const Reporter& reporter) {
    const std::expected<core::OpenedFile, core::OpenError> opened = core::openProject(files, path);
    if (!opened) {
        reporter.failed(path + ": " + std::string{reasonOf(opened.error())});
        return false;
    }

    const core::Project& project = opened->project;
    const core::SourceFile& source = project.sourceFile();

    reporter.say(3, path + ": " + std::to_string(opened->bytes) + " bytes read");
    // Most detailed first, least detailed last: that order is what makes each
    // level contain the one below it line for line.
    reporter.say(3,
                 path + ": " + core::countOf(opened->diagnostics.size(), "diagnostic") +
                     " while reading");
    sayDiagnostics(reporter, path, opened->diagnostics);
    reporter.say(
        2,
        path + ": " + std::string{nameOf(source.format)} + ", " +
            std::string{source.encoding.charset()} + ", " +
            (source.encoding.byteOrderMark() == core::ByteOrderMark::Present ? "BOM" : "no BOM") +
            ", " + std::string{nameOf(source.newline)} + " line endings");
    reporter.say(1, path + ": " + core::countOf(project.subtitles().size(), "subtitle"));

    out << path << '\n';
    out << "  format: " << nameOf(source.format) << '\n';
    // Read off the model since phase 8, where the core learned to read other
    // encodings. Nothing yet asks it to — no option names one, and detection is
    // the issue after this — so the line still says UTF-8 every time; what
    // changed is that it says it because the file was read that way, and not
    // because there was nothing else to write.
    out << "  encoding: " << source.encoding.charset() << '\n';
    out << "  byte order mark: "
        << (source.encoding.byteOrderMark() == core::ByteOrderMark::Present ? "present" : "absent")
        << '\n';
    out << "  line endings: " << lineEndings(source.newline, opened->diagnostics) << '\n';
    out << "  subtitles: " << project.subtitles().size() << '\n';
    out << "  span: " << span(project.subtitles()) << '\n';
    sayGrid(out, project);
    out << "  anomalies: " << anomalies(project) << '\n';

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
