#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

using subedit::core::ChangeKind;
using subedit::core::CommandKind;
using subedit::core::Document;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::SetTextCommand;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;

constexpr SubtitleIndex kFirst = SubtitleIndex::fromValue(0);

[[nodiscard]] Project projectOf() {
    Project project;
    project.setSubtitles({
        Subtitle{.mainText = "Bonjour.", .translationText = "Hello."},
        Subtitle{.mainText = "Au revoir.", .translationText = "Goodbye."},
    });
    return project;
}

} // namespace

TEST_CASE("setting a text replaces it", "[edit][settext]") {
    Project project = projectOf();
    SetTextCommand command{project, kFirst, Document::Main, "Salut."};

    command.apply(project);

    CHECK(project.subtitleAt(kFirst).mainText == "Salut.");
}

TEST_CASE("undoing a text change puts the old text back", "[edit][settext]") {
    Project project = projectOf();
    SetTextCommand command{project, kFirst, Document::Main, "Salut."};

    command.apply(project);
    command.revert(project);

    CHECK(project.subtitleAt(kFirst).mainText == "Bonjour.");
}

TEST_CASE("a text change leaves the other document alone", "[edit][settext]") {
    // The two texts of a subtitle are independent; only their positions are
    // shared. Touching one must not touch the other.
    Project project = projectOf();
    SetTextCommand command{project, kFirst, Document::Translation, "Hi."};

    command.apply(project);

    CHECK(project.subtitleAt(kFirst).translationText == "Hi.");
    CHECK(project.subtitleAt(kFirst).mainText == "Bonjour.");
}

TEST_CASE("a text change reports the document it touched", "[edit][settext]") {
    // What the modification counter reads to know which document differs from
    // its file: a change of main text must not mark the translation.
    const Project project = projectOf();
    const SetTextCommand main{project, kFirst, Document::Main, "Salut."};
    const SetTextCommand translation{project, kFirst, Document::Translation, "Hi."};

    REQUIRE(main.describe().size() == 1);
    CHECK(main.describe()[0].kind == ChangeKind::MainText);
    CHECK(main.describe()[0].subtitles == Selection::range(kFirst, kFirst));

    REQUIRE(translation.describe().size() == 1);
    CHECK(translation.describe()[0].kind == ChangeKind::TranslationText);
}

TEST_CASE("a text change says what it is", "[edit][settext]") {
    const Project project = projectOf();
    const SetTextCommand command{project, kFirst, Document::Main, "Salut."};

    CHECK(command.kind() == CommandKind::SetText);
}

TEST_CASE("the modification counter follows the document that changed", "[edit][settext]") {
    Session session{projectOf()};

    session.apply(
        std::make_unique<SetTextCommand>(session.project(), kFirst, Document::Main, "Salut."));

    CHECK(session.modificationCount(Document::Main) == 1);
    CHECK(session.modificationCount(Document::Translation) == 0);
}

TEST_CASE("undoing then redoing a text change restores the exact state", "[edit][settext]") {
    Session session{projectOf()};
    const std::string before = session.project().subtitleAt(kFirst).mainText;

    session.apply(
        std::make_unique<SetTextCommand>(session.project(), kFirst, Document::Main, "Salut."));
    session.undo();
    CHECK(session.project().subtitleAt(kFirst).mainText == before);
    CHECK(session.modificationCount(Document::Main) == 0);

    session.redo();
    CHECK(session.project().subtitleAt(kFirst).mainText == "Salut.");
    CHECK(session.modificationCount(Document::Main) == 1);
}

TEST_CASE("setting a text to what it already was is still a command", "[edit][settext]") {
    // Not silently dropped: whoever built the command asked for it, and the
    // history has to stay a faithful account of what was done. Deciding that
    // an operation was pointless is the caller's business.
    Project project = projectOf();
    SetTextCommand command{project, kFirst, Document::Main, "Bonjour."};

    command.apply(project);
    command.revert(project);

    CHECK(project.subtitleAt(kFirst).mainText == "Bonjour.");
    CHECK(command.describe().size() == 1);
}
