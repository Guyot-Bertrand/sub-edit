// `Transcribe Audio…`, from the window: the dialog, the confirmation before the
// subtitles are replaced, the background run, and what a failure or a cancellation leaves.
//
// The runner is a seam, like `Prompts`: the real one needs an engine and a model of several
// hundred megabytes. Its own cases below run it against stand-ins for the engine and
// `ffmpeg`.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/transcribe_dialog.hpp>
#include <subedit/gui/transcription_runner.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QComboBox>
#include <QLineEdit>
#include <QStatusBar>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"
#include "player_harness.hpp"
#include "scratch_directory.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::AudioTrack;
using subedit::core::InMemoryFileSystem;
using subedit::core::Subtitle;
using subedit::core::Timestamp;
using subedit::gui::BackgroundTranscriptionRunner;
using subedit::gui::MainWindow;
using subedit::gui::TranscribeChoice;
using subedit::gui::TranscribeDialog;
using subedit::gui::TranscriptionFailure;
using subedit::gui::TranscriptionJob;
using subedit::gui::TranscriptionOutcome;
using subedit::gui::TranscriptionRunner;
using subedit::test::FakePrompts;
using subedit::test::fileIn;
using subedit::test::projecting;
using subedit::test::Projectionist;
using subedit::test::ScratchDirectory;

constexpr const char* kThree = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                               "2\n00:00:02,500 --> 00:00:03,500\nDeux.\n\n"
                               "3\n00:00:05,000 --> 00:00:06,000\nTrois.\n\n";

const std::filesystem::path kFake{SUBEDIT_FAKE_TRANSCRIBER_BINARY};

/// The two tracks of a film with a commentary.
const std::vector<AudioTrack> kTracks{
    {.id = 1, .language = "fra", .title = "Original", .selected = true},
    {.id = 2, .language = "eng", .title = "Commentary", .selected = false}};

[[nodiscard]] InMemoryFileSystem directoryHolding() {
    InMemoryFileSystem files;
    files.addFile("/films/film.mkv", "");
    files.addFile("/films/film.fr.srt", kThree);
    return files;
}

[[nodiscard]] Subtitle subtitle(std::int64_t start, const char* text) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(start + 1000),
                    .mainText = text};
}

/// A runner that answers what the case says, and keeps what it was asked.
class ScriptedRunner final : public TranscriptionRunner {
public:
    std::expected<TranscriptionOutcome, TranscriptionFailure> answer = TranscriptionOutcome{
        .subtitles = {subtitle(0, "Hello."), subtitle(1500, "World.")}, .language = "en"};
    std::vector<TranscriptionJob> asked{};

    [[nodiscard]] std::expected<TranscriptionOutcome, TranscriptionFailure>
    run(const TranscriptionJob& job, QWidget* /*parent*/) override {
        asked.push_back(job);
        return answer;
    }
};

[[nodiscard]] std::vector<std::string> textsOf(const MainWindow& window) {
    std::vector<std::string> texts;
    const QAbstractItemModel* model = window.table()->model();
    for (int row = 0; row < model->rowCount(); ++row)
        texts.push_back(
            model->data(model->index(row, 4), Qt::DisplayRole).toString().toStdString());
    return texts;
}

/// A window with a film of two audio tracks and three subtitles, the runner being `runner`.
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

TEST_CASE("the dialog offers the tracks of the film and follows the declared language",
          "[gui][GUI-TRANSCRIBE-01]") {
    TranscribeDialog dialog{kTracks, {}};

    CHECK(dialog.trackBox()->count() == 2);
    CHECK(dialog.languageEdit()->text() == "fr");

    dialog.trackBox()->setCurrentIndex(1);
    CHECK(dialog.languageEdit()->text() == "en");
    CHECK(dialog.choice().track == 1);
}

TEST_CASE("a language that was typed is kept when the track changes", "[gui][GUI-TRANSCRIBE-01]") {
    TranscribeDialog dialog{kTracks, {}};

    dialog.languageEdit()->setText("de");
    QMetaObject::invokeMethod(
        dialog.languageEdit(), "textEdited", Q_ARG(QString, QStringLiteral("de")));
    dialog.trackBox()->setCurrentIndex(1);

    CHECK(dialog.choice().language == "de");
}

TEST_CASE("a language from the settings starts the field, and the model comes with it",
          "[gui][GUI-TRANSCRIBE-01]") {
    TranscribeDialog dialog{kTracks, {.track = 1, .language = "es", .model = "/models/small.bin"}};

    CHECK(dialog.choice() ==
          TranscribeChoice{.track = 1, .language = "es", .model = "/models/small.bin"});
}

TEST_CASE("a film without audio tracks still offers the first one", "[gui][GUI-TRANSCRIBE-01]") {
    TranscribeDialog dialog{{}, {}};

    CHECK(dialog.trackBox()->count() == 1);
    CHECK(dialog.choice().track == 0);
    CHECK(dialog.languageEdit()->text().isEmpty());
}

TEST_CASE("the command is out until a film is open and in once it is", "[gui][GUI-TRANSCRIBE-01]") {
    InMemoryFileSystem files;
    files.addFile("/films/seul.srt", kThree);
    FakePrompts prompts;
    {
        Projectionist booth;
        MainWindow window{files, fileIn(files, "/films/seul.srt"), prompts, projecting(booth)};
        window.show();
        CHECK_FALSE(window.transcribeAudioAction()->isEnabled());
    }

    Stage stage;
    CHECK(stage.window->transcribeAudioAction()->isEnabled());
}

TEST_CASE("the window asks before replacing, and a refusal changes nothing",
          "[gui][GUI-TRANSCRIBE-03]") {
    Stage stage;
    stage.prompts.nextRun = true;

    stage.window->transcribeAudioAction()->trigger();

    REQUIRE(stage.prompts.replacements.size() == 1);
    CHECK_THAT(stage.prompts.replacements.front(), ContainsSubstring("3 subtitles"));
    CHECK(stage.runner->asked.empty());
    CHECK(textsOf(*stage.window) == std::vector<std::string>{"Un.", "Deux.", "Trois."});
}

TEST_CASE("once confirmed the subtitles are replaced, and one undo gives them back",
          "[gui][GUI-TRANSCRIBE-03]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.prompts.nextReplacementAccepted = true;
    stage.prompts.fill = [](QDialog& dialog) {
        auto& form = dynamic_cast<TranscribeDialog&>(dialog);
        form.trackBox()->setCurrentIndex(1);
        form.modelEdit()->setText("/models/small.bin");
    };

    stage.window->transcribeAudioAction()->trigger();

    REQUIRE(stage.runner->asked.size() == 1);
    const TranscriptionJob& job = stage.runner->asked.front();
    CHECK(job.video == "/films/film.mkv");
    CHECK(job.track == 1);
    CHECK(job.language == "en");
    CHECK(job.model == "/models/small.bin");
    CHECK(textsOf(*stage.window) == std::vector<std::string>{"Hello.", "World."});
    CHECK(stage.window->statusBar()->currentMessage().toStdString() ==
          "2 subtitles transcribed, language: en");

    stage.window->undoAction()->trigger();
    CHECK(textsOf(*stage.window) == std::vector<std::string>{"Un.", "Deux.", "Trois."});
}

TEST_CASE("a project with nothing in it is not asked about", "[gui][GUI-TRANSCRIBE-03]") {
    InMemoryFileSystem files = directoryHolding();
    FakePrompts prompts;
    Projectionist booth;
    booth.tracks = kTracks;
    subedit::core::OpenedFile opened = fileIn(files, "/films/film.fr.srt");
    opened.project.setSubtitles({});
    MainWindow window{files, std::move(opened), prompts, projecting(booth)};
    auto scripted = std::make_unique<ScriptedRunner>();
    ScriptedRunner* runner = scripted.get();
    window.setTranscriptionRunner(std::move(scripted));
    window.show();
    window.selectVideoAction()->trigger();
    prompts.nextRun = true;

    window.transcribeAudioAction()->trigger();

    CHECK(prompts.replacements.empty());
    CHECK(textsOf(window) == std::vector<std::string>{"Hello.", "World."});
    (void)runner;
}

TEST_CASE("a failure leaves the project as it was and says why", "[gui][GUI-TRANSCRIBE-02]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.prompts.nextReplacementAccepted = true;
    stage.runner->answer = std::unexpected(TranscriptionFailure{.message = "the model is gone"});

    stage.window->transcribeAudioAction()->trigger();

    CHECK(stage.prompts.failures == std::vector<std::string>{"the model is gone"});
    CHECK(textsOf(*stage.window) == std::vector<std::string>{"Un.", "Deux.", "Trois."});
}

TEST_CASE("a cancellation leaves the project as it was and is not reported as a failure",
          "[gui][GUI-TRANSCRIBE-02]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.prompts.nextReplacementAccepted = true;
    stage.runner->answer = std::unexpected(TranscriptionFailure{.cancelled = true});

    stage.window->transcribeAudioAction()->trigger();

    CHECK(stage.prompts.failures.empty());
    CHECK(stage.window->statusBar()->currentMessage().toStdString() ==
          "the transcription was cancelled");
    CHECK(textsOf(*stage.window) == std::vector<std::string>{"Un.", "Deux.", "Trois."});
}

TEST_CASE("choosing nothing in the dialog does nothing", "[gui][GUI-TRANSCRIBE-02]") {
    Stage stage;
    stage.prompts.nextRun = false;

    stage.window->transcribeAudioAction()->trigger();

    CHECK(stage.prompts.replacements.empty());
    CHECK(stage.runner->asked.empty());
}

// ## The real runner, against stand-ins for the engine and ffmpeg

namespace {

/// A directory holding stand-ins for `whisper-cli` and `ffmpeg`, a model and a film.
struct Tools {
    ScratchDirectory scratch;
    std::string sample = std::string{SUBEDIT_TEST_DATA_DIR} + "/whisper/jfk.full.json";
    std::filesystem::path film = scratch.write("film.mkv", "film");
    std::filesystem::path model =
        scratch.write("model.bin", std::string{"lmgg\x99\xca\x00\x00", 8} + std::string(8, '\0'));

    void everything() {
        (void)scratch.standIn("whisper-cli", kFake, "--progress=2 --output-from='" + sample + "'");
        const auto audio = scratch.write("audio.wav", "pcm");
        (void)scratch.standIn("ffmpeg", kFake, "--output-from='" + audio.string() + "'");
    }

    [[nodiscard]] TranscriptionJob job() const {
        return TranscriptionJob{.video = film, .track = 0, .language = "en", .model = model};
    }
};

} // namespace

TEST_CASE("the real runner transcribes through the tools it finds", "[gui][GUI-TRANSCRIBE-02]") {
    Tools tools;
    tools.everything();
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};

    const auto outcome = runner.run(tools.job(), nullptr);

    REQUIRE(outcome.has_value());
    CHECK(outcome->language == "en");
    CHECK(outcome->subtitles.size() >= 2);
    CHECK(outcome->subtitles.front().mainText.starts_with("And so"));
}

TEST_CASE("the real runner can be stopped, and says it was", "[gui][GUI-TRANSCRIBE-02]") {
    Tools tools;
    (void)tools.scratch.standIn(
        "whisper-cli", kFake, "--linger=30 --output-from='" + tools.sample + "'");
    const auto audio = tools.scratch.write("audio.wav", "pcm");
    (void)tools.scratch.standIn("ffmpeg", kFake, "--output-from='" + audio.string() + "'");
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};
    runner.setCancelProbe([] { return true; });

    const auto outcome = runner.run(tools.job(), nullptr);

    REQUIRE_FALSE(outcome.has_value());
    CHECK(outcome.error().cancelled);
}

TEST_CASE("without the tools the real runner says what is missing and where to read",
          "[gui][GUI-TRANSCRIBE-04]") {
    Tools tools;
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};

    const auto outcome = runner.run(tools.job(), nullptr);

    REQUIRE_FALSE(outcome.has_value());
    CHECK_FALSE(outcome.error().cancelled);
    CHECK_THAT(outcome.error().message, ContainsSubstring("ffmpeg was not found"));
    CHECK_THAT(outcome.error().message, ContainsSubstring("Installing the transcription tools"));
}

TEST_CASE("an engine that is not installed is said with the guide, the window unaffected",
          "[gui][GUI-TRANSCRIBE-04]") {
    Tools tools;
    const auto audio = tools.scratch.write("audio.wav", "pcm");
    (void)tools.scratch.standIn("ffmpeg", kFake, "--output-from='" + audio.string() + "'");
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};

    const auto outcome = runner.run(tools.job(), nullptr);

    REQUIRE_FALSE(outcome.has_value());
    CHECK_THAT(outcome.error().message,
               ContainsSubstring("the transcription engine was not found"));
    CHECK_THAT(outcome.error().message, ContainsSubstring("Installing the transcription tools"));
}

TEST_CASE("an engine nobody registered is named", "[gui][GUI-TRANSCRIBE-04]") {
    Tools tools;
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};
    TranscriptionJob job = tools.job();
    job.settings.engine = "other";

    const auto outcome = runner.run(job, nullptr);

    REQUIRE_FALSE(outcome.has_value());
    CHECK_THAT(outcome.error().message,
               ContainsSubstring("no transcription engine is called other"));
}
