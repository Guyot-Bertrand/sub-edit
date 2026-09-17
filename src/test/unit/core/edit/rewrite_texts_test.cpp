// The one loop every text-rewriting operation shares — issue #404.
//
// Italics, letter case, dialogue dashes and cut all build on `rewriteTexts`;
// what is under test here is the loop itself, not any one vocabulary of it.

#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cctype>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::Command;
using subedit::core::CommandKind;
using subedit::core::Document;
using subedit::core::Project;
using subedit::core::rewriteTexts;
using subedit::core::rewrittenCount;
using subedit::core::Selection;
using subedit::core::SourceFile;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle saying(std::string_view text, std::int64_t start) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(start + 1000),
                    .mainText = std::string{text}};
}

[[nodiscard]] Project documentOf(std::vector<Subtitle> subtitles) {
    Project project;
    project.setSubtitles(std::move(subtitles));
    project.setSourceFile(SourceFile{.format = SubtitleFormat::SubRip});
    return project;
}

} // namespace

TEST_CASE("rewriteTexts skips what the transformation leaves unchanged", "[core][edit]") {
    const Project project = documentOf({saying("Bonjour.", 0), saying("Au revoir.", 2000)});

    const std::unique_ptr<Command> command =
        rewriteTexts(project,
                     Selection::all(project),
                     Document::Main,
                     CommandKind::ChangeCase,
                     [](const std::string& text) { return text; });

    CHECK(command == nullptr);
}

TEST_CASE("rewriteTexts groups every real change under the kind it is given", "[core][edit]") {
    const Project project = documentOf({saying("Bonjour.", 0), saying("Au revoir.", 2000)});

    const std::unique_ptr<Command> command = rewriteTexts(
        project,
        Selection::all(project),
        Document::Main,
        CommandKind::ChangeCase,
        [](const std::string& text) {
            std::string upper = text;
            for (char& letter : upper)
                letter = static_cast<char>(std::toupper(static_cast<unsigned char>(letter)));
            return upper;
        });

    REQUIRE(command != nullptr);
    CHECK(command->kind() == CommandKind::ChangeCase);
    CHECK(rewrittenCount(*command) == 2);
}
