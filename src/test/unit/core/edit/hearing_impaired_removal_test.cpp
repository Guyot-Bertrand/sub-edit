// What the command adds to the rule: a project, an undo, and one entry in the
// history.
//
// The rule itself is not retested here — `mentions.cas` covers it, case by
// case, through the pure function. What is under test is the composition: the
// right subtitles rewritten, the emptied ones taken out, and the whole of it
// undone as one.

#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

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
using subedit::core::removeHearingImpaired;
using subedit::core::Session;
using subedit::core::Subtitle;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle saying(std::string_view text, std::int64_t start) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(start + 1000),
                    .mainText = std::string{text}};
}

/// Four subtitles: one untouched, one rewritten, one emptied, one untouched.
[[nodiscard]] Project fourSubtitles() {
    Project project;
    project.setSubtitles({saying("Bonjour.", 0),
                          saying("Attends [il tousse] Marie", 2000),
                          saying("[Bruit de pas]", 4000),
                          saying("Voir [1] la note", 6000)});
    return project;
}

[[nodiscard]] std::vector<std::string> textsOf(const Project& project) {
    std::vector<std::string> texts;
    texts.reserve(project.count());
    for (const Subtitle& subtitle : project.subtitles())
        texts.push_back(subtitle.mainText);
    return texts;
}

} // namespace

TEST_CASE("removing hearing impaired mentions rewrites what bites", "[edit]") {
    Project project = fourSubtitles();

    const std::unique_ptr<Command> command = removeHearingImpaired(project, Document::Main);
    REQUIRE(command != nullptr);
    command->apply(project);

    // The emptied subtitle is gone, the reference is left alone.
    CHECK(textsOf(project) ==
          std::vector<std::string>{"Bonjour.", "Attends Marie", "Voir [1] la note"});
}

TEST_CASE("the removal names itself in the history", "[edit]") {
    const Project project = fourSubtitles();

    const std::unique_ptr<Command> command = removeHearingImpaired(project, Document::Main);
    REQUIRE(command != nullptr);
    CHECK(command->kind() == CommandKind::RemoveHearingImpaired);
}

TEST_CASE("a file with no mention yields no command at all", "[edit]") {
    // Not an empty group: applying one would do nothing and still push an
    // entry the user would find in « undo » without understanding it.
    Project project;
    project.setSubtitles({saying("Bonjour.", 0), saying("Voir [1] la note", 2000)});

    CHECK(removeHearingImpaired(project, Document::Main) == nullptr);
}

TEST_CASE("reverting the removal restores the project exactly", "[edit]") {
    const Project before = fourSubtitles();
    Project project = before;

    const std::unique_ptr<Command> command = removeHearingImpaired(project, Document::Main);
    REQUIRE(command != nullptr);
    command->apply(project);
    command->revert(project);

    REQUIRE(project.count() == before.count());
    for (std::size_t rank = 0; rank < before.count(); ++rank) {
        INFO("sous-titre " << rank);
        CHECK(project.subtitles()[rank] == before.subtitles()[rank]);
    }
}

TEST_CASE("a discontinuous removal puts every subtitle back where it was", "[edit]") {
    // Two emptied subtitles with a survivor between them: the indices to put
    // back are not contiguous, which is where a naive undo would land them in
    // the wrong order.
    Project before;
    before.setSubtitles({saying("[Musique]", 0),
                         saying("Bonjour.", 2000),
                         saying("(soupir)", 4000),
                         saying("Adieu.", 6000)});
    Project project = before;

    const std::unique_ptr<Command> command = removeHearingImpaired(project, Document::Main);
    REQUIRE(command != nullptr);
    command->apply(project);
    REQUIRE(textsOf(project) == std::vector<std::string>{"Bonjour.", "Adieu."});

    command->revert(project);
    REQUIRE(project.count() == before.count());
    for (std::size_t rank = 0; rank < before.count(); ++rank) {
        INFO("sous-titre " << rank);
        CHECK(project.subtitles()[rank] == before.subtitles()[rank]);
    }
}

TEST_CASE("the whole removal is one entry in the history", "[edit]") {
    // The point of grouping: a user who asks for one cleaning undoes it once,
    // not once per subtitle it touched.
    Session session{fourSubtitles()};

    std::unique_ptr<Command> command = removeHearingImpaired(session.project(), Document::Main);
    REQUIRE(command != nullptr);
    session.apply(std::move(command));

    CHECK(session.undoableCount() == 1);
    session.undo();
    CHECK(session.project().count() == 4);
    CHECK(session.undoableCount() == 0);
}

TEST_CASE("the translation document is cleaned on demand", "[edit]") {
    // The core takes the document; the command line only ever passes the main
    // one, because it has no notion of a translation. The window of phase 5
    // will, and this is what says the core is ready for it.
    Project project;
    Subtitle both = saying("Bonjour [rires]", 0);
    both.translationText = "Hello [laughs]";
    project.setSubtitles({both});

    const std::unique_ptr<Command> command = removeHearingImpaired(project, Document::Translation);
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(project.subtitles().front().mainText == "Bonjour [rires]");
    CHECK(project.subtitles().front().translationText == "Hello");
}
