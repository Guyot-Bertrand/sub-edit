#include <subedit/core/command/change.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {

using subedit::core::affects;
using subedit::core::Change;
using subedit::core::ChangeKind;
using subedit::core::Document;
using subedit::core::invert;
using subedit::core::Selection;
using subedit::core::SubtitleIndex;

} // namespace

TEST_CASE("a change names what it touched and where", "[command][change]") {
    const Change change{
        .kind = ChangeKind::Positions,
        .subtitles = Selection::range(SubtitleIndex::fromValue(3), SubtitleIndex::fromValue(4)),
    };

    CHECK(change.kind == ChangeKind::Positions);
    CHECK(change.subtitles.count() == 2);
    CHECK(change.subtitles.contains(SubtitleIndex::fromValue(3)));
}

TEST_CASE("a change of positions makes both documents stale", "[command][change]") {
    // A subtitle has one pair of positions for both texts: moving it moves the
    // translation too, so both documents differ from what is on disk.
    CHECK(affects(ChangeKind::Positions, Document::Main));
    CHECK(affects(ChangeKind::Positions, Document::Translation));
}

TEST_CASE("a change of text makes only its own document stale", "[command][change]") {
    CHECK(affects(ChangeKind::MainText, Document::Main));
    CHECK_FALSE(affects(ChangeKind::MainText, Document::Translation));

    CHECK(affects(ChangeKind::TranslationText, Document::Translation));
    CHECK_FALSE(affects(ChangeKind::TranslationText, Document::Main));
}

TEST_CASE("inserting or removing a subtitle makes both documents stale", "[command][change]") {
    // A subtitle carries both texts, so adding or removing one changes the two
    // documents at once, whatever their contents.
    CHECK(affects(ChangeKind::Insertion, Document::Main));
    CHECK(affects(ChangeKind::Insertion, Document::Translation));
    CHECK(affects(ChangeKind::Removal, Document::Main));
    CHECK(affects(ChangeKind::Removal, Document::Translation));
}

TEST_CASE("undoing an insertion is a removal, and the other way round", "[command][change]") {
    // What the interface needs to hear when a command is undone: the rows it
    // added are the rows that go. The indices are the same in both directions,
    // so only the kind turns over.
    CHECK(invert(ChangeKind::Insertion) == ChangeKind::Removal);
    CHECK(invert(ChangeKind::Removal) == ChangeKind::Insertion);
}

TEST_CASE("undoing anything else changes nothing about what it touched", "[command][change]") {
    // A text put back is still a text changed, and the rows to refresh are the
    // same ones. Only a change of structure has a direction.
    CHECK(invert(ChangeKind::Positions) == ChangeKind::Positions);
    CHECK(invert(ChangeKind::MainText) == ChangeKind::MainText);
    CHECK(invert(ChangeKind::TranslationText) == ChangeKind::TranslationText);
    CHECK(invert(ChangeKind::Reordering) == ChangeKind::Reordering);
}
