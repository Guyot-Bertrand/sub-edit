// The screenshots of the manual, taken from the real window.
//
// **What this program is.** The manual of the command line carries `console`
// blocks that `generate-manual.sh` rewrites by running the binary: that is what
// keeps them from lying. The manual of the window had no counterpart, and #116
// then #161 wrote it out by hand each time — a window does not fit in a
// `console` block. Here is the counterpart: the window is built, shown, and
// photographed.
//
// **It never writes a reference.** Every shot is named `<name>.new.png`, and it
// is `compare-screenshots.py` that decides afterwards whether to promote it or
// throw it away. An image therefore enters a diff only on the day the window
// changed.
//
// **It is authoritative only under the settings it lays down itself** — a
// platform with no screen, the Fusion style, a named font, a fixed window size.
// Those are the four things that make one version of the code give the same
// image on two machines, and the font is the only one it cannot manufacture: it
// refuses rather than photograph under a stand-in. See ADR 0024.
//
// **It lives here and not in src/exe** for the reason the other programs of
// this directory do: nothing that is delivered contains it, and no install rule
// names it.

#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/config/insert_placement.hpp>
#include <subedit/core/config/theme.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/gui/grid_analysis_dialog.hpp>
#include <subedit/gui/insert_dialog.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/manual_window.hpp>
#include <subedit/gui/qt_prompts.hpp>
#include <subedit/gui/save_shape.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/theme.hpp>

#include <QAbstractItemView>
#include <QApplication>
#include <QByteArray>
#include <QFileDialog>
#include <QFont>
#include <QFontInfo>
#include <QHeaderView>
#include <QMessageLogContext>
#include <QModelIndex>
#include <QPixmap>
#include <QRect>
#include <QSplitter>
#include <QString>
#include <QStyleFactory>
#include <QWidget>
#include <QtGlobal>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace {

/// The font the shots are authoritative under.
///
/// DejaVu Sans rather than "the system font": it is on every desktop Linux
/// distribution — `fonts-dejavu-core` — and it is the one thing in this file a
/// machine may not have. Naming a font Qt would silently replace would come to
/// naming none.
constexpr const char* kFontFamily = "DejaVu Sans";
constexpr int kFontPointSize = 10;

/// The size of the window that is photographed.
///
/// Fixed, and not left to `sizeHint()`: the window opens large enough to read
/// the table — #211 — but "large enough" depends on the font, and so on the
/// machine. A shot whose dimensions vary is a shot the comparator promotes
/// every single time.
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 800;

/// The height of the shots that show nothing but the table.
///
/// The window is tall so that the table is; the empty video strip beside it
/// does not enter the image, since the table is what is being photographed.
constexpr int kTallWindowHeight = 900;

/// The height the table needs to show every one of its rows.
[[nodiscard]] int heightOfEveryRow(const subedit::gui::SubtitleTable& table) {
    int needed = table.horizontalHeader()->height() + 2;
    for (int row = 0; row < table.model()->rowCount(); ++row)
        needed += table.rowHeight(row);
    return needed;
}

/// Shows the window and gives the table exactly its rows, no more and no less.
///
/// Two reasons, and the second is the one that counts. The window shares its
/// height between the video strip and the table, and it is the strip that takes
/// the give — enlarging it enlarges the emptiness. And a table taller than what
/// it holds photographs mostly white: what is left under the last row teaches
/// nobody anything, and a shot fits its subject the way an image of a manual
/// does.
///
/// The share is set where a user sets it, by dragging the handle. The three
/// numbers are the three children of the splitter, in order: the video view,
/// the invitation to choose one, the table.
void showWithTheTableFitted(subedit::gui::MainWindow& window) {
    window.resize(kWindowWidth, kTallWindowHeight);
    window.show();
    QApplication::processEvents();

    const int needed = heightOfEveryRow(*window.table());
    if (auto* split = window.findChild<QSplitter*>(); split != nullptr)
        split->setSizes({0, kTallWindowHeight, needed});
    QApplication::processEvents();
}

/// The `Text` column, the fifth — see docs/manual/subedit-gui/table.md.
constexpr int kTextColumn = 4;

/// The row whose text is shown open: the third, because it carries two lines
/// and that is what the section explains.
constexpr int kEditedRow = 2;

/// The analysis dialog, tall enough for its eight candidates.
constexpr int kDialogWidth = 330;
constexpr int kDialogHeight = 430;

/// The save dialog, and the height of the strip kept out of it.
///
/// **A fixed size and a fixed strip**, because those are the only two things
/// that make the image the same twice: the dialog takes its height from this
/// number, and the six bottom rows take theirs from the font, which this
/// program names. What sits above — directories, dates, sidebar shortcuts —
/// depends on the machine, and that is precisely what the strip leaves out.
///
/// A hundred and sixty pixels: the hundred and fifty-three the six rows take at
/// that size, and a little air above them.
constexpr int kSaveAsWidth = 760;
constexpr int kSaveAsHeight = 500;
constexpr int kSaveAsStripHeight = 160;

/// The manual window, wide enough for its tables.
///
/// Smaller than the size it opens at: the shot shows enough to recognise the
/// window, and an image nine hundred pixels wide reads less well in a page of
/// the manual than one of seven hundred.
constexpr int kManualWidth = 700;
constexpr int kManualHeight = 480;

/// The manual the photographed window shows: the one of the repository.
///
/// **The real manual and not one made for the picture.** The installed copy is
/// this one page for page — `cmake/Installation.cmake` lays down the whole of
/// `docs/manual/` — so the shot shows what a reader will see.
[[nodiscard]] std::filesystem::path manualDirectory() {
    return std::filesystem::path{SUBEDIT_MANUAL_DIR};
}

/// The one sentence the screenless platform repeats at every window shown.
///
/// True, and of no consequence: nothing here has a window manager to propagate
/// a size to. The harness of the window tests silences it the same way and for
/// the same reason — a warning repeated six times is a warning one stops
/// reading.
constexpr const char* kOffscreenSizeHints = "This plugin does not support propagateSizeHints()";

void withoutOffscreenNoise(QtMsgType type, const QMessageLogContext& context, const QString& text) {
    if (text == QLatin1StringView{kOffscreenSizeHints})
        return;

    qt_message_output(type, context, text);
}

[[nodiscard]] std::filesystem::path corpus(const std::string& relative) {
    return std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / relative;
}

/// Writes a shot already taken, under the name `compare-screenshots.py` expects.
[[nodiscard]] bool
writeShot(const QPixmap& shot, const std::filesystem::path& directory, const std::string& name) {
    const std::filesystem::path target = directory / (name + ".new.png");

    if (shot.isNull() || !shot.save(QString::fromStdString(target.string()), "PNG")) {
        std::cerr << "subedit_screenshots: " << target.string() << ": nothing written\n";
        return false;
    }

    std::cout << "  " << target.filename().string() << " — " << shot.width() << "×" << shot.height()
              << "\n";
    return true;
}

/// Photographs `subject` under the name given, never touching the reference.
///
/// `shown` is what is shown, `subject` what is photographed — often the same,
/// sometimes not: the table alone says better what the table does, and the
/// whole window has to be shown all the same for the table to exist at all. A
/// window that was never shown has no real geometry and its layout has not
/// run: that is what #191 changed, and without it there would be nothing to
/// photograph.
[[nodiscard]] bool capture(QWidget& shown,
                           QWidget& subject,
                           const std::filesystem::path& directory,
                           const std::string& name) {
    shown.show();
    // The layout has really run only once the events are through: without this,
    // what is photographed is a window whose sizes are still the ones its
    // constructor gave it.
    QApplication::processEvents();

    return writeShot(subject.grab(), directory, name);
}

/// The bottom strip of a « Save As… » dialog, under the theme already applied.
///
/// The dialog is the one the window opens — `saveDialogFor` — so what is
/// photographed is what the user sees, alignment included.
[[nodiscard]] bool captureSaveAsStrip(const std::filesystem::path& directory,
                                      const std::string& name) {
    const subedit::core::SourceFile source;
    const std::unique_ptr<QFileDialog> dialog = subedit::gui::saveDialogFor(
        source, subedit::core::Encoding::utf8(subedit::core::ByteOrderMark::Absent), nullptr);

    dialog->resize(kSaveAsWidth, kSaveAsHeight);
    dialog->show();
    QApplication::processEvents();

    const QRect strip{0, kSaveAsHeight - kSaveAsStripHeight, kSaveAsWidth, kSaveAsStripHeight};
    return writeShot(dialog->grab(strip), directory, name);
}

/// A window opened on the fixture named.
///
/// Returned by value and built in one stroke: `MainWindow` is neither copyable
/// nor movable, so only the guaranteed elision of a prvalue gets one out of a
/// function. The size is set afterwards, by the caller.
[[nodiscard]] subedit::gui::MainWindow windowOn(subedit::core::FileSystem& files,
                                                subedit::gui::Prompts& prompts,
                                                const std::string& fixture) {
    return subedit::gui::MainWindow{
        files, subedit::core::openProject(files, corpus(fixture)).value(), prompts, {}, {}};
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3 || std::string{argv[1]} != "--output-dir") {
        std::cerr << "usage: subedit_screenshots --output-dir <directory>\n";
        return 2;
    }
    const std::filesystem::path directory{argv[2]};

    // Screenless, as the harness of the window tests is: a shot asks for no
    // display server, and the CI has none.
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");

    // Set before the `QApplication`, which warns on its own account too.
    qInstallMessageHandler(withoutOffscreenNoise);

    const QApplication application{argc, argv};

    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    const QFont font{QLatin1StringView{kFontFamily}, kFontPointSize};
    QApplication::setFont(font);

    // Refuse rather than photograph under a stand-in: a shot taken under
    // another font would be promoted by whoever came next, and the reference
    // would swing with whoever ran the command.
    const QFontInfo actual{font};
    if (actual.family() != QLatin1StringView{kFontFamily}) {
        std::cerr << "subedit_screenshots: the font \"" << kFontFamily
                  << "\" is missing, replaced by \"" << actual.family().toStdString() << "\"\n"
                  << "  the shots are authoritative under that font alone; install it\n"
                  << "  (Debian, Ubuntu: fonts-dejavu-core)\n";
        return 1;
    }

    std::error_code failed;
    std::filesystem::create_directories(directory, failed);
    if (failed) {
        std::cerr << "subedit_screenshots: " << directory.string() << " : " << failed.message()
                  << "\n";
        return 1;
    }

    subedit::core::RealFileSystem files;
    subedit::gui::QtPrompts prompts;

    bool written = true;

    // **Every screen is photographed twice, under the two palettes the
    // application lays down** — decision D4 of the scoping of phase 7. They are
    // reachable precisely because "light" and "dark" are palettes we write: a
    // program can lay one down the way a user picks it. It would be impossible
    // were the theme a reading of the system, which Qt 6.4 cannot do.
    //
    // The palette is laid down **before** what is shown is built, and not
    // after: it comes from the application, and a widget reads it at birth.
    //
    // Every call names its reference with a literal, and it has to:
    // `check-screenshots.py` reads those names out of this source to confront
    // what is generated with what the manual shows. A computed name would
    // escape it.

    // The window as it opens, with no film: what a reader of the manual will
    // see on starting the program, and nothing arranged.
    {
        subedit::gui::applyTheme(subedit::core::Theme::Light);
        subedit::gui::MainWindow window = windowOn(files, prompts, "manuel/scene.srt");
        window.resize(kWindowWidth, kWindowHeight);
        written = capture(window, window, directory, "fenetre") && written;
    }
    {
        subedit::gui::applyTheme(subedit::core::Theme::Dark);
        subedit::gui::MainWindow window = windowOn(files, prompts, "manuel/scene.srt");
        window.resize(kWindowWidth, kWindowHeight);
        written = capture(window, window, directory, "fenetre-sombre") && written;
    }

    // The table alone, and the window tall so that it shows enough to read.
    // What the section describes is the table; framing it in the window would
    // be showing mostly the empty video strip.
    {
        subedit::gui::applyTheme(subedit::core::Theme::Light);
        subedit::gui::MainWindow window = windowOn(files, prompts, "manuel/scene.srt");
        showWithTheTableFitted(window);
        written = capture(window, *window.table(), directory, "table") && written;
    }
    {
        subedit::gui::applyTheme(subedit::core::Theme::Dark);
        subedit::gui::MainWindow window = windowOn(files, prompts, "manuel/scene.srt");
        showWithTheTableFitted(window);
        written = capture(window, *window.table(), directory, "table-sombre") && written;
    }

    {
        subedit::gui::applyTheme(subedit::core::Theme::Light);
        subedit::gui::MainWindow window = windowOn(files, prompts, "manuel/scene-anomalies.srt");
        showWithTheTableFitted(window);
        written = capture(window, *window.table(), directory, "anomalies") && written;
    }
    {
        subedit::gui::applyTheme(subedit::core::Theme::Dark);
        subedit::gui::MainWindow window = windowOn(files, prompts, "manuel/scene-anomalies.srt");
        showWithTheTableFitted(window);
        written = capture(window, *window.table(), directory, "anomalies-sombre") && written;
    }

    // One cell open, which is the whole subject of the section: the mark of an
    // edit exists only between the double click and the validation.
    {
        subedit::gui::applyTheme(subedit::core::Theme::Light);
        subedit::gui::MainWindow window = windowOn(files, prompts, "manuel/scene.srt");
        showWithTheTableFitted(window);

        // `openPersistentEditor` rather than `edit`, and it is a difference of
        // tidiness and not of image: both open the same editor, but only the
        // first has a `close` that answers it. An editor opened by `edit` and
        // left there makes the view cry out twice when it is destroyed —
        // "commitData called with an editor that does not belong to this view"
        // — and a program that leaves noise behind teaches one to stop reading
        // it.
        const QModelIndex edited = window.table()->model()->index(kEditedRow, kTextColumn);
        window.table()->openPersistentEditor(edited);
        QApplication::processEvents();

        written = capture(window, *window.table(), directory, "edition") && written;

        window.table()->closePersistentEditor(edited);
    }
    {
        subedit::gui::applyTheme(subedit::core::Theme::Dark);
        subedit::gui::MainWindow window = windowOn(files, prompts, "manuel/scene.srt");
        showWithTheTableFitted(window);

        const QModelIndex edited = window.table()->model()->index(kEditedRow, kTextColumn);
        window.table()->openPersistentEditor(edited);
        QApplication::processEvents();

        written = capture(window, *window.table(), directory, "edition-sombre") && written;

        window.table()->closePersistentEditor(edited);
    }

    {
        subedit::gui::applyTheme(subedit::core::Theme::Light);
        subedit::gui::ShiftDialog dialog{3};
        written = capture(dialog, dialog, directory, "decalage") && written;
    }
    {
        subedit::gui::applyTheme(subedit::core::Theme::Dark);
        subedit::gui::ShiftDialog dialog{3};
        written = capture(dialog, dialog, directory, "decalage-sombre") && written;
    }

    // The insertion dialog, on a document that carries rows: that is the state
    // where the choice of side is on offer, and it is the one the section
    // describes.
    {
        subedit::gui::applyTheme(subedit::core::Theme::Light);
        subedit::gui::InsertDialog dialog{true, subedit::core::InsertPlacement::Below};
        written = capture(dialog, dialog, directory, "insertion") && written;
    }
    {
        subedit::gui::applyTheme(subedit::core::Theme::Dark);
        subedit::gui::InsertDialog dialog{true, subedit::core::InsertPlacement::Below};
        written = capture(dialog, dialog, directory, "insertion-sombre") && written;
    }

    // The shape `Save As…` offers, **inside the dialog that carries it** and
    // not on its own.
    //
    // It was the widget alone until issue #321, for a good reason — the content
    // of a file chooser depends on the machine — and with a consequence that
    // was less good: the image avoided the very place the defect lived in. The
    // three fields did not line up with the dialog's, and no capture could show
    // it.
    //
    // The bottom strip settles both: the six rows are seen together, so their
    // alignment is, and nothing that varies from one machine to the next is.
    {
        subedit::gui::applyTheme(subedit::core::Theme::Light);
        written = captureSaveAsStrip(directory, "enregistrer-sous") && written;
    }
    {
        subedit::gui::applyTheme(subedit::core::Theme::Dark);
        written = captureSaveAsStrip(directory, "enregistrer-sous-sombre") && written;
    }

    // The manual read in its own window, on the real manual of the repository:
    // that is what `Help ▸ Manual` opens once the tool is installed, save that
    // the installed copy lives under `share/subedit/manual` rather than under
    // `docs/`.
    {
        subedit::gui::applyTheme(subedit::core::Theme::Light);
        subedit::gui::ManualWindow manual{files, manualDirectory()};
        manual.resize(kManualWidth, kManualHeight);
        written = capture(manual, manual, directory, "manuel") && written;
    }
    {
        subedit::gui::applyTheme(subedit::core::Theme::Dark);
        subedit::gui::ManualWindow manual{files, manualDirectory()};
        manual.resize(kManualWidth, kManualHeight);
        written = capture(manual, manual, directory, "manuel-sombre") && written;
    }

    {
        subedit::gui::applyTheme(subedit::core::Theme::Light);
        const subedit::core::OpenedFile grid =
            subedit::core::openProject(files, corpus("grilles/grille-25.srt")).value();
        subedit::gui::GridAnalysisDialog dialog{subedit::core::deduceFrameRate(grid.project)};
        // Tall enough for the eight candidates: the dialog opens on six and
        // the section promises eight, so the image has to hold them.
        dialog.resize(kDialogWidth, kDialogHeight);
        written = capture(dialog, dialog, directory, "analyse-de-grille") && written;
    }
    {
        subedit::gui::applyTheme(subedit::core::Theme::Dark);
        const subedit::core::OpenedFile grid =
            subedit::core::openProject(files, corpus("grilles/grille-25.srt")).value();
        subedit::gui::GridAnalysisDialog dialog{subedit::core::deduceFrameRate(grid.project)};
        dialog.resize(kDialogWidth, kDialogHeight);
        written = capture(dialog, dialog, directory, "analyse-de-grille-sombre") && written;
    }

    return written ? 0 : 1;
}
