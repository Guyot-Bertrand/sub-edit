// `Transcribe Audio…`, from the window: the dialog, what it says of the tools and of the
// language, the confirmation before the subtitles are replaced, the background run, and what a
// failure or a cancellation leaves — and the preferences where the program and the model are set.
//
// The runner is a seam, like `Prompts`: the real one needs an engine and a model of several
// hundred megabytes. Its own cases below run it against stand-ins for the engine and
// `ffmpeg`.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/preferences_dialog.hpp>
#include <subedit/gui/transcribe_dialog.hpp>
#include <subedit/gui/transcription_runner.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
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
using subedit::core::ModelState;
using subedit::core::Subtitle;
using subedit::core::Timestamp;
using subedit::core::TranscriptionSettings;
using subedit::core::TranscriptionTools;
using subedit::gui::BackgroundTranscriptionRunner;
using subedit::gui::ChosenFile;
using subedit::gui::MainWindow;
using subedit::gui::PreferencesDialog;
using subedit::gui::ToolsView;
using subedit::gui::TranscribeChoice;
using subedit::gui::TranscribeDialog;
using subedit::gui::TranscriptionFailure;
using subedit::gui::TranscriptionJob;
using subedit::gui::TranscriptionOutcome;
using subedit::gui::TranscriptionPreferences;
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

/// The two tracks of a film with a commentary; the second declares no language.
const std::vector<AudioTrack> kTracks{
    {.id = 1, .language = "fra", .title = "Original", .selected = true},
    {.id = 2, .language = "", .title = "Commentary", .selected = false}};

/// Every tool there, and a model that detects the language.
const TranscriptionTools kReady{.engine = std::filesystem::path{"/usr/bin/whisper-cli"},
                                .extractor = true,
                                .model = ModelState::Multilingual};

[[nodiscard]] TranscribeDialog::ToolsProbe probing(const TranscriptionTools& tools) {
    return [tools] { return ToolsView{.settings = {}, .tools = tools}; };
}

void nothing() {}

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
    TranscriptionTools there = kReady;

    [[nodiscard]] std::expected<TranscriptionOutcome, TranscriptionFailure>
    run(const TranscriptionJob& job, QWidget* /*parent*/) override {
        asked.push_back(job);
        return answer;
    }

    [[nodiscard]] TranscriptionTools tools(const TranscriptionSettings& /*settings*/) override {
        return there;
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

/// A window with a film of two audio tracks and three subtitles, the runner being scripted.
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
    const TranscribeDialog dialog{kTracks, {}, probing(kReady), nothing};

    CHECK(dialog.trackBox()->count() == 2);
    CHECK(dialog.languageEdit()->text() == "fr");

    dialog.trackBox()->setCurrentIndex(1);
    CHECK(dialog.languageEdit()->text().isEmpty());
    CHECK(dialog.choice().track == 1);
}

TEST_CASE("a language that was typed is kept when the track changes", "[gui][GUI-TRANSCRIBE-01]") {
    const TranscribeDialog dialog{kTracks, {}, probing(kReady), nothing};

    dialog.languageEdit()->setText("de");
    QMetaObject::invokeMethod(
        dialog.languageEdit(), "textEdited", Q_ARG(QString, QStringLiteral("de")));
    dialog.trackBox()->setCurrentIndex(1);

    CHECK(dialog.choice().language == "de");
}

TEST_CASE("a language from the settings starts the field", "[gui][GUI-TRANSCRIBE-01]") {
    const TranscribeDialog dialog{
        kTracks, {.track = 1, .language = "es"}, probing(kReady), nothing};

    CHECK(dialog.choice() == TranscribeChoice{.track = 1, .language = "es"});
}

TEST_CASE("a film without audio tracks still offers the first one", "[gui][GUI-TRANSCRIBE-01]") {
    const TranscribeDialog dialog{{}, {}, probing(kReady), nothing};

    CHECK(dialog.trackBox()->count() == 1);
    CHECK(dialog.choice().track == 0);
    CHECK(dialog.languageEdit()->text().isEmpty());
}

TEST_CASE("the dialog says where the language comes from, and when it must be given",
          "[gui][GUI-TRANSCRIBE-01]") {
    const TranscribeDialog declared{kTracks, {}, probing(kReady), nothing};
    CHECK_THAT(declared.languageHint()->text().toStdString(),
               ContainsSubstring("Proposed from the language the file declares"));

    // The second track declares nothing: the model will listen, and the person is told so.
    declared.trackBox()->setCurrentIndex(1);
    CHECK_THAT(declared.languageHint()->text().toStdString(),
               ContainsSubstring("declares no language for this track"));
    CHECK_THAT(declared.languageHint()->text().toStdString(),
               ContainsSubstring("detect it from the sound"));

    TranscriptionTools englishOnly = kReady;
    englishOnly.model = ModelState::EnglishOnly;
    const TranscribeDialog blind{kTracks, {}, probing(englishOnly), nothing};
    CHECK_THAT(blind.languageHint()->text().toStdString(),
               ContainsSubstring("cannot detect the language: type it"));

    const TranscribeDialog typed{kTracks, {.language = "de"}, probing(kReady), nothing};
    CHECK_THAT(typed.languageHint()->text().toStdString(), ContainsSubstring("you typed is used"));
}

TEST_CASE("the dialog says which tool is missing, points to the guide, and cannot start",
          "[gui][GUI-TRANSCRIBE-04]") {
    const TranscribeDialog dialog{kTracks, {}, probing({}), nothing};

    const std::string said = dialog.toolsLabel()->text().toStdString();
    CHECK_THAT(said, ContainsSubstring("whisper.cpp: not found"));
    CHECK_THAT(said, ContainsSubstring("ffmpeg: not found"));
    CHECK_THAT(said, ContainsSubstring("Model: none set"));
    CHECK_THAT(said, ContainsSubstring("Set them in the preferences."));
    CHECK_THAT(said, ContainsSubstring("Installing the transcription tools"));
    CHECK_FALSE(dialog.buttons()->button(QDialogButtonBox::Ok)->isEnabled());
}

TEST_CASE("with everything there the dialog can start and does not send the person elsewhere",
          "[gui][GUI-TRANSCRIBE-04]") {
    const TranscribeDialog dialog{kTracks, {}, probing(kReady), nothing};

    CHECK(dialog.buttons()->button(QDialogButtonBox::Ok)->isEnabled());
    CHECK_THAT(dialog.toolsLabel()->text().toStdString(), ContainsSubstring("whisper.cpp: found"));
    CHECK_FALSE(dialog.toolsLabel()->text().toStdString().find("Installing") != std::string::npos);
}

TEST_CASE("the Preferences button opens the preferences, and the dialog looks again afterwards",
          "[gui][GUI-TRANSCRIBE-04]") {
    TranscriptionTools there{};
    int opened = 0;
    const TranscribeDialog dialog{kTracks,
                                  {},
                                  [&there] { return ToolsView{.settings = {}, .tools = there}; },
                                  [&] {
                                      ++opened;
                                      there = kReady;
                                  }};
    REQUIRE_FALSE(dialog.buttons()->button(QDialogButtonBox::Ok)->isEnabled());

    dialog.preferencesButton()->click();

    CHECK(opened == 1);
    CHECK(dialog.buttons()->button(QDialogButtonBox::Ok)->isEnabled());
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

TEST_CASE("once confirmed the subtitles are replaced with the model of the settings, and one undo "
          "gives them back",
          "[gui][GUI-TRANSCRIBE-03]") {
    Stage stage;
    subedit::core::Settings settings;
    settings.transcription.model = "/models/small.bin";
    stage.window->applySettings(settings);
    stage.prompts.nextRun = true;
    stage.prompts.nextReplacementAccepted = true;
    stage.prompts.fill = [](QDialog& dialog) {
        dynamic_cast<TranscribeDialog&>(dialog).trackBox()->setCurrentIndex(1);
        dynamic_cast<TranscribeDialog&>(dialog).languageEdit()->setText("en");
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
    window.setTranscriptionRunner(std::make_unique<ScriptedRunner>());
    window.show();
    window.selectVideoAction()->trigger();
    prompts.nextRun = true;

    window.transcribeAudioAction()->trigger();

    CHECK(prompts.replacements.empty());
    CHECK(textsOf(window) == std::vector<std::string>{"Hello.", "World."});
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

TEST_CASE("the Preferences button of the dialog opens the preferences of the window, where the "
          "model is set",
          "[gui][GUI-TRANSCRIBE-04]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.prompts.nextReplacementAccepted = true;
    stage.prompts.fill = [](QDialog& dialog) {
        if (auto* transcribe = dynamic_cast<TranscribeDialog*>(&dialog))
            transcribe->preferencesButton()->click();
        else if (auto* preferences = dynamic_cast<PreferencesDialog*>(&dialog))
            preferences->modelEdit()->setText("/models/set-in-preferences.bin");
    };

    stage.window->transcribeAudioAction()->trigger();

    REQUIRE(stage.runner->asked.size() == 1);
    CHECK(stage.runner->asked.front().model == "/models/set-in-preferences.bin");
    CHECK(stage.window->settings().transcription.model == "/models/set-in-preferences.bin");
}

TEST_CASE(
    "the Browse button in the preferences of the window asks the chooser, and keeps what it gives",
    "[gui][GUI-TRANSCRIBE-04]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.prompts.nextReplacementAccepted = true;
    stage.prompts.nextFileToChoose = std::filesystem::path{"/models/chosen.bin"};
    stage.prompts.fill = [](QDialog& dialog) {
        if (auto* transcribe = dynamic_cast<TranscribeDialog*>(&dialog))
            transcribe->preferencesButton()->click();
        else if (auto* preferences = dynamic_cast<PreferencesDialog*>(&dialog))
            preferences->modelButton()->click();
    };

    stage.window->transcribeAudioAction()->trigger();

    REQUIRE(stage.prompts.filesChosenFrom.size() == 1);
    CHECK(stage.window->settings().transcription.model == "/models/chosen.bin");
}

// ## The preferences: where the program and the model are set

TEST_CASE("the preferences hold the program and the model, and give them back as edited",
          "[gui][GUI-TRANSCRIBE-04]") {
    TranscriptionSettings settings;
    settings.executable = "/opt/whisper/whisper-cli";
    settings.model = "/models/small.bin";
    settings.mergeGapMilliseconds = 250;
    const PreferencesDialog dialog{
        subedit::core::Theme::System, {}, {}, {}, TranscriptionPreferences{.settings = settings}};

    CHECK(dialog.programEdit()->text() == "/opt/whisper/whisper-cli");
    CHECK(dialog.modelEdit()->text() == "/models/small.bin");

    dialog.modelEdit()->setText("/models/medium.bin");
    CHECK(dialog.transcription().model == "/models/medium.bin");
    CHECK(dialog.transcription().executable == "/opt/whisper/whisper-cli");
    // Everything else of the transcription settings is handed back as it was given.
    CHECK(dialog.transcription().mergeGapMilliseconds == 250);
}

TEST_CASE("the preferences say what is there of what the fields name, as they are edited",
          "[gui][GUI-TRANSCRIBE-04]") {
    const PreferencesDialog dialog{
        subedit::core::Theme::System,
        {},
        {},
        {},
        TranscriptionPreferences{
            .settings = {}, .tools = [](const TranscriptionSettings& settings) {
                return TranscriptionTools{
                    .engine = settings.executable.empty() ? std::optional<std::filesystem::path>{}
                                                          : std::optional{settings.executable},
                    .extractor = true,
                    .model = settings.model.empty() ? ModelState::NotSet : ModelState::EnglishOnly};
            }}};
    CHECK_THAT(dialog.toolsLabel()->text().toStdString(),
               ContainsSubstring("whisper.cpp: not found"));
    CHECK_THAT(dialog.toolsLabel()->text().toStdString(),
               ContainsSubstring("Installing the transcription tools"));

    dialog.programEdit()->setText("/opt/whisper-cli");
    dialog.modelEdit()->setText("/models/tiny.en.bin");

    CHECK_THAT(dialog.toolsLabel()->text().toStdString(),
               ContainsSubstring("whisper.cpp: found (/opt/whisper-cli)"));
    CHECK_THAT(dialog.toolsLabel()->text().toStdString(),
               ContainsSubstring("Model: English only (/models/tiny.en.bin)"));
    CHECK_FALSE(dialog.toolsLabel()->text().toStdString().find("Installing") != std::string::npos);
}

TEST_CASE("the Browse buttons ask the window for a program and for a model, and keep a refusal out",
          "[gui][GUI-TRANSCRIBE-04]") {
    std::vector<std::pair<ChosenFile, std::filesystem::path>> asked;
    std::optional<std::filesystem::path> answer = std::filesystem::path{"/chosen/file"};
    const PreferencesDialog dialog{
        subedit::core::Theme::System,
        {},
        {},
        {},
        TranscriptionPreferences{
            .settings = {.model = "/models/old.bin"},
            .choose = [&](ChosenFile what, const std::filesystem::path& current) {
                asked.emplace_back(what, current);
                return answer;
            }}};

    dialog.programButton()->click();
    dialog.modelButton()->click();
    CHECK(dialog.programEdit()->text() == "/chosen/file");
    CHECK(dialog.modelEdit()->text() == "/chosen/file");
    REQUIRE(asked.size() == 2);
    CHECK(asked[0].first == ChosenFile::Program);
    CHECK(asked[1].first == ChosenFile::Model);
    CHECK(asked[1].second == "/models/old.bin");

    answer = std::nullopt;
    dialog.modelEdit()->setText("/kept");
    dialog.modelButton()->click();
    CHECK(dialog.modelEdit()->text() == "/kept");
}

TEST_CASE("a preferences dialog with nothing to ask of the window still opens",
          "[gui][GUI-TRANSCRIBE-04]") {
    const PreferencesDialog dialog{subedit::core::Theme::System};

    dialog.programButton()->click();

    CHECK(dialog.programEdit()->text().isEmpty());
    CHECK(dialog.toolsLabel()->text().isEmpty());
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

    void everything() const {
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
    const Tools tools;
    tools.everything();
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};

    const auto outcome = runner.run(tools.job(), nullptr);

    REQUIRE(outcome.has_value());
    CHECK(outcome->language == "en");
    CHECK(outcome->subtitles.size() >= 2);
    CHECK(outcome->subtitles.front().mainText.starts_with("And so"));
}

TEST_CASE("the real runner can be stopped, and says it was", "[gui][GUI-TRANSCRIBE-02]") {
    const Tools tools;
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
    const Tools tools;
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};

    const auto outcome = runner.run(tools.job(), nullptr);

    REQUIRE_FALSE(outcome.has_value());
    CHECK_FALSE(outcome.error().cancelled);
    CHECK_THAT(outcome.error().message, ContainsSubstring("ffmpeg was not found"));
    CHECK_THAT(outcome.error().message, ContainsSubstring("Installing the transcription tools"));
}

TEST_CASE("an engine that is not installed is said with the guide, the window unaffected",
          "[gui][GUI-TRANSCRIBE-04]") {
    const Tools tools;
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
    const Tools tools;
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};
    TranscriptionJob job = tools.job();
    job.settings.engine = "other";

    const auto outcome = runner.run(job, nullptr);

    REQUIRE_FALSE(outcome.has_value());
    CHECK_THAT(outcome.error().message,
               ContainsSubstring("no transcription engine is called other"));
}

TEST_CASE("the real runner names the tracks a video has when one is missing",
          "[gui][GUI-TRANSCRIBE-04]") {
    const Tools tools;
    (void)tools.scratch.standIn("ffprobe",
                                kFake,
                                "--no-echo --stdout-from='" + std::string{SUBEDIT_TEST_DATA_DIR} +
                                    "/audio-tracks/ffprobe.audio-2.txt'");
    (void)tools.scratch.standIn("ffmpeg", kFake, "");
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};
    TranscriptionJob job = tools.job();
    job.track = 5;

    const auto outcome = runner.run(job, nullptr);

    REQUIRE_FALSE(outcome.has_value());
    CHECK_THAT(outcome.error().message, ContainsSubstring("the video has no audio track 5"));
    CHECK_THAT(outcome.error().message, ContainsSubstring("audio track 1: eng, Commentary"));
}

TEST_CASE("stopping during the extraction is a cancellation too", "[gui][GUI-TRANSCRIBE-02]") {
    const Tools tools;
    (void)tools.scratch.standIn("ffmpeg", kFake, "--linger=30");
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};
    runner.setCancelProbe([] { return true; });

    const auto outcome = runner.run(tools.job(), nullptr);

    REQUIRE_FALSE(outcome.has_value());
    CHECK(outcome.error().cancelled);
}
