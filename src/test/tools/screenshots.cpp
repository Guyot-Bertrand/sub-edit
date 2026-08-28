// Les captures d'écran du manuel, engendrées depuis la vraie fenêtre.
//
// **Ce que ce programme est.** Le manuel de la ligne de commande porte des
// blocs `console` que `generate-manual.sh` réécrit en exécutant le binaire :
// c'est ce qui les empêche de mentir. Le manuel d'interface n'avait pas
// d'équivalent, et #116 puis #161 l'ont écrit chaque fois — une fenêtre ne
// s'écrit pas dans un bloc `console`. Voici l'équivalent : la fenêtre est
// construite, montrée, et photographiée.
//
// **Il n'écrit jamais une référence.** Chaque capture s'appelle
// `<nom>.new.png`, et c'est `compare-screenshots.py` qui décide ensuite de la
// promouvoir ou de l'effacer. Une image n'entre donc dans un diff que le jour
// où l'interface a changé.
//
// **Il ne fait foi que sous les réglages qu'il pose lui-même** — plateforme
// sans écran, style Fusion, police nommée, taille de fenêtre fixée. Ce sont les
// quatre choses qui font qu'une même version du code rend la même image sur
// deux machines, et la police est la seule qu'il ne peut pas fabriquer : il
// refuse plutôt que de photographier avec une remplaçante. Voir l'ADR 0024.
//
// **Il vit ici et non dans src/exe** pour la raison des autres programmes de ce
// répertoire : rien de ce qui est livré ne le contient, aucune règle
// d'installation ne le nomme.

#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/gui/grid_analysis_dialog.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/qt_prompts.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/subtitle_table.hpp>

#include <QAbstractItemView>
#include <QApplication>
#include <QByteArray>
#include <QFont>
#include <QFontInfo>
#include <QHeaderView>
#include <QMessageLogContext>
#include <QModelIndex>
#include <QPixmap>
#include <QSplitter>
#include <QString>
#include <QStyleFactory>
#include <QWidget>
#include <QtGlobal>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

namespace {

/// La police sous laquelle les captures font foi.
///
/// DejaVu Sans plutôt que « la police du système » : elle est présente sur
/// toute distribution Linux de bureau — `fonts-dejavu-core` — et elle est
/// la seule chose de ce fichier qu'une machine peut ne pas avoir. Nommer une
/// police que Qt remplacerait en silence reviendrait à ne rien nommer.
constexpr const char* kFontFamily = "DejaVu Sans";
constexpr int kFontPointSize = 10;

/// La taille de la fenêtre photographiée.
///
/// Fixée, et non laissée à `sizeHint()` : la fenêtre s'ouvre assez grande pour
/// qu'on lise la table — #211 — mais « assez grande » dépend de la police, donc
/// de la machine. Une capture dont les dimensions varient est une capture que
/// le comparateur promeut à chaque fois.
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 800;

/// La hauteur des captures qui ne montrent que la table.
///
/// La fenêtre est haute pour que la table le soit ; la bande vidéo vide qui
/// l'accompagne n'entre pas dans l'image, puisque c'est la table qu'on
/// photographie.
constexpr int kTallWindowHeight = 900;

/// La hauteur qu'il faut à la table pour montrer toutes ses lignes.
[[nodiscard]] int heightOfEveryRow(const subedit::gui::SubtitleTable& table) {
    int needed = table.horizontalHeader()->height() + 2;
    for (int row = 0; row < table.model()->rowCount(); ++row)
        needed += table.rowHeight(row);
    return needed;
}

/// Montre la fenêtre et donne à la table exactement ses lignes, ni plus ni
/// moins.
///
/// Deux raisons, et la seconde est celle qui compte. La fenêtre partage sa
/// hauteur entre la bande vidéo et la table, et c'est la bande qui prend
/// l'élasticité — l'agrandir agrandit le vide. Et une table plus haute que son
/// contenu photographie surtout du blanc : ce qui reste sous la dernière ligne
/// n'apprend rien à personne, et la capture s'y ajuste comme une image de
/// manuel s'ajuste à son sujet.
///
/// Le partage se règle là où un utilisateur le règle, en tirant la poignée. Les
/// trois nombres sont les trois enfants du séparateur, dans l'ordre : la vue
/// vidéo, l'invitation à en choisir une, la table.
void showWithTheTableFitted(subedit::gui::MainWindow& window) {
    window.resize(kWindowWidth, kTallWindowHeight);
    window.show();
    QApplication::processEvents();

    const int needed = heightOfEveryRow(*window.table());
    if (auto* split = window.findChild<QSplitter*>(); split != nullptr)
        split->setSizes({0, kTallWindowHeight, needed});
    QApplication::processEvents();
}

/// La colonne `Text`, la cinquième — voir docs/manual/subedit-gui/table.md.
constexpr int kTextColumn = 4;

/// La ligne dont le texte est montré ouvert : la troisième, parce qu'elle en
/// porte deux et que c'est ce que la section explique.
constexpr int kEditedRow = 2;

/// La modale d'analyse, assez haute pour ses huit candidates.
constexpr int kDialogWidth = 330;
constexpr int kDialogHeight = 430;

/// La seule phrase que la plateforme sans écran répète à chaque fenêtre montrée.
///
/// Vraie et sans conséquence : rien ici n'a de gestionnaire de fenêtres à qui
/// propager une taille. Le harnais des tests d'interface la tait de la même
/// façon et pour la même raison — un avertissement répété six fois est un
/// avertissement qu'on cesse de lire.
constexpr const char* kOffscreenSizeHints = "This plugin does not support propagateSizeHints()";

void withoutOffscreenNoise(QtMsgType type, const QMessageLogContext& context, const QString& text) {
    if (text == QLatin1StringView{kOffscreenSizeHints})
        return;

    qt_message_output(type, context, text);
}

[[nodiscard]] std::filesystem::path corpus(const std::string& relative) {
    return std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / relative;
}

/// Photographie `subject` sous le nom donné, sans jamais toucher la référence.
///
/// `shown` est ce qu'on montre, `subject` ce qu'on photographie — souvent le
/// même, parfois non : la table seule dit mieux ce que la table fait, et il
/// faut pourtant montrer la fenêtre entière pour que la table existe. Une
/// fenêtre qui n'a jamais été montrée n'a pas de vraie géométrie et sa
/// disposition n'a pas tourné : c'est ce que #191 a changé, et sans quoi il n'y
/// aurait rien à photographier.
[[nodiscard]] bool capture(QWidget& shown,
                           QWidget& subject,
                           const std::filesystem::path& directory,
                           const std::string& name) {
    shown.show();
    // La disposition n'a réellement tourné qu'une fois les événements traités :
    // sans cela, on photographie une fenêtre dont les tailles sont encore
    // celles du constructeur.
    QApplication::processEvents();

    const std::filesystem::path target = directory / (name + ".new.png");
    const QPixmap shot = subject.grab();

    if (shot.isNull() || !shot.save(QString::fromStdString(target.string()), "PNG")) {
        std::cerr << "subedit_screenshots: " << target.string() << ": nothing written\n";
        return false;
    }

    std::cout << "  " << target.filename().string() << " — " << shot.width() << "×" << shot.height()
              << "\n";
    return true;
}

/// La fenêtre du manuel, ouverte sur le fichier nommé.
///
/// Rendue par valeur et construite d'un trait : `MainWindow` n'est ni copiable
/// ni déplaçable, donc seule l'élision garantie d'une prvalue la fait sortir
/// d'une fonction. La taille se pose ensuite, chez l'appelant.
[[nodiscard]] subedit::gui::MainWindow windowOn(subedit::core::FileSystem& files,
                                                subedit::gui::Prompts& prompts,
                                                const std::string& fixture) {
    return subedit::gui::MainWindow{
        files, subedit::core::openProject(files, corpus(fixture)).value(), prompts, {}, {}};
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3 || std::string{argv[1]} != "--output-dir") {
        std::cerr << "usage: subedit_screenshots --output-dir <répertoire>\n";
        return 2;
    }
    const std::filesystem::path directory{argv[2]};

    // Sans écran, comme le harnais de tests d'interface : une capture ne
    // demande pas de serveur graphique, et la CI n'en a pas.
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");

    // Posé avant la `QApplication`, qui avertit pour son propre compte aussi.
    qInstallMessageHandler(withoutOffscreenNoise);

    const QApplication application{argc, argv};

    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    const QFont font{QLatin1StringView{kFontFamily}, kFontPointSize};
    QApplication::setFont(font);

    // Refuser plutôt que photographier avec une remplaçante : une capture faite
    // sous une autre police se ferait promouvoir chez le suivant, et la
    // référence oscillerait au gré de qui a lancé la commande.
    const QFontInfo actual{font};
    if (actual.family() != QLatin1StringView{kFontFamily}) {
        std::cerr << "subedit_screenshots: police « " << kFontFamily
                  << " » absente, remplacée par « " << actual.family().toStdString() << " »\n"
                  << "  les captures ne font foi que sous cette police ; l'installer\n"
                  << "  (Debian, Ubuntu : fonts-dejavu-core)\n";
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

    // La fenêtre telle qu'elle s'ouvre, film absent : ce qu'un lecteur du
    // manuel verra en lançant le programme, et rien d'arrangé.
    {
        subedit::gui::MainWindow window = windowOn(files, prompts, "manuel/scene.srt");
        window.resize(kWindowWidth, kWindowHeight);
        written = capture(window, window, directory, "fenetre") && written;
    }

    // La table seule, et la fenêtre haute pour qu'elle montre de quoi lire.
    // Ce que la section décrit est la table ; l'entourer de la fenêtre serait
    // montrer surtout la bande vidéo vide.
    {
        subedit::gui::MainWindow window = windowOn(files, prompts, "manuel/scene.srt");
        showWithTheTableFitted(window);
        written = capture(window, *window.table(), directory, "table") && written;
    }

    {
        subedit::gui::MainWindow window = windowOn(files, prompts, "manuel/scene-anomalies.srt");
        showWithTheTableFitted(window);
        written = capture(window, *window.table(), directory, "anomalies") && written;
    }

    // Une cellule ouverte, ce qui est le sujet entier de la section : le
    // marqueur d'édition n'existe qu'entre le double-clic et la validation.
    {
        subedit::gui::MainWindow window = windowOn(files, prompts, "manuel/scene.srt");
        showWithTheTableFitted(window);

        // `openPersistentEditor` plutôt que `edit`, et c'est une différence de
        // propreté et non d'image : les deux ouvrent le même éditeur, mais
        // seul le premier a un `close` qui lui répond. Un éditeur ouvert par
        // `edit` et laissé là fait crier la vue deux fois quand elle est
        // détruite — « commitData called with an editor that does not belong
        // to this view » —, et un programme qui laisse du bruit derrière lui
        // apprend à ne plus le lire.
        const QModelIndex edited = window.table()->model()->index(kEditedRow, kTextColumn);
        window.table()->openPersistentEditor(edited);
        QApplication::processEvents();

        written = capture(window, *window.table(), directory, "edition") && written;

        window.table()->closePersistentEditor(edited);
    }

    {
        subedit::gui::ShiftDialog dialog{3};
        written = capture(dialog, dialog, directory, "decalage") && written;
    }

    {
        const subedit::core::OpenedFile grid =
            subedit::core::openProject(files, corpus("grilles/grille-25.srt")).value();
        subedit::gui::GridAnalysisDialog dialog{subedit::core::deduceFrameRate(grid.project)};
        // Assez haute pour les huit candidates : la modale s'ouvre sur six et
        // la section promet les huit, donc l'image doit les tenir.
        dialog.resize(kDialogWidth, kDialogHeight);
        written = capture(dialog, dialog, directory, "analyse-de-grille") && written;
    }

    return written ? 0 : 1;
}
