// The dialogs and wizards under a second language — issue #665: every dialog of the window,
// every page of `Correct Texts…`, the prompts and the messages they show follow the catalogue
// chosen at start-up. Read through the fixture `data/i18n/window.po`, compiled as Russian, which
// has three plural forms.
//
// **One check for every dialog**: each is built twice, under English and under Russian, and what
// it shows — window title, labels, buttons, group boxes, tool tips, combo entries, the suffix of
// a spin box, the placeholder of a field, a page's title, a table's headers — is read in the
// order it was built. A text the fixture holds must have become its translation; a text it does
// not hold, assembled from words it does, must have changed; and no text the fixture holds may
// be shown untranslated. What stays as written is named, dialog by dialog: a number, a file's
// name, an encoding's name, the standard buttons Qt words itself.

#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/config/duration_adjustment_settings.hpp>
#include <subedit/core/config/insert_placement.hpp>
#include <subedit/core/config/theme.hpp>
#include <subedit/core/i18n/catalogue.hpp>
#include <subedit/core/i18n/language.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/core/text/spell_check_walk.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/about_dialog.hpp>
#include <subedit/gui/correction_confirmation_page.hpp>
#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_result_model.hpp>
#include <subedit/gui/correction_wizard.hpp>
#include <subedit/gui/disjoint_dialog.hpp>
#include <subedit/gui/duration_adjust_dialog.hpp>
#include <subedit/gui/fixes_proposal_dialog.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>
#include <subedit/gui/grid_analysis_dialog.hpp>
#include <subedit/gui/hearing_impaired_dialog.hpp>
#include <subedit/gui/insert_dialog.hpp>
#include <subedit/gui/interface_language.hpp>
#include <subedit/gui/invocation.hpp>
#include <subedit/gui/manual_window.hpp>
#include <subedit/gui/open_translation_dialog.hpp>
#include <subedit/gui/operation_dialog.hpp>
#include <subedit/gui/pattern_list.hpp>
#include <subedit/gui/preferences_dialog.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/qt_prompts.hpp>
#include <subedit/gui/save_shape.hpp>
#include <subedit/gui/search_dialog.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/snap_dialog.hpp>
#include <subedit/gui/spell_check_dialog.hpp>
#include <subedit/gui/spell_check_settings_dialog.hpp>
#include <subedit/gui/split_project_dialog.hpp>
#include <subedit/gui/transform_dialog.hpp>
#include <subedit/gui/unsaved_documents_dialog.hpp>

#include <QAbstractButton>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QSpinBox>
#include <QString>
#include <QTableView>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QWidget>
#include <QWizard>
#include <QWizardPage>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "grid_fixtures.hpp"

namespace {

using subedit::core::Catalogue;
using subedit::core::EnvironmentLookup;
using subedit::core::FrameRate;
using subedit::core::StandardFrameRate;
using subedit::gui::InterfaceLanguage;

/// Where the fixture is installed, as `cmake/Translations.cmake` installs a catalogue.
const std::filesystem::path kLocale{SUBEDIT_TEST_LOCALE_DIR};

/// A Russian user's environment, the test's own and never the process's.
[[nodiscard]] EnvironmentLookup russian() {
    return [](std::string_view name) -> std::optional<std::string> {
        if (name == "LANG")
            return "ru_RU.UTF-8";
        return std::nullopt;
    };
}

/// The fixture, read directly: the test asks it what each English text should have become.
[[nodiscard]] Catalogue fixture() {
    auto catalogue = Catalogue::fromFile(kLocale / "ru" / "LC_MESSAGES" / "subedit.mo");
    REQUIRE(catalogue.has_value());
    return *std::move(catalogue);
}

/// The buttons Qt words itself — a box's standard buttons, a wizard's own — which follow Qt's
/// catalogue and not ours (spec 15, D5), and are left out of what a dialog is checked for.
[[nodiscard]] std::set<const QWidget*> qtWorded(const QWidget& root) {
    std::set<const QWidget*> worded;
    for (const QDialogButtonBox* box : root.findChildren<QDialogButtonBox*>()) {
        for (QAbstractButton* button : box->buttons())
            if (box->standardButton(button) != QDialogButtonBox::NoButton)
                worded.insert(button);
    }
    if (const auto* wizard = qobject_cast<const QWizard*>(&root)) {
        for (const QWizard::WizardButton which : {QWizard::BackButton,
                                                  QWizard::NextButton,
                                                  QWizard::CommitButton,
                                                  QWizard::FinishButton,
                                                  QWizard::CancelButton,
                                                  QWizard::HelpButton})
            worded.insert(wizard->button(which));
    }
    return worded;
}

/// The texts one widget shows, by what it is.
void appendTextsOf(const QWidget& widget, std::vector<QString>& texts) {
    const auto add = [&texts](const QString& text) {
        if (!text.isEmpty())
            texts.push_back(text);
    };
    add(widget.windowTitle());
    add(widget.toolTip());
    for (const QAction* action : widget.actions())
        add(action->text());
    if (const auto* label = qobject_cast<const QLabel*>(&widget))
        add(label->text());
    if (const auto* button = qobject_cast<const QAbstractButton*>(&widget))
        add(button->text());
    if (const auto* group = qobject_cast<const QGroupBox*>(&widget))
        add(group->title());
    if (const auto* line = qobject_cast<const QLineEdit*>(&widget))
        add(line->placeholderText());
    if (const auto* page = qobject_cast<const QWizardPage*>(&widget)) {
        add(page->title());
        add(page->subTitle());
    }
    if (const auto* combo = qobject_cast<const QComboBox*>(&widget)) {
        for (int index = 0; index < combo->count(); ++index)
            add(combo->itemText(index));
    }
    if (const auto* spin = qobject_cast<const QSpinBox*>(&widget)) {
        add(spin->suffix());
        add(spin->specialValueText());
    }
    if (const auto* spin = qobject_cast<const QDoubleSpinBox*>(&widget))
        add(spin->suffix());
    if (const auto* view = qobject_cast<const QAbstractItemView*>(&widget);
        view != nullptr && view->model() != nullptr) {
        const QAbstractItemModel& model = *view->model();
        for (int column = 0; column < model.columnCount(); ++column)
            add(model.headerData(column, Qt::Horizontal).toString());
    }
    if (const auto* table = qobject_cast<const QTableWidget*>(&widget)) {
        for (int row = 0; row < table->rowCount(); ++row)
            for (int column = 0; column < table->columnCount(); ++column)
                add(table->item(row, column)->text());
    }
}

/// Every text `root` shows that a catalogue may translate, in the order it was built.
[[nodiscard]] std::vector<QString> textsOf(const QWidget& root) {
    const std::set<const QWidget*> worded = qtWorded(root);
    std::vector<QString> texts;
    appendTextsOf(root, texts);
    for (const QWidget* child : root.findChildren<QWidget*>())
        if (!worded.contains(child))
            appendTextsOf(*child, texts);
    return texts;
}

[[nodiscard]] bool hasWord(const QString& text) {
    for (qsizetype at = 1; at < text.size(); ++at)
        if (text.at(at - 1).isLetter() && text.at(at).isLetter())
            return true;
    return false;
}

/// A dialog as a test builds it: the widget, and whatever must outlive it.
using Build = std::function<std::shared_ptr<QWidget>()>;

/// What `build` shows under English and under Russian, compared; returns how many of its texts
/// the fixture translated. `kept` are the texts that are data and stay as written.
std::size_t checkTranslated(const Build& build, const std::set<QString>& kept = {}) {
    std::vector<QString> english;
    {
        const std::shared_ptr<QWidget> shown = build();
        english = textsOf(*shown);
    }

    std::vector<QString> translated;
    {
        const InterfaceLanguage language{russian(), kLocale, QString{}};
        REQUIRE(language.language() == "ru");
        const std::shared_ptr<QWidget> shown = build();
        translated = textsOf(*shown);
    }

    const Catalogue catalogue = fixture();
    REQUIRE(translated.size() == english.size());
    std::size_t count = 0;
    for (std::size_t at = 0; at < english.size(); ++at) {
        const std::string source = english.at(at).toStdString();
        INFO(source);
        if (const std::optional<std::string_view> expected = catalogue.find({}, source)) {
            CHECK(translated.at(at).toStdString() == *expected);
            ++count;
        } else if (kept.contains(english.at(at)) || !hasWord(english.at(at))) {
            CHECK(translated.at(at) == english.at(at));
        } else {
            // Not a message of its own: a sentence assembled from some, or the core's words.
            CHECK(translated.at(at) != english.at(at));
        }
    }

    // **And no key of the fixture shown untranslated**, wherever it sits.
    for (const QString& shown : translated) {
        const std::optional<std::string_view> left = catalogue.find({}, shown.toStdString());
        INFO(shown.toStdString());
        CHECK((!left.has_value() || *left == shown.toStdString()));
    }
    CHECK(count > 0);
    return count;
}

/// A project of `count` subtitles, « Hello. » each, one a second.
[[nodiscard]] std::shared_ptr<subedit::core::Project> projectOf(int count) {
    auto project = std::make_shared<subedit::core::Project>();
    std::vector<subedit::core::Subtitle> subtitles;
    for (int index = 0; index < count; ++index) {
        const std::int64_t start = static_cast<std::int64_t>(index) * 1000;
        subtitles.push_back({.start = subedit::core::Timestamp::fromMilliseconds(start),
                             .end = subedit::core::Timestamp::fromMilliseconds(start + 900),
                             .mainText = "qqq"});
    }
    project->setSubtitles(std::move(subtitles));
    return project;
}

/// A widget together with what it borrows, destroyed after it.
template<typename Owned>
[[nodiscard]] std::shared_ptr<QWidget> holding(std::shared_ptr<Owned> owned,
                                               std::unique_ptr<QWidget> widget) {
    // In this order, so that the widget goes first.
    struct Both {
        std::shared_ptr<Owned> owned;
        std::unique_ptr<QWidget> widget;
    };

    auto both = std::make_shared<Both>(Both{std::move(owned), std::move(widget)});
    QWidget* shown = both->widget.get();
    return {std::move(both), shown};
}

/// The spelling languages a dialog lists: codes, « en_US », which the machine decides and no
/// catalogue translates.
[[nodiscard]] std::set<QString> spellCodesOf(const QWidget& dialog) {
    std::set<QString> codes;
    for (const QComboBox* combo : dialog.findChildren<QComboBox*>())
        for (int index = 0; index < combo->count(); ++index)
            if (combo->itemText(index).contains(
                    QRegularExpression{QStringLiteral("^[a-z]{2,3}(_[A-Z]{2})?$")}))
                codes.insert(combo->itemText(index));
    return codes;
}

[[nodiscard]] subedit::core::PatternCatalogue emptyCatalogue() {
    const subedit::core::InMemoryFileSystem files;
    return subedit::core::readPatternCatalogue(files, "/patterns", {});
}

} // namespace

TEST_CASE("every dialog shows its words in the chosen language", "[gui][i18n][GUI-I18N-01]") {
    SECTION("About subedit") {
        checkTranslated([] { return std::make_shared<subedit::gui::AboutDialog>(); });
    }
    SECTION("Preferences") {
        // A language is listed by its own name, which no catalogue translates (#669).
        checkTranslated(
            [] {
                return std::make_shared<subedit::gui::PreferencesDialog>(
                    subedit::core::Theme::System);
            },
            {QStringLiteral("English")});
    }
    SECTION("Correct Texts, every page") {
        const auto catalogue = std::make_shared<subedit::core::PatternCatalogue>(emptyCatalogue());
        const Build wizard = [catalogue] {
            return holding(
                catalogue,
                std::make_unique<subedit::gui::CorrectionWizard>(
                    *catalogue, nullptr, subedit::core::CorrectionSettings{}, true, true));
        };
        checkTranslated(wizard, spellCodesOf(*wizard()));
    }
    SECTION("Correct Texts, the table of proposed corrections") {
        checkTranslated([] {
            auto model = std::make_shared<subedit::gui::CorrectionResultModel>(
                std::vector<subedit::core::ProposedCorrection>{});
            auto view = std::make_unique<QTableView>();
            view->setModel(model.get());
            return holding(model, std::move(view));
        });
    }
    SECTION("Check Spelling") {
        checkTranslated([] {
            struct Held {
                std::shared_ptr<subedit::core::Project> project = projectOf(2);
                std::unique_ptr<subedit::core::SpellCheckWalk> walk;
            };
            auto held = std::make_shared<Held>();
            subedit::core::WordList list;
            list.words = {"ok"};
            subedit::core::WordListSpellProvider provider;
            provider.add("en", std::move(list));
            const subedit::core::InMemoryFileSystem files;
            held->walk = std::make_unique<subedit::core::SpellCheckWalk>(
                subedit::core::openSpellChecker(provider, "en", files, "/none.repl").value(),
                std::vector<subedit::core::CorrectionTarget>{
                    {.project = held->project.get(),
                     .selection = subedit::core::Selection::all(*held->project),
                     .document = subedit::core::Document::Main}});
            return holding(held, std::make_unique<subedit::gui::SpellCheckDialog>(*held->walk));
        });
    }
    SECTION("Spell-Check Settings") {
        const Build settings = [] {
            return std::make_shared<subedit::gui::SpellCheckSettingsDialog>(nullptr, true, true);
        };
        checkTranslated(settings, spellCodesOf(*settings()));
    }
    SECTION("Adjust Durations") {
        checkTranslated([] {
            return std::make_shared<subedit::gui::DurationAdjustDialog>(
                subedit::gui::OperationScope{4, 2}, subedit::core::DurationAdjustmentSettings{});
        });
    }
    SECTION("Convert Frame Rate, with everything it can say") {
        checkTranslated([] {
            return std::make_shared<subedit::gui::FrameRateDialog>(
                4,
                FrameRate{StandardFrameRate::Fps25},
                FrameRate{StandardFrameRate::Fps24},
                FrameRate{StandardFrameRate::Fps25},
                FrameRate{StandardFrameRate::Fps25});
        });
    }
    SECTION("Snap, Shift, Transform, Remove Hearing-Impaired Mentions") {
        checkTranslated([] {
            return std::make_shared<subedit::gui::SnapDialog>(4,
                                                              FrameRate{StandardFrameRate::Fps24});
        });
        checkTranslated([] { return std::make_shared<subedit::gui::ShiftDialog>(4); });
        checkTranslated([] { return std::make_shared<subedit::gui::TransformDialog>(4); });
        checkTranslated([] { return std::make_shared<subedit::gui::HearingImpairedDialog>(4); });
    }
    SECTION("an operation's preview") {
        checkTranslated([] {
            auto dialog = std::make_shared<subedit::gui::HearingImpairedDialog>(4);
            dialog->offerPreview([] {
                return subedit::gui::OperationPreview{
                    .rows = {{.number = QStringLiteral("1"),
                              .before = QStringLiteral("00:00:01,000"),
                              .after = QStringLiteral("00:00:02,000"),
                              .text = QStringLiteral("…")}},
                    .changed = 3};
            });
            dialog->findChild<QPushButton*>(QStringLiteral("preview-button"))->click();
            return dialog;
        });
    }
    SECTION("Insert Subtitles") {
        checkTranslated([] {
            return std::make_shared<subedit::gui::InsertDialog>(
                true, subedit::core::InsertPlacement::Below);
        });
    }
    SECTION("Make Ranges Disjoint, and Repair Subtitles") {
        checkTranslated([] {
            return std::make_shared<subedit::gui::DisjointDialog>(subedit::gui::DisjointChoice{});
        });
        checkTranslated([] {
            return std::make_shared<subedit::gui::DisjointDialog>(
                subedit::gui::DisjointChoice{}, nullptr, true);
        });
    }
    SECTION("the fixes proposed when a file opens") {
        checkTranslated(
            [] { return std::make_shared<subedit::gui::FixesProposalDialog>(1, 3, 5); });
    }
    SECTION("Frame Rate Analysis") {
        checkTranslated([] {
            return std::make_shared<subedit::gui::GridAnalysisDialog>(
                subedit::core::deduceFrameRate(subedit::test::gridStarts("melange-disperse.srt")));
        });
    }
    SECTION("Open Translation") {
        checkTranslated([] {
            return std::make_shared<subedit::gui::OpenTranslationDialog>(
                QStringLiteral("film.ru.srt"));
        });
    }
    SECTION("Find and Replace") {
        checkTranslated([] { return std::make_shared<subedit::gui::SearchDialog>(); });
    }
    SECTION("Split Project") {
        checkTranslated([] { return std::make_shared<subedit::gui::SplitProjectDialog>(10, 5); });
    }
    SECTION("the unsaved documents of a closing window") {
        const std::array<subedit::gui::ModifiedDocument, 2> documents{
            subedit::gui::ModifiedDocument{.document = subedit::core::Document::Main,
                                           .name = "film.srt"},
            subedit::gui::ModifiedDocument{.document = subedit::core::Document::Translation,
                                           .name = "film.ru.srt",
                                           .missing = true}};
        checkTranslated([&documents] {
            return std::make_shared<subedit::gui::UnsavedDocumentsDialog>(documents);
        });
    }
    SECTION("the fields Save As adds to the file dialog") {
        checkTranslated(
            [] {
                auto host = std::make_shared<QWidget>();
                new QGridLayout{host.get()};
                static_cast<void>(subedit::gui::addSaveShapeTo(
                    *host,
                    subedit::core::Encoding::utf8(subedit::core::ByteOrderMark::Absent),
                    subedit::core::Newline::Lf));
                return host;
            },
            {QStringLiteral("UTF-8 (Unicode)"),
             QStringLiteral("UTF-16LE (Unicode)"),
             QStringLiteral("UTF-16BE (Unicode)"),
             QStringLiteral("LF"),
             QStringLiteral("CRLF"),
             QStringLiteral("CR")});
    }
    SECTION("the manual, and what it says of a page it cannot read") {
        checkTranslated([] {
            auto files = std::make_shared<subedit::core::InMemoryFileSystem>();
            return holding(
                files, std::make_unique<subedit::gui::ManualWindow>(*files, "/opt/subedit/manual"));
        });
    }
}

TEST_CASE("a count takes the form the language calls for, in a dialog as in a message",
          "[gui][i18n][GUI-I18N-01]") {
    const InterfaceLanguage language{russian(), kLocale, QString{}};

    // One, three and five subtitles: Russian's three forms.
    const subedit::gui::FixesProposalDialog dialog{1, 3, 5};
    const QStringList lines = dialog.message().split(QLatin1Char('\n'));
    REQUIRE(lines.size() == 3);
    CHECK(lines.at(0).toStdString() == "Объединить повторы: 1 субтитр повторяет соседний");
    CHECK(lines.at(1).toStdString() == "Упорядочить субтитры: 3 субтитра не на своём месте");
    CHECK(lines.at(2).toStdString() ==
          "Разнести интервалы: 5 субтитров начинаются до конца предыдущего");
}

TEST_CASE("a translation may put a sentence's arguments in another order",
          "[gui][i18n][GUI-I18N-01]") {
    const InterfaceLanguage language{russian(), kLocale, QString{}};

    // « Measured on %1 starts, over %2. » — Russian says the span first.
    const subedit::gui::GridAnalysisDialog dialog{
        subedit::core::deduceFrameRate(subedit::test::gridStarts("melange-disperse.srt"))};
    const subedit::core::FrameRateDeduction deduction =
        subedit::core::deduceFrameRate(subedit::test::gridStarts("melange-disperse.srt"));
    const std::string summary = dialog.summary().toStdString();
    INFO(summary);
    const std::string starts = std::to_string(deduction.starts);
    CHECK(summary.find("За ") != std::string::npos);
    CHECK(summary.find(" измерено " + starts + " начал") != std::string::npos);
}

TEST_CASE("the prompts follow the language", "[gui][i18n][GUI-I18N-01]") {
    const InterfaceLanguage language{russian(), kLocale, QString{}};

    CHECK(subedit::gui::sentenceAboutUnsaved({.document = subedit::core::Document::Translation,
                                              .name = "film.ru.srt",
                                              .missing = true})
              .toStdString() == "Перевод больше не там, откуда был прочитан.");
    CHECK(subedit::gui::videoFilters().startsWith(QStringLiteral("Видео (")));
    CHECK(subedit::gui::videoFilters().endsWith(QStringLiteral(";;Все файлы (*)")));
    CHECK(subedit::gui::subtitleFilters().startsWith(QStringLiteral("Субтитры (")));

    std::ostringstream errors;
    subedit::gui::reportFatal(errors, "boom");
    CHECK(errors.str() == "subedit-gui — boom\n");
}

TEST_CASE("what stops the window is said behind its name", "[gui][i18n]") {
    std::ostringstream errors;
    subedit::gui::reportFatal(errors, "out of memory");
    CHECK(errors.str() == "subedit-gui: out of memory\n");
}

namespace {

/// A pattern file as it was: its bytes and the time it was last written.
struct FileState {
    std::string bytes;
    std::filesystem::file_time_type written;

    friend bool operator==(const FileState&, const FileState&) = default;
};

[[nodiscard]] std::map<std::filesystem::path, FileState>
statesOf(const std::vector<std::filesystem::path>& directories) {
    std::map<std::filesystem::path, FileState> states;
    for (const std::filesystem::path& directory : directories) {
        for (const auto& entry : std::filesystem::directory_iterator{directory}) {
            std::ifstream in{entry.path(), std::ios::binary};
            states.emplace(entry.path(),
                           FileState{.bytes = std::string{std::istreambuf_iterator<char>{in}, {}},
                                     .written = std::filesystem::last_write_time(entry.path())});
        }
    }
    return states;
}

/// The box `list` shows for the pattern whose English name is `name` — its identifier.
[[nodiscard]] const QCheckBox& boxOf(const subedit::gui::PatternList& list, const char* name) {
    const auto* box = list.findChild<QCheckBox*>(QString::fromUtf8(name));
    REQUIRE(box != nullptr);
    return *box;
}

} // namespace

TEST_CASE("Gaupol's patterns are shown in the language, a user's own as written",
          "[gui][i18n][GUI-I18N-04]") {
    // The shipped files, and one a user dropped: both read as they are (ADR 0037).
    const QTemporaryDir user;
    REQUIRE(user.isValid());
    const std::filesystem::path userDirectory = user.path().toStdString();
    std::ofstream{userDirectory / "Latn-en.common-error"}
        << "[Common Error Pattern]\nName=A pattern of my own\n"
           "Description=Mine, and in no catalogue\nClasses=OCR;\nPattern=a\nReplacement=b\n";
    const std::vector<std::filesystem::path> directories{SUBEDIT_PATTERNS_DIR, userDirectory};
    const auto before = statesOf(directories);

    const subedit::core::RealFileSystem files;
    const subedit::core::PatternCatalogue catalogue =
        subedit::core::readPatternCatalogue(files, SUBEDIT_PATTERNS_DIR, userDirectory);

    SECTION("in English, as their files say them") {
        subedit::gui::PatternList list{catalogue, subedit::core::PatternKind::CommonError};
        list.setCode("Latn-en", subedit::core::CorrectionSettings{});
        const QCheckBox& gaupol = boxOf(list, "Letter \"I\" in a lower case word");
        CHECK(gaupol.text().toStdString() == "Letter \"I\" in a lower case word");
        CHECK(gaupol.toolTip().toStdString() ==
              "Replace letter \"I\" with letter \"l\" in a lower case word");
        CHECK(boxOf(list, "A pattern of my own").text().toStdString() == "A pattern of my own");
    }
    SECTION("under the catalogue, Gaupol's translated and the user's as written") {
        const InterfaceLanguage language{russian(), kLocale, QString{}};
        subedit::gui::PatternList common{catalogue, subedit::core::PatternKind::CommonError};
        common.setCode("Latn-en", subedit::core::CorrectionSettings{});
        const QCheckBox& gaupol = boxOf(common, "Letter \"I\" in a lower case word");
        CHECK(gaupol.text().toStdString() == "Буква «I» в слове из строчных букв");
        CHECK(gaupol.toolTip().toStdString() ==
              "Заменить букву «I» на «l» в слове из строчных букв");
        const QCheckBox& own = boxOf(common, "A pattern of my own");
        CHECK(own.text().toStdString() == "A pattern of my own");
        CHECK(own.toolTip().toStdString() == "Mine, and in no catalogue");

        subedit::gui::PatternList capitals{catalogue, subedit::core::PatternKind::Capitalization};
        capitals.setCode("Latn-en", subedit::core::CorrectionSettings{});
        const QCheckBox& pronoun = boxOf(capitals, "First person pronoun");
        CHECK(pronoun.text().toStdString() == "Местоимение первого лица");
        CHECK(pronoun.toolTip().toStdString() == "Писать местоимение первого лица «I» с прописной");

        // A pattern that gave up is named the same way on the confirmation page.
        subedit::gui::CorrectionProgressPage progress;
        progress.setComputation([] {
            subedit::core::CorrectionProposal result;
            for (const char* name : {"Letter \"I\" in a lower case word", "A pattern of my own"})
                result.failures.push_back(subedit::core::PatternFailure{
                    .kind = subedit::core::FailureKind::Untranslatable,
                    .code = "Latn-en",
                    .rank = 1,
                    .name = name,
                    .text = std::nullopt,
                    .detail = ""});
            return result;
        });
        QSignalSpy spy{&progress, &subedit::gui::CorrectionProgressPage::completeChanged};
        progress.initializePage();
        REQUIRE(spy.wait(2000));
        subedit::gui::CorrectionConfirmationPage page;
        page.setProgressPage(&progress);
        page.initializePage();
        const std::string abandoned =
            page.findChild<QLabel*>(QStringLiteral("abandonedPatterns"))->text().toStdString();
        INFO(abandoned);
        CHECK(abandoned.find("Буква «I» в слове из строчных букв (") != std::string::npos);
        CHECK(abandoned.find("A pattern of my own (") != std::string::npos);
        CHECK(abandoned.find("lower case") == std::string::npos);
    }

    // The translation was looked up, never written: no pattern file has changed.
    CHECK(statesOf(directories) == before);
}
