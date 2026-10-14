// Every action of the window and the keyboard shortcuts it answers, one per line.
//
// **What `check-gui-manual.py` reads.** The manual of the window writes its shortcuts by hand,
// in tables, and nothing confronted them with what the window declares — the defect
// `check-cli-manual.py` settled for the command line. Here is the window's side of the
// comparison: the real `WindowActions`, built without a screen, and every action written as
// `<label>TAB<sequence>[, <sequence>…]` — the label as the menu shows it (the ampersand of the
// mnemonic removed), the sequences in the portable form Qt prints them in (`Ctrl+Shift+S`), and
// nothing after the tab for an action with none. **Those without one are listed too**: the
// manual says « none » for them, and that is a claim like any other.
//
// **In any language the window ships** (#672): `--language <name>` builds the actions under that
// language's catalogue, the one the build tree lays down beside `bin/` as an installation does,
// and the labels are then the ones that language's menus show — what a translated manual quotes.
// Without it, the window is English, the source language. A language with no catalogue of ours
// is refused (exit 1): a listing silently in English would compare a translated manual with the
// wrong window. Only our catalogue is asked for, not Qt's: every label listed is ours.
//
// **It lives here and not in src/exe** for the reason the other programs of this directory do:
// nothing that is delivered contains it, and no install rule names it.

#include <subedit/gui/interface_language.hpp>
#include <subedit/gui/window_actions.hpp>
#include <subedit/platform/locations.hpp>

#include <QAction>
#include <QApplication>
#include <QKeySequence>
#include <QLibraryInfo>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <Qt>
#include <qglobal.h>

#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <string_view>

int main(int argc, char** argv) {
    const bool withLanguage = argc == 3 && std::string_view{argv[1]} == "--language";
    if (argc != 1 && !withLanguage) {
        std::cerr << "usage: subedit_list_shortcuts [--language <name>]\n";
        return 2;
    }
    const std::optional<std::string> requested =
        withLanguage ? std::optional<std::string>{argv[2]} : std::nullopt;

    // Screenless, as the harness of the window tests is.
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");

    const QApplication application{argc, argv};

    // The language, before the actions are built, asked by name and never read from the
    // environment: the listing of a language does not depend on the locale of whoever runs it.
    std::optional<subedit::gui::InterfaceLanguage> language;
    if (requested) {
        language.emplace(
            requested,
            [](std::string_view) -> std::optional<std::string> { return std::nullopt; },
            subedit::platform::installedLocalePath(),
            QLibraryInfo::path(QLibraryInfo::TranslationsPath));
        if (language->language() != requested) {
            std::cerr << "subedit_list_shortcuts: the language \"" << *requested
                      << "\" has no catalogue under "
                      << subedit::platform::installedLocalePath().string() << "\n"
                      << "  build the catalogues (the subedit_locale target), or add the language\n"
                      << "  to src/po/LINGUAS\n";
            return 1;
        }
    }

    QObject owner;
    const subedit::gui::WindowActions actions{&owner};

    // A set: the order of `findChildren` is the order of construction, and a listing that
    // depended on it would change whenever somebody reordered the file.
    std::set<std::string> lines;
    for (const QAction* action : owner.findChildren<QAction*>()) {
        QStringList sequences;
        for (const QKeySequence& sequence : action->shortcuts())
            sequences << sequence.toString(QKeySequence::PortableText);
        QString label = action->text();
        label.remove(QLatin1Char('&'));
        // A separator, or an action with neither a name nor a key, says nothing.
        if (label.isEmpty() && sequences.isEmpty())
            continue;
        lines.insert(label.toStdString() + "\t" +
                     sequences.join(QStringLiteral(", ")).toStdString());
    }

    for (const std::string& line : lines)
        std::cout << line << "\n";
    return 0;
}
