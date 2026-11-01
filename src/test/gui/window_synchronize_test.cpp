// `Synchronize with Audio…`, from the window: the dialog, the background run, the preview of
// where each subtitle would go, and the one undoable operation that moves them (issue #814).
//
// The runner is a seam, like `Prompts`: the real one needs an engine and a model of several
// hundred megabytes.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/engine_capabilities.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/synchronization_preview_dialog.hpp>
#include <subedit/gui/transcribe_dialog.hpp>
#include <subedit/gui/transcription_runner.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableView>
#include <QTableWidget>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "fake_prompts.hpp"
#include "player_harness.hpp"
#include "scratch_directory.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::AudioTrack;
using subedit::core::InMemoryFileSystem;
using subedit::core::LineMatch;
using subedit::core::ModelState;
using subedit::core::Timestamp;
using subedit::core::TranscriptionSettings;
using subedit::core::TranscriptionTools;
using subedit::gui::AlignmentJob;
using subedit::gui::AlignmentOutcome;
using subedit::gui::BackgroundTranscriptionRunner;
using subedit::gui::MainWindow;
using subedit::gui::SynchronizationPreviewDialog;
using subedit::gui::TranscribeDialog;
using subedit::gui::TranscriptionFailure;
using subedit::gui::TranscriptionRunner;
using subedit::test::FakePrompts;
using subedit::test::fileIn;
using subedit::test::projecting;
using subedit::test::Projectionist;
using subedit::test::ScratchDirectory;

constexpr const char* kThree = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                               "2\n00:00:02,500 --> 00:00:03,500\nDeux.\n\n"
                               "3\n00:00:05,000 --> 00:00:06,000\nTrois.\n\n";

const std::vector<AudioTrack> kTracks{
    {.id = 1, .language = "fra", .title = "Original", .selected = true}};

const TranscriptionTools kReady{
    .engine = std::filesystem::path{"/usr/bin/whisper-cli"},
    .extractor = true,
    .model = ModelState::Multilingual,
    .voiceTool = std::filesystem::path{"/usr/bin/whisper-vad-speech-segments"},
    .voiceModel = true,
    .capabilities = subedit::core::EngineCapabilities{
        .languageDetection = true, .languages = {"en", "fr"}, .suppressNotes = true}};

Timestamp at(std::int64_t milliseconds) {
    return Timestamp::fromMilliseconds(milliseconds);
}

/// A subtitle heard at `start`, all of its words.
LineMatch heard(std::int64_t start) {
    return {.place = {.start = at(start), .end = at(start + 800)}, .words = 1, .matched = 1};
}

/// A runner that answers what the case says, and keeps what it was asked.
class ScriptedRunner final : public TranscriptionRunner {
public:
    std::expected<AlignmentOutcome, TranscriptionFailure> answer =
        AlignmentOutcome{.matches = {heard(1500), heard(3000), heard(5500)}};
    std::vector<AlignmentJob> asked{};
    TranscriptionTools there = kReady;

    [[nodiscard]] std::expected<subedit::gui::TranscriptionOutcome, TranscriptionFailure>
    run(const subedit::gui::TranscriptionJob& /*job*/, QWidget* /*parent*/) override {
        return std::unexpected(TranscriptionFailure{.message = "not asked"});
    }

    [[nodiscard]] std::expected<AlignmentOutcome, TranscriptionFailure>
    align(const AlignmentJob& job, QWidget* /*parent*/) override {
        asked.push_back(job);
        return answer;
    }

    [[nodiscard]] TranscriptionTools tools(const TranscriptionSettings& /*settings*/) override {
        return there;
    }
};

[[nodiscard]] InMemoryFileSystem directoryHolding() {
    InMemoryFileSystem files;
    files.addFile("/films/film.mkv", "");
    files.addFile("/films/film.fr.srt", kThree);
    return files;
}

/// The starts of the project, in milliseconds.
[[nodiscard]] std::vector<std::int64_t> startsOf(const MainWindow& window) {
    std::vector<std::int64_t> starts;
    const QAbstractItemModel* model = window.table()->model();
    for (int row = 0; row < model->rowCount(); ++row) {
        const QString text = model->data(model->index(row, 1), Qt::DisplayRole).toString();
        starts.push_back(Timestamp::parse(text.toStdString()).value_or(at(-1)).milliseconds());
    }
    return starts;
}

/// A window with a film and three subtitles, the runner being scripted.
struct Stage {
    InMemoryFileSystem files = directoryHolding();
    FakePrompts prompts;
    Projectionist booth;
    ScriptedRunner* runner = nullptr;
    std::unique_ptr<MainWindow> window;

    Stage() {
        booth.tracks = kTracks;
        window = std::make_unique<MainWindow>(
            files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth));
        auto scripted = std::make_unique<ScriptedRunner>();
        runner = scripted.get();
        window->setTranscriptionRunner(std::move(scripted));
        window->show();
        REQUIRE(booth.player != nullptr);
    }
};

} // namespace

TEST_CASE("the command opens a dialog with the track, the language and the three "
          "choices of an alignment",
          "[gui][GUI-SYNC-01]") {
    Stage stage;
    stage.prompts.nextRun = false;
    bool seen = false;
    stage.prompts.fill = [&seen](QDialog& dialog) {
        const auto* sync = dynamic_cast<TranscribeDialog*>(&dialog);
        REQUIRE(sync != nullptr);
        seen = true;
        CHECK(sync->windowTitle() == "Synchronize with audio");
        CHECK(sync->trackBox()->count() == 1);
        CHECK(sync->minimumMatchBox()->value() == 50);
        CHECK(sync->doubtGapBox()->value() == 2000);
        // Nothing is selected: "only the selected" is offered and out.
        CHECK_FALSE(sync->selectionBox()->isEnabled());
    };

    stage.window->synchronizeWithAudioAction()->trigger();

    CHECK(seen);
    CHECK(stage.runner->asked.empty());
}

TEST_CASE("the choices of the dialog go to the runner and are kept for the next time",
          "[gui][GUI-SYNC-01]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.prompts.fill = [](QDialog& dialog) {
        if (auto* sync = dynamic_cast<TranscribeDialog*>(&dialog)) {
            sync->minimumMatchBox()->setValue(80);
            sync->doubtGapBox()->setValue(1500);
        }
    };

    stage.window->synchronizeWithAudioAction()->trigger();

    REQUIRE(stage.runner->asked.size() == 1);
    const AlignmentJob& job = stage.runner->asked.front();
    CHECK(job.lines == std::vector<std::string>{"Un.", "Deux.", "Trois."});
    CHECK(job.alignment.minimumMatchPercent == 80);
    CHECK(job.alignment.doubtGapMilliseconds == 1500);
    CHECK_FALSE(job.start.has_value());

    // Asked again, the dialog starts from what was chosen.
    int shown = 0;
    stage.prompts.fill = [&shown](QDialog& dialog) {
        if (auto* sync = dynamic_cast<TranscribeDialog*>(&dialog)) {
            shown = sync->minimumMatchBox()->value();
        }
    };
    stage.prompts.nextRun = false;
    stage.window->synchronizeWithAudioAction()->trigger();
    CHECK(shown == 80);
}

TEST_CASE("only the selected subtitles are looked for, in the part of the film around them",
          "[gui][GUI-SYNC-01]") {
    Stage stage;
    stage.window->table()->selectRow(1);
    stage.prompts.nextRun = true;
    stage.prompts.fill = [](QDialog& dialog) {
        if (auto* sync = dynamic_cast<TranscribeDialog*>(&dialog)) {
            CHECK(sync->selectionBox()->isEnabled());
            sync->selectionBox()->setChecked(true);
        }
    };
    stage.runner->answer = AlignmentOutcome{.matches = {heard(3000)}};

    stage.window->synchronizeWithAudioAction()->trigger();

    REQUIRE(stage.runner->asked.size() == 1);
    const AlignmentJob& job = stage.runner->asked.front();
    CHECK(job.lines == std::vector<std::string>{"Deux."});
    // The subtitle is at 2.5 s to 3.5 s: a minute on each side, not before the beginning.
    CHECK(job.start == at(0));
    CHECK(job.end == at(63500));
}

TEST_CASE("a failure leaves the project as it was and says why; a cancellation is not a failure",
          "[gui][GUI-SYNC-02]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.runner->answer = std::unexpected(TranscriptionFailure{.message = "the model is gone"});

    stage.window->synchronizeWithAudioAction()->trigger();

    CHECK(stage.prompts.failures == std::vector<std::string>{"the model is gone"});
    CHECK(startsOf(*stage.window) == std::vector<std::int64_t>{1000, 2500, 5000});

    stage.prompts.failures.clear();
    stage.runner->answer = std::unexpected(TranscriptionFailure{.cancelled = true});
    stage.window->synchronizeWithAudioAction()->trigger();
    CHECK(stage.prompts.failures.empty());
    CHECK(startsOf(*stage.window) == std::vector<std::int64_t>{1000, 2500, 5000});
}

TEST_CASE("a run that finds none of the subtitles is reported and shows nothing to apply",
          "[gui][GUI-SYNC-02]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.runner->answer = AlignmentOutcome{
        .matches = {LineMatch{.words = 1}, LineMatch{.words = 1}, LineMatch{.words = 1}}};

    stage.window->synchronizeWithAudioAction()->trigger();

    REQUIRE(stage.prompts.failures.size() == 1);
    CHECK_THAT(stage.prompts.failures.front(), ContainsSubstring("was found in the audio"));
    CHECK(startsOf(*stage.window) == std::vector<std::int64_t>{1000, 2500, 5000});
}

TEST_CASE("the result is shown before it moves anything, and applied as one undoable operation",
          "[gui][GUI-SYNC-03]") {
    Stage stage;
    stage.prompts.nextRun = true;
    bool previewed = false;
    stage.prompts.fill = [&](QDialog& dialog) {
        if (auto* preview = dynamic_cast<SynchronizationPreviewDialog*>(&dialog)) {
            previewed = true;
            // Nothing has moved yet.
            CHECK(startsOf(*stage.window) == std::vector<std::int64_t>{1000, 2500, 5000});
            CHECK(preview->table()->rowCount() == 3);
            CHECK(preview->table()->item(0, 2)->text() == "00:00:01.000");
            CHECK(preview->table()->item(0, 3)->text() == "00:00:01.500");
            CHECK_THAT(preview->summary()->text().toStdString(),
                       ContainsSubstring("3 of 3 subtitles placed"));
        }
    };

    stage.window->synchronizeWithAudioAction()->trigger();

    CHECK(previewed);
    CHECK(startsOf(*stage.window) == std::vector<std::int64_t>{1500, 3000, 5500});

    // One entry of the history, named after what it was.
    stage.window->undoAction()->trigger();
    CHECK(startsOf(*stage.window) == std::vector<std::int64_t>{1000, 2500, 5000});
    CHECK_FALSE(stage.window->undoAction()->isEnabled());
}

TEST_CASE("ignoring the result changes nothing", "[gui][GUI-SYNC-03]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.prompts.fill = [&stage](QDialog& dialog) {
        if (dynamic_cast<SynchronizationPreviewDialog*>(&dialog) != nullptr)
            stage.prompts.nextRun = false;
    };

    stage.window->synchronizeWithAudioAction()->trigger();

    REQUIRE(stage.runner->asked.size() == 1);
    CHECK(startsOf(*stage.window) == std::vector<std::int64_t>{1000, 2500, 5000});
    CHECK_FALSE(stage.window->undoAction()->isEnabled());
}

TEST_CASE("the subtitles to check are marked, and can be shown alone", "[gui][GUI-SYNC-03]") {
    Stage stage;
    stage.prompts.nextRun = true;
    // The second is not found; the third is only partly heard.
    stage.runner->answer = AlignmentOutcome{
        .matches = {
            heard(1500),
            LineMatch{.words = 3, .matched = 0},
            LineMatch{.place = {.start = at(5500), .end = at(6300)}, .words = 4, .matched = 2}}};
    bool checked = false;
    stage.prompts.fill = [&checked](QDialog& dialog) {
        auto* preview = dynamic_cast<SynchronizationPreviewDialog*>(&dialog);
        if (preview == nullptr)
            return;
        checked = true;
        CHECK(preview->table()->item(0, 5)->text().isEmpty());
        CHECK_THAT(preview->table()->item(1, 5)->text().toStdString(),
                   ContainsSubstring("not found in the audio"));
        CHECK_THAT(preview->table()->item(2, 5)->text().toStdString(),
                   ContainsSubstring("only partly heard"));
        // A subtitle not found keeps its time, which is shown unchanged and with no shift.
        CHECK(preview->table()->item(1, 3)->text() == "00:00:02.500");
        CHECK(preview->table()->item(1, 4)->text().isEmpty());

        preview->onlyToCheckBox()->setChecked(true);
        CHECK(preview->table()->isRowHidden(0));
        CHECK_FALSE(preview->table()->isRowHidden(1));
        CHECK_FALSE(preview->table()->isRowHidden(2));
        preview->onlyToCheckBox()->setChecked(false);
        CHECK_FALSE(preview->table()->isRowHidden(0));
    };

    stage.window->synchronizeWithAudioAction()->trigger();

    CHECK(checked);
    // The one not found is where it was.
    CHECK(startsOf(*stage.window) == std::vector<std::int64_t>{1500, 2500, 5500});
}

TEST_CASE("a result in which nothing would move cannot be applied", "[gui][GUI-SYNC-03]") {
    Stage stage;
    stage.prompts.nextRun = true;
    const auto exactly = [](std::int64_t start) {
        return LineMatch{
            .place = {.start = at(start), .end = at(start + 1000)}, .words = 1, .matched = 1};
    };
    stage.runner->answer =
        AlignmentOutcome{.matches = {exactly(1000), exactly(2500), exactly(5000)}};
    bool refused = false;
    stage.prompts.fill = [&refused](QDialog& dialog) {
        const auto* preview = dynamic_cast<SynchronizationPreviewDialog*>(&dialog);
        if (preview == nullptr)
            return;
        for (QAbstractButton* button : preview->buttons()->buttons())
            if (preview->buttons()->buttonRole(button) == QDialogButtonBox::AcceptRole)
                refused = !button->isEnabled();
    };

    stage.window->synchronizeWithAudioAction()->trigger();

    CHECK(refused);
    CHECK_FALSE(stage.window->undoAction()->isEnabled());
}

TEST_CASE("without the program or the model the dialog says what is missing and cannot start",
          "[gui][GUI-SYNC-04]") {
    Stage stage;
    stage.runner->there = TranscriptionTools{.engine = std::nullopt, .extractor = true};
    bool seen = false;
    stage.prompts.fill = [&seen](QDialog& dialog) {
        const auto* sync = dynamic_cast<TranscribeDialog*>(&dialog);
        REQUIRE(sync != nullptr);
        seen = true;
        sync->methodBox()->setCurrentIndex(
            sync->methodBox()->findData(static_cast<int>(subedit::core::AlignmentMethod::Words)));
        CHECK_THAT(sync->toolsLabel()->text().toStdString(),
                   ContainsSubstring("Set them in the preferences."));
        CHECK_FALSE(sync->buttons()->button(QDialogButtonBox::Ok)->isEnabled());
    };

    stage.window->synchronizeWithAudioAction()->trigger();

    CHECK(seen);
    CHECK(stage.runner->asked.empty());
}

TEST_CASE("the action is out until a film is open", "[gui][GUI-SYNC-01]") {
    InMemoryFileSystem files;
    files.addFile("/films/film.fr.srt", kThree);
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();

    CHECK_FALSE(window.synchronizeWithAudioAction()->isEnabled());
}

TEST_CASE("the Preferences button of the dialog opens the preferences", "[gui][GUI-SYNC-04]") {
    Stage stage;
    stage.prompts.nextRun = false;
    stage.prompts.fill = [](QDialog& dialog) {
        if (auto* sync = dynamic_cast<TranscribeDialog*>(&dialog))
            sync->preferencesButton()->click();
    };
    // The preferences dialog is a dialog too: it is refused by the fake, as everything.
    stage.window->synchronizeWithAudioAction()->trigger();

    CHECK(stage.prompts.runAsked >= 2);
}

TEST_CASE("a range that holds no subtitle asks nothing of the runner", "[gui][GUI-SYNC-02]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.prompts.fill = [](QDialog& dialog) {
        if (auto* sync = dynamic_cast<TranscribeDialog*>(&dialog))
            sync->startEdit()->setText("01:00:00");
    };

    stage.window->synchronizeWithAudioAction()->trigger();

    CHECK(stage.runner->asked.empty());
}

TEST_CASE("a subtitle cut back, and one left overlapping, say so in the result",
          "[gui][GUI-SYNC-03]") {
    Stage stage;
    stage.prompts.nextRun = true;
    // The first is heard from 2.4 s to 3.0 s, and the second (not found) is at 2.5 s: cut back.
    // The third is heard at 0.5 s, before the second's 2.5 s which it does not reach: left.
    stage.runner->answer = AlignmentOutcome{
        .matches = {
            LineMatch{.place = {.start = at(2400), .end = at(3000)}, .words = 1, .matched = 1},
            LineMatch{.words = 2, .matched = 0},
            LineMatch{.place = {.start = at(500), .end = at(900)}, .words = 1, .matched = 1}}};
    bool checked = false;
    stage.prompts.fill = [&checked](QDialog& dialog) {
        const auto* preview = dynamic_cast<SynchronizationPreviewDialog*>(&dialog);
        if (preview == nullptr)
            return;
        checked = true;
        CHECK_THAT(preview->table()->item(0, 5)->text().toStdString(),
                   ContainsSubstring("cut back to the start of the next subtitle"));
    };

    stage.window->synchronizeWithAudioAction()->trigger();

    CHECK(checked);
}

TEST_CASE("a subtitle that still overlaps the next one is said", "[gui][GUI-SYNC-03]") {
    Stage stage;
    stage.prompts.nextRun = true;
    // The first moves to 6 s, past the second (not found, at 2.5 s), which cutting cannot cure.
    stage.runner->answer = AlignmentOutcome{
        .matches = {
            LineMatch{.place = {.start = at(6000), .end = at(6800)}, .words = 1, .matched = 1},
            LineMatch{.words = 2, .matched = 0},
            LineMatch{.words = 2, .matched = 0}}};
    bool checked = false;
    stage.prompts.fill = [&checked](QDialog& dialog) {
        const auto* preview = dynamic_cast<SynchronizationPreviewDialog*>(&dialog);
        if (preview == nullptr)
            return;
        checked = true;
        CHECK_THAT(preview->table()->item(0, 5)->text().toStdString(),
                   ContainsSubstring("still overlaps the next subtitle"));
    };

    stage.window->synchronizeWithAudioAction()->trigger();

    CHECK(checked);
}

// ## The real runner, against stand-ins for the engine and ffmpeg

namespace {

struct Tools {
    ScratchDirectory scratch;
    std::string heard =
        std::string{SUBEDIT_TEST_DATA_DIR} + "/alignment/offset-plus-2400/heard.json";
    std::filesystem::path film = scratch.write("film.mkv", "film");
    std::filesystem::path model =
        scratch.write("model.bin", std::string{"lmgg\x99\xca\x00\x00", 8} + std::string(8, '\0'));

    void everything() const {
        (void)scratch.standIn("whisper-cli",
                              std::filesystem::path{SUBEDIT_FAKE_TRANSCRIBER_BINARY},
                              "--progress=2 --output-from='" + heard + "'");
        const auto audio = scratch.write("audio.wav", "pcm");
        (void)scratch.standIn("ffmpeg",
                              std::filesystem::path{SUBEDIT_FAKE_TRANSCRIBER_BINARY},
                              "--output-from='" + audio.string() + "'");
    }

    [[nodiscard]] AlignmentJob job() const {
        return AlignmentJob{
            .video = film,
            .track = 0,
            .language = "en",
            .model = model,
            .method = subedit::core::AlignmentMethod::Words,
            .lines = {"The harbour was quiet at dawn.", "Nobody had lit the lamps yet."},
            .times = {}};
    }
};

} // namespace

TEST_CASE("the real runner synchronizes through the tools it finds", "[gui][GUI-SYNC-02]") {
    const Tools tools;
    tools.everything();
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};
    AlignmentJob job = tools.job();
    job.settings.vadModel = tools.scratch.write("vad.bin", "model");
    job.start = at(0);
    job.end = at(60000);

    const auto outcome = runner.align(job, nullptr);

    REQUIRE(outcome.has_value());
    REQUIRE(outcome->matches.size() == 2);
    CHECK(outcome->matches.front().place.start == at(1000));
}

TEST_CASE("the real runner says an engine nobody registered, and what it could not do",
          "[gui][GUI-SYNC-02]") {
    const Tools tools;
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};

    AlignmentJob unknown = tools.job();
    unknown.settings.engine = "nobody";
    const auto refused = runner.align(unknown, nullptr);
    REQUIRE_FALSE(refused.has_value());
    CHECK_THAT(refused.error().message, ContainsSubstring("nobody"));

    // No tool on the search path: the engine is missing, and the box says so.
    const auto missing = runner.align(tools.job(), nullptr);
    REQUIRE_FALSE(missing.has_value());
    CHECK_FALSE(missing.error().cancelled);
}

TEST_CASE("a runner that cannot synchronize says so", "[gui][GUI-SYNC-02]") {
    class Silent final : public TranscriptionRunner {
    public:
        [[nodiscard]] std::expected<subedit::gui::TranscriptionOutcome, TranscriptionFailure>
        run(const subedit::gui::TranscriptionJob& /*job*/, QWidget* /*parent*/) override {
            return subedit::gui::TranscriptionOutcome{};
        }

        [[nodiscard]] TranscriptionTools tools(const TranscriptionSettings& /*settings*/) override {
            return {};
        }
    };

    Silent runner;

    const auto outcome = runner.align(AlignmentJob{}, nullptr);

    REQUIRE_FALSE(outcome.has_value());
    CHECK_FALSE(outcome.error().message.empty());
}

TEST_CASE("a project with nothing in it has nothing to synchronize", "[gui][GUI-SYNC-02]") {
    InMemoryFileSystem files = directoryHolding();
    FakePrompts prompts;
    Projectionist booth;
    booth.tracks = kTracks;
    subedit::core::OpenedFile opened = fileIn(files, "/films/film.fr.srt");
    opened.project.setSubtitles({});
    MainWindow window{files, std::move(opened), prompts, projecting(booth)};
    auto scripted = std::make_unique<ScriptedRunner>();
    const ScriptedRunner* runner = scripted.get();
    window.setTranscriptionRunner(std::move(scripted));
    window.show();
    window.selectVideoAction()->trigger();
    prompts.nextRun = true;

    window.synchronizeWithAudioAction()->trigger();

    CHECK(prompts.runAsked == 0);
    CHECK(runner->asked.empty());
}

TEST_CASE("the dialog offers the speech first, says what it does not do, and keeps the choice",
          "[gui][GUI-SYNC-05]") {
    Stage stage;
    stage.prompts.nextRun = true;
    bool seen = false;
    stage.prompts.fill = [&seen](QDialog& dialog) {
        auto* sync = dynamic_cast<TranscribeDialog*>(&dialog);
        if (sync == nullptr)
            return;
        seen = true;
        // The speech by default: it needs no language, and says what it does not do.
        CHECK(sync->choice().method == subedit::core::AlignmentMethod::Speech);
        CHECK_THAT(sync->methodHint()->text().toStdString(),
                   ContainsSubstring("Only the file as a whole was put in line"));
        CHECK_FALSE(sync->minimumMatchBox()->isEnabled());
        CHECK_FALSE(sync->languageBox()->isEnabled());
        CHECK(sync->buttons()->button(QDialogButtonBox::Ok)->isEnabled());

        // The words need the language, and carry no such warning.
        sync->methodBox()->setCurrentIndex(
            sync->methodBox()->findData(static_cast<int>(subedit::core::AlignmentMethod::Words)));
        CHECK(sync->methodHint()->text().isEmpty());
        CHECK(sync->minimumMatchBox()->isEnabled());
    };

    stage.window->synchronizeWithAudioAction()->trigger();

    REQUIRE(seen);
    REQUIRE(stage.runner->asked.size() == 1);
    const AlignmentJob& job = stage.runner->asked.front();
    CHECK(job.method == subedit::core::AlignmentMethod::Words);
    CHECK(job.times.size() == 3);
    CHECK(job.times.front().start == at(1000));
}

TEST_CASE("the preview of a result on the speech repeats what it does not do",
          "[gui][GUI-SYNC-05]") {
    Stage stage;
    stage.prompts.nextRun = true;
    std::vector<std::string> summaries;
    stage.prompts.fill = [&summaries](QDialog& dialog) {
        if (auto* preview = dynamic_cast<SynchronizationPreviewDialog*>(&dialog))
            summaries.push_back(preview->summary()->text().toStdString());
    };
    stage.runner->answer = AlignmentOutcome{
        .matches = {heard(1500), heard(3000), heard(5500)}, .fitted = true, .ratio = 25.0 / 23.976};

    stage.window->synchronizeWithAudioAction()->trigger();

    REQUIRE(summaries.size() == 1);
    CHECK_THAT(summaries.front(), ContainsSubstring("Only the file as a whole was put in line"));
    CHECK_THAT(summaries.front(), ContainsSubstring("The frame rate of the file was put right"));
}

TEST_CASE("a file that does not fit the sound is refused with the reason, and nothing moves",
          "[gui][GUI-SYNC-05]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.runner->answer =
        AlignmentOutcome{.matches = {LineMatch{}, LineMatch{}, LineMatch{}}, .fitted = false};

    stage.window->synchronizeWithAudioAction()->trigger();

    REQUIRE(stage.prompts.failures.size() == 1);
    CHECK_THAT(stage.prompts.failures.front(), ContainsSubstring("too little in common"));
    CHECK(startsOf(*stage.window) == std::vector<std::int64_t>{1000, 2500, 5000});
}

TEST_CASE("without the program or the model of the speech the dialog says so and cannot start",
          "[gui][GUI-SYNC-05]") {
    Stage stage;
    stage.runner->there = TranscriptionTools{.extractor = true};
    bool seen = false;
    stage.prompts.fill = [&seen](QDialog& dialog) {
        const auto* sync = dynamic_cast<TranscribeDialog*>(&dialog);
        REQUIRE(sync != nullptr);
        seen = true;
        CHECK_THAT(sync->toolsLabel()->text().toStdString(),
                   ContainsSubstring("whisper-vad-speech-segments"));
        CHECK_FALSE(sync->buttons()->button(QDialogButtonBox::Ok)->isEnabled());
    };

    stage.prompts.nextRun = false;
    stage.window->synchronizeWithAudioAction()->trigger();

    CHECK(seen);
    CHECK(stage.runner->asked.empty());
}

TEST_CASE("the real runner lays the file on the speech it finds, without transcribing",
          "[gui][GUI-SYNC-05]") {
    const Tools tools;
    tools.everything();
    // A voice detector that finds a stretch every four seconds, and a file three seconds late.
    std::string detected;
    AlignmentJob job = tools.job();
    job.method = subedit::core::AlignmentMethod::Speech;
    job.lines.clear();
    constexpr int kLines = 40;
    for (int line = 0; line < kLines; ++line) {
        const std::int64_t start = 5000 + (static_cast<std::int64_t>(line) * 4000);
        detected += "Speech segment " + std::to_string(line) +
                    ": start = " + std::to_string(start / 10) +
                    ".00, end = " + std::to_string((start + 2000) / 10) + ".00\n";
        job.lines.emplace_back("text");
        job.times.push_back({.start = at(start + 3000), .end = at(start + 5000)});
    }
    const auto listing = tools.scratch.write("segments.txt", detected);
    (void)tools.scratch.write("whisper-vad-speech-segments",
                              "#!/bin/sh\ncat '" + listing.string() + "'\n");
    std::filesystem::permissions(tools.scratch.file("whisper-vad-speech-segments"),
                                 std::filesystem::perms::owner_all,
                                 std::filesystem::perm_options::add);
    job.settings.vadModel = tools.scratch.write("vad.bin", "model");
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};

    const auto outcome = runner.align(job, nullptr);

    REQUIRE(outcome.has_value());
    CHECK(outcome->fitted);
    CHECK(outcome->ratio == 1.0);
    REQUIRE(outcome->matches.size() == static_cast<std::size_t>(kLines));
    CHECK(outcome->matches.front().place.start == at(5000));
}

TEST_CASE("the real runner says what the speech lacks", "[gui][GUI-SYNC-05]") {
    const Tools tools;
    tools.everything();
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};
    AlignmentJob job = tools.job();
    job.method = subedit::core::AlignmentMethod::Speech;

    // No program that finds speech on the search path.
    const auto noTool = runner.align(job, nullptr);
    REQUIRE_FALSE(noTool.has_value());
    CHECK_THAT(noTool.error().message, ContainsSubstring("whisper-vad-speech-segments"));

    // The program, and no model.
    (void)tools.scratch.write("whisper-vad-speech-segments", "#!/bin/sh\nexit 0\n");
    std::filesystem::permissions(tools.scratch.file("whisper-vad-speech-segments"),
                                 std::filesystem::perms::owner_all,
                                 std::filesystem::perm_options::add);
    const auto noModel = runner.align(job, nullptr);
    REQUIRE_FALSE(noModel.has_value());
    CHECK_THAT(noModel.error().message, ContainsSubstring("voice-activity model"));

    // The program, a model, and a detector that fails.
    (void)tools.scratch.write("whisper-vad-speech-segments", "#!/bin/sh\nexit 1\n");
    job.settings.vadModel = tools.scratch.write("vad.bin", "model");
    const auto fails = runner.align(job, nullptr);
    REQUIRE_FALSE(fails.has_value());
    CHECK_FALSE(fails.error().cancelled);
}

TEST_CASE("the dialog says which of the speech's tools is missing", "[gui][GUI-SYNC-05]") {
    const auto labelFor = [](const TranscriptionTools& there) {
        Stage stage;
        stage.runner->there = there;
        stage.prompts.nextRun = false;
        std::string label;
        stage.prompts.fill = [&label](QDialog& dialog) {
            if (const auto* sync = dynamic_cast<TranscribeDialog*>(&dialog))
                label = sync->toolsLabel()->text().toStdString();
        };
        stage.window->synchronizeWithAudioAction()->trigger();
        return label;
    };
    const auto program = std::filesystem::path{"/usr/bin/whisper-vad-speech-segments"};

    CHECK_THAT(labelFor(TranscriptionTools{.extractor = true, .voiceTool = program}),
               ContainsSubstring("voice-activity model"));
    CHECK_THAT(labelFor(TranscriptionTools{.voiceTool = program, .voiceModel = true}),
               ContainsSubstring("ffmpeg: not found"));
}
