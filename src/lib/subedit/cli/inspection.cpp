#include <subedit/cli/batch.hpp>
#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/inspection.hpp>
#include <subedit/cli/json.hpp>
#include <subedit/cli/opening.hpp>
#include <subedit/cli/pairing.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/analysis/anomaly.hpp>
#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/analysis/grid_verdict.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/translation.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/core/wording/formats.hpp>
#include <subedit/core/wording/translation.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <utility>
#include <variant>
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

/// The encoding, and how the reading arrived at it.
///
/// **Three answers, and they are not worth the same**: a mark is what the file
/// declares, an encoding asked for is what the user decided, and a detection is
/// a proposal that Latin-1 and CP1252 can both fit. Saying which of the three
/// it was is what lets a reader know whether to trust the line.
///
/// The mark comes first here as it does in the reading: a file that declares
/// its encoding is not read against its own word, even when another was asked
/// for — and the reading says so in its diagnostics.
std::string encoding(const core::Encoding& read, bool asked) {
    if (read.byteOrderMark() == core::ByteOrderMark::Present)
        return std::string{read.charset()} + ", from its byte order mark";

    return std::string{read.charset()} + (asked ? ", as asked for" : ", detected");
}

/// The rate a file counted in frames was read at, and where that rate came from.
///
/// **The same shape as the encoding line, and for the same reason**: a number
/// every position on screen rests on is worth nothing without knowing who chose
/// it. Two answers rather than three, because a frames file declares nothing —
/// that absence is the whole difficulty ADR 0030 was written for.
///
/// Which of the two it was is read from the reading itself: `AssumedFrameRate`
/// is left behind exactly when nobody chose, so the report never has to be told
/// twice.
std::string frameRate(core::FrameRate rate, const std::vector<core::Diagnostic>& diagnostics) {
    const bool assumed = std::ranges::any_of(diagnostics, [](const core::Diagnostic& d) {
        return d.kind == DiagnosticKind::AssumedFrameRate;
    });
    return std::string{core::nameOf(rate)} + " fps, " + (assumed ? "assumed" : "as asked for");
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

/// What is wrong with the document, subtitle by subtitle — **one per line**.
///
/// **By subtitle number and not by line**, which is the distinction ADR 0018
/// draws: a line only exists while a file is being read, and this report
/// describes what the document holds. One subtitle may appear twice — starting
/// before the previous one ends and before it starts are two statements, fixed
/// two different ways.
///
/// **A list and not a sentence**, since a badly made file carries hundreds of
/// them, and a hundred comma-joined statements on one line cannot be read nor
/// searched. A document with nothing wrong keeps its single line, `none`, which
/// is what a script looks for.
std::string anomalies(const core::Project& project) {
    const std::vector<core::Anomaly> found = core::scanAnomalies(project);
    if (found.empty())
        return " none\n";

    std::string text = "\n";
    for (const core::Anomaly& anomaly : found) {
        text += "    " + statementOf(anomaly) + "\n";
    }

    return text;
}

std::string_view idOf(core::Newline newline) {
    switch (newline) {
    case core::Newline::Lf:
        return "lf";
    case core::Newline::CrLf:
        return "crlf";
    case core::Newline::Cr:
        return "cr";
    }
    std::unreachable();
}

std::string_view idOf(core::AnomalyKind kind) {
    switch (kind) {
    case core::AnomalyKind::EndBeforeStart:
        return "end-before-start";
    case core::AnomalyKind::OverlappingSubtitles:
        return "overlapping-subtitles";
    case core::AnomalyKind::OutOfOrder:
        return "out-of-order";
    }
    std::unreachable();
}

std::string_view idOf(core::GridVerdict verdict) {
    switch (verdict) {
    case core::GridVerdict::Clean:
        return "clean";
    case core::GridVerdict::Partial:
        return "partial";
    case core::GridVerdict::Silent:
        return "silent";
    }
    std::unreachable();
}

/// A concentration runs from 0 to 100; the record gives it in thousandths of the
/// whole, so that a perfect grid is 1000 and no number carries a decimal point.
[[nodiscard]] std::int64_t permilleOf(double concentration) {
    constexpr double kThousandthsPerPoint = 10.0;
    return std::llround(concentration * kThousandthsPerPoint);
}

/// Where a file's encoding came from, as an identifier.
[[nodiscard]] std::string_view originOf(const core::Encoding& read, bool asked) {
    if (read.byteOrderMark() == core::ByteOrderMark::Present)
        return "byte-order-mark";
    return asked ? "asked" : "detected";
}

/// What the positions say about the grid, as an object — the same facts as
/// `sayGrid`, **none of them left for a script to read out of a sentence**.
///
/// Rates are strings (a rate is a ratio, and a ratio is not a floating point
/// number) and a concentration is an integer in thousandths.
[[nodiscard]] Json gridOf(const core::Project& project) {
    const core::FrameRateDeduction grid = core::deduceFrameRate(project);
    const bool named = grid.enoughStarts && grid.verdict != core::GridVerdict::Silent;

    Json object = Json::object();
    object.set("verdict", idOf(grid.verdict));
    object.set("enough_starts", grid.enoughStarts);
    object.set("rate", named ? Json{core::nameOf(grid.retained.rate)} : Json{});
    if (!grid.enoughStarts) {
        object.set("concentration_permille", Json{});
    } else {
        // Silent names no rate, and its best candidate is still worth the number.
        const double concentration = grid.verdict == core::GridVerdict::Silent
                                         ? grid.ranked.front().concentration
                                         : grid.retained.concentration;
        object.set("concentration_permille", permilleOf(concentration));
    }
    object.set("offset_ms", named ? Json{grid.retained.phase.milliseconds()} : Json{});
    object.set("also_fits",
               named && grid.harmonic.has_value() ? Json{core::nameOf(*grid.harmonic)} : Json{});
    Json separated = Json::array();
    if (named) {
        for (const core::FrameRate other : grid.notSeparated)
            separated.push(core::nameOf(other));
    }
    object.set("not_separated", std::move(separated));
    object.set("strays", named ? Json{grid.strays.size()} : Json{});
    object.set("starts", grid.starts);
    return object;
}

/// The description of one file, for `--format json`: what `inspect` prints,
/// as keys.
[[nodiscard]] Json descriptionOf(std::string_view command,
                                 const std::string& path,
                                 const core::OpenedFile& opened,
                                 bool encodingAsked,
                                 const std::optional<Pairing>& pairing,
                                 const std::optional<core::TranslationOutcome>& alignment) {
    const core::Project& project = opened.project;
    const core::SourceFile& source = project.sourceFile();

    Json warnings = warningsOf(opened.diagnostics);
    Json record = recordOf(command, path, true, warnings);

    record.set("format", core::optionNameOf(source.format));
    record.set("encoding",
               Json::object()
                   .set("charset", std::string{source.encoding.charset()})
                   .set("origin", originOf(source.encoding, encodingAsked))
                   .set("byte_order_mark",
                        source.encoding.byteOrderMark() == core::ByteOrderMark::Present));

    const auto mixed = std::ranges::find_if(opened.diagnostics, [](const core::Diagnostic& d) {
        return d.kind == DiagnosticKind::MixedNewlines;
    });
    record.set("line_endings",
               Json::object()
                   .set("kind", idOf(source.newline))
                   .set("mixed_from_line",
                        mixed != opened.diagnostics.end() ? Json{mixed->line} : Json{}));

    record.set("subtitles", project.subtitles().size());

    core::Timestamp first = project.subtitles().front().start;
    core::Timestamp last = project.subtitles().front().end;
    for (const core::Subtitle& subtitle : project.subtitles()) {
        first = std::min(first, subtitle.start);
        last = std::max(last, subtitle.end);
    }
    record.set("span_ms",
               Json::object().set("start", first.milliseconds()).set("end", last.milliseconds()));

    // A file counted in frames has a rate and no grid, as in the text.
    if (const auto* frames = std::get_if<core::MicroDvdFile>(&source.extras)) {
        const bool assumed = std::ranges::any_of(opened.diagnostics, [](const core::Diagnostic& d) {
            return d.kind == DiagnosticKind::AssumedFrameRate;
        });
        record.set("frame_rate",
                   Json::object()
                       .set("rate", core::nameOf(frames->rate))
                       .set("origin", assumed ? "assumed" : "asked"));
        record.set("grid", Json{});
    } else {
        record.set("frame_rate", Json{});
        record.set("grid", gridOf(project));
    }

    Json anomalies = Json::array();
    for (const core::Anomaly& anomaly : core::scanAnomalies(project)) {
        anomalies.push(
            Json::object().set("subtitle", anomaly.index.number()).set("kind", idOf(anomaly.kind)));
    }
    record.set("anomalies", std::move(anomalies));
    // **Only when a translation was asked for**: a file read alone has no
    // alignment to report, and the key is not written for it.
    if (pairing && alignment) {
        record.set("translation", alignmentOf(*pairing, *alignment));
    }
    record.set("warnings", std::move(warnings));
    return record;
}

} // namespace

bool inspectFile(const core::FileSystem& files,
                 const std::string& path,
                 const core::ReadingChoices& reading,
                 std::ostream& out,
                 const Reporter& reporter,
                 const std::optional<Pairing>& pairing) {
    std::optional<core::OpenedFile> opened = openReporting(files, path, reading, reporter);
    if (!opened) {
        return false;
    }

    // The translation is laid over a copy: the report of the main file below is
    // of the file as it was read, and an attachment born of a line would change
    // its count.
    std::optional<core::TranslationOutcome> alignment;
    if (pairing) {
        core::Session session{opened->project};
        const std::expected<Paired, Failure> paired = pair(files, session, *pairing, reading);
        if (!paired) {
            // The file working on is the main one, and the failure is the
            // translation's: it names it.
            reportFailure(
                reporter,
                path,
                Failure{paired.error().kind, pairing->translation + ": " + paired.error().message});
            return false;
        }
        alignment = paired->outcome;
        // What reading the translation decided is said with the rest.
        opened->diagnostics.insert(
            opened->diagnostics.end(), paired->diagnostics.begin(), paired->diagnostics.end());
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
    reporter.say(2,
                 path + ": " + std::string{nameOf(source.format)} + ", " + nameOf(source.encoding) +
                     ", " + std::string{nameOf(source.newline)} + " line endings");
    reporter.say(1, path + ": " + core::countOf(project.subtitles().size(), "subtitle"));
    if (alignment) {
        reporter.say(1, path + ": " + core::noticeOf(*alignment));
    }

    // The description as a record, and not as lines: what the text says for a
    // human, the record says as keys, and the two never both go to the output.
    if (reporter.recording()) {
        reporter.record(descriptionOf(
            reporter.command(), path, *opened, reading.encoding.has_value(), pairing, alignment));
        return true;
    }

    out << path << '\n';
    out << "  format: " << nameOf(source.format) << '\n';
    out << "  encoding: " << encoding(source.encoding, reading.encoding.has_value()) << '\n';
    out << "  byte order mark: "
        << (source.encoding.byteOrderMark() == core::ByteOrderMark::Present ? "present" : "absent")
        << '\n';
    out << "  line endings: " << lineEndings(source.newline, opened->diagnostics) << '\n';
    out << "  subtitles: " << project.subtitles().size() << '\n';
    out << "  span: " << span(project.subtitles()) << '\n';
    // **A file counted in frames gets its rate, not a grid.** Deducing one from
    // positions that were computed *from* frames at that very rate would answer
    // with the number it was given, dressed as a measurement.
    if (const auto* frames = std::get_if<core::MicroDvdFile>(&source.extras)) {
        out << "  frame rate: " << frameRate(frames->rate, opened->diagnostics) << '\n';
    } else {
        sayGrid(out, project);
    }
    out << "  anomalies:" << anomalies(project);
    if (alignment && pairing) {
        out << "  translation file: " << pairing->translation << ", matched by "
            << (pairing->method == core::TranslationMethod::Position ? "position" : "number")
            << '\n';
        out << "  " << core::noticeOf(*alignment) << '\n';
    }

    return true;
}

ExitCode inspectAll(const core::FileSystem& files,
                    const std::vector<std::string>& paths,
                    const core::ReadingChoices& reading,
                    std::ostream& out,
                    const Reporter& reporter,
                    const std::optional<Pairing>& pairing) {
    std::size_t done = 0;
    for (const std::string& path : paths) {
        if (inspectFile(files, path, reading, out, reporter, pairing)) {
            ++done;
        }
    }
    return tally(reporter, "inspected", done, paths.size());
}

} // namespace subedit::cli
