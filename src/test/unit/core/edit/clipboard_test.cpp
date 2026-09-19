// The clipboard of texts — issue #382.
//
// What is under test is Gaupol's rule and the one thing the phase adds to it:
// texts and nothing else, a hole kept where a row was not selected, rows laid
// down past the end to receive what does not fit, and tags translated when the
// texts come from a document of another format.

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/clipboard.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using subedit::core::ClipboardTexts;
using subedit::core::CommandKind;
using subedit::core::copyTexts;
using subedit::core::cutTexts;
using subedit::core::Document;
using subedit::core::PastedTexts;
using subedit::core::pasteTexts;
using subedit::core::plainTextOf;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::SourceFile;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::SubtitleIndex;
using subedit::core::textsFromPlain;
using subedit::core::Timestamp;

using Texts = std::vector<std::optional<std::string>>;

[[nodiscard]] Subtitle saying(std::string_view text, std::int64_t start) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(start + 1000),
                    .mainText = std::string{text}};
}

/// Three subtitles of `format`, at zero, two and four seconds.
[[nodiscard]] Project threeOf(SubtitleFormat format = SubtitleFormat::SubRip) {
    Project project;
    project.setSubtitles({saying("Un.", 0), saying("Deux.", 2000), saying("Trois.", 4000)});
    project.setSourceFile(SourceFile{.format = format});
    return project;
}

/// Three subtitles, the second with no text at all.
[[nodiscard]] Project withAnEmptyMiddle() {
    Project project;
    project.setSubtitles({saying("Un.", 0), saying("", 2000), saying("Trois.", 4000)});
    project.setSourceFile(SourceFile{.format = SubtitleFormat::SubRip});
    return project;
}

[[nodiscard]] SubtitleIndex at(std::size_t index) {
    return SubtitleIndex::fromValue(index);
}

[[nodiscard]] Selection rows(std::initializer_list<std::size_t> values) {
    std::vector<SubtitleIndex> indices;
    for (const std::size_t value : values)
        indices.push_back(at(value));
    return Selection::of(indices);
}

[[nodiscard]] std::vector<std::string> textsOf(const Project& project) {
    std::vector<std::string> texts;
    for (const Subtitle& subtitle : project.subtitles())
        texts.push_back(subtitle.mainText);
    return texts;
}

} // namespace

TEST_CASE("a copy carries the texts and the format of the document", "[edit][clipboard]") {
    const Project project = threeOf(SubtitleFormat::WebVtt);

    const ClipboardTexts copied = copyTexts(project, rows({0, 1}), Document::Main);

    CHECK(copied.texts == Texts{"Un.", "Deux."});
    CHECK(copied.format == SubtitleFormat::WebVtt);
}

TEST_CASE("a discontinuous copy keeps a hole where a row was not selected", "[edit][clipboard]") {
    const Project project = threeOf();

    const ClipboardTexts copied = copyTexts(project, rows({0, 2}), Document::Main);

    CHECK(copied.texts == Texts{"Un.", std::nullopt, "Trois."});
}

TEST_CASE("copying nothing gives an empty clipboard", "[edit][clipboard]") {
    CHECK(copyTexts(threeOf(), Selection::of({}), Document::Main).isEmpty());
}

TEST_CASE("the plain form glues texts by a blank line, and a hole reads back as a hole",
          "[edit][clipboard]") {
    // Gaupol's `get_string` and `set_string`: a hole goes out as an empty text,
    // and an empty text comes back as a hole. What a selected row whose text is
    // empty becomes on that road is pinned by the cases below (issue #417).
    const ClipboardTexts copied{.texts = {"Un.", std::nullopt, "Deux lignes,\nla seconde."},
                                .format = SubtitleFormat::SubRip};

    const std::string plain = plainTextOf(copied);
    CHECK(plain == "Un.\n\n\n\nDeux lignes,\nla seconde.");

    const ClipboardTexts read = textsFromPlain(plain);
    CHECK(read.texts == copied.texts);
    // Read from the system, it has no format: nothing says where it came from.
    CHECK_FALSE(read.format.has_value());
    CHECK(textsFromPlain("").isEmpty());
}

TEST_CASE("a selected row with no text is copied as a hole", "[edit][clipboard]") {
    // The plain form cannot tell an empty text from a hole: both are an empty
    // piece between two blank lines. So the copy says the same thing before and
    // after that road, or the same copy would paste two ways — issue #417.
    const Project project = withAnEmptyMiddle();

    const ClipboardTexts copied = copyTexts(project, rows({0, 1, 2}), Document::Main);

    CHECK(copied.texts == Texts{"Un.", std::nullopt, "Trois."});
}

TEST_CASE("a copy read back from the plain form is the copy it was", "[edit][clipboard]") {
    // Gaupol's own comment on `set_string` — « so a copy read back is the copy
    // it was » — held for a hole and not for an empty text, until the empty text
    // became a hole on the way in.
    const Project project = withAnEmptyMiddle();

    for (const Selection& selection : {rows({0, 1, 2}), rows({0, 1}), rows({1, 2}), rows({0, 2})}) {
        const ClipboardTexts copied = copyTexts(project, selection, Document::Main);
        INFO("selection of " << selection.count() << " row(s)");
        CHECK(textsFromPlain(plainTextOf(copied)).texts == copied.texts);
    }
}

TEST_CASE("an empty text pastes as a hole, and leaves its row as it was", "[edit][clipboard]") {
    // A target whose three texts all differ from what is copied, so that each
    // row that is written shows as a change and the one that is not shows as
    // its own text.
    Project target;
    target.setSubtitles({saying("A.", 0), saying("B.", 2000), saying("C.", 4000)});
    target.setSourceFile(SourceFile{.format = SubtitleFormat::SubRip});
    Session session{target};

    const ClipboardTexts copied = copyTexts(withAnEmptyMiddle(), rows({0, 1, 2}), Document::Main);
    PastedTexts pasted = pasteTexts(session.project(), copied, at(0), Document::Main);
    REQUIRE(pasted.command != nullptr);
    session.apply(std::move(pasted.command));

    // The middle row keeps its text: what was copied there was nothing.
    CHECK(textsOf(session.project()) == std::vector<std::string>{"Un.", "B.", "Trois."});
}

TEST_CASE("a lone empty text pastes nothing, from either road", "[edit][clipboard]") {
    // Read back from the plain form, a lone hole is an empty string, which is
    // an empty clipboard; kept as it was copied, it is a clipboard of one hole.
    // Neither writes a row.
    const Project source = withAnEmptyMiddle();
    const Project target = threeOf();

    const ClipboardTexts kept = copyTexts(source, rows({1}), Document::Main);
    const ClipboardTexts read = textsFromPlain(plainTextOf(kept));

    CHECK(pasteTexts(target, kept, at(0), Document::Main).command == nullptr);
    CHECK(pasteTexts(target, read, at(0), Document::Main).command == nullptr);
}

TEST_CASE("cutting empties the texts, and undoing gives them back", "[edit][clipboard]") {
    Session session{threeOf()};

    std::unique_ptr<subedit::core::Command> command =
        cutTexts(session.project(), rows({1, 2}), Document::Main);
    REQUIRE(command != nullptr);
    CHECK(command->kind() == CommandKind::Cut);
    static_cast<void>(session.apply(std::move(command)));

    CHECK(textsOf(session.project()) == std::vector<std::string>{"Un.", "", ""});

    static_cast<void>(session.undo());
    CHECK(textsOf(session.project()) == std::vector<std::string>{"Un.", "Deux.", "Trois."});
    CHECK_FALSE(session.canUndo());
}

TEST_CASE("cutting texts that are already empty is no operation", "[edit][clipboard]") {
    Project project;
    project.setSubtitles({saying("", 0)});

    CHECK(cutTexts(project, rows({0}), Document::Main) == nullptr);
}

TEST_CASE("pasting writes the texts from the row given, and moves no position",
          "[edit][clipboard]") {
    Session session{threeOf()};
    const ClipboardTexts clipboard{.texts = {"A.", "B."}, .format = SubtitleFormat::SubRip};

    PastedTexts pasted = pasteTexts(session.project(), clipboard, at(1), Document::Main);
    REQUIRE(pasted.command != nullptr);
    CHECK(pasted.command->kind() == CommandKind::Paste);
    CHECK(pasted.inserted == 0);
    static_cast<void>(session.apply(std::move(pasted.command)));

    CHECK(textsOf(session.project()) == std::vector<std::string>{"Un.", "A.", "B."});
    CHECK(session.project().subtitleAt(at(1)).start == Timestamp::fromMilliseconds(2000));
    CHECK(session.project().subtitleAt(at(2)).end == Timestamp::fromMilliseconds(5000));
}

TEST_CASE("a hole in the clipboard leaves its row as it was", "[edit][clipboard]") {
    Project project = threeOf();
    const ClipboardTexts clipboard{.texts = {"A.", std::nullopt, "C."},
                                   .format = SubtitleFormat::SubRip};

    PastedTexts pasted = pasteTexts(project, clipboard, at(0), Document::Main);
    REQUIRE(pasted.command != nullptr);
    pasted.command->apply(project);

    CHECK(textsOf(project) == std::vector<std::string>{"A.", "Deux.", "C."});
}

TEST_CASE("pasting past the end lays rows down, and one undo takes them back",
          "[edit][clipboard]") {
    Session session{threeOf()};
    const ClipboardTexts clipboard{.texts = {"A.", std::nullopt, "C.", "D."},
                                   .format = std::nullopt};

    PastedTexts pasted = pasteTexts(session.project(), clipboard, at(2), Document::Main);
    REQUIRE(pasted.command != nullptr);
    CHECK(pasted.inserted == 3);
    static_cast<void>(session.apply(std::move(pasted.command)));

    // The hole past the end is a blank row: there was nothing there to keep.
    CHECK(textsOf(session.project()) ==
          std::vector<std::string>{"Un.", "Deux.", "A.", "", "C.", "D."});
    // Laid down as an insertion lays them: after the last, three seconds each.
    CHECK(session.project().subtitleAt(at(3)).start == Timestamp::fromMilliseconds(5000));
    CHECK(session.project().subtitleAt(at(5)).end == Timestamp::fromMilliseconds(14000));

    static_cast<void>(session.undo());
    CHECK(textsOf(session.project()) == std::vector<std::string>{"Un.", "Deux.", "Trois."});
    CHECK_FALSE(session.canUndo());
}

TEST_CASE("pasting into an empty document lays every row down", "[edit][clipboard]") {
    Project project;
    const ClipboardTexts clipboard{.texts = {"A.", "B."}, .format = std::nullopt};

    PastedTexts pasted = pasteTexts(project, clipboard, at(0), Document::Main);
    REQUIRE(pasted.command != nullptr);
    CHECK(pasted.inserted == 2);
    pasted.command->apply(project);

    CHECK(textsOf(project) == std::vector<std::string>{"A.", "B."});
}

TEST_CASE("pasting the texts already there is no operation", "[edit][clipboard]") {
    const Project project = threeOf();
    const ClipboardTexts clipboard = copyTexts(project, rows({0, 1}), Document::Main);

    CHECK(pasteTexts(project, clipboard, at(0), Document::Main).command == nullptr);
}

TEST_CASE("texts from another format have their tags translated", "[edit][clipboard]") {
    // ADR 0031 at the clipboard's door: the braces of an Advanced SSA do not
    // land in a SubRip as they are.
    Project project = threeOf(SubtitleFormat::SubRip);
    const ClipboardTexts clipboard{.texts = {R"({\i1}Bonjour{\i0})"},
                                   .format = SubtitleFormat::AdvancedSubStationAlpha};

    PastedTexts pasted = pasteTexts(project, clipboard, at(0), Document::Main);
    REQUIRE(pasted.command != nullptr);
    CHECK(pasted.droppedTags == 0);
    pasted.command->apply(project);

    CHECK(project.subtitleAt(at(0)).mainText == "<i>Bonjour</i>");
}

TEST_CASE("what a format cannot write is counted", "[edit][clipboard]") {
    // TMPlayer carries no style at all: the italic goes, and the count says so.
    Project project = threeOf(SubtitleFormat::TMPlayer);
    const ClipboardTexts clipboard{.texts = {"<i>Bonjour</i>"}, .format = SubtitleFormat::SubRip};

    PastedTexts pasted = pasteTexts(project, clipboard, at(0), Document::Main);
    REQUIRE(pasted.command != nullptr);
    // One pair of tags, one post: the count is of styles lost, not of brackets.
    CHECK(pasted.droppedTags == 1);
    pasted.command->apply(project);

    CHECK(project.subtitleAt(at(0)).mainText == "Bonjour");
}

TEST_CASE("texts of no format are pasted as they are", "[edit][clipboard]") {
    // Copied from outside this program: there is nothing to translate from.
    Project project = threeOf(SubtitleFormat::SubRip);
    const ClipboardTexts clipboard{.texts = {R"({\i1}Bonjour)"}, .format = std::nullopt};

    PastedTexts pasted = pasteTexts(project, clipboard, at(0), Document::Main);
    REQUIRE(pasted.command != nullptr);
    CHECK(pasted.droppedTags == 0);
    pasted.command->apply(project);

    CHECK(project.subtitleAt(at(0)).mainText == R"({\i1}Bonjour)");
}
