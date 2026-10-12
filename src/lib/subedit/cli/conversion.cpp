#include <subedit/cli/batch.hpp>
#include <subedit/cli/conversion.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/disjoining.hpp>
#include <subedit/cli/encoding_grammar.hpp>
#include <subedit/cli/joining.hpp>
#include <subedit/cli/opening.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/sorting.hpp>
#include <subedit/cli/writing.hpp>
#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/format/degradation.hpp>
#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/core/wording/conversion.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/core/wording/formats.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace subedit::cli {

namespace {

using core::SubtitleFormat;

/// Converts one file. Returns true when it was written.
/// The rate a file written in frames is counted at, or why there is none.
///
/// **Three sources, and the second is what phase 16 was written for.** What the
/// caller said comes first. Then the grid the positions fall on — the rate a
/// time-based file was timed at *is* its grid, and deducing it is the one place
/// that measurement decides something rather than informing. Then nothing: the
/// only move left would be to invent a number that displaces every subtitle.
[[nodiscard]] std::expected<core::FrameRate, Failure>
frameRateForFrames(const core::Project& project,
                   const std::optional<core::FrameRate>& asked,
                   const std::string& path,
                   const Reporter& reporter) {
    if (asked.has_value())
        return *asked;

    const core::FrameRateDeduction deduced = core::deduceFrameRate(project);
    if (deduced.verdict == core::GridVerdict::Silent) {
        return std::unexpected(
            Failure{"no-frame-rate", [] {
                        return core::translate(
                            "writing frames needs a frame rate, and the positions fall on "
                            "no grid to take one from — give --frame-rate");
                    }});
    }

    reporter.say(2,
                 core::substitute(
                     // TRANSLATORS: %1 is a file, %2 a frame rate.
                     core::translate("%1: counted in frames at %2, the grid the positions fall on"),
                     {path, core::nameOf(deduced.retained.rate)}));
    return deduced.retained.rate;
}

/// Which of the corrections that come before the writing were asked for.
struct Preparation {
    bool join = false;
    bool sort = false;
    bool disjoint = false;

    [[nodiscard]] bool any() const { return join || sort || disjoint; }
};

/// What the corrections made of a project: a copy when any was asked for, and
/// what each reports.
struct Arranged {
    std::optional<core::Project> copy;
    std::optional<core::JoinReport> joined;
    std::size_t moved = 0;
    std::optional<core::DisjointReport> cut;
};

/// What a conversion counts: the subtitles, and each post of what it lost.
///
/// The posts are the ones `noticeOf` words, each as an integer — a flag is 0 or
/// 1 — so that the sentence and the record come from the one `ConversionLoss`.
[[nodiscard]] std::vector<Count> countsOfConversion(const core::ConvertedProject& converted,
                                                    const Arranged& arranged,
                                                    const Preparation& preparation) {
    const core::ConversionLoss& loss = converted.loss;
    std::vector<Count> counts = {
        {"subtitles", static_cast<std::int64_t>(converted.subtitles.size())},
        {"lost_ends", loss.ends ? 1 : 0},
        {"joined_lines", static_cast<std::int64_t>(loss.joined)},
        {"lost_tags", static_cast<std::int64_t>(loss.tags)},
        {"lost_header", loss.header ? 1 : 0},
        {"lost_fields", static_cast<std::int64_t>(loss.fields)},
        {"furthest_ms", loss.precision}};
    // Only with the options: a record that grew a field nobody asked for would
    // change the shape of every existing script's input.
    if (arranged.joined.has_value())
        counts.emplace_back("joined", static_cast<std::int64_t>(arranged.joined->joined));
    if (preparation.sort)
        counts.emplace_back("moved", static_cast<std::int64_t>(arranged.moved));
    addCountsOfDisjoint(counts, arranged.cut);
    return counts;
}

/// Applies the corrections to a copy of `source`, in the only order that works:
/// the repeats joined, then the sort, then the ranges made disjoint — the groups
/// are runs of neighbours, so the order comes first. A project in order that
/// asks for nothing is not copied.
[[nodiscard]] std::expected<Arranged, Failure> arrangedFor(const core::Project& source,
                                                           const Preparation& preparation) {
    Arranged arranged;
    if (!preparation.any() ||
        (!preparation.join && !preparation.disjoint && preparation.sort && source.isInOrder())) {
        return arranged;
    }

    arranged.copy = source;
    if (preparation.join)
        arranged.joined = joinedInPlace(*arranged.copy);
    if (preparation.sort)
        arranged.moved = sortedInPlace(*arranged.copy);
    if (preparation.disjoint) {
        const std::expected<core::DisjointReport, Failure> made = disjointedInPlace(*arranged.copy);
        if (!made.has_value()) {
            return std::unexpected{made.error()};
        }
        arranged.cut = *made;
    }
    return arranged;
}

bool convertFile(core::FileSystem& files,
                 const Job& job,
                 const core::ReadingChoices& reading,
                 SubtitleFormat target,
                 const WriteShape& shape,
                 const Preparation& preparation,
                 bool dryRun,
                 const Reporter& reporter) {
    const std::string& path = job.input;
    const std::optional<core::OpenedFile> opened = openReporting(files, path, reading, reporter);
    if (!opened) {
        return false;
    }

    const core::SourceFile& source = opened->project.sourceFile();

    // Empty means "as the source had it": the model kept both so that a
    // conversion would not throw them away.
    const core::Newline newline = shape.newline.value_or(source.newline);
    // The source's unless another is asked for, exactly as the line ending and
    // the mark are — and the mark asked for is put on whichever of the two.
    const core::Encoding asked = shape.encoding.value_or(source.encoding);
    const core::Encoding encoding = shape.bom ? asked.withByteOrderMark(*shape.bom) : asked;

    // **`--bom` on an encoding that has none is refused, not ignored.** A byte
    // order mark exists for the Unicode encodings and for no other, so asking
    // for one on a Windows-1252 file asks for something that does not exist.
    // Writing the file without it would answer a question the user did ask.
    if (encoding.byteOrderMark() == core::ByteOrderMark::Present &&
        encoding.byteOrderMarkBytes().empty()) {
        reportFailure(
            reporter, path, Failure{"no-byte-order-mark", [&] {
                                        // TRANSLATORS: %1 is the name of an encoding.
                                        return core::substitute(
                                            core::translate("%1 has no byte order mark to write"),
                                            {encoding.charset()});
                                    }});
        return false;
    }

    // **A file written in frames needs a rate, and the source may not have one.**
    // What the document carries crosses only into its own format; converting
    // into MicroDVD from anywhere else has to take the rate from somewhere, and
    // choosing one silently would move every position in the file.
    const core::FileExtras crossed = core::extrasFor(source, target);
    core::FrameRate rate{core::MicroDvdFile{}.rate};
    if (const auto* frames = std::get_if<core::MicroDvdFile>(&crossed)) {
        rate = frames->rate;
    } else if (target == SubtitleFormat::MicroDvd) {
        // **The one thing this surface answers for itself**, because it can
        // refuse: the window always has a rate to offer, a batch may have none.
        const std::expected<core::FrameRate, Failure> settled =
            frameRateForFrames(opened->project, reading.frameRate, path, reporter);
        if (!settled.has_value()) {
            reportFailure(reporter, path, settled.error());
            return false;
        }
        rate = *settled;
    }

    // **The conversion happens once, in the core, and it measures itself.** The
    // markup is carried into the arriving vocabulary — ADR 0031 — the header and
    // the declared fields cross only into their own format, and the same walk
    // counts what that format will not be able to hold.
    // **Sorted and made disjoint on one copy, when asked**: the file read is left
    // as it was, and a file out of order is refused with the option that would
    // have put it right.
    std::expected<Arranged, Failure> arranged = arrangedFor(opened->project, preparation);
    if (!arranged.has_value()) {
        reportFailure(reporter, path, arranged.error());
        return false;
    }
    const core::ConvertedProject converted = core::convertProjectFor(
        arranged->copy.has_value() ? *arranged->copy : opened->project, target, rate);

    const core::WriteRequest request{
        .subtitles = converted.subtitles,
        .document = core::Document::Main,
        .newline = newline,
        .encoding = encoding,
        .header = converted.header,
        .extras = converted.extras,
    };
    const std::filesystem::path& out = job.output;
    const std::expected<std::size_t, Failure> written =
        writeSubtitlesTo(files, out, target, request, dryRun);
    if (!written) {
        reportFailure(reporter, path, written.error());
        return false;
    }

    reporter.say(
        3,
        core::substitute(dryRun ? core::translatePlural("%1: %2 bytes read, %3 would be written",
                                                        "%1: %2 bytes read, %3 would be written",
                                                        opened->bytes)
                                : core::translatePlural("%1: %2 bytes read, %3 written",
                                                        "%1: %2 bytes read, %3 written",
                                                        opened->bytes),
                         {path, std::to_string(opened->bytes), std::to_string(*written)}));
    sayDiagnostics(reporter, path, opened->diagnostics);
    if (arranged->joined.has_value())
        reporter.say(1, narrationOfJoin(path, *arranged->joined));
    if (preparation.sort)
        reporter.say(1, narrationOfSort(path, arranged->moved));
    if (arranged->cut.has_value())
        reporter.say(1, narrationOfDisjoint(path, *arranged->cut));
    reporter.say(
        2,
        // TRANSLATORS: %1 is a file, %2 and %3 formats, %4 an encoding, %5 a kind of line ending.
        core::substitute(
            core::translate("%1: %2 -> %3, %4, %5 line endings"),
            {path, nameOf(source.format), nameOf(target), nameOf(encoding), nameOf(newline)}));
    const std::size_t count = converted.subtitles.size();
    reporter.say(
        1,
        // TRANSLATORS: %1 is a file, %2 a number of subtitles, %3 a format, %4 the file written.
        dryRun ? core::substitute(core::translatePlural(
                                      "%1: %2 subtitle converted to %3 (dry run, nothing written)",
                                      "%1: %2 subtitles converted to %3 (dry run, nothing written)",
                                      count),
                                  {path, std::to_string(count), nameOf(target)})
               : core::substitute(core::translatePlural("%1: %2 subtitle written as %3 -> %4",
                                                        "%1: %2 subtitles written as %3 -> %4",
                                                        count),
                                  {path, std::to_string(count), nameOf(target), out.string()}));
    // **Said last, and only when there is something to say.** A conversion that
    // loses nothing is silent, which is what makes the line worth reading when
    // it does appear.
    if (const std::string notice = core::noticeOf(converted.loss, source.format, target);
        !notice.empty())
        reporter.say(1, core::substitute(core::translate("%1: %2"), {path, notice}));
    if (dryRun) {
        reporter.record(dryRunRecord(reporter.command(),
                                     path,
                                     countsOfConversion(converted, *arranged, preparation),
                                     warningsOf(opened->diagnostics)));
    } else {
        reporter.record(writtenRecord(reporter.command(),
                                      path,
                                      out,
                                      countsOfConversion(converted, *arranged, preparation),
                                      warningsOf(opened->diagnostics)));
    }
    return true;
}

/// The line ending a word names; the three are the closed set `--line-endings` accepts.
[[nodiscard]] core::Newline newlineNamed(const std::string& name) {
    if (name == "windows") {
        return core::Newline::CrLf;
    }
    return name == "mac" ? core::Newline::Cr : core::Newline::Lf;
}

} // namespace

std::expected<WriteShape, std::string>
writeShapeOf(const std::string& lineEndings, const std::string& encoding, bool bom, bool noBom) {
    if (bom && noBom) {
        return std::unexpected{std::string{
            core::translate("--bom and --no-bom ask for opposite things; give one or the other")}};
    }

    WriteShape shape;
    if (!lineEndings.empty()) {
        shape.newline = newlineNamed(lineEndings);
    }
    if (!encoding.empty()) {
        const std::expected<core::Encoding, std::string> named = encodingNamed(encoding);
        if (!named) {
            return std::unexpected(named.error());
        }
        shape.encoding = *named;
    }
    if (bom) {
        shape.bom = core::ByteOrderMark::Present;
    }
    if (noBom) {
        shape.bom = core::ByteOrderMark::Absent;
    }
    return shape;
}

std::optional<std::string>
refusalOfInPlaceRename(bool inPlace, const std::vector<std::string>& paths, SubtitleFormat target) {
    if (inPlace && wouldMisname(paths, target)) {
        return std::string{core::translate("--in-place cannot change the format: the file would "
                                           "keep a name its content no longer matches")};
    }
    return std::nullopt;
}

bool wouldMisname(const std::vector<std::string>& paths, SubtitleFormat target) {
    const std::string_view wanted = extensionOf(target);
    return std::ranges::any_of(paths, [wanted](const std::string& path) {
        std::string extension = std::filesystem::path{path}.extension().string();
        std::ranges::transform(extension, extension.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return extension != wanted;
    });
}

ExitCode convertAll(core::FileSystem& files,
                    const std::vector<std::string>& paths,
                    const core::ReadingChoices& reading,
                    SubtitleFormat target,
                    const WriteShape& shape,
                    const Destination& destination,
                    const Reporter& reporter,
                    bool sort,
                    bool disjoint,
                    bool join) {
    const std::expected<std::vector<Job>, ExitCode> jobs =
        arrange(files, destination, paths, extensionOf(target), reporter);
    if (!jobs) {
        return jobs.error();
    }

    std::size_t done = 0;
    for (const Job& job : *jobs) {
        if (convertFile(files,
                        job,
                        reading,
                        target,
                        shape,
                        Preparation{.join = join, .sort = sort, .disjoint = disjoint},
                        destination.isDryRun(),
                        reporter)) {
            ++done;
        }
    }
    return tally(reporter, Verb::Converted, done, paths.size());
}

} // namespace subedit::cli
