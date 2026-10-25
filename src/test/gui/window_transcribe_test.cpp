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
#include <subedit/core/transcription/engine_capabilities.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/preferences_dialog.hpp>
#include <subedit/gui/transcribe_dialog.hpp>
#include <subedit/gui/transcription_runner.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QStatusBar>
#include <QTabBar>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
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
               ContainsSubstring("first 30 seconds of sound"));

    TranscriptionTools englishOnly = kReady;
    englishOnly.model = ModelState::EnglishOnly;
    const TranscribeDialog blind{kTracks, {}, probing(englishOnly), nothing};
    CHECK_THAT(blind.languageHint()->text().toStdString(),
               ContainsSubstring("This model is for English only"));
    // And the language is English, and cannot be changed.
    CHECK(blind.languageEdit()->text() == "en");
    CHECK(blind.languageEdit()->isReadOnly());
    CHECK(blind.choice().language == "en");

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

// ## A range of the film

TEST_CASE("the dialog takes a range, and gives it back as moments of the film",
          "[gui][GUI-TRANSCRIBE-06]") {
    const TranscribeDialog dialog{
        kTracks, {}, [] { return ToolsView{.tools = kReady}; }, [] {}, nullptr};
    CHECK_FALSE(dialog.choice().start.has_value());
    CHECK_FALSE(dialog.choice().end.has_value());
    CHECK(dialog.buttons()->button(QDialogButtonBox::Ok)->isEnabled());
    // The fields ask for the one form: hours, minutes, seconds.
    CHECK(dialog.startEdit()->placeholderText() == "HH:MM:SS");
    CHECK(dialog.endEdit()->placeholderText() == "HH:MM:SS");
    CHECK(dialog.startEdit()->toolTip().contains("HH:MM:SS"));

    dialog.startEdit()->setText("2:00");
    dialog.endEdit()->setText("3:30");

    CHECK(dialog.choice().start == Timestamp::fromMilliseconds(120000));
    CHECK(dialog.choice().end == Timestamp::fromMilliseconds(210000));
    CHECK(dialog.rangeHint()->text().isEmpty());
    CHECK(dialog.buttons()->button(QDialogButtonBox::Ok)->isEnabled());
}

TEST_CASE("a range given to the dialog starts in its fields", "[gui][GUI-TRANSCRIBE-06]") {
    const TranscribeDialog dialog{kTracks,
                                  TranscribeChoice{.start = Timestamp::fromMilliseconds(120000),
                                                   .end = Timestamp::fromMilliseconds(210000)},
                                  probing(kReady),
                                  nothing};

    CHECK(dialog.startEdit()->text() == "00:02:00");
    CHECK(dialog.endEdit()->text() == "00:03:30");
    CHECK(dialog.choice().start == Timestamp::fromMilliseconds(120000));
}

TEST_CASE("a range that cannot be used says why and keeps OK out", "[gui][GUI-TRANSCRIBE-06]") {
    const TranscribeDialog dialog{
        kTracks, {}, [] { return ToolsView{.tools = kReady}; }, [] {}, nullptr};
    const auto* ok = dialog.buttons()->button(QDialogButtonBox::Ok);

    dialog.startEdit()->setText("soon");
    CHECK_FALSE(ok->isEnabled());
    CHECK(dialog.rangeHint()->text().contains("\"soon\" is not a time"));

    dialog.startEdit()->setText("");
    dialog.endEdit()->setText("later");
    CHECK_FALSE(ok->isEnabled());
    CHECK(dialog.rangeHint()->text().contains("\"later\" is not a time"));

    dialog.startEdit()->setText("3:00");
    dialog.endEdit()->setText("2:00");
    CHECK_FALSE(ok->isEnabled());
    CHECK(dialog.rangeHint()->text().contains("must come after"));

    dialog.endEdit()->setText("4:00");
    CHECK(ok->isEnabled());
    CHECK(dialog.rangeHint()->text().isEmpty());

    // A range does not make up for tools that are missing.
    const TranscribeDialog bare{kTracks, {}, [] { return ToolsView{}; }, [] {}, nullptr};
    bare.startEdit()->setText("1:00");
    CHECK_FALSE(bare.buttons()->button(QDialogButtonBox::Ok)->isEnabled());
}

TEST_CASE("a range replaces the subtitles like any transcription, after the same question",
          "[gui][GUI-TRANSCRIBE-06]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.prompts.nextReplacementAccepted = true;
    stage.prompts.fill = [](QDialog& dialog) {
        dynamic_cast<TranscribeDialog&>(dialog).startEdit()->setText("0:30");
        dynamic_cast<TranscribeDialog&>(dialog).endEdit()->setText("1:00");
    };
    const int tabs = stage.window->tabBar()->count();

    stage.window->transcribeAudioAction()->trigger();

    REQUIRE(stage.runner->asked.size() == 1);
    CHECK(stage.runner->asked.front().start == Timestamp::fromMilliseconds(30000));
    CHECK(stage.runner->asked.front().end == Timestamp::fromMilliseconds(60000));
    // The question was asked, the project replaced, no tab opened.
    CHECK(stage.prompts.replacements.size() == 1);
    CHECK(stage.window->tabBar()->count() == tabs);
    CHECK(textsOf(*stage.window) == std::vector<std::string>{"Hello.", "World."});
    stage.window->undoAction()->trigger();
    CHECK(textsOf(*stage.window) == std::vector<std::string>{"Un.", "Deux.", "Trois."});
}

// ## The box

TEST_CASE("the box says the step, then where it is, the time left and the last text",
          "[gui][GUI-TRANSCRIBE-05]") {
    using subedit::core::TranscriptionProgress;
    using subedit::core::TranscriptionStage;
    using subedit::gui::transcriptionBoxLabel;
    using namespace std::chrono_literals;

    CHECK(transcriptionBoxLabel({.stage = TranscriptionStage::Extracting}, std::nullopt) ==
          "Extracting the audio…");
    CHECK(transcriptionBoxLabel({}, std::nullopt).split('\n') ==
          QStringList{"Transcribing the audio…"});

    const QString full = transcriptionBoxLabel({.fraction = 0.5,
                                                .position = Timestamp::fromMilliseconds(60000),
                                                .total = Timestamp::fromMilliseconds(120000),
                                                .lastText = "And so, my fellow Americans"},
                                               150s);
    // The step, the time left, the position **just above** the last text.
    CHECK(full.split('\n') == QStringList{"Transcribing the audio…",
                                          "about 2 min left",
                                          "0:01:00 of 0:02:00",
                                          "Last text: And so, my fellow Americans"});

    const QString timed =
        transcriptionBoxLabel({.stage = TranscriptionStage::Extracting}, std::nullopt, 75s);
    CHECK(timed.split('\n') == QStringList{"Extracting the audio…", "elapsed: 0:01:15"});
}

TEST_CASE("a long text is cut so that the box does not widen", "[gui][GUI-TRANSCRIBE-05]") {
    const QString label =
        subedit::gui::transcriptionBoxLabel({.lastText = std::string(300, 'x')}, std::nullopt);

    CHECK(label.size() < 160);
}

TEST_CASE("the dialog reads a three-letter code as the two-letter one, and refuses what is no code",
          "[gui][GUI-TRANSCRIBE-01]") {
    const TranscribeDialog dialog{kTracks, {}, probing(kReady), nothing};
    const auto* ok = dialog.buttons()->button(QDialogButtonBox::Ok);

    dialog.languageEdit()->setText("fra");
    CHECK(dialog.choice().language == "fr");
    CHECK(ok->isEnabled());

    dialog.languageEdit()->setText("f1");
    CHECK_FALSE(ok->isEnabled());
    CHECK(dialog.languageHint()->text().contains("is not a language code"));

    dialog.languageEdit()->setText("");
    CHECK(ok->isEnabled());
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

    // The engine is asked first, before any work: with nothing installed it is what is missing.
    const auto outcome = runner.run(tools.job(), nullptr);
    REQUIRE_FALSE(outcome.has_value());
    CHECK_FALSE(outcome.error().cancelled);
    CHECK_THAT(outcome.error().message,
               ContainsSubstring("the transcription engine was not found"));
    CHECK_THAT(outcome.error().message, ContainsSubstring("Installing the transcription tools"));

    // With the engine there, `ffmpeg` is what a video lacks.
    (void)tools.scratch.standIn("whisper-cli", kFake, "");
    const auto video = runner.run(tools.job(), nullptr);
    REQUIRE_FALSE(video.has_value());
    CHECK_THAT(video.error().message, ContainsSubstring("ffmpeg was not found"));
    CHECK_THAT(video.error().message, ContainsSubstring("Installing the transcription tools"));
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
    (void)tools.scratch.standIn("whisper-cli", kFake, "");
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
    (void)tools.scratch.standIn("whisper-cli", kFake, "");
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};
    runner.setCancelProbe([] { return true; });

    const auto outcome = runner.run(tools.job(), nullptr);

    REQUIRE_FALSE(outcome.has_value());
    CHECK(outcome.error().cancelled);
}

TEST_CASE("the real runner hands the range to the engine", "[gui][GUI-TRANSCRIBE-06]") {
    const Tools tools;
    const auto log = tools.scratch.file("arguments");
    (void)tools.scratch.standIn("whisper-cli", kFake, "--output-from='" + tools.sample + "'");
    // A spy in front of the stand-in keeps the arguments the engine was given.
    const auto spy =
        tools.scratch.write("whisper-cli-spy",
                            "#!/bin/sh\nprintf '%s\\n' \"$@\" > '" + log.string() + "'\nexec '" +
                                tools.scratch.file("whisper-cli").string() + "' \"$@\"\n");
    std::filesystem::permissions(
        spy, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
    const auto audio = tools.scratch.write("audio.wav", "pcm");
    (void)tools.scratch.standIn("ffmpeg", kFake, "--output-from='" + audio.string() + "'");
    TranscriptionJob job = tools.job();
    job.settings.executable = tools.scratch.file("whisper-cli-spy");
    job.start = Timestamp::fromMilliseconds(30000);
    job.end = Timestamp::fromMilliseconds(90000);
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};

    const auto outcome = runner.run(job, nullptr);

    REQUIRE(outcome.has_value());
    std::ifstream written{log};
    const std::string arguments{std::istreambuf_iterator<char>{written},
                                std::istreambuf_iterator<char>{}};
    CHECK(arguments.find("-ot\n30000\n") != std::string::npos);
    CHECK(arguments.find("-d\n60000\n") != std::string::npos);
}

TEST_CASE("the box follows the engine while it works", "[gui][GUI-TRANSCRIBE-05]") {
    const Tools tools;
    // The progress comes at once and the result a second later: the box has time to read it.
    (void)tools.scratch.standIn(
        "whisper-cli", kFake, "--progress=2 --linger=1 --output-from='" + tools.sample + "'");
    const auto audio = tools.scratch.write("audio.wav", "pcm");
    (void)tools.scratch.standIn("ffmpeg", kFake, "--output-from='" + audio.string() + "'");
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};

    const auto outcome = runner.run(tools.job(), nullptr);

    REQUIRE(outcome.has_value());
    CHECK(outcome->subtitles.size() >= 2);
}

TEST_CASE("the box is on screen at once and wide enough for its title",
          "[gui][GUI-TRANSCRIBE-05]") {
    const Tools tools;
    (void)tools.scratch.standIn(
        "whisper-cli", kFake, "--linger=30 --output-from='" + tools.sample + "'");
    const auto audio = tools.scratch.write("audio.wav", "pcm");
    (void)tools.scratch.standIn("ffmpeg", kFake, "--output-from='" + audio.string() + "'");
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};
    bool visible = false;
    int width = 0;
    // The first look the box takes at the cancellation is a tenth of a second in: Qt's own delay
    // before showing a box is several seconds.
    runner.setCancelProbe([&] {
        for (QWidget* top : QApplication::topLevelWidgets()) {
            if (const auto* box = qobject_cast<QProgressDialog*>(top)) {
                visible = box->isVisible();
                width = box->width();
            }
        }
        return true;
    });

    const auto outcome = runner.run(tools.job(), nullptr);

    REQUIRE_FALSE(outcome.has_value());
    CHECK(visible);
    CHECK(width >= 520);
}

TEST_CASE("changing to a model that speaks every language gives the field back",
          "[gui][GUI-TRANSCRIBE-01]") {
    TranscriptionTools tools = kReady;
    tools.model = ModelState::EnglishOnly;
    const TranscribeDialog dialog{
        kTracks, {}, [&tools] { return ToolsView{.tools = tools}; }, [] {}, nullptr};
    REQUIRE(dialog.languageEdit()->isReadOnly());

    tools.model = ModelState::Multilingual;
    dialog.preferencesButton()->click();

    CHECK_FALSE(dialog.languageEdit()->isReadOnly());
    // The declared language of the track comes back, as it was before the model was chosen.
    CHECK(dialog.languageEdit()->text() == "fr");
}

TEST_CASE("a transcription made of notes in brackets says the language was probably wrong",
          "[gui][GUI-TRANSCRIBE-05]") {
    Stage stage;
    stage.runner->answer = TranscriptionOutcome{
        .subtitles = {subtitle(0, "[MUSIC PLAYING]")}, .language = "en", .notes = 5, .pieces = 9};
    stage.prompts.nextRun = true;
    stage.prompts.nextReplacementAccepted = true;

    stage.window->transcribeAudioAction()->trigger();

    REQUIRE(stage.prompts.outcomes.size() == 1);
    CHECK_THAT(stage.prompts.outcomes.front(),
               ContainsSubstring("5 of the 9 pieces are only notes"));
    CHECK_THAT(stage.prompts.outcomes.front(), ContainsSubstring("language used (en)"));
}

TEST_CASE("a few notes among the speech are no cause for a warning", "[gui][GUI-TRANSCRIBE-05]") {
    Stage stage;
    stage.runner->answer = TranscriptionOutcome{
        .subtitles = {subtitle(0, "Hello.")}, .language = "en", .notes = 1, .pieces = 20};
    stage.prompts.nextRun = true;
    stage.prompts.nextReplacementAccepted = true;

    stage.window->transcribeAudioAction()->trigger();

    CHECK(stage.prompts.outcomes.empty());
}

TEST_CASE("a range made of notes in brackets says so too", "[gui][GUI-TRANSCRIBE-05]") {
    Stage stage;
    stage.runner->answer = TranscriptionOutcome{
        .subtitles = {subtitle(0, "[MUSIC PLAYING]")}, .language = "en", .notes = 4, .pieces = 6};
    stage.prompts.nextRun = true;
    stage.prompts.nextReplacementAccepted = true;
    stage.prompts.fill = [](QDialog& dialog) {
        dynamic_cast<TranscribeDialog&>(dialog).startEdit()->setText("00:01:00");
    };

    stage.window->transcribeAudioAction()->trigger();

    REQUIRE(stage.prompts.outcomes.size() == 1);
    CHECK_THAT(stage.prompts.outcomes.front(), ContainsSubstring("4 of the 6 pieces"));
}

// ## Sounds written as notes

TEST_CASE(
    "the dialog offers not writing sounds as notes, from the setting, and gives the choice back",
    "[gui][GUI-TRANSCRIBE-01]") {
    const TranscribeDialog on{kTracks, {}, probing(kReady), nothing};
    CHECK(on.suppressNotesBox()->isChecked());
    CHECK(on.suppressNotesBox()->isEnabled());
    CHECK(on.choice().suppressNotes);

    const TranscribeDialog off{kTracks, {.suppressNotes = false}, probing(kReady), nothing};
    CHECK_FALSE(off.suppressNotesBox()->isChecked());
    CHECK_FALSE(off.choice().suppressNotes);

    on.suppressNotesBox()->setChecked(false);
    CHECK_FALSE(on.choice().suppressNotes);
}

TEST_CASE("an engine that cannot be told leaves the box grey, with the reason",
          "[gui][GUI-TRANSCRIBE-01]") {
    TranscriptionTools tools = kReady;
    tools.capabilities = subedit::core::EngineCapabilities{.suppressNotes = false};
    const TranscribeDialog dialog{kTracks, {}, probing(tools), nothing};

    CHECK_FALSE(dialog.suppressNotesBox()->isEnabled());
    CHECK_THAT(dialog.suppressNotesBox()->toolTip().toStdString(),
               ContainsSubstring("cannot be told not to write sounds as notes"));

    // One that says it can, or says nothing yet, leaves it free.
    tools.capabilities = subedit::core::EngineCapabilities{.suppressNotes = true};
    CHECK(TranscribeDialog{kTracks, {}, probing(tools), nothing}.suppressNotesBox()->isEnabled());
    CHECK(TranscribeDialog{kTracks, {}, probing(kReady), nothing}.suppressNotesBox()->isEnabled());
}

TEST_CASE("the choice about notes goes to the runner and is kept for the next time",
          "[gui][GUI-TRANSCRIBE-01]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.prompts.nextReplacementAccepted = true;
    stage.prompts.fill = [](QDialog& dialog) {
        dynamic_cast<TranscribeDialog&>(dialog).suppressNotesBox()->setChecked(false);
    };

    stage.window->transcribeAudioAction()->trigger();

    REQUIRE(stage.runner->asked.size() == 1);
    CHECK_FALSE(stage.runner->asked.front().settings.suppressNotes);
    CHECK_FALSE(stage.window->settings().transcription.suppressNotes);
}

TEST_CASE("the preferences hold the choice about notes", "[gui][GUI-TRANSCRIBE-04]") {
    TranscriptionSettings settings;
    const PreferencesDialog dialog{
        subedit::core::Theme::System, {}, {}, {}, TranscriptionPreferences{.settings = settings}};
    CHECK(dialog.suppressNotesBox()->isChecked());

    dialog.suppressNotesBox()->setChecked(false);
    CHECK_FALSE(dialog.transcription().suppressNotes);
}

TEST_CASE("the real runner tells the engine not to write notes, unless asked not to",
          "[gui][GUI-TRANSCRIBE-01]") {
    const Tools tools;
    const auto log = tools.scratch.file("arguments");
    (void)tools.scratch.standIn("whisper-cli", kFake, "--output-from='" + tools.sample + "'");
    const auto spy =
        tools.scratch.write("whisper-cli-spy",
                            "#!/bin/sh\nprintf '%s\\n' \"$@\" > '" + log.string() + "'\nexec '" +
                                tools.scratch.file("whisper-cli").string() + "' \"$@\"\n");
    std::filesystem::permissions(
        spy, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
    const auto audio = tools.scratch.write("audio.wav", "pcm");
    (void)tools.scratch.standIn("ffmpeg", kFake, "--output-from='" + audio.string() + "'");
    BackgroundTranscriptionRunner runner{tools.scratch.path().string()};
    const auto argumentsOf = [&log] {
        std::ifstream written{log};
        return std::string{std::istreambuf_iterator<char>{written},
                           std::istreambuf_iterator<char>{}};
    };

    TranscriptionJob job = tools.job();
    job.settings.executable = spy;
    REQUIRE(runner.run(job, nullptr).has_value());
    CHECK(argumentsOf().find("-sns\n") != std::string::npos);

    job.settings.suppressNotes = false;
    REQUIRE(runner.run(job, nullptr).has_value());
    CHECK(argumentsOf().find("-sns") == std::string::npos);
}
