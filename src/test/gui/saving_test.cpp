// Écrire le fichier qu'une fenêtre tient — issue #131.
//
// **L'aller-retour octet pour octet est le critère**, et il n'est pas
// décoratif : un fichier arrivé avec un BOM et des fins de ligne CRTL réécrit
// sans eux montrerait un diff sur chacune de ses lignes là où l'utilisateur
// attendait un sous-titre corrigé.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/gui/opening.hpp>
#include <subedit/gui/saving.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <string>

namespace {

using subedit::core::FileError;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::Project;
using subedit::core::SubtitleFormat;
using subedit::gui::openProject;
using subedit::gui::saveProject;

/// Dans la disposition que le projet écrit — ligne vide close comprise, y
/// compris après le dernier bloc. C'est sur celle-là que l'aller-retour est
/// fidèle octet pour octet ; la spec de la phase 1 le dit ainsi.
constexpr const char* kSubRip = "1\n"
                                "00:00:01,000 --> 00:00:02,000\n"
                                "Un.\n"
                                "\n"
                                "2\n"
                                "00:00:03,000 --> 00:00:04,000\n"
                                "Deux.\n"
                                "\n";

/// Le même sans sa ligne vide finale : lisible, courant, et hors disposition.
constexpr const char* kSubRipUnclosed = "1\n"
                                        "00:00:01,000 --> 00:00:02,000\n"
                                        "Un.\n"
                                        "\n"
                                        "2\n"
                                        "00:00:03,000 --> 00:00:04,000\n"
                                        "Deux.\n";

/// Le même, tel qu'un éditeur de Windows l'aurait laissé : BOM et CRLF.
[[nodiscard]] std::string withBomAndCrLf(const std::string& content) {
    std::string out = "\xEF\xBB\xBF";
    for (const char letter : content) {
        if (letter == '\n')
            out += '\r';
        out += letter;
    }
    return out;
}

[[nodiscard]] Project opened(const InMemoryFileSystem& files, const char* path) {
    const auto result = openProject(files, path);
    REQUIRE(result.has_value());
    return result->project;
}

} // namespace

TEST_CASE("a file already in the layout the project writes comes back identical byte for byte",
          "[gui][GUI-SAVE-01]") {
    const std::string original = withBomAndCrLf(kSubRip);
    InMemoryFileSystem files;
    files.addFile("film.srt", original);

    const Project project = opened(files, "film.srt");
    const std::expected<void, FileError> saved =
        saveProject(files, project, "film.srt", SubtitleFormat::SubRip);

    REQUIRE(saved.has_value());
    CHECK(files.contentOf("film.srt").value_or("") == original);
}

TEST_CASE("saving keeps the line endings and the byte order mark of the file it came from",
          "[gui][GUI-SAVE-01]") {
    // Chacun séparément, pour qu'un échec dise lequel des deux a été perdu.
    InMemoryFileSystem files;
    files.addFile("unix.srt", kSubRip);

    const Project project = opened(files, "unix.srt");
    REQUIRE(saveProject(files, project, "unix.srt", SubtitleFormat::SubRip).has_value());

    const std::string written = files.contentOf("unix.srt").value_or("");
    CHECK(written.find('\r') == std::string::npos);
    CHECK_FALSE(written.starts_with("\xEF\xBB\xBF"));
}

TEST_CASE("saving under another name leaves the first file alone", "[gui][GUI-SAVE-02]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kSubRip);

    const Project project = opened(files, "film.srt");
    REQUIRE(saveProject(files, project, "copie.srt", SubtitleFormat::SubRip).has_value());

    CHECK(files.contentOf("film.srt").value_or("") == kSubRip);
    CHECK(files.contentOf("copie.srt").value_or("") == kSubRip);
}

TEST_CASE("saving in the other format writes that format", "[gui][GUI-SAVE-02]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kSubRip);

    const Project project = opened(files, "film.srt");
    REQUIRE(saveProject(files, project, "film.vtt", SubtitleFormat::WebVtt).has_value());

    const std::string written = files.contentOf("film.vtt").value_or("");
    CHECK(written.starts_with("WEBVTT"));
    CHECK(written.find(",000") == std::string::npos);
    // Le point, et non la virgule : c'est ce que WebVTT écrit.
    // Le point pour virgule, et les heures omises sous une heure : les deux
    // sont ce que WebVTT écrit, et les deux distinguent le fichier produit de
    // celui qu'on a lu.
    CHECK(written.find("00:01.000 --> 00:02.000") != std::string::npos);
}

TEST_CASE("a save that cannot be written says so", "[gui][GUI-SAVE-01]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kSubRip);
    const Project project = opened(files, "film.srt");
    files.failNextWrite(FileErrorKind::PermissionDenied);

    const std::expected<void, FileError> saved =
        saveProject(files, project, "film.srt", SubtitleFormat::SubRip);

    CHECK_FALSE(saved.has_value());
}

TEST_CASE("a file outside that layout is normalised once and never again", "[gui][GUI-SAVE-01]") {
    // La seconde moitié de la garantie, telle que la spec de la phase 1
    // l'énonce : le premier enregistrement ferme le dernier bloc par la ligne
    // vide que la disposition demande, et aucun enregistrement ensuite ne
    // touche plus rien. Le fichier bouge une fois, pas à chaque sauvegarde.
    InMemoryFileSystem files;
    files.addFile("film.srt", kSubRipUnclosed);

    const Project first = opened(files, "film.srt");
    REQUIRE(saveProject(files, first, "film.srt", SubtitleFormat::SubRip).has_value());
    const std::string once = files.contentOf("film.srt").value_or("");

    CHECK(once != kSubRipUnclosed);
    CHECK(once == kSubRip);

    const Project second = opened(files, "film.srt");
    REQUIRE(saveProject(files, second, "film.srt", SubtitleFormat::SubRip).has_value());

    CHECK(files.contentOf("film.srt").value_or("") == once);
}

TEST_CASE("changing format leaves the other variant's extras out of the file",
          "[gui][GUI-SAVE-02]") {
    // **Le point que le ticket demandait de regarder plutôt que de supposer.**
    // Un projet lu en WebVTT porte des `WebVttExtras` — un identifiant de cue,
    // des réglages de placement — qui n'ont aucun sens en SubRip, et
    // réciproquement pour les coordonnées de SubRip.
    //
    // Les écrivains interrogent la variante par `std::get_if` et retombent sur
    // leurs valeurs par défaut quand elle appartient à l'autre format. Rien à
    // changer donc, mais rien qui le disait : ce test le dit.
    InMemoryFileSystem files;
    files.addFile("film.vtt",
                  "WEBVTT\n"
                  "\n"
                  "chapitre-1\n"
                  "00:01.000 --> 00:02.000 align:start position:10%\n"
                  "Un.\n"
                  "\n");

    const Project project = opened(files, "film.vtt");
    REQUIRE(saveProject(files, project, "film.srt", SubtitleFormat::SubRip).has_value());

    const std::string written = files.contentOf("film.srt").value_or("");
    CHECK(written.find("chapitre-1") == std::string::npos);
    CHECK(written.find("align:start") == std::string::npos);
    // Ce qui compte est bien là, dans la forme de SubRip.
    CHECK(written.find("00:00:01,000 --> 00:00:02,000") != std::string::npos);
    CHECK(written.find("Un.") != std::string::npos);
}

TEST_CASE("the extras of the format written are kept", "[gui][GUI-SAVE-02]") {
    // Le pendant du précédent : ce qui est ignoré l'est parce qu'il appartient
    // à l'autre format, et non parce que les extras seraient perdus.
    InMemoryFileSystem files;
    files.addFile("film.vtt",
                  "WEBVTT\n"
                  "\n"
                  "chapitre-1\n"
                  "00:01.000 --> 00:02.000 align:start position:10%\n"
                  "Un.\n"
                  "\n");

    const Project project = opened(files, "film.vtt");
    REQUIRE(saveProject(files, project, "copie.vtt", SubtitleFormat::WebVtt).has_value());

    const std::string written = files.contentOf("copie.vtt").value_or("");
    CHECK(written.find("chapitre-1") != std::string::npos);
    CHECK(written.find("align:start position:10%") != std::string::npos);
}
