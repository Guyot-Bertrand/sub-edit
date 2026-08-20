#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/order_policy.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

using subedit::core::Change;
using subedit::core::ChangeKind;
using subedit::core::Command;
using subedit::core::CommandKind;
using subedit::core::Document;
using subedit::core::Duration;
using subedit::core::OrderPolicy;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle at(std::int64_t start, std::string_view text) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(start + 1000),
                    .mainText = std::string{text}};
}

[[nodiscard]] Project projectOf(std::vector<Subtitle> subtitles) {
    Project project;
    project.setSubtitles(std::move(subtitles));
    return project;
}

[[nodiscard]] std::vector<std::string> textsOf(const Project& project) {
    std::vector<std::string> texts;
    texts.reserve(project.count());
    for (const Subtitle& subtitle : project.subtitles())
        texts.push_back(subtitle.mainText);
    return texts;
}

/// Moves one subtitle's start, announcing itself as a shift.
///
/// The smallest command that can break the order, which is what the strict
/// policy is about. The real shift arrives with issue #27.
class ShiftOne final : public Command {

public:
    ShiftOne(SubtitleIndex index, Duration shift) : m_index(index), m_shift(shift) {}

    void apply(Project& project) override {
        Subtitle& subtitle = project.subtitleAt(m_index);
        subtitle.start += m_shift;
        subtitle.end += m_shift;
    }

    void revert(Project& project) override {
        Subtitle& subtitle = project.subtitleAt(m_index);
        subtitle.start -= m_shift;
        subtitle.end -= m_shift;
    }

    [[nodiscard]] CommandKind kind() const override { return CommandKind::Shift; }

    [[nodiscard]] std::vector<Change> describe() const override {
        return {
            Change{.kind = ChangeKind::Positions, .subtitles = Selection::range(m_index, m_index)}};
    }

private:
    SubtitleIndex m_index;
    Duration m_shift;
};

/// Sets a text, announcing itself as what it is: an operation that cannot
/// disturb the order.
class SetText final : public Command {

public:
    SetText(const Project& project, SubtitleIndex index, std::string text)
        : m_index(index),
          m_newText(std::move(text)),
          m_oldText(project.subtitleAt(index).mainText) {}

    void apply(Project& project) override { project.subtitleAt(m_index).mainText = m_newText; }

    void revert(Project& project) override { project.subtitleAt(m_index).mainText = m_oldText; }

    [[nodiscard]] CommandKind kind() const override { return CommandKind::SetText; }

    [[nodiscard]] std::vector<Change> describe() const override {
        return {
            Change{.kind = ChangeKind::MainText, .subtitles = Selection::range(m_index, m_index)}};
    }

private:
    SubtitleIndex m_index;
    std::string m_newText;
    std::string m_oldText;
};

[[nodiscard]] std::unique_ptr<Command> shiftLast(Duration shift) {
    return std::make_unique<ShiftOne>(SubtitleIndex::fromValue(2), shift);
}

template<typename S>
concept ExposesMutableProject = requires(S session) { session.project().setSubtitles({}); };

} // namespace

TEST_CASE("a session hands its project out for reading only", "[edit][session]") {
    // The answer to what ADR 0010 asks: inside a session the only road to a
    // change is a command, and the compiler holds it rather than a convention.
    static_assert(!ExposesMutableProject<Session>);
    static_assert(std::is_same_v<decltype(std::declval<Session&>().project()), const Project&>);
}

TEST_CASE("a fresh session is lenient and has nothing to undo", "[edit][session]") {
    const Session session;

    CHECK(session.orderPolicy() == OrderPolicy::Lenient);
    CHECK_FALSE(session.canUndo());
    CHECK_FALSE(session.canRedo());
    CHECK(session.project().count() == 0);
}

TEST_CASE("a command applied through a session changes the project", "[edit][session]") {
    Session session{projectOf({at(0, "a"), at(2000, "b"), at(4000, "c")})};

    session.apply(std::make_unique<SetText>(session.project(), SubtitleIndex::fromValue(0), "z"));

    CHECK(textsOf(session.project()) == std::vector<std::string>{"z", "b", "c"});
    CHECK(session.canUndo());
    CHECK(session.undoableCount() == 1);
}

TEST_CASE("a lenient session leaves the disorder an operation caused", "[edit][session]") {
    // The core never sorts of its own accord: the file keeps the order it came
    // with, and `isInOrder` says whether there is any.
    Session session{projectOf({at(0, "a"), at(2000, "b"), at(4000, "c")})};

    session.apply(shiftLast(Duration::fromMilliseconds(-3000)));

    CHECK(textsOf(session.project()) == std::vector<std::string>{"a", "b", "c"});
    CHECK_FALSE(session.project().isInOrder());
}

TEST_CASE("a strict session sorts what an operation disordered", "[edit][session]") {
    Session session{projectOf({at(0, "a"), at(2000, "b"), at(4000, "c")}), OrderPolicy::Strict};

    session.apply(shiftLast(Duration::fromMilliseconds(-3000)));

    CHECK(textsOf(session.project()) == std::vector<std::string>{"a", "c", "b"});
    CHECK(session.project().isInOrder());
}

TEST_CASE("in strict mode the operation and its sort are one entry", "[edit][session]") {
    Session session{projectOf({at(0, "a"), at(2000, "b"), at(4000, "c")}), OrderPolicy::Strict};

    session.apply(shiftLast(Duration::fromMilliseconds(-3000)));

    // Both halves matter. The count alone would also hold for a session that
    // never appended the sort, so the reordering is asserted alongside it.
    CHECK(session.undoableCount() == 1);
    CHECK(textsOf(session.project()) == std::vector<std::string>{"a", "c", "b"});
}

TEST_CASE("one undo takes back both the operation and its sort", "[edit][session]") {
    Session session{projectOf({at(0, "a"), at(2000, "b"), at(4000, "c")}), OrderPolicy::Strict};

    session.apply(shiftLast(Duration::fromMilliseconds(-3000)));
    session.undo();

    CHECK(textsOf(session.project()) == std::vector<std::string>{"a", "b", "c"});
    CHECK(session.project().subtitleAt(SubtitleIndex::fromValue(2)).start.milliseconds() == 4000);
    CHECK_FALSE(session.canUndo());
    CHECK(session.canRedo());
}

TEST_CASE("redoing puts back both the operation and its sort", "[edit][session]") {
    Session session{projectOf({at(0, "a"), at(2000, "b"), at(4000, "c")}), OrderPolicy::Strict};

    session.apply(shiftLast(Duration::fromMilliseconds(-3000)));
    session.undo();
    session.redo();

    CHECK(textsOf(session.project()) == std::vector<std::string>{"a", "c", "b"});
}

TEST_CASE("a strict session leaves alone what cannot break the order", "[edit][session]") {
    // A text change gets no sort appended: the policy reads the kind of the
    // operation, and a text cannot move a start.
    Session session{projectOf({at(4000, "c"), at(0, "a")}), OrderPolicy::Lenient};
    session.setOrderPolicy(OrderPolicy::Strict);

    session.apply(std::make_unique<SetText>(session.project(), SubtitleIndex::fromValue(0), "z"));

    CHECK(textsOf(session.project()) == std::vector<std::string>{"z", "a"});
}

TEST_CASE("changing the policy does not reorder what is already there", "[edit][session]") {
    // A policy says what happens next. Sorting as a side effect of a setting
    // would be a change nobody asked for, and one nothing could undo.
    Session session{projectOf({at(4000, "c"), at(0, "a")})};

    session.setOrderPolicy(OrderPolicy::Strict);

    CHECK(textsOf(session.project()) == std::vector<std::string>{"c", "a"});
    CHECK_FALSE(session.canUndo());
}

TEST_CASE("opening a disordered file in strict mode sorts it, undoably", "[edit][session]") {
    // And marks the document as differing from its file — which it does, since
    // the file is the disordered one. Gaupol sorts at opening too, but before
    // its history exists, so nothing can take it back.
    Session session{projectOf({at(4000, "c"), at(0, "a")}), OrderPolicy::Strict};

    CHECK(textsOf(session.project()) == std::vector<std::string>{"a", "c"});
    CHECK(session.hasUnsavedChanges(Document::Main));

    session.undo();

    CHECK(textsOf(session.project()) == std::vector<std::string>{"c", "a"});
    CHECK_FALSE(session.hasUnsavedChanges(Document::Main));
}

TEST_CASE("opening an ordered file in strict mode touches nothing", "[edit][session]") {
    const Session session{projectOf({at(0, "a"), at(4000, "c")}), OrderPolicy::Strict};

    CHECK_FALSE(session.canUndo());
    CHECK_FALSE(session.hasUnsavedChanges(Document::Main));
}

TEST_CASE("a session counts modifications and forgets them once saved", "[edit][session]") {
    Session session{projectOf({at(0, "a"), at(2000, "b"), at(4000, "c")})};

    session.apply(std::make_unique<SetText>(session.project(), SubtitleIndex::fromValue(0), "z"));
    CHECK(session.modificationCount(Document::Main) == 1);

    session.markSaved(Document::Main);
    CHECK_FALSE(session.hasUnsavedChanges(Document::Main));

    session.undo();
    CHECK(session.modificationCount(Document::Main) == -1);
}

TEST_CASE("applying a command says what it changed", "[edit][session]") {
    // The core knows no signal mechanism, deliberately: it hands the report
    // back and the caller does what it likes with it. This is what lets the
    // window refresh the rows that moved instead of redrawing the table.
    Session session{projectOf({at(0, "a"), at(2000, "b"), at(4000, "c")})};

    const std::vector<Change> changes = session.apply(
        std::make_unique<SetText>(session.project(), SubtitleIndex::fromValue(0), "Bonjour."));

    REQUIRE(changes.size() == 1);
    CHECK(changes[0].kind == ChangeKind::MainText);
    CHECK(changes[0].subtitles ==
          Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(0)));
}

TEST_CASE("undoing says what it changed, the other way round", "[edit][session]") {
    Session session{projectOf({at(0, "a"), at(2000, "b"), at(4000, "c")})};
    session.apply(
        std::make_unique<SetText>(session.project(), SubtitleIndex::fromValue(0), "Bonjour."));

    const std::vector<Change> changes = session.undo();

    REQUIRE(changes.size() == 1);
    CHECK(changes[0].kind == ChangeKind::MainText);
    CHECK(changes[0].subtitles ==
          Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(0)));
}

TEST_CASE("redoing says what it changed, as the command did", "[edit][session]") {
    Session session{projectOf({at(0, "a"), at(2000, "b"), at(4000, "c")})};
    session.apply(
        std::make_unique<SetText>(session.project(), SubtitleIndex::fromValue(0), "Bonjour."));
    session.undo();

    const std::vector<Change> changes = session.redo();

    REQUIRE(changes.size() == 1);
    CHECK(changes[0].kind == ChangeKind::MainText);
}

TEST_CASE("undoing nothing says nothing", "[edit][session]") {
    // Not a case for the caller to guard against: an empty report asks for an
    // empty refresh, which is what should happen.
    Session session{projectOf({at(0, "a"), at(2000, "b"), at(4000, "c")})};

    CHECK(session.undo().empty());
    CHECK(session.redo().empty());
}

TEST_CASE("a strict sort is reported with the operation that caused it", "[edit][session]") {
    // One action for the user, one undo — and one report, holding both what the
    // command did and what the sort had to do about it.
    Session session{projectOf({at(0, "a"), at(2000, "b"), at(4000, "c")}), OrderPolicy::Strict};

    const std::vector<Change> changes = session.apply(std::make_unique<ShiftOne>(
        SubtitleIndex::fromValue(2), Duration::fromMilliseconds(-10000)));

    REQUIRE(changes.size() == 2);
    CHECK(changes[0].kind == ChangeKind::Positions);
    CHECK(changes[1].kind == ChangeKind::Reordering);
}
