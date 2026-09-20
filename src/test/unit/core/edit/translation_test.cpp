// Opening a translation over a main document — issue #430, decision D4.
//
// **The cases are not written here.** `src/test/data/paires/` holds eight of
// them, each with the result every method must lead to, written by hand from a
// reading of Gaupol and then observed on it. This file is the harness those
// cases were left waiting for: it opens each pair, attaches the translation by
// each method, and asks that what comes out is, byte for byte, what the
// directory says.

#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/translation.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/translation_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace {

using subedit::core::AttachedTranslation;
using subedit::core::attachTranslation;
using subedit::core::CommandKind;
using subedit::core::Document;
using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::core::openTranslation;
using subedit::core::Project;
using subedit::core::RealFileSystem;
using subedit::core::SameFileAsMain;
using subedit::core::Session;
using subedit::core::SourceFile;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::SubtitleIndex;
using subedit::core::TranslationFile;
using subedit::core::TranslationMethod;
using subedit::core::TranslationOutcome;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

/// What each method makes of one case: how many lines found their subtitle, how
/// many gave one of their own, how many subtitles no line reached, and how many
/// lines were out of order in the file.
///
/// **Written by hand, from the cases**, and not derived from the code under
/// test — the table is what the rest of the file is checked against.
struct Expectation {
    std::string_view directory;
    TranslationOutcome byPosition;
    TranslationOutcome byNumber;
};

constexpr std::array<Expectation, 8> kExpectations = {
    Expectation{.directory = "temoin",
                .byPosition = {.attached = 4, .born = 0, .untranslated = 0, .outOfOrder = 0},
                .byNumber = {.attached = 4, .born = 0, .untranslated = 0, .outOfOrder = 0}},
    Expectation{.directory = "traduction-plus-courte",
                .byPosition = {.attached = 3, .born = 0, .untranslated = 1, .outOfOrder = 0},
                .byNumber = {.attached = 3, .born = 0, .untranslated = 1, .outOfOrder = 0}},
    Expectation{.directory = "traduction-dans-le-desordre",
                .byPosition = {.attached = 4, .born = 0, .untranslated = 0, .outOfOrder = 2},
                .byNumber = {.attached = 4, .born = 0, .untranslated = 0, .outOfOrder = 2}},
    Expectation{.directory = "une-ligne-de-moins-au-milieu",
                .byPosition = {.attached = 3, .born = 0, .untranslated = 1, .outOfOrder = 0},
                .byNumber = {.attached = 3, .born = 0, .untranslated = 1, .outOfOrder = 0}},
    Expectation{.directory = "une-ligne-de-plus-a-la-fin",
                .byPosition = {.attached = 4, .born = 1, .untranslated = 0, .outOfOrder = 0},
                .byNumber = {.attached = 4, .born = 1, .untranslated = 0, .outOfOrder = 0}},
    Expectation{.directory = "une-ligne-dans-un-intervalle-vide",
                .byPosition = {.attached = 4, .born = 1, .untranslated = 0, .outOfOrder = 0},
                .byNumber = {.attached = 4, .born = 1, .untranslated = 0, .outOfOrder = 0}},
    Expectation{.directory = "deux-lignes-dans-un-meme-sous-titre",
                .byPosition = {.attached = 4, .born = 1, .untranslated = 0, .outOfOrder = 0},
                .byNumber = {.attached = 4, .born = 1, .untranslated = 0, .outOfOrder = 0}},
    // The one where the methods part ways in what they count: by position, the
    // first line lands on the wrong subtitle and the other three are born.
    Expectation{.directory = "positions-decalees",
                .byPosition = {.attached = 1, .born = 3, .untranslated = 3, .outOfOrder = 0},
                .byNumber = {.attached = 4, .born = 0, .untranslated = 0, .outOfOrder = 0}},
};

[[nodiscard]] std::filesystem::path pairs() {
    return std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "paires";
}

[[nodiscard]] std::string bytesOf(const std::filesystem::path& path) {
    const RealFileSystem files;
    std::expected<std::string, subedit::core::FileError> content = files.readFile(path);
    if (!content.has_value()) {
        FAIL("fixture not found: " + path.string());
        return {};
    }
    return *std::move(content);
}

/// The main document of a case, opened as a user would open it.
[[nodiscard]] Project mainOf(const std::filesystem::path& directory) {
    const RealFileSystem files;
    std::expected<OpenedFile, subedit::core::OpenError> opened =
        openProject(files, directory / "principal.srt");
    if (!opened.has_value()) {
        FAIL("main document unreadable: " + directory.string());
        return {};
    }
    return std::move(opened->project);
}

[[nodiscard]] TranslationFile translationOf(const Project& project,
                                            const std::filesystem::path& file) {
    const RealFileSystem files;
    std::expected<TranslationFile, subedit::core::TranslationError> opened =
        openTranslation(files, project, file);
    if (!opened.has_value()) {
        FAIL("translation unreadable: " + file.string());
        return {};
    }
    return std::move(*opened);
}

/// One document of a project, written as SubRip.
[[nodiscard]] std::string writtenAs(const Project& project, Document document) {
    const std::expected<std::string, subedit::core::WriteError> written =
        writeSubtitles(SubtitleFormat::SubRip,
                       WriteRequest{.subtitles = project.subtitles(), .document = document});
    if (!written.has_value()) {
        FAIL("could not write the document");
        return {};
    }
    return *written;
}

[[nodiscard]] const char* nameOf(TranslationMethod method) {
    return method == TranslationMethod::Position ? "position" : "numero";
}

[[nodiscard]] AttachedTranslation
attached(const Project& project, const TranslationFile& file, TranslationMethod method) {
    return attachTranslation(project, file.lines, file.source, method);
}

} // namespace

TEST_CASE("each method leads, byte for byte, to the result the pairs give",
          "[edit][translation][pairs]") {
    for (const Expectation& expectation : kExpectations) {
        const std::filesystem::path directory = pairs() / expectation.directory;

        for (const TranslationMethod method :
             {TranslationMethod::Position, TranslationMethod::Number}) {
            INFO("case: " << expectation.directory << ", method: " << nameOf(method));
            Project project = mainOf(directory);
            const TranslationFile file = translationOf(project, directory / "traduction.srt");

            AttachedTranslation result = attached(project, file, method);
            REQUIRE(result.command != nullptr);
            result.command->apply(project);

            const std::string expected = std::string{"attendu-"} + nameOf(method);
            CHECK(writtenAs(project, Document::Main) ==
                  bytesOf(directory / (expected + ".principal.srt")));
            CHECK(writtenAs(project, Document::Translation) ==
                  bytesOf(directory / (expected + ".traduction.srt")));
        }
    }
}

TEST_CASE("each method counts what it did, case by case", "[edit][translation][pairs]") {
    for (const Expectation& expectation : kExpectations) {
        const std::filesystem::path directory = pairs() / expectation.directory;
        const Project project = mainOf(directory);
        const TranslationFile file = translationOf(project, directory / "traduction.srt");

        INFO("case: " << expectation.directory);
        CHECK(attached(project, file, TranslationMethod::Position).outcome ==
              expectation.byPosition);
        CHECK(attached(project, file, TranslationMethod::Number).outcome == expectation.byNumber);
    }
}

TEST_CASE("undoing an opening gives the document back as it was, born subtitles included",
          "[edit][translation]") {
    for (const Expectation& expectation : kExpectations) {
        const std::filesystem::path directory = pairs() / expectation.directory;

        for (const TranslationMethod method :
             {TranslationMethod::Position, TranslationMethod::Number}) {
            INFO("case: " << expectation.directory << ", method: " << nameOf(method));
            Project project = mainOf(directory);
            const std::vector<Subtitle> before{project.subtitles().begin(),
                                               project.subtitles().end()};
            const TranslationFile file = translationOf(project, directory / "traduction.srt");

            AttachedTranslation result = attached(project, file, method);
            REQUIRE(result.command != nullptr);
            result.command->apply(project);
            result.command->revert(project);

            CHECK(std::ranges::equal(project.subtitles(), before));
            CHECK_FALSE(project.translationFile().has_value());
        }
    }
}

TEST_CASE("an opening is one entry in the history, and the translation is what it modifies",
          "[edit][translation]") {
    const std::filesystem::path directory = pairs() / "temoin";
    Session session{mainOf(directory)};
    const TranslationFile file = translationOf(session.project(), directory / "traduction.srt");

    AttachedTranslation result = attached(session.project(), file, TranslationMethod::Position);
    REQUIRE(result.command != nullptr);
    session.apply(std::move(result.command));

    CHECK(session.nextUndoKind() == CommandKind::AttachTranslation);
    CHECK(session.hasUnsavedChanges(Document::Translation));
    // Every line found its subtitle: nothing about the main document moved.
    CHECK_FALSE(session.hasUnsavedChanges(Document::Main));

    session.undo();
    CHECK_FALSE(session.canUndo());
    CHECK(session.project().subtitleAt(SubtitleIndex::fromValue(0)).translationText.empty());
}

TEST_CASE("a subtitle born of a line makes the main document modified too", "[edit][translation]") {
    const std::filesystem::path directory = pairs() / "une-ligne-de-plus-a-la-fin";
    Session session{mainOf(directory)};
    const TranslationFile file = translationOf(session.project(), directory / "traduction.srt");

    AttachedTranslation result = attached(session.project(), file, TranslationMethod::Position);
    REQUIRE(result.command != nullptr);
    session.apply(std::move(result.command));

    CHECK(session.hasUnsavedChanges(Document::Main));
}

TEST_CASE("opening erases the translations that were there", "[edit][translation]") {
    // Gaupol does the same: what a translation held before is gone, whether or
    // not the new one reaches the subtitle.
    const std::filesystem::path directory = pairs() / "traduction-plus-courte";
    Project project = mainOf(directory);
    // The fourth subtitle carries a translation, and the file has no line for it.
    std::vector<Subtitle> all{project.subtitles().begin(), project.subtitles().end()};
    all[3].translationText = "Ancienne traduction.";
    project.setSubtitles(std::move(all));
    const TranslationFile file = translationOf(project, directory / "traduction.srt");

    AttachedTranslation result = attached(project, file, TranslationMethod::Position);
    REQUIRE(result.command != nullptr);
    result.command->apply(project);

    CHECK(project.subtitleAt(SubtitleIndex::fromValue(3)).translationText.empty());

    result.command->revert(project);
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(3)).translationText ==
          "Ancienne traduction.");
}

TEST_CASE("the translation keeps the file, and the format, it came from", "[edit][translation]") {
    // Advanced SSA over a SubRip main: the two documents do not have to look
    // alike (ADR 0032), and what the translation file says about itself is
    // what a later `Save Translation` will write back.
    const std::filesystem::path formats = std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "formats";
    const RealFileSystem files;
    std::expected<OpenedFile, subedit::core::OpenError> opened =
        openProject(files, formats / "scene.srt");
    REQUIRE(opened.has_value());
    Project project = std::move(opened->project);

    const TranslationFile file = translationOf(project, formats / "scene.ass");
    AttachedTranslation result = attached(project, file, TranslationMethod::Position);
    REQUIRE(result.command != nullptr);
    result.command->apply(project);

    CHECK(project.sourceFile(Document::Main).format == SubtitleFormat::SubRip);
    CHECK(project.sourceFile(Document::Translation).format ==
          SubtitleFormat::AdvancedSubStationAlpha);
    CHECK(project.translationFile().has_value());

    result.command->revert(project);
    CHECK_FALSE(project.translationFile().has_value());
}

TEST_CASE("undoing the second opening gives back the file of the first", "[edit][translation]") {
    // A translation already laid over the document, and another laid over it:
    // undoing the second is not « no translation », it is the first — its texts
    // and its file, which is what a later save would write to.
    const std::filesystem::path directory = pairs() / "temoin";
    const std::filesystem::path formats = std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "formats";
    Project project = mainOf(directory);

    const TranslationFile first = translationOf(project, directory / "traduction.srt");
    AttachedTranslation firstOpening = attached(project, first, TranslationMethod::Position);
    REQUIRE(firstOpening.command != nullptr);
    firstOpening.command->apply(project);
    const std::string textsBefore = writtenAs(project, Document::Translation);

    const TranslationFile second = translationOf(project, formats / "scene.ass");
    AttachedTranslation secondOpening = attached(project, second, TranslationMethod::Position);
    REQUIRE(secondOpening.command != nullptr);
    secondOpening.command->apply(project);
    CHECK(project.sourceFile(Document::Translation).format ==
          SubtitleFormat::AdvancedSubStationAlpha);

    secondOpening.command->revert(project);

    // Field by field, not `SourceFile == SourceFile`: whole files would
    // instantiate the comparison of every kind of file extras, which nothing
    // runs, and the coverage ratchet counts those lines.
    CHECK(project.sourceFile(Document::Translation).format == SubtitleFormat::SubRip);
    CHECK(project.sourceFile(Document::Translation).path == first.source.path);
    CHECK(project.translationFile().has_value());
    CHECK(writtenAs(project, Document::Translation) == textsBefore);
}

TEST_CASE("a translation in frames, and one with no end, both open and attach",
          "[edit][translation]") {
    // MicroDVD counts in frames and is read at the project's rate; LRC holds a
    // start and nothing else, and the reading invents the ends (ADR 0029) — the
    // middle of a line is computed on them. Both are the same scene as the main
    // document, so every line has to find its own subtitle.
    const std::filesystem::path formats = std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "formats";
    const RealFileSystem files;

    for (const std::string_view name : {"scene.microdvd.sub", "scene.lrc"}) {
        INFO("translation: " << name);
        std::expected<OpenedFile, subedit::core::OpenError> opened =
            openProject(files, formats / "scene.srt");
        REQUIRE(opened.has_value());
        Project project = std::move(opened->project);
        // MicroDVD states no rate: the scene was written at twenty-five.
        project.setFrameRate(subedit::core::FrameRate{subedit::core::StandardFrameRate::Fps25});

        const TranslationFile file = translationOf(project, formats / name);
        const AttachedTranslation result = attached(project, file, TranslationMethod::Position);

        CHECK(result.outcome.attached == 4);
        CHECK(result.outcome.born == 0);
    }
}

TEST_CASE("a file already open as the main document is refused, however it is spelled",
          "[edit][translation]") {
    InMemoryFileSystem files;
    files.addFile("/films/film.fr.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n");
    Project project;
    project.setSourceFile(SourceFile{.path = "/films/film.fr.srt"});

    for (const std::string_view spelled :
         {"/films/film.fr.srt", "/films/../films/film.fr.srt", "/films/./film.fr.srt"}) {
        INFO("spelled: " << spelled);
        const std::expected<TranslationFile, subedit::core::TranslationError> opened =
            openTranslation(files, project, std::filesystem::path{spelled});
        REQUIRE_FALSE(opened.has_value());
        CHECK(std::holds_alternative<SameFileAsMain>(opened.error()));
    }
}

TEST_CASE("a file that is not there is an error of the file system, not a refusal",
          "[edit][translation]") {
    const InMemoryFileSystem files;
    const Project project;

    const std::expected<TranslationFile, subedit::core::TranslationError> opened =
        openTranslation(files, project, "/films/absent.srt");

    REQUIRE_FALSE(opened.has_value());
    CHECK(std::holds_alternative<subedit::core::FileError>(opened.error()));
}
