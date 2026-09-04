// Ce que `Save As…` ajoute sous la liste des fichiers — issue #299.
//
// Un widget à nous, donc conduit sans clic et sans boucle d'événements : ce
// qu'un test ne peut pas atteindre est `exec()`, et `exec()` seul.

#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/gui/save_shape.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QLineEdit>
#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace {

using subedit::core::ByteOrderMark;
using subedit::core::Encoding;
using subedit::core::Newline;
using subedit::core::SubtitleFormat;
using subedit::gui::addSaveShapeTo;
using subedit::gui::kOfferedEncodings;
using subedit::gui::saveDialogFor;
using subedit::gui::SaveShape;
using subedit::gui::SaveTarget;
using subedit::gui::targetOf;

/// L'encodage de ce nom, ou un test en échec.
[[nodiscard]] Encoding named(const char* name) {
    const std::optional<Encoding> encoding = Encoding::create(name, ByteOrderMark::Absent);
    if (!encoding.has_value()) {
        FAIL("ICU ne connaît pas cet encodage");
        return Encoding::utf8(ByteOrderMark::Absent);
    }
    return *encoding;
}

} // namespace

TEST_CASE("the shape opens on what the file carries", "[gui][GUI-ENC-02]") {
    // Les défauts sont ceux du fichier lu : le réécrire autrement sans qu'on
    // l'ait demandé perdrait ce que la lecture a gardé.
    const SaveShape shape{named("windows-1252"), Newline::CrLf};

    CHECK(shape.encoding() == named("windows-1252"));
    CHECK(shape.newline() == Newline::CrLf);
    CHECK_FALSE(shape.wantsByteOrderMark());
}

TEST_CASE("a mark the file carried is proposed again", "[gui][GUI-ENC-02]") {
    const SaveShape shape{Encoding::utf8(ByteOrderMark::Present), Newline::Lf};

    CHECK(shape.wantsByteOrderMark());
    CHECK(shape.encoding() == Encoding::utf8(ByteOrderMark::Present));
}

TEST_CASE("every encoding offered is one ICU can write", "[gui][GUI-ENC-02]") {
    // La liste est courte et écrite ici — D2 du cadrage —, mais elle n'est pas
    // une seconde vérité : chacune de ses entrées doit exister chez ICU, sans
    // quoi le menu proposerait un encodage que rien n'écrirait.
    for (const auto& offered : kOfferedEncodings) {
        INFO("encodage : " << offered.charset);
        CHECK(Encoding::create(offered.charset, ByteOrderMark::Absent).has_value());
    }
}

TEST_CASE("an encoding that is not on the list is typed", "[gui][GUI-ENC-02]") {
    // « Autre… » est ce qui empêche la liste courte d'être un plafond : le jeu
    // d'encodages est celui d'ICU, et la liste n'en propose qu'une part.
    const SaveShape shape{Encoding::utf8(ByteOrderMark::Absent), Newline::Lf};

    shape.encodingBox()->setCurrentIndex(shape.encodingBox()->count() - 1);
    // Tapé sous n'importe lequel de ses noms — ICU les connaît tous.
    shape.otherName()->setText(QStringLiteral("cp1257"));

    CHECK(shape.encoding() == named("windows-1257"));
}

TEST_CASE("the name field shows itself only for the other entry", "[gui][GUI-ENC-02]") {
    SaveShape shape{Encoding::utf8(ByteOrderMark::Absent), Newline::Lf};
    shape.show();

    CHECK_FALSE(shape.otherName()->isVisible());

    shape.encodingBox()->setCurrentIndex(shape.encodingBox()->count() - 1);

    CHECK(shape.otherName()->isVisible());
}

TEST_CASE("a name nobody knows is no encoding at all", "[gui][GUI-ENC-02]") {
    // Rendre vide plutôt que retomber sur l'UTF-8 : écrire un fichier dans un
    // encodage que personne n'a nommé n'est pas une chose à trancher soi-même.
    const SaveShape shape{Encoding::utf8(ByteOrderMark::Absent), Newline::Lf};

    shape.encodingBox()->setCurrentIndex(shape.encodingBox()->count() - 1);
    shape.otherName()->setText(QStringLiteral("klingon-1"));

    CHECK_FALSE(shape.encoding().has_value());
}

TEST_CASE("a file opened in an encoding the list does not offer opens on the other field",
          "[gui][GUI-ENC-02]") {
    const SaveShape shape{named("cp1257"), Newline::Lf};

    // Le nom qu'ICU lui donne, et non celui qu'on a tapé pour l'obtenir : c'est
    // celui que le rapport et le fichier de réglages portent.
    CHECK(shape.otherName()->text() == QStringLiteral("windows-1257"));
    CHECK(shape.encoding() == named("cp1257"));
}

TEST_CASE("the mark is offered only where one exists", "[gui][GUI-ENC-02]") {
    // Une marque d'ordre des octets existe pour les encodages Unicode et pour
    // aucun autre. Éteinte plutôt que cachée : une case grisée dit pourquoi le
    // choix ne s'offre pas.
    const SaveShape shape{Encoding::utf8(ByteOrderMark::Present), Newline::Lf};

    CHECK(shape.markBox()->isEnabled());

    shape.encodingBox()->setCurrentIndex(
        shape.encodingBox()->findText(QStringLiteral("windows-1252"), Qt::MatchStartsWith));

    CHECK_FALSE(shape.markBox()->isEnabled());
    CHECK_FALSE(shape.wantsByteOrderMark());
}

TEST_CASE("the line endings offered are the three the core knows", "[gui][GUI-ENC-02]") {
    const SaveShape shape{Encoding::utf8(ByteOrderMark::Absent), Newline::Lf};

    REQUIRE(shape.newlineBox()->count() == 3);
    shape.newlineBox()->setCurrentIndex(2);
    CHECK(shape.newline() == Newline::Cr);
}

TEST_CASE("the save dialog opens on the file and its shape", "[gui][GUI-ENC-02]") {
    // Ce que `Save As…` montre, construit sans être ouvert : seul `exec()`
    // échappe à un test, et il est seul à y échapper.
    const subedit::core::SourceFile current{.path = std::filesystem::path{"/films/film.srt"},
                                            .newline = Newline::CrLf};

    const std::unique_ptr<QFileDialog> dialog =
        saveDialogFor(current, named("windows-1252"), nullptr);

    REQUIRE(dialog != nullptr);
    CHECK(dialog->acceptMode() == QFileDialog::AcceptSave);
    const auto* shape = dialog->findChild<const SaveShape*>();
    REQUIRE(shape != nullptr);
    CHECK(shape->encoding() == named("windows-1252"));
    CHECK(shape->newline() == Newline::CrLf);
}

TEST_CASE("what the save dialog was filled with is read back", "[gui][GUI-ENC-02]") {
    const std::unique_ptr<QFileDialog> dialog =
        saveDialogFor(subedit::core::SourceFile{}, Encoding::utf8(ByteOrderMark::Absent), nullptr);
    dialog->selectFile(QStringLiteral("/films/copie.vtt"));
    dialog->selectNameFilter(QStringLiteral("WebVTT (*.vtt)"));
    auto* shape = dialog->findChild<SaveShape*>();
    REQUIRE(shape != nullptr);
    shape->newlineBox()->setCurrentIndex(1);
    shape->markBox()->setChecked(true);

    const std::expected<SaveTarget, std::string> target = targetOf(*dialog);

    REQUIRE(target.has_value());
    // Le nom, et non le chemin entier : une boîte de fichiers résout ce qu'on
    // lui donne contre un répertoire qui dépend de la machine.
    CHECK(target->path.filename() == std::filesystem::path{"copie.vtt"});
    CHECK(target->format == SubtitleFormat::WebVtt);
    CHECK(target->newline == Newline::CrLf);
    CHECK(target->encoding == Encoding::utf8(ByteOrderMark::Present));
}

TEST_CASE("a name nobody knows writes no file at all", "[gui][GUI-ENC-02]") {
    // Le refus porte le nom qu'on a tapé : c'est ce que la fenêtre affichera,
    // et c'est la seule chose que l'utilisateur peut corriger.
    const std::unique_ptr<QFileDialog> dialog =
        saveDialogFor(subedit::core::SourceFile{}, Encoding::utf8(ByteOrderMark::Absent), nullptr);
    dialog->selectFile(QStringLiteral("/films/copie.srt"));
    auto* shape = dialog->findChild<SaveShape*>();
    REQUIRE(shape != nullptr);
    shape->encodingBox()->setCurrentIndex(shape->encodingBox()->count() - 1);
    shape->otherName()->setText(QStringLiteral("klingon-1"));

    const std::expected<SaveTarget, std::string> target = targetOf(*dialog);

    REQUIRE_FALSE(target.has_value());
    CHECK(target.error() == "no encoding is named \"klingon-1\"");
}

TEST_CASE("the shape sits inside the file dialog", "[gui][GUI-ENC-02]") {
    // Photographié et éprouvé au même endroit : ce que la fenêtre montre est ce
    // que ce test conduit, à l'`exec()` près.
    QFileDialog dialog;
    dialog.setOption(QFileDialog::DontUseNativeDialog);

    const SaveShape* shape =
        addSaveShapeTo(dialog, Encoding::utf8(ByteOrderMark::Absent), Newline::Lf);

    REQUIRE(shape != nullptr);
    CHECK(shape->parent() == &dialog);
    CHECK(dialog.findChild<const SaveShape*>() == shape);
}
