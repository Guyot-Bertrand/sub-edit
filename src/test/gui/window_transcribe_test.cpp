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
#include <subedit/core/transcription/subtitle_assembly.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/preferences_dialog.hpp>
#include <subedit/gui/transcribe_dialog.hpp>
#include <subedit/gui/transcription_preview_dialog.hpp>
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
#include <QScrollBar>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabBar>
#include <QTableWidget>
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
using subedit::core::Timestamp;
using subedit::core::TranscribedSegment;
namespace core = subedit::core;
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
using subedit::gui::TranscriptionPreview;
using subedit::gui::TranscriptionPreviewDialog;
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

/// Every tool there, and a model that detects the language and offers five.
const TranscriptionTools kReady{
    .engine = std::filesystem::path{"/usr/bin/whisper-cli"},
    .extractor = true,
    .model = ModelState::Multilingual,
    .capabilities = subedit::core::EngineCapabilities{.languageDetection = true,
                                                      .languages = {"en", "fr", "de", "es", "ja"},
                                                      .suppressNotes = true}};

/// Every tool there, and a model for English alone.
[[nodiscard]] TranscriptionTools englishOnlyTools() {
    TranscriptionTools tools = kReady;
    tools.model = ModelState::EnglishOnly;
    tools.capabilities =
        subedit::core::EngineCapabilities{.languages = {"en"}, .suppressNotes = true};
    return tools;
}

/// What the person does with the list of languages: picks one by its code.
void pick(const TranscribeDialog& dialog, const char* code) {
    QComboBox* box = dialog.languageBox();
    const int at = box->findData(QString::fromUtf8(code));
    REQUIRE(at >= 0);
    box->setCurrentIndex(at);
    Q_EMIT box->activated(at);
}

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

/// A piece as the engine cut it: a second long.
[[nodiscard]] TranscribedSegment piece(std::int64_t start, const char* text) {
    return TranscribedSegment{.text = text,
                              .start = Timestamp::fromMilliseconds(start),
                              .end = Timestamp::fromMilliseconds(start + 1000)};
}

/// A runner that answers what the case says, and keeps what it was asked.
class ScriptedRunner final : public TranscriptionRunner {
public:
    std::expected<TranscriptionOutcome, TranscriptionFailure> answer = TranscriptionOutcome{
        .segments = {piece(0, "Hello."), piece(1500, "World.")}, .language = "en"};
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
        // The project holds three subtitles, so the question before the work is asked: yes by
        // default, the cases about the refusal say no.
        prompts.nextReplacementAccepted = true;
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
    CHECK(dialog.choice().language == "fr");

    dialog.trackBox()->setCurrentIndex(1);
    CHECK(dialog.choice().language.empty());
    CHECK(dialog.choice().track == 1);
}

TEST_CASE("a language that was chosen is kept when the track changes", "[gui][GUI-TRANSCRIBE-01]") {
    const TranscribeDialog dialog{kTracks, {}, probing(kReady), nothing};

    pick(dialog, "de");
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
    CHECK(dialog.choice().language.empty());
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

    const TranscribeDialog blind{kTracks, {}, probing(englishOnlyTools()), nothing};
    CHECK_THAT(blind.languageHint()->text().toStdString(),
               ContainsSubstring("This model is for English only"));
    // And the language is English, and cannot be changed.
    CHECK(blind.languageBox()->count() == 1);
    CHECK(blind.languageBox()->currentData().toString() == "en");
    CHECK_FALSE(blind.languageBox()->isEnabled());
    CHECK(blind.choice().language == "en");

    const TranscribeDialog typed{kTracks, {.language = "de"}, probing(kReady), nothing};
    CHECK_THAT(typed.languageHint()->text().toStdString(), ContainsSubstring("you chose is used"));
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

TEST_CASE("the window asks before the work, and a refusal costs nothing",
          "[gui][GUI-TRANSCRIBE-03]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.prompts.nextReplacementAccepted = false;

    stage.window->transcribeAudioAction()->trigger();

    // Asked once, with the number of subtitles that would go; refused, nothing was started.
    REQUIRE(stage.prompts.replacements.size() == 1);
    CHECK_THAT(stage.prompts.replacements.front(), ContainsSubstring("3 subtitles"));
    CHECK(stage.runner->asked.empty());
    CHECK(textsOf(*stage.window) == std::vector<std::string>{"Un.", "Deux.", "Trois."});
}

TEST_CASE("ignoring the preview changes nothing", "[gui][GUI-TRANSCRIBE-03]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.prompts.fill = [&stage](QDialog& dialog) {
        // The dialog of the transcription is accepted; the preview of its result is ignored.
        if (dynamic_cast<TranscriptionPreviewDialog*>(&dialog) != nullptr)
            stage.prompts.nextRun = false;
    };

    stage.window->transcribeAudioAction()->trigger();

    // The work was done and shown; the person lost it, and the project is as it was.
    REQUIRE(stage.runner->asked.size() == 1);
    CHECK(textsOf(*stage.window) == std::vector<std::string>{"Un.", "Deux.", "Trois."});
    CHECK_FALSE(stage.window->undoAction()->isEnabled());
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
    stage.runner->answer = std::unexpected(TranscriptionFailure{.message = "the model is gone"});

    stage.window->transcribeAudioAction()->trigger();

    CHECK(stage.prompts.failures == std::vector<std::string>{"the model is gone"});
    CHECK(textsOf(*stage.window) == std::vector<std::string>{"Un.", "Deux.", "Trois."});
}

TEST_CASE("a cancellation leaves the project as it was and is not reported as a failure",
          "[gui][GUI-TRANSCRIBE-02]") {
    Stage stage;
    stage.prompts.nextRun = true;
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

TEST_CASE("a range replaces the subtitles like any transcription, after the same preview",
          "[gui][GUI-TRANSCRIBE-06]") {
    Stage stage;
    stage.prompts.nextRun = true;
    stage.prompts.fill = [](QDialog& dialog) {
        if (auto* ask = dynamic_cast<TranscribeDialog*>(&dialog)) {
            ask->startEdit()->setText("0:30");
            ask->endEdit()->setText("1:00");
        }
    };
    const int tabs = stage.window->tabBar()->count();

    stage.window->transcribeAudioAction()->trigger();

    REQUIRE(stage.runner->asked.size() == 1);
    CHECK(stage.runner->asked.front().start == Timestamp::fromMilliseconds(30000));
    CHECK(stage.runner->asked.front().end == Timestamp::fromMilliseconds(60000));
    // The project replaced, no tab opened.
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

TEST_CASE("the language is chosen in the list the engine offers, in the language of the window",
          "[gui][GUI-TRANSCRIBE-01]") {
    const TranscribeDialog dialog{kTracks, {}, probing(kReady), nothing};
    QComboBox* box = dialog.languageBox();

    // "Detect" first, then the five, by name.
    REQUIRE(box->count() == 6);
    CHECK(box->itemData(0).toString().isEmpty());
    CHECK(box->itemText(0).contains("first 30 seconds"));
    QStringList codes;
    for (int at = 1; at < box->count(); ++at)
        codes << box->itemData(at).toString();
    CHECK(codes.size() == 5);
    for (const char* wanted : {"en", "fr", "de", "es", "ja"})
        CHECK(codes.contains(wanted));
    CHECK(box->itemText(box->findData("fr")) == "French (fr)");

    // Sorted by name, in English here: English, French, German, Japanese, Spanish.
    CHECK(codes == QStringList{"en", "fr", "de", "ja", "es"});
}

TEST_CASE("without a model nothing is known of the languages: detect, and what was wanted",
          "[gui][GUI-TRANSCRIBE-01]") {
    const TranscriptionTools nothingYet{.engine = std::filesystem::path{"/usr/bin/whisper-cli"},
                                        .extractor = true};
    const TranscribeDialog dialog{kTracks, {.language = "de"}, probing(nothingYet), nothing};

    CHECK(dialog.languageBox()->count() == 2);
    CHECK(dialog.choice().language == "de");
    CHECK_FALSE(dialog.buttons()->button(QDialogButtonBox::Ok)->isEnabled());
}

TEST_CASE("a language the engine does not offer falls back to detection",
          "[gui][GUI-TRANSCRIBE-01]") {
    const TranscribeDialog dialog{kTracks, {.language = "xx"}, probing(kReady), nothing};

    CHECK(dialog.choice().language.empty());
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
          "[gui][GUI-TRANSCRIBE-04][GUI-TRANSCRIBE-09]") {
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

    dialog.voiceModelButton()->click();
    CHECK(dialog.voiceModelEdit()->text() == "/chosen/file");
    CHECK(dialog.transcription().vadModel == "/chosen/file");
    REQUIRE(asked.size() == 3);
    CHECK(asked[2].first == ChosenFile::VoiceModel);

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
    CHECK_FALSE(outcome->segments.empty());
    CHECK(outcome->segments.front().text.starts_with("And so"));
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
    CHECK_FALSE(outcome->segments.empty());
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
    TranscriptionTools tools = englishOnlyTools();
    const TranscribeDialog dialog{
        kTracks, {}, [&tools] { return ToolsView{.tools = tools}; }, [] {}, nullptr};
    REQUIRE_FALSE(dialog.languageBox()->isEnabled());

    tools = kReady;
    dialog.preferencesButton()->click();

    CHECK(dialog.languageBox()->isEnabled());
    CHECK(dialog.languageBox()->count() == 6);
    // The declared language of the track comes back, as it was before the model was chosen.
    CHECK(dialog.choice().language == "fr");
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
    stage.prompts.fill = [](QDialog& dialog) {
        if (auto* ask = dynamic_cast<TranscribeDialog*>(&dialog))
            ask->suppressNotesBox()->setChecked(false);
    };

    stage.window->transcribeAudioAction()->trigger();

    REQUIRE(stage.runner->asked.size() == 1);
    CHECK_FALSE(stage.runner->asked.front().settings.suppressNotes);
    CHECK_FALSE(stage.window->settings().transcription.suppressNotes);
}

TEST_CASE("the preferences hold the choice about notes", "[gui][GUI-TRANSCRIBE-04]") {
    const TranscriptionSettings settings;
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

// ## The preview

TEST_CASE(
    "the result is shown in a preview, the engine's pieces and the final subtitles side by side",
    "[gui][GUI-TRANSCRIBE-07]") {
    Stage stage;
    stage.runner->answer = TranscriptionOutcome{.segments = {piece(0, "I was going to"),
                                                             piece(1100, "tell you."),
                                                             piece(5000, "..."),
                                                             piece(9000, "Bye.")},
                                                .language = "en"};
    stage.prompts.nextRun = true;
    std::vector<std::string> left;
    std::vector<std::string> right;
    stage.prompts.fill = [&](QDialog& dialog) {
        if (auto* preview = dynamic_cast<TranscriptionPreviewDialog*>(&dialog)) {
            for (int row = 0; row < preview->piecesTable()->rowCount(); ++row)
                left.push_back(preview->piecesTable()->item(row, 2)->text().toStdString());
            for (int row = 0; row < preview->subtitlesTable()->rowCount(); ++row)
                right.push_back(preview->subtitlesTable()->item(row, 2)->text().toStdString());
        }
    };

    stage.window->transcribeAudioAction()->trigger();

    // Left: everything, the placeholder too. Right: merged, and the placeholder gone.
    CHECK(left == std::vector<std::string>{"I was going to", "tell you.", "...", "Bye."});
    CHECK(right == std::vector<std::string>{"I was going to tell you.", "Bye."});
    // Applying put the right-hand side in the project.
    CHECK(textsOf(*stage.window) == right);
}

TEST_CASE("each side of the preview has its own start and end", "[gui][GUI-TRANSCRIBE-07]") {
    const core::Assembly assembly = core::assemble(
        std::vector<TranscribedSegment>{piece(0, "I was going to"), piece(1100, "tell you.")});
    const TranscriptionPreviewDialog dialog{TranscriptionPreview{
        .pieces = {piece(0, "I was going to"), piece(1100, "tell you.")}, .assembly = assembly}};

    CHECK(dialog.piecesTable()->rowCount() == 2);
    CHECK(dialog.subtitlesTable()->rowCount() == 1);
    // The pieces: 0 to 1 s and 1.1 to 2.1 s. The subtitle: 0 to 2.1 s.
    CHECK(dialog.piecesTable()->item(1, 0)->text() == "00:00:01.100");
    CHECK(dialog.piecesTable()->item(1, 1)->text() == "00:00:02.100");
    CHECK(dialog.subtitlesTable()->item(0, 0)->text() == "00:00:00.000");
    CHECK(dialog.subtitlesTable()->item(0, 1)->text() == "00:00:02.100");
}

TEST_CASE("selecting a subtitle highlights the pieces it was made from and shows them",
          "[gui][GUI-TRANSCRIBE-07]") {
    std::vector<TranscribedSegment> pieces;
    // Forty pieces, so that the left side has to scroll; pieces 30 and 31 merge, the rest stand
    // alone.
    constexpr std::int64_t kStep = 10000;
    pieces.reserve(40);
    for (std::int64_t index = 0; index < 40; ++index)
        pieces.push_back(piece(index * kStep, "Said."));
    pieces[30] = piece(30 * kStep, "Going to");
    pieces[31] = piece((31 * kStep) - 9000, "tell you.");
    const core::Assembly assembly = core::assemble(pieces);
    TranscriptionPreviewDialog dialog{TranscriptionPreview{.pieces = pieces, .assembly = assembly}};
    dialog.resize(1000, 400);
    dialog.show();

    // Nothing selected, nothing highlighted.
    CHECK(dialog.highlightedPieces().empty());

    // The subtitle made of two pieces: both are highlighted, and the left side scrolled to them.
    int merged = -1;
    for (std::size_t at = 0; at < assembly.sources.size(); ++at)
        if (assembly.sources[at].size() == 2)
            merged = static_cast<int>(at);
    REQUIRE(merged >= 0);
    dialog.subtitlesTable()->setCurrentCell(merged, 2);
    const std::vector<int> expected(assembly.sources[static_cast<std::size_t>(merged)].begin(),
                                    assembly.sources[static_cast<std::size_t>(merged)].end());
    CHECK(dialog.highlightedPieces() == expected);
    CHECK(dialog.piecesTable()->item(expected.front(), 2)->background().style() != Qt::NoBrush);
    CHECK(dialog.piecesTable()->verticalScrollBar()->value() > 0);

    // Moving the selection moves the highlight: the old one is cleared.
    dialog.subtitlesTable()->setCurrentCell(0, 2);
    CHECK(dialog.piecesTable()->item(expected.front(), 2)->background().style() == Qt::NoBrush);
    CHECK(dialog.highlightedPieces().size() == 1);
}

TEST_CASE("clearing the selection clears the highlight", "[gui][GUI-TRANSCRIBE-07]") {
    const auto pieces = std::vector<TranscribedSegment>{piece(0, "Hello."), piece(5000, "World.")};
    const TranscriptionPreviewDialog dialog{
        TranscriptionPreview{.pieces = pieces, .assembly = core::assemble(pieces)}};
    dialog.subtitlesTable()->setCurrentCell(1, 2);
    REQUIRE(dialog.highlightedPieces().size() == 1);

    dialog.subtitlesTable()->setCurrentCell(-1, -1);

    CHECK(dialog.highlightedPieces().empty());
}

TEST_CASE("the left side cannot be selected or edited", "[gui][GUI-TRANSCRIBE-07]") {
    const TranscriptionPreviewDialog dialog{
        TranscriptionPreview{.pieces = {piece(0, "Hello.")},
                             .assembly = core::assemble(std::vector{piece(0, "Hello.")})}};

    CHECK(dialog.piecesTable()->selectionMode() == QAbstractItemView::NoSelection);
    CHECK(dialog.piecesTable()->editTriggers() == QAbstractItemView::NoEditTriggers);
    CHECK(dialog.subtitlesTable()->editTriggers() == QAbstractItemView::NoEditTriggers);
    // One subtitle at a time on the right.
    CHECK(dialog.subtitlesTable()->selectionMode() == QAbstractItemView::SingleSelection);
}

TEST_CASE("nothing to apply, nothing to apply it with", "[gui][GUI-TRANSCRIBE-07]") {
    const TranscriptionPreviewDialog dialog{TranscriptionPreview{
        .pieces = {piece(0, "..."), piece(5000, "...")},
        .assembly = core::assemble(std::vector{piece(0, "..."), piece(5000, "...")})}};

    CHECK(dialog.piecesTable()->rowCount() == 2);
    CHECK(dialog.subtitlesTable()->rowCount() == 0);
    CHECK_FALSE(dialog.applyButton()->isEnabled());
    CHECK(dialog.ignoreButton()->isEnabled());
}

TEST_CASE("a cancelled run offers what it had obtained, titled as partial",
          "[gui][GUI-TRANSCRIBE-07]") {
    Stage stage;
    stage.runner->answer = std::unexpected(TranscriptionFailure{
        .cancelled = true, .partial = {piece(0, "First."), piece(60000, "Second.")}});
    stage.prompts.nextRun = true;
    QString summary;
    stage.prompts.fill = [&summary](QDialog& dialog) {
        if (auto* preview = dynamic_cast<TranscriptionPreviewDialog*>(&dialog))
            summary = preview->summary()->text();
    };

    stage.window->transcribeAudioAction()->trigger();

    // Not a failure, so nothing is reported as one; the partial result is offered, and was applied.
    CHECK(stage.prompts.failures.empty());
    CHECK(summary.contains("Partial result"));
    CHECK(summary.contains("0:01:01"));
    CHECK(textsOf(*stage.window) == std::vector<std::string>{"First.", "Second."});
    CHECK_THAT(stage.window->statusBar()->currentMessage().toStdString(),
               ContainsSubstring("2 subtitles kept from a partial transcription, up to 0:01:01"));
}

TEST_CASE("a cancelled run offers its partial result, and ignoring it loses it",
          "[gui][GUI-TRANSCRIBE-07]") {
    Stage stage;
    stage.runner->answer =
        std::unexpected(TranscriptionFailure{.cancelled = true, .partial = {piece(0, "First.")}});
    stage.prompts.nextRun = true;
    stage.prompts.fill = [&stage](QDialog& dialog) {
        if (dynamic_cast<TranscriptionPreviewDialog*>(&dialog) != nullptr)
            stage.prompts.nextRun = false;
    };

    stage.window->transcribeAudioAction()->trigger();

    CHECK(textsOf(*stage.window) == std::vector<std::string>{"Un.", "Deux.", "Trois."});
}

TEST_CASE("a failed run says why, and offers what it had obtained", "[gui][GUI-TRANSCRIBE-07]") {
    Stage stage;
    stage.runner->answer = std::unexpected(
        TranscriptionFailure{.message = "the engine crashed", .partial = {piece(0, "First.")}});
    stage.prompts.nextRun = true;

    stage.window->transcribeAudioAction()->trigger();

    CHECK(stage.prompts.failures == std::vector<std::string>{"the engine crashed"});
    CHECK(textsOf(*stage.window) == std::vector<std::string>{"First."});
}

TEST_CASE("a cancelled run that had obtained nothing shows no preview",
          "[gui][GUI-TRANSCRIBE-07]") {
    Stage stage;
    stage.runner->answer = std::unexpected(TranscriptionFailure{.cancelled = true});
    stage.prompts.nextRun = true;

    stage.window->transcribeAudioAction()->trigger();

    // The dialog of the transcription, and no preview after it.
    CHECK(stage.prompts.runAsked == 1);
}

// ## Tuning the post-processing in the preview

TEST_CASE("changing a limit makes the right side again at once, and the selection follows",
          "[gui][GUI-TRANSCRIBE-08]") {
    const std::vector<TranscribedSegment> pieces{
        piece(0, "I was going to"), piece(1100, "tell you."), piece(20000, "Bye.")};
    const TranscriptionPreviewDialog dialog{
        TranscriptionPreview{.pieces = pieces, .assembly = core::assemble(pieces)}};
    REQUIRE(dialog.subtitlesTable()->rowCount() == 2);
    dialog.subtitlesTable()->setCurrentCell(1, 2);
    REQUIRE(dialog.highlightedPieces() == std::vector<int>{2});

    // A merge gap of nothing: the first two pieces part.
    dialog.mergeGapBox()->setValue(0);

    CHECK(dialog.subtitlesTable()->rowCount() == 3);
    CHECK(dialog.applied().size() == 3);
    // The subtitle of "Bye." is now the third, and still the one selected.
    CHECK(dialog.subtitlesTable()->currentRow() == 2);
    CHECK(dialog.highlightedPieces() == std::vector<int>{2});
    // The pieces of the engine are what they were.
    CHECK(dialog.piecesTable()->rowCount() == 3);
}

TEST_CASE("the limits are tuned for this transcript, and kept only when asked",
          "[gui][GUI-TRANSCRIBE-08]") {
    Stage stage;
    const core::TranscriptionSettings before = stage.window->settings().transcription;
    stage.prompts.nextRun = true;
    stage.prompts.fill = [](QDialog& dialog) {
        if (auto* preview = dynamic_cast<TranscriptionPreviewDialog*>(&dialog))
            preview->maxCharactersBox()->setValue(10);
    };

    stage.window->transcribeAudioAction()->trigger();

    const core::TranscriptionSettings& after = stage.window->settings().transcription;
    CHECK(after.mergeGapMilliseconds == before.mergeGapMilliseconds);
    CHECK(after.maxDurationMilliseconds == before.maxDurationMilliseconds);
    CHECK(after.maxCharacters == before.maxCharacters);
}

TEST_CASE("the keep button hands the tuned limits to the settings, and says so",
          "[gui][GUI-TRANSCRIBE-08]") {
    Stage stage;
    stage.prompts.nextRun = true;
    QString note;
    stage.prompts.fill = [&](QDialog& dialog) {
        if (auto* preview = dynamic_cast<TranscriptionPreviewDialog*>(&dialog)) {
            preview->mergeGapBox()->setValue(250);
            preview->maxDurationBox()->setValue(5000);
            preview->maxCharactersBox()->setValue(60);
            preview->keepButton()->click();
            note = preview->keptNote()->text();
            // Tuned again after: the kept values are those of the click.
            preview->maxCharactersBox()->setValue(30);
        }
    };

    stage.window->transcribeAudioAction()->trigger();

    const core::TranscriptionSettings& kept = stage.window->settings().transcription;
    CHECK(kept.mergeGapMilliseconds == 250);
    CHECK(kept.maxDurationMilliseconds == 5000);
    CHECK(kept.maxCharacters == 60);
    CHECK_FALSE(note.isEmpty());
}
