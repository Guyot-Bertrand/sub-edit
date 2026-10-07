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
// **It lives here and not in src/exe** for the reason the other programs of this directory do:
// nothing that is delivered contains it, and no install rule names it.

#include <subedit/gui/window_actions.hpp>

#include <QAction>
#include <QApplication>
#include <QKeySequence>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <Qt>
#include <qglobal.h>

#include <iostream>
#include <set>
#include <string>

int main(int argc, char** argv) {
    // Screenless, as the harness of the window tests is.
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");

    const QApplication application{argc, argv};

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
