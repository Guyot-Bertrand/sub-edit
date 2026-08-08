// What the eight operations of phase 2 cost on a full-length file.
//
// Measured through a `Session`, which is how an application runs them: the
// command is built, applied, and pushed onto the history. Undo is measured
// apart, because the two are not symmetrical — a shift undoes by arithmetic, a
// transform by restoring what it kept.
//
// The document is generated rather than read from the corpus, for the reason
// the format benchmark already gives: a benchmark that depended on a file would
// measure the disk as much as the code.
//
// Every operation here mutates its project, so each run needs a fresh one — and
// copying a project of four thousand subtitles costs more than any of the
// operations. `BENCHMARK_ADVANCED` is what keeps that copy out of the measured
// region: the copies are made first, the chronometer starts after.

#include <subedit/core/edit/convert_frame_rate_command.hpp>
#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/edit/sort_command.hpp>
#include <subedit/core/edit/transform_command.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
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
#include <utility>
#include <vector>

namespace {

using subedit::core::ConvertFrameRateCommand;
using subedit::core::Document;
using subedit::core::Duration;
using subedit::core::FrameRate;
using subedit::core::InsertCommand;
using subedit::core::Project;
using subedit::core::RemoveCommand;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::SetTextCommand;
using subedit::core::ShiftCommand;
using subedit::core::SortCommand;
using subedit::core::StandardFrameRate;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;
using subedit::core::TransformCommand;
using subedit::core::TransformReference;

/// Roughly a feature film: two subtitles every five seconds for three hours.
/// The same shape the format benchmark uses, so the figures compare.
constexpr std::size_t kSubtitleCount = 4000;
constexpr std::int64_t kStepMilliseconds = 2500;
constexpr std::int64_t kDurationMilliseconds = 2000;

[[nodiscard]] Project fullLengthProject() {
    std::vector<Subtitle> subtitles;
    subtitles.reserve(kSubtitleCount);
    for (std::size_t index = 0; index < kSubtitleCount; ++index) {
        const std::int64_t start = static_cast<std::int64_t>(index) * kStepMilliseconds;
        subtitles.push_back(
            Subtitle{.start = Timestamp::fromMilliseconds(start),
                     .end = Timestamp::fromMilliseconds(start + kDurationMilliseconds),
                     .mainText = "Une réplique de longueur ordinaire,\nsur deux lignes."});
    }

    Project project;
    project.setSubtitles(std::move(subtitles));
    project.setFrameRate(FrameRate{StandardFrameRate::Fps25});
    return project;
}

/// The subtitles in reverse order, the worst case a sort can meet.
[[nodiscard]] Project reversedProject() {
    Project project = fullLengthProject();
    std::vector<Subtitle> subtitles{project.subtitles().begin(), project.subtitles().end()};
    std::ranges::reverse(subtitles);
    project.setSubtitles(std::move(subtitles));
    return project;
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
    const Project project = fullLengthProject();
    const SubtitleIndex middle = SubtitleIndex::fromValue(kSubtitleCount / 2);

    BENCHMARK_ADVANCED("modification d'un texte, à travers une session")
    (Catch::Benchmark::Chronometer meter) {
        std::vector<Session> sessions;
        sessions.reserve(static_cast<std::size_t>(meter.runs()));
        for (int run = 0; run < meter.runs(); ++run)
            sessions.emplace_back(project);

        meter.measure([&](int run) {
            Session& session = sessions[static_cast<std::size_t>(run)];
            session.apply(std::make_unique<SetTextCommand>(
                session.project(), middle, Document::Main, "Autre."));
            return session.undoableCount();
        });
    };
}
