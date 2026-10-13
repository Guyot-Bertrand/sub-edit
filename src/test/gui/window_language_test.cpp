// The window under a second language — issue #664: the shell of the window (menus, actions,
// toolbar, columns, tabs, status line) follows the catalogue chosen at start-up, and English is
// what remains when the language asked for is not installed. Read through the fixture
// `data/i18n/window.po`, compiled as Russian, which has three plural forms.
//
// **The harness of #669 is this one**: `InterfaceLanguage` given an environment, a settings file
// and the fixture's locale directory, then a window built with no screen, as `main` builds it.
// The setting `interface.language` is proven here too (`GUI-I18N-02`): offered by `Preferences…`,
// kept in the settings file, and applied at the next launch — never to the window it was chosen in.

#include <subedit/core/config/settings.hpp>
#include <subedit/core/config/theme.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/i18n/catalogue.hpp>
#include <subedit/core/i18n/language.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/gui/diagnostics_button.hpp>
#include <subedit/gui/interface_language.hpp>
#include <subedit/gui/invocation.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/preferences_dialog.hpp>
#include <subedit/gui/status_line.hpp>
#include <subedit/gui/target.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QByteArray>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QStatusBar>
#include <QString>
#include <QTabBar>
#include <QTableView>
#include <QTemporaryDir>
#include <QToolBar>
#include <QToolButton>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::Catalogue;
using subedit::core::EnvironmentLookup;
using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::gui::InterfaceLanguage;
using subedit::gui::MainWindow;

/// Where the fixture is installed, as `cmake/Translations.cmake` installs a catalogue.
const std::filesystem::path kLocale{SUBEDIT_TEST_LOCALE_DIR};

/// An environment of the test's own, never the process's.
[[nodiscard]] EnvironmentLookup
environmentOf(std::map<std::string, std::string, std::less<>> variables) {
    return [variables = std::move(variables)](std::string_view name) -> std::optional<std::string> {
        const auto found = variables.find(name);
        if (found == variables.end())
            return std::nullopt;
        return found->second;
    };
}

/// A Russian user's environment.
[[nodiscard]] EnvironmentLookup russian() {
    return environmentOf({{"LANG", "ru_RU.UTF-8"}});
}

/// Appends `value` to `out`, most significant byte first, as Qt's catalogues are written.
void appendBigEndian(QByteArray& out, std::uint32_t value) {
    for (int shift = 24; shift >= 0; shift -= 8)
        out.append(static_cast<char>((value >> static_cast<unsigned>(shift)) & 0xFFU));
}

/// Qt's hash of a source text, by which its catalogues index their messages.
[[nodiscard]] std::uint32_t qtHash(std::string_view text) {
    std::uint32_t hash = 0;
    for (const char character : text) {
        hash = (hash << 4U) + static_cast<unsigned char>(character);
        const std::uint32_t high = hash & 0xF0000000U;
        if (high != 0)
            hash ^= high >> 24U;
        hash &= ~high;
    }
    return hash == 0 ? 1 : hash;
}

/// A Qt catalogue of one message, written by hand: `lrelease`, the tool that writes them, is not
/// part of the toolchain, and the test must not lean on the catalogues of the machine it runs on.
[[nodiscard]] QByteArray
qtCatalogueOf(std::string_view context, std::string_view source, const QString& translation) {
    constexpr std::array<unsigned char, 16> kMagic = {0x3c,
                                                      0xb8,
                                                      0x64,
                                                      0x18,
                                                      0xca,
                                                      0xef,
                                                      0x9c,
                                                      0x95,
                                                      0xcd,
                                                      0x21,
                                                      0x1c,
                                                      0xbf,
                                                      0x60,
                                                      0xa1,
                                                      0xbd,
                                                      0xdd};
    constexpr char kHashes = 0x42;
    constexpr char kMessages = 0x69;
    constexpr char kTranslation = 3;
    constexpr char kSource = 6;
    constexpr char kContext = 7;
    constexpr char kEnd = 1;

    QByteArray utf16;
    for (const QChar character : translation) {
        utf16.append(static_cast<char>(character.unicode() >> 8U));
        utf16.append(static_cast<char>(character.unicode() & 0xFFU));
    }

    QByteArray message;
    message.append(kTranslation);
    appendBigEndian(message, static_cast<std::uint32_t>(utf16.size()));
    message.append(utf16);
    message.append(kSource);
    appendBigEndian(message, static_cast<std::uint32_t>(source.size()));
    message.append(source.data(), static_cast<qsizetype>(source.size()));
    message.append(kContext);
    appendBigEndian(message, static_cast<std::uint32_t>(context.size()));
    message.append(context.data(), static_cast<qsizetype>(context.size()));
    message.append(kEnd);

    QByteArray hashes;
    appendBigEndian(hashes, qtHash(source));
    appendBigEndian(hashes, 0);

    QByteArray catalogue;
    for (const unsigned char byte : kMagic)
        catalogue.append(static_cast<char>(byte));
    catalogue.append(kHashes);
    appendBigEndian(catalogue, static_cast<std::uint32_t>(hashes.size()));
    catalogue.append(hashes);
    catalogue.append(kMessages);
    appendBigEndian(catalogue, static_cast<std::uint32_t>(message.size()));
    catalogue.append(message);
    return catalogue;
}

/// A directory holding Qt's Russian catalogue, reduced to the word of one standard button.
class QtRussian {
public:
    QtRussian() {
        REQUIRE(m_directory.isValid());
        QFile file{m_directory.filePath(QStringLiteral("qtbase_ru.qm"))};
        REQUIRE(file.open(QIODevice::WriteOnly));
        REQUIRE(file.write(qtCatalogueOf("QPlatformTheme", "Cancel", QStringLiteral("Отмена"))) >
                0);
    }

    [[nodiscard]] QString path() const { return m_directory.path(); }

private:
    QTemporaryDir m_directory;
};

/// The fixture, read directly: the test asks it what each English text should have become.
[[nodiscard]] Catalogue fixture() {
    auto catalogue = Catalogue::fromFile(kLocale / "ru" / "LC_MESSAGES" / "subedit.mo");
    REQUIRE(catalogue.has_value());
    return *std::move(catalogue);
}

/// Every text of the shell that a catalogue may translate, in the order the window builds it:
/// the menus' titles and their entries, submenus included; the toolbar's words; the columns.
[[nodiscard]] std::vector<QString> shellTexts(MainWindow& window) {
    std::vector<QString> texts;
    const auto walk = [&texts](const auto& self, const QList<QAction*>& actions) -> void {
        for (const QAction* action : actions) {
            if (action->isSeparator())
                continue;
            texts.push_back(action->text());
            if (const QMenu* menu = action->menu())
                self(self, menu->actions());
        }
    };
    walk(walk, window.menuBar()->actions());

    // The toolbar's title is not among them: it is the one word the catalogue holds under a
    // context, checked on its own below.
    for (const QToolBar* bar : window.findChildren<QToolBar*>()) {
        for (const QAction* action : bar->actions())
            if (!action->isSeparator())
                texts.push_back(action->iconText());
    }

    const QAbstractItemModel* model = window.findChild<QTableView*>()->model();
    for (int column = 0; column < model->columnCount(); ++column)
        texts.push_back(model->headerData(column, Qt::Horizontal).toString());

    return texts;
}

[[nodiscard]] std::vector<QString> menuTitles(const MainWindow& window) {
    std::vector<QString> titles;
    for (const QAction* action : window.menuBar()->actions())
        titles.push_back(action->text());
    return titles;
}

/// A window with nothing open, as a user meets it.
struct EmptyWindow {
    InMemoryFileSystem files;
    subedit::test::FakePrompts prompts;
    MainWindow window{files, OpenedFile{}, prompts};
};

/// `count` diagnostics of as many kinds: the button counts the lines it lists, one per kind.
[[nodiscard]] std::vector<subedit::core::Diagnostic> diagnostics(int count) {
    std::vector<subedit::core::Diagnostic> found;
    found.reserve(static_cast<std::size_t>(count));
    for (int kind = 0; kind < count; ++kind)
        found.push_back({.severity = subedit::core::Severity::Recovered,
                         .line = 1,
                         .kind = static_cast<subedit::core::DiagnosticKind>(kind),
                         .detail = {}});
    return found;
}

} // namespace

TEST_CASE("the window shows its menus, toolbar and columns in the chosen language",
          "[gui][i18n][GUI-I18N-01]") {
    std::vector<QString> english;
    {
        EmptyWindow shell;
        english = shellTexts(shell.window);
    }

    const InterfaceLanguage language{russian(), kLocale, QString{}};
    REQUIRE(language.language() == "ru");
    EmptyWindow shell;

    // Every menu of the bar, by the word the catalogue gives it, accelerator and all.
    CHECK(menuTitles(shell.window) == std::vector<QString>{QStringLiteral("&Файл"),
                                                           QStringLiteral("&Правка"),
                                                           QStringLiteral("&Вид"),
                                                           QStringLiteral("В&идео"),
                                                           QStringLiteral("&Инструменты"),
                                                           QStringLiteral("П&роекты"),
                                                           QStringLiteral("&Справка")});

    // **And no text the catalogue holds left in English**, wherever the window shows it: each
    // English text of the shell is either translated as the catalogue says, or one the
    // catalogue does not hold and therefore shown as written.
    const Catalogue catalogue = fixture();
    const std::vector<QString> russianTexts = shellTexts(shell.window);
    REQUIRE(russianTexts.size() == english.size());
    std::size_t translated = 0;
    for (std::size_t at = 0; at < english.size(); ++at) {
        const std::string source = english.at(at).toStdString();
        const std::optional<std::string_view> expected = catalogue.find({}, source);
        INFO(source);
        if (expected.has_value()) {
            CHECK(russianTexts.at(at).toStdString() == *expected);
            ++translated;
        } else {
            CHECK(russianTexts.at(at) == english.at(at));
        }
    }
    // The fixture's sample, met in the shell: what a vacuous walk would not count.
    CHECK(translated >= 36);

    // The two words the catalogue tells apart by a context, each where it belongs.
    CHECK(shell.window.findChild<QToolBar*>()->windowTitle() == QStringLiteral("Основная"));
    CHECK(subedit::gui::documentName(subedit::core::Document::Main) == QStringLiteral("Основной"));

    // What only a pointer shows: tool tips, the tab of a project with no name yet.
    const auto* const newProject =
        shell.window.menuBar()->actions().front()->menu()->actions().front();
    CHECK(newProject->toolTip() == QStringLiteral("Открыть пустой проект в новой вкладке"));
    CHECK(shell.window.tabBar()->tabText(0) == QStringLiteral("без имени"));
}

TEST_CASE("the status line follows the language, a count taking the form it calls for",
          "[gui][i18n][GUI-I18N-01]") {
    const InterfaceLanguage language{russian(), kLocale, QString{}};

    QStatusBar bar;
    subedit::gui::StatusLine line{bar};
    line.showTarget(subedit::core::Document::Translation);
    const QList<QLabel*> labels = bar.findChildren<QLabel*>();
    CHECK(std::ranges::any_of(labels, [](const QLabel* label) {
        return label->text() == QStringLiteral("Текст: Перевод");
    }));

    // 1 takes the first form, 3 the second, 5 the third.
    subedit::gui::DiagnosticsButton button;
    button.setDiagnostics(diagnostics(1));
    CHECK(button.text().toStdString() == "1 замечание");
    button.setDiagnostics(diagnostics(3));
    CHECK(button.text().toStdString() == "3 замечания (форма 2–4)");
    button.setDiagnostics(diagnostics(5));
    CHECK(button.text().toStdString() == "5 замечаний");
}

TEST_CASE("the window is English when the language asked for is not installed",
          "[gui][i18n][GUI-I18N-01]") {
    const auto english = [](const EnvironmentLookup& environment) {
        const InterfaceLanguage language{environment, kLocale, QString{}};
        CHECK_FALSE(language.language().has_value());
        CHECK_FALSE(language.hasQtCatalogue());
        const EmptyWindow shell;
        CHECK(menuTitles(shell.window).front() == QStringLiteral("&File"));
    };

    SECTION("a language nobody installed") {
        english(environmentOf({{"LANGUAGE", "de"}, {"LANG", "de_DE.UTF-8"}}));
    }
    SECTION("LC_ALL=C, whatever LANGUAGE asks") {
        english(environmentOf({{"LC_ALL", "C"}, {"LANGUAGE", "ru"}}));
    }
    SECTION("nothing asked at all") {
        english(environmentOf({}));
    }
}

TEST_CASE("English comes back when the language goes", "[gui][i18n][GUI-I18N-01]") {
    { const InterfaceLanguage language{russian(), kLocale, QString{}}; }

    const EmptyWindow shell;
    CHECK(menuTitles(shell.window).front() == QStringLiteral("&File"));
}

TEST_CASE("Qt's own words follow the language when Qt has a catalogue for it",
          "[gui][i18n][GUI-I18N-01]") {
    SECTION("installed: the standard buttons are translated") {
        const QtRussian qt;
        const InterfaceLanguage language{russian(), kLocale, qt.path()};
        CHECK(language.hasQtCatalogue());

        const QDialogButtonBox box{QDialogButtonBox::Cancel};
        CHECK(box.button(QDialogButtonBox::Cancel)->text() == QStringLiteral("Отмена"));
    }
    SECTION("absent: the window is translated, the buttons stay English, nothing is said") {
        const QTemporaryDir empty;
        const InterfaceLanguage language{russian(), kLocale, empty.path()};
        CHECK(language.language() == "ru");
        CHECK_FALSE(language.hasQtCatalogue());

        const QDialogButtonBox box{QDialogButtonBox::Cancel};
        CHECK(box.button(QDialogButtonBox::Cancel)->text() == QStringLiteral("Cancel"));
    }

    // And gone with the language: the next box is English again.
    const QDialogButtonBox box{QDialogButtonBox::Cancel};
    CHECK(box.button(QDialogButtonBox::Cancel)->text() == QStringLiteral("Cancel"));
}

TEST_CASE("the test binary itself starts in English, whatever the machine says", "[gui][i18n]") {
    // The process's own environment, beside the catalogues of the build tree, where `fr` is
    // installed: the harness asks for English (spec 15, D4), and English is what comes.
    const InterfaceLanguage language;
    CHECK_FALSE(language.language().has_value());
}

TEST_CASE("a message's arguments are substituted in one pass, whatever they hold", "[gui][i18n]") {
    CHECK(subedit::gui::substitute(QStringLiteral("%2 before %1"),
                                   QStringLiteral("%2"),
                                   QStringLiteral("é")) == QStringLiteral("é before %2"));
    CHECK(subedit::gui::translateIn("toolbar", "Main") == QStringLiteral("Main"));
}

// ## The setting `interface.language` — issue #669

namespace {

using subedit::gui::LanguageChoice;
using subedit::gui::PreferencesDialog;

/// Where the settings file is, in memory: a test never resolves a location, it is given one.
constexpr const char* kSettings = "/config/subedit/settings.conf";

/// An English user's environment.
[[nodiscard]] EnvironmentLookup english() {
    return environmentOf({{"LANG", "en_US.UTF-8"}});
}

/// The language the window starts in, as `main` chooses it: the settings in `files`, then
/// `environment`, against the fixture's catalogues.
[[nodiscard]] std::optional<std::string> startedIn(const InMemoryFileSystem& files,
                                                   const EnvironmentLookup& environment) {
    const InterfaceLanguage language{
        subedit::gui::settingsLanguage(files, kSettings), environment, kLocale, QString{}};
    const EmptyWindow shell;
    // The window follows the language chosen, whatever it is.
    CHECK(menuTitles(shell.window).front() ==
          (language.language() == "ru" ? QStringLiteral("&Файл") : QStringLiteral("&File")));
    return language.language();
}

/// A settings file holding `content`.
[[nodiscard]] InMemoryFileSystem settingsOf(const std::string& content) {
    InMemoryFileSystem files;
    files.addFile(kSettings, content);
    return files;
}

/// The texts of a combo box, in order.
[[nodiscard]] std::vector<QString> entriesOf(const QComboBox& box) {
    std::vector<QString> entries;
    entries.reserve(static_cast<std::size_t>(box.count()));
    for (int index = 0; index < box.count(); ++index)
        entries.push_back(box.itemText(index));
    return entries;
}

/// Picks `language` in the dialog the window opens, when it is `Preferences…`.
void pick(subedit::test::FakePrompts& prompts, const QString& language) {
    prompts.fill = [language](QDialog& dialog) {
        if (auto* preferences = dynamic_cast<PreferencesDialog*>(&dialog)) {
            QComboBox& box = *preferences->languageBox();
            box.setCurrentIndex(box.findData(language));
        }
    };
}

} // namespace

TEST_CASE("Preferences offers the system's language and every installed one, by its own name",
          "[gui][i18n][GUI-I18N-02]") {
    const PreferencesDialog dialog{
        subedit::core::Theme::System,
        {},
        {},
        LanguageChoice{.installed = subedit::core::availableLanguages(kLocale)}};

    // English, the source, is always there; the fixture installs Russian.
    CHECK(entriesOf(*dialog.languageBox()) == std::vector<QString>{QStringLiteral("System default"),
                                                                   QStringLiteral("English"),
                                                                   QStringLiteral("Русский")});
    CHECK(dialog.languageBox()->currentIndex() == 0);
    CHECK_FALSE(dialog.interfaceLanguage().has_value());

    // And it says when the choice applies.
    const QList<QLabel*> labels = dialog.findChildren<QLabel*>();
    CHECK(std::ranges::any_of(labels, [](const QLabel* label) {
        return label->text() == QStringLiteral("Takes effect at the next launch.");
    }));

    dialog.languageBox()->setCurrentIndex(2);
    CHECK(dialog.interfaceLanguage() == "ru");
    dialog.languageBox()->setCurrentIndex(1);
    CHECK(dialog.interfaceLanguage() == "en");
}

TEST_CASE("Preferences opens on the language the settings name, kept even when not installed",
          "[gui][i18n][GUI-I18N-02]") {
    const PreferencesDialog russian{
        subedit::core::Theme::System, {}, {}, LanguageChoice{.chosen = "ru", .installed = {"ru"}}};
    CHECK(russian.languageBox()->currentText() == QStringLiteral("Русский"));
    CHECK(russian.interfaceLanguage() == "ru");

    // A setting brought from another machine is shown as it is, and given back unchanged.
    const PreferencesDialog german{
        subedit::core::Theme::System, {}, {}, LanguageChoice{.chosen = "de"}};
    CHECK(german.languageBox()->currentText() == QStringLiteral("Deutsch"));
    CHECK(german.interfaceLanguage() == "de");
    const PreferencesDialog unknown{
        subedit::core::Theme::System, {}, {}, LanguageChoice{.chosen = "xx"}};
    CHECK(unknown.languageBox()->currentText() == QStringLiteral("xx"));
    CHECK(unknown.interfaceLanguage() == "xx");
}

TEST_CASE("the language chosen is kept to the next session, and applies at the next launch",
          "[gui][i18n][GUI-I18N-02]") {
    InMemoryFileSystem settingsFiles;
    std::ostringstream errors;
    {
        EmptyWindow first;
        first.window.setLocaleDirectory(kLocale);
        pick(first.prompts, QStringLiteral("ru"));
        first.prompts.nextRun = true;
        first.window.preferencesAction()->trigger();

        // Not applied to the window it was chosen in: it was built in English and stays so.
        CHECK(menuTitles(first.window).front() == QStringLiteral("&File"));
        CHECK(first.window.settings().interfaceLanguage == "ru");
        subedit::gui::writeUserSettings(settingsFiles, kSettings, first.window.settings(), errors);
    }

    // The next launch: an English environment, and the setting wins.
    CHECK(startedIn(settingsFiles, english()) == "ru");

    // Read back by the window, and handed back to the file it came from.
    EmptyWindow second;
    second.window.applySettings(subedit::gui::readUserSettings(settingsFiles, kSettings, errors));
    CHECK(second.window.settings().interfaceLanguage == "ru");
    CHECK(errors.str().empty());
}

TEST_CASE("main reads the language where the window's settings are written",
          "[gui][i18n][GUI-I18N-02]") {
    // In memory, at the location the harness moved (`XDG_CONFIG_HOME`): the resolving overloads
    // are used as keys and nothing else, as `configureFromEnvironment`'s test uses them.
    InMemoryFileSystem files;
    std::ostringstream errors;
    subedit::gui::writeUserSettings(
        files, subedit::core::Settings{.interfaceLanguage = "ru"}, errors);

    CHECK(subedit::gui::settingsLanguage(files) == "ru");
    CHECK(errors.str().empty());
}

TEST_CASE("cancelled preferences leave the interface language as it was",
          "[gui][i18n][GUI-I18N-02]") {
    EmptyWindow shell;
    shell.window.setLocaleDirectory(kLocale);
    shell.window.applySettings(subedit::core::Settings{.interfaceLanguage = "en"});
    pick(shell.prompts, QStringLiteral("ru"));
    shell.prompts.nextRun = false;

    shell.window.preferencesAction()->trigger();

    CHECK(shell.window.settings().interfaceLanguage == "en");
}

TEST_CASE("the setting wins over the environment, and system follows it",
          "[gui][i18n][GUI-I18N-02]") {
    SECTION("a language installed, under an English environment") {
        CHECK(startedIn(settingsOf("interface.language = ru\n"), english()) == "ru");
    }
    SECTION("a language installed, under LC_ALL=C") {
        CHECK(startedIn(settingsOf("interface.language = ru\n"),
                        environmentOf({{"LC_ALL", "C"}})) == "ru");
    }
    SECTION("English, under a Russian environment") {
        CHECK_FALSE(startedIn(settingsOf("interface.language = en\n"), russian()).has_value());
    }
    SECTION("system, under a Russian environment and under an English one") {
        CHECK(startedIn(settingsOf("interface.language = system\n"), russian()) == "ru");
        CHECK_FALSE(startedIn(settingsOf("interface.language = system\n"), english()).has_value());
    }
    SECTION("no settings file at all") {
        CHECK(startedIn(InMemoryFileSystem{}, russian()) == "ru");
    }
}

TEST_CASE("a language that is not installed, or a value that cannot be read, leaves the default",
          "[gui][i18n][GUI-I18N-02]") {
    SECTION("not installed: the environment decides, silently") {
        const InMemoryFileSystem files = settingsOf("interface.language = de\n");
        CHECK_FALSE(startedIn(files, english()).has_value());
        CHECK(startedIn(files, russian()) == "ru");

        std::ostringstream errors;
        CHECK(subedit::gui::readUserSettings(files, kSettings, errors).interfaceLanguage == "de");
        CHECK(errors.str().empty());
    }
    SECTION("unreadable: system, and the diagnostic every setting gives") {
        const InMemoryFileSystem files = settingsOf("interface.language = Klingon\n");
        CHECK_FALSE(subedit::gui::settingsLanguage(files, kSettings).has_value());
        CHECK_FALSE(startedIn(files, english()).has_value());
        CHECK(startedIn(files, russian()) == "ru");

        std::ostringstream errors;
        CHECK_FALSE(
            subedit::gui::readUserSettings(files, kSettings, errors).interfaceLanguage.has_value());
        CHECK(errors.str() == "subedit-gui: /config/subedit/settings.conf: interface.language: "
                              "\"Klingon\" cannot be read, keeping the default\n");
    }
}
