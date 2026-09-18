// What the operations of the edit layer cost on a full-length file: the eight
// of phase 2, then the search, the duration adjustment, the case and the paste
// of phase 10.
//
// Measured through a `Session`, which is how an application runs them: the
// command is built, applied, and pushed onto the history. Undo is measured
// apart, because the two are not symmetrical — a shift undoes by arithmetic, a
// transform by restoring what it kept. (The cases of phase 2 apply their
// command to a bare project, which is the same work without the history; those
// of phase 10 go through the session.)
//
// The document is generated rather than read from the corpus, for the reason
// the format benchmark already gives: a benchmark that depended on a file would
// measure the disk as much as the code. What it is made of — frozen positions,
// text that looks like a real subtitling — is `full_length_project.hpp`.
//
// Every operation here but one mutates its project, so each run needs a fresh
// one — and copying a project of four thousand subtitles costs more than any of
// the operations. `BENCHMARK_ADVANCED` is what keeps that copy out of the
// measured region: the copies are made first, the chronometer starts after.
//
// **That pattern has a floor, and it is the operation's own cost.** Catch2
// chooses how many runs make a measurable sample: the slower the operation, the
// fewer. Every whole-file operation below is slow enough to get one run, so one
// copy — a megabyte. An operation fast enough to be asked for thousands of runs
// would ask for gigabytes, and did: the single-subtitle edit below exhausted
// the machine's memory until it stopped copying at all.
//
// So: a whole-file operation copies, a single-subtitle one must not. Anything
// added here that is faster than a few microseconds belongs in the second
// group, whatever its shape.
//
// **A third case needs no rule: an operation that changes nothing.** A search
// that finds nothing reads the project and writes no byte of it, so every run
// can read the same one — no copy to make, and none to keep out of the measured
// region.

#include <subedit/core/command/command.hpp>
#include <subedit/core/config/search_options.hpp>
#include <subedit/core/edit/clipboard.hpp>
#include <subedit/core/edit/convert_frame_rate_command.hpp>
#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/edit/italics_command.hpp>
#include <subedit/core/edit/letter_case_command.hpp>
#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/edit/search.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/edit/snap_command.hpp>
#include <subedit/core/edit/sort_command.hpp>
#include <subedit/core/edit/transform_command.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/letter_case.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "full_length_project.hpp"

namespace {

using subedit::core::adjustDurations;
using subedit::core::ClipboardTexts;
using subedit::core::Command;
using subedit::core::ConvertFrameRateCommand;
using subedit::core::copyTexts;
using subedit::core::Document;
using subedit::core::Duration;
using subedit::core::DurationAdjustment;
using subedit::core::DurationConstraints;
using subedit::core::findNext;
using subedit::core::FrameRate;
using subedit::core::InsertCommand;
using subedit::core::kDefaultMaximumMilliseconds;
using subedit::core::kDefaultMinimumMilliseconds;
using subedit::core::kDefaultReadingSpeed;
using subedit::core::LetterCase;
using subedit::core::PastedTexts;
using subedit::core::pasteTexts;
using subedit::core::Project;
using subedit::core::ReadingSpeed;
using subedit::core::RemoveCommand;
using subedit::core::removeHearingImpaired;
using subedit::core::replaceAll;
using subedit::core::ReplacedAll;
using subedit::core::SearchOptions;
using subedit::core::SearchPattern;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::setItalics;
using subedit::core::setLetterCase;
using subedit::core::SetTextCommand;
using subedit::core::ShiftCommand;
using subedit::core::SnapCommand;
using subedit::core::SortCommand;
using subedit::core::StandardFrameRate;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::TextMatch;
using subedit::core::Timestamp;
using subedit::core::TransformCommand;
using subedit::core::TransformReference;
using subedit::core::wouldItalicise;

using subedit::test::fullLengthProject;
using subedit::test::kSubtitleCount;

/// The subtitles in reverse order, the worst case a sort can meet.
[[nodiscard]] Project reversedProject() {
    Project project = fullLengthProject();
    std::vector<Subtitle> subtitles{project.subtitles().begin(), project.subtitles().end()};
    std::ranges::reverse(subtitles);
    project.setSubtitles(std::move(subtitles));
    return project;
}

/// One session per run, each on a copy of `project` of its own.
///
/// Made before the chronometer starts, for the reason the header gives: the
/// copy is not what is measured.
[[nodiscard]] std::vector<Session> freshSessions(const Project& project, int runs) {
    std::vector<Session> sessions;
    sessions.reserve(static_cast<std::size_t>(runs));
    for (int run = 0; run < runs; ++run)
        sessions.emplace_back(project);
    return sessions;
}

/// Replaces every match of `pattern` in `session`, the way the window's
/// « Replace All » does: the pattern is compiled, the command is built from the
/// project, and it is applied through the session.
///
/// Returns how many matches were replaced. That is what a measurement hands
/// back to Catch2 — an answer that depends on the search, so that the search
/// cannot be optimised away — and what a case checks, outside the measured
/// region, to be sure the run did something.
[[nodiscard]] std::size_t replaceAllThrough(Session& session,
                                            const Selection& target,
                                            std::string_view pattern,
                                            SearchOptions options,
                                            std::string_view replacement) {
    const auto compiled = SearchPattern::compile(pattern, options);
    if (!compiled.has_value())
        return 0;

    ReplacedAll replaced = replaceAll(session.project(), target, *compiled, replacement);
    const std::size_t count = replaced.count;
    if (replaced.command)
        session.apply(std::move(replaced.command));
    return count;
}

/// The four constraints of the duration adjustment, all active.
///
/// Gaupol's own values for the three it has defaults for and a gap of a few
/// frames for the fourth — with the reading speed set to move an end both ways,
/// so that a text shorter than its time is brought in as well as a longer one
/// stretched.
[[nodiscard]] DurationConstraints fourConstraints() {
    constexpr std::int64_t kGapMilliseconds = 120;
    return DurationConstraints{.speed = ReadingSpeed::create(kDefaultReadingSpeed, true, true),
                               .minimum = Duration::fromMilliseconds(kDefaultMinimumMilliseconds),
                               .maximum = Duration::fromMilliseconds(kDefaultMaximumMilliseconds),
                               .gap = Duration::fromMilliseconds(kGapMilliseconds)};
}

} // namespace

TEST_CASE("shifting a full-length file", "[benchmark]") {
    const Project project = fullLengthProject();
    const Selection everything = Selection::all(project);

    BENCHMARK_ADVANCED("décalage de 4000 sous-titres")(Catch::Benchmark::Chronometer meter) {
        std::vector<Project> copies(static_cast<std::size_t>(meter.runs()), project);
        meter.measure([&](int run) {
            ShiftCommand command{everything, Duration::fromMilliseconds(2500)};
            command.apply(copies[static_cast<std::size_t>(run)]);
            return copies[static_cast<std::size_t>(run)].count();
        });
    };

    BENCHMARK_ADVANCED("décalage puis annulation")(Catch::Benchmark::Chronometer meter) {
        std::vector<Project> copies(static_cast<std::size_t>(meter.runs()), project);
        meter.measure([&](int run) {
            Project& copy = copies[static_cast<std::size_t>(run)];
            ShiftCommand command{everything, Duration::fromMilliseconds(2500)};
            command.apply(copy);
            command.revert(copy);
            return copy.count();
        });
    };
}

TEST_CASE("transforming a full-length file", "[benchmark]") {
    const Project project = fullLengthProject();
    const SubtitleIndex first = SubtitleIndex::fromValue(0);
    const SubtitleIndex last = SubtitleIndex::fromValue(kSubtitleCount - 1);

    BENCHMARK_ADVANCED("transformation de 4000 sous-titres")(Catch::Benchmark::Chronometer meter) {
        std::vector<Project> copies(static_cast<std::size_t>(meter.runs()), project);
        meter.measure([&](int run) {
            Project& copy = copies[static_cast<std::size_t>(run)];
            std::optional<TransformCommand> command = TransformCommand::create(
                copy,
                Selection::all(copy),
                TransformReference{.index = first, .target = Timestamp::fromMilliseconds(1000)},
                TransformReference{.index = last, .target = Timestamp::fromMilliseconds(10001000)});
            if (command.has_value())
                command->apply(copy);
            return copy.count();
        });
    };
}

TEST_CASE("converting the frame rate of a full-length file", "[benchmark]") {
    // The measurement issue #9 got wrong: it went through the frames, which is
    // the path ADR 0013 rejects. This one runs the command that exists.
    const Project project = fullLengthProject();
    const FrameRate from{StandardFrameRate::Fps25};
    const FrameRate to{StandardFrameRate::Fps23976};

    BENCHMARK_ADVANCED("conversion de fréquence sur 4000 sous-titres")
    (Catch::Benchmark::Chronometer meter) {
        std::vector<Project> copies(static_cast<std::size_t>(meter.runs()), project);
        meter.measure([&](int run) {
            Project& copy = copies[static_cast<std::size_t>(run)];
            ConvertFrameRateCommand command{copy, Selection::all(copy), from, to};
            command.apply(copy);
            return copy.count();
        });
    };
}

TEST_CASE("aligning a full-length file on a frame rate", "[benchmark]") {
    // Next to the conversion on purpose: the two are the pair D11 of the phase
    // 16 spec opposes, and reading their two figures side by side in the
    // journal is part of what keeps them distinct.
    const Project project = fullLengthProject();
    const FrameRate to{StandardFrameRate::Fps25};

    BENCHMARK_ADVANCED("alignement sur 4000 sous-titres")
    (Catch::Benchmark::Chronometer meter) {
        std::vector<Project> copies(static_cast<std::size_t>(meter.runs()), project);
        meter.measure([&](int run) {
            Project& copy = copies[static_cast<std::size_t>(run)];
            SnapCommand command{copy, Selection::all(copy), to};
            command.apply(copy);
            return copy.count();
        });
    };
}

TEST_CASE("sorting a full-length file", "[benchmark]") {
    const Project project = reversedProject();

    BENCHMARK_ADVANCED("tri de 4000 sous-titres à l'envers")(Catch::Benchmark::Chronometer meter) {
        std::vector<Project> copies(static_cast<std::size_t>(meter.runs()), project);
        meter.measure([&](int run) {
            Project& copy = copies[static_cast<std::size_t>(run)];
            SortCommand command;
            command.apply(copy);
            return copy.count();
        });
    };
}

TEST_CASE("inserting into and removing from a full-length file", "[benchmark]") {
    const Project project = fullLengthProject();

    std::vector<SubtitleIndex> everyOther;
    everyOther.reserve(kSubtitleCount / 2);
    for (std::size_t index = 0; index < kSubtitleCount; index += 2)
        everyOther.push_back(SubtitleIndex::fromValue(index));
    const Selection halfOfThem = Selection::of(everyOther);

    BENCHMARK_ADVANCED("suppression d'un sous-titre sur deux")
    (Catch::Benchmark::Chronometer meter) {
        std::vector<Project> copies(static_cast<std::size_t>(meter.runs()), project);
        meter.measure([&](int run) {
            Project& copy = copies[static_cast<std::size_t>(run)];
            RemoveCommand command{halfOfThem};
            command.apply(copy);
            return copy.count();
        });
    };

    // The undo of that same removal, and the other half of issue #45: putting
    // the subtitles back one at a time shifts the tail once per subtitle.
    BENCHMARK_ADVANCED("suppression puis annulation")
    (Catch::Benchmark::Chronometer meter) {
        std::vector<Project> copies(static_cast<std::size_t>(meter.runs()), project);
        meter.measure([&](int run) {
            Project& copy = copies[static_cast<std::size_t>(run)];
            RemoveCommand command{halfOfThem};
            command.apply(copy);
            command.revert(copy);
            return copy.count();
        });
    };

    BENCHMARK_ADVANCED("insertion de 100 sous-titres vides au milieu")
    (Catch::Benchmark::Chronometer meter) {
        std::vector<Project> copies(static_cast<std::size_t>(meter.runs()), project);
        meter.measure([&](int run) {
            Project& copy = copies[static_cast<std::size_t>(run)];
            InsertCommand command =
                InsertCommand::blank(copy, SubtitleIndex::fromValue(kSubtitleCount / 2), 100);
            command.apply(copy);
            return copy.count();
        });
    };
}

TEST_CASE("editing one subtitle of a full-length file", "[benchmark]") {
    // The operation an interface runs on every keystroke. Its cost must not
    // depend on the size of the file, and this is what says so.
    //
    // **One session, reused.** The operations above copy a project per run
    // because each mutates the whole of it; this one does not need to. It also
    // must not: it is fast enough that Catch2 asks for thousands of runs, and a
    // project copy each would be gigabytes — measured, on this very benchmark,
    // as an allocation failure under a four-gigabyte cap.
    //
    // Reusing the session is faithful rather than merely cheap: an interface
    // does exactly this, typing into a file it keeps open. The history grows to
    // its bound and then drops its oldest, which is the state a real session
    // spends its life in.
    const SubtitleIndex middle = SubtitleIndex::fromValue(kSubtitleCount / 2);

    BENCHMARK_ADVANCED("modification d'un texte, à travers une session")
    (Catch::Benchmark::Chronometer meter) {
        Session session{fullLengthProject()};

        meter.measure([&](int run) {
            session.apply(std::make_unique<SetTextCommand>(
                session.project(), middle, Document::Main, "Autre " + std::to_string(run)));
            return session.undoableCount();
        });
    };
}

TEST_CASE("removing hearing impaired mentions from a full-length file", "[benchmark]") {
    // The first measurement of text in the journal, and the reason the fixture
    // of #89 was written: one subtitle in five carries a mention, and 11.6 % of
    // them are taken away by the removal — the expensive case.
    //
    // Measured through the command, building included, because that is what a
    // user triggers. The scan alone would be steadier to measure and would
    // measure less.
    const Project project = fullLengthProject();

    BENCHMARK_ADVANCED("suppression des mentions sur 4000 sous-titres")
    (Catch::Benchmark::Chronometer meter) {
        std::vector<Project> copies(static_cast<std::size_t>(meter.runs()), project);
        meter.measure([&](int run) {
            Project& copy = copies[static_cast<std::size_t>(run)];
            std::unique_ptr<Command> command =
                removeHearingImpaired(copy, Selection::all(copy), Document::Main);
            if (command)
                command->apply(copy);
            return copy.count();
        });
    };
}

TEST_CASE("putting a full-length file in italics", "[benchmark]") {
    // The same shape as the removal above — one command per subtitle rewritten,
    // grouped — with one thing of its own: deciding which way to go reads every
    // text through the markup reader of the format. Both halves are measured,
    // because both are what the button costs.
    const Project project = fullLengthProject();

    BENCHMARK_ADVANCED("mise en italique de 4000 sous-titres")
    (Catch::Benchmark::Chronometer meter) {
        std::vector<Project> copies(static_cast<std::size_t>(meter.runs()), project);
        meter.measure([&](int run) {
            Project& copy = copies[static_cast<std::size_t>(run)];
            const Selection all = Selection::all(copy);
            std::unique_ptr<Command> command =
                setItalics(copy, all, Document::Main, wouldItalicise(copy, all, Document::Main));
            if (command)
                command->apply(copy);
            return copy.count();
        });
    };
}

TEST_CASE("replacing a frequent word in a full-length file", "[benchmark]") {
    // « Replace All » of a word that is everywhere: about a thousand matches
    // over four thousand subtitles, some subtitles carrying two. Plain text and
    // regular expression are measured apart, because they are two paths through
    // ICU — a literal and a pattern — for the same result.
    //
    // A whole-file operation that rewrites a good part of the file, so the first
    // rule of the header applies: a session and its copy per run, made before
    // the chronometer starts. Measured as the window runs it, which means the
    // pattern is compiled inside the measured region — it is compiled at every
    // press of the button — and the command is built and applied through the
    // session.
    const Project project = fullLengthProject();
    const Selection everything = Selection::all(project);
    constexpr std::string_view kWord = "rien";
    constexpr std::string_view kWordAsPattern = R"(\brien\b)";
    constexpr std::string_view kReplacement = "jamais";
    const SearchOptions plain{.regex = false, .ignoreCase = true};
    const SearchOptions regex{.regex = true, .ignoreCase = true};

    // What is measured has to be something, and the same thing both ways: a
    // search that finds nothing would say the operation is free.
    Session plainProbe{project};
    Session regexProbe{project};
    const std::size_t replacedInPlain =
        replaceAllThrough(plainProbe, everything, kWord, plain, kReplacement);
    const std::size_t replacedInRegex =
        replaceAllThrough(regexProbe, everything, kWordAsPattern, regex, kReplacement);
    REQUIRE(replacedInPlain > kSubtitleCount / 20);
    REQUIRE(replacedInRegex == replacedInPlain);
    REQUIRE(plainProbe.undoableCount() == 1);

    BENCHMARK_ADVANCED("remplacement d'un mot fréquent sur 4000 sous-titres, texte simple")
    (Catch::Benchmark::Chronometer meter) {
        std::vector<Session> sessions = freshSessions(project, meter.runs());
        meter.measure([&](int run) {
            Session& session = sessions[static_cast<std::size_t>(run)];
            return replaceAllThrough(session, everything, kWord, plain, kReplacement);
        });
    };

    BENCHMARK_ADVANCED("remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière")
    (Catch::Benchmark::Chronometer meter) {
        std::vector<Session> sessions = freshSessions(project, meter.runs());
        meter.measure([&](int run) {
            Session& session = sessions[static_cast<std::size_t>(run)];
            return replaceAllThrough(session, everything, kWordAsPattern, regex, kReplacement);
        });
    };
}

TEST_CASE("searching a full-length file for what it does not hold", "[benchmark]") {
    // « Find Next » that goes the whole way round and comes back with nothing:
    // the search that costs the most, since it never stops early. It is also
    // what the window does at every press of the button until the user gives up
    // or edits the pattern — and it starts from no previous match, since a
    // search that found nothing forgets where it was.
    //
    // **The third case of the header: nothing is written, so nothing is
    // copied.** The project is read, the same one at every run, and the
    // measurement is a plain `BENCHMARK`. The pattern is compiled inside the
    // measured region, as the window compiles it at every press.
    const Project project = fullLengthProject();
    const Selection everything = Selection::all(project);
    constexpr std::string_view kAbsent = "introuvable";
    const SearchOptions options{};

    // The measurement means « nothing found » only if that is what it finds.
    const auto probe = SearchPattern::compile(kAbsent, options);
    REQUIRE(probe.has_value());
    REQUIRE_FALSE(findNext(project, everything, *probe, std::nullopt).has_value());

    BENCHMARK("recherche sans résultat sur 4000 sous-titres") {
        const auto pattern = SearchPattern::compile(kAbsent, options);
        if (!pattern.has_value())
            return std::optional<TextMatch>{};
        return findNext(project, everything, *pattern, std::nullopt);
    };
}

TEST_CASE("adjusting the durations of a full-length file", "[benchmark]") {
    // The four constraints at once — reading speed both ways, minimum, maximum
    // and gap — which is the most the dialog can ask, and which ADR 0008 orders
    // for every subtitle. The fixture makes most of them bite: a short text is
    // brought up to the minimum, a long one asks for more time than the gap to
    // its neighbour leaves and has its speed sacrificed. The maximum is the
    // exception — no text of the fixture needs six seconds — but it is active,
    // and its step is taken all the same.
    //
    // A whole-file operation, so a session and its copy per run, made before
    // the chronometer starts; the adjustment is read from the project and
    // applied through the session.
    const Project project = fullLengthProject();
    const Selection everything = Selection::all(project);
    const DurationConstraints constraints = fourConstraints();

    const DurationAdjustment probe = adjustDurations(project, everything, constraints);
    REQUIRE(probe.command != nullptr);
    REQUIRE(probe.adjusted > kSubtitleCount / 2);
    REQUIRE(probe.sacrificed.speed > 0);

    BENCHMARK_ADVANCED("ajustement des durées de 4000 sous-titres, quatre contraintes")
    (Catch::Benchmark::Chronometer meter) {
        std::vector<Session> sessions = freshSessions(project, meter.runs());
        meter.measure([&](int run) {
            Session& session = sessions[static_cast<std::size_t>(run)];
            DurationAdjustment adjustment =
                adjustDurations(session.project(), everything, constraints);
            const std::size_t adjusted = adjustment.adjusted;
            if (adjustment.command)
                session.apply(std::move(adjustment.command));
            return adjusted;
        });
    };
}

TEST_CASE("putting a full-length file in title case", "[benchmark]") {
    // The one of the four cases that has to find where each word begins. Most
    // texts of the fixture change — seven in eight — so the command holds
    // nearly one entry per subtitle.
    //
    // A whole-file operation: a session and its copy per run, made before the
    // chronometer starts.
    const Project project = fullLengthProject();
    const Selection everything = Selection::all(project);

    REQUIRE(setLetterCase(project, everything, Document::Main, LetterCase::Title) != nullptr);

    BENCHMARK_ADVANCED("casse de titre sur 4000 sous-titres")(Catch::Benchmark::Chronometer meter) {
        std::vector<Session> sessions = freshSessions(project, meter.runs());
        meter.measure([&](int run) {
            Session& session = sessions[static_cast<std::size_t>(run)];
            std::unique_ptr<Command> command =
                setLetterCase(session.project(), everything, Document::Main, LetterCase::Title);
            if (command)
                session.apply(std::move(command));
            return session.undoableCount();
        });
    };
}

TEST_CASE("pasting four thousand texts into a full-length file", "[benchmark]") {
    // A paste that rewrites every text of the document. The texts are those of
    // the document itself, each moved up by one row: pasted as they were they
    // would change nothing, and a paste that changes nothing builds no command
    // at all — it would measure the comparison and not the paste.
    //
    // What is measured is the paste and its application. The copy that made the
    // clipboard is not: it happened before, when the user pressed Ctrl+C. The
    // clipboard is in the format of the document, as a copy made in this
    // program is, so no tag is translated on the way in.
    //
    // A whole-file operation: a session and its copy per run, made before the
    // chronometer starts.
    const Project project = fullLengthProject();
    const Selection everything = Selection::all(project);
    const SubtitleIndex first = SubtitleIndex::fromValue(0);

    ClipboardTexts clipboard = copyTexts(project, everything, Document::Main);
    std::ranges::rotate(clipboard.texts, clipboard.texts.begin() + 1);

    const PastedTexts probe = pasteTexts(project, clipboard, first, Document::Main);
    REQUIRE(clipboard.texts.size() == kSubtitleCount);
    REQUIRE(probe.command != nullptr);
    REQUIRE(probe.inserted == 0);

    BENCHMARK_ADVANCED("collage de 4000 textes")(Catch::Benchmark::Chronometer meter) {
        std::vector<Session> sessions = freshSessions(project, meter.runs());
        meter.measure([&](int run) {
            Session& session = sessions[static_cast<std::size_t>(run)];
            PastedTexts pasted = pasteTexts(session.project(), clipboard, first, Document::Main);
            const std::size_t inserted = pasted.inserted;
            if (pasted.command)
                session.apply(std::move(pasted.command));
            return inserted + session.undoableCount();
        });
    };
}
