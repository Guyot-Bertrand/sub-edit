// What a project remembers of the file each of its two documents came from —
// ADR 0032.
//
// The main file is the one the plain accessor has always named, and it has to
// stay that way: seventeen places read it, and none of them is about to learn
// that there are two. The translation is a file of its own, or — until it has
// one — the main file's format.

#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <optional>

namespace {

using subedit::core::Document;
using subedit::core::Project;
using subedit::core::SourceFile;
using subedit::core::SubtitleFormat;

} // namespace

TEST_CASE("a fresh project has no file of its own for the translation", "[model][document]") {
    const Project project;

    CHECK_FALSE(project.translationFile().has_value());
}

TEST_CASE("the translation follows the main file until it has a file of its own",
          "[model][document]") {
    // A translation typed into an empty column will be written like the main
    // document, until it is given a format of its own.
    Project project;
    project.setSourceFile(SourceFile{.format = SubtitleFormat::WebVtt});

    CHECK(project.sourceFile(Document::Translation).format == SubtitleFormat::WebVtt);
    CHECK_FALSE(project.translationFile().has_value());
}

TEST_CASE("a translation file is a file of its own", "[model][document]") {
    Project project;
    project.setSourceFile(SourceFile{.format = SubtitleFormat::SubRip});

    project.setSourceFile(Document::Translation,
                          SourceFile{.path = "/films/film.fr.ass",
                                     .format = SubtitleFormat::AdvancedSubStationAlpha});

    // Field by field, and not `SourceFile == SourceFile`: comparing whole files
    // would instantiate the comparison of every kind of file extras, which no
    // case runs, and the coverage ratchet counts those lines. Equality of files
    // comes with the issue that needs it, and covers it then.
    CHECK(project.sourceFile(Document::Main).format == SubtitleFormat::SubRip);
    CHECK(project.sourceFile(Document::Translation).format ==
          SubtitleFormat::AdvancedSubStationAlpha);
    CHECK(project.sourceFile(Document::Translation).path ==
          std::optional<std::filesystem::path>{"/films/film.fr.ass"});
    CHECK(project.translationFile().has_value());
}

TEST_CASE("the plain accessor is the main file, whatever the translation holds",
          "[model][document]") {
    Project project;
    project.setSourceFile(SourceFile{.path = "/films/film.srt", .format = SubtitleFormat::SubRip});

    project.setSourceFile(Document::Translation,
                          SourceFile{.format = SubtitleFormat::AdvancedSubStationAlpha});

    CHECK(project.sourceFile().format == SubtitleFormat::SubRip);
    CHECK(project.sourceFile().path == std::optional<std::filesystem::path>{"/films/film.srt"});
}

TEST_CASE("writing the main file through the document accessor is the plain setter",
          "[model][document]") {
    Project project;

    project.setSourceFile(Document::Main, SourceFile{.format = SubtitleFormat::WebVtt});

    CHECK(project.sourceFile().format == SubtitleFormat::WebVtt);
    CHECK_FALSE(project.translationFile().has_value());
}
