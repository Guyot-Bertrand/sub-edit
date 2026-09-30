// The spell-check walk over several projects — issue #509, decision D6. The
// dictionary is written in the test; no project is touched by the walk.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/spell_check_walk.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

using subedit::core::applyCorrections;
using subedit::core::CorrectionTarget;
using subedit::core::Document;
using subedit::core::InMemoryFileSystem;
using subedit::core::openSpellChecker;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::SpellChecker;
using subedit::core::SpellCheckWalk;
using subedit::core::SpellStop;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;
using subedit::core::WordList;
using subedit::core::WordListSpellProvider;

SpellChecker checkerOver(const std::vector<std::string>& words) {
    WordList list;
    list.words.insert(words.begin(), words.end());
    WordListSpellProvider provider;
    provider.add("fr", std::move(list));
    const InMemoryFileSystem files;
    return openSpellChecker(provider, "fr", files, "/none.repl").value();
}

Project projectOf(const std::vector<std::string>& texts, Document document = Document::Main) {
    Project project;
    std::vector<Subtitle> subtitles;
    std::int64_t start = 0;
    for (const std::string& text : texts) {
        Subtitle subtitle{.start = Timestamp::fromMilliseconds(start),
                          .end = Timestamp::fromMilliseconds(start + 900),
                          .mainText = document == Document::Main ? text : "ok"};
        if (document == Document::Translation)
            subtitle.translationText = text;
        subtitles.push_back(std::move(subtitle));
        start += 1000;
    }
    project.setSubtitles(std::move(subtitles));
    return project;
}

CorrectionTarget whole(const Project& project, Document document = Document::Main) {
    return {.project = &project, .selection = Selection::all(project), .document = document};
}

const std::vector<std::string> kWords{"ok", "salut", "bien", "va"};

/// The stop the walk answered, the test failing right here if there is none.
SpellStop expectStop(const std::optional<SpellStop>& stop) {
    REQUIRE(stop.has_value());
    return stop.value_or(SpellStop{.project = nullptr,
                                   .index = SubtitleIndex::fromValue(0),
                                   .document = Document::Main,
                                   .text = {},
                                   .pos = 0,
                                   .endPos = 0,
                                   .word = {}});
}

} // namespace

TEST_CASE("the walk goes over two projects in order", "[text][spell][walk]") {
    const Project first = projectOf({"ok qqq", "bien"});
    const Project second = projectOf({"ok", "yyy va"});
    SpellCheckWalk walk{checkerOver(kWords), {whole(first), whole(second)}};

    SpellStop stop = expectStop(walk.advance());
    CHECK(stop.project == &first);
    CHECK(stop.index == SubtitleIndex::fromValue(0));
    CHECK(stop.word == "qqq");
    CHECK(stop.pos == 3);
    CHECK(stop.endPos == 6);
    CHECK(stop.text == "ok qqq");
    walk.ignore();

    stop = expectStop(walk.advance());
    CHECK(stop.project == &second);
    CHECK(stop.index == SubtitleIndex::fromValue(1));
    CHECK(stop.word == "yyy");
    walk.ignore();

    CHECK_FALSE(walk.advance().has_value());
    CHECK(walk.corrections().empty());
}

TEST_CASE("a partial selection is walked and no more", "[text][spell][walk]") {
    const Project project = projectOf({"aaa", "bbb", "ccc", "ddd"});
    const std::array<SubtitleIndex, 2> chosen{SubtitleIndex::fromValue(1),
                                              SubtitleIndex::fromValue(3)};
    SpellCheckWalk walk{checkerOver(kWords),
                        {{.project = &project, .selection = Selection::of(chosen)}}};

    SpellStop stop = expectStop(walk.advance());
    CHECK(stop.word == "bbb");
    walk.ignore();
    stop = expectStop(walk.advance());
    CHECK(stop.word == "ddd");
    walk.ignore();
    CHECK_FALSE(walk.advance().has_value());
}

TEST_CASE("the walk reads the translation when told to", "[text][spell][walk]") {
    const Project project = projectOf({"ok qqq", "bien"}, Document::Translation);
    SpellCheckWalk walk{checkerOver(kWords), {whole(project, Document::Translation)}};

    SpellStop stop = expectStop(walk.advance());
    CHECK(stop.document == Document::Translation);
    CHECK(stop.word == "qqq");
    walk.replace("salut");

    const auto corrections = walk.corrections();
    REQUIRE(corrections.size() == 1);
    CHECK(corrections[0].document == Document::Translation);
    CHECK(corrections[0].original == "ok qqq");
    CHECK(corrections[0].proposed == std::optional<std::string>{"ok salut"});
}

TEST_CASE("a text left as it was earns no correction", "[text][spell][walk]") {
    const Project project = projectOf({"ok qqq", "bien zzz"});
    SpellCheckWalk walk{checkerOver(kWords), {whole(project)}};

    REQUIRE(walk.advance().has_value());
    walk.ignore();
    REQUIRE(walk.advance().has_value());
    walk.replace("va");
    CHECK_FALSE(walk.advance().has_value());

    const auto corrections = walk.corrections();
    REQUIRE(corrections.size() == 1);
    CHECK(corrections[0].index == SubtitleIndex::fromValue(1));
    CHECK(corrections[0].proposed == std::optional<std::string>{"bien va"});
}

TEST_CASE("closing in the middle keeps what was done and nothing else", "[text][spell][walk]") {
    const Project project = projectOf({"qqq ok", "zzz ok", "yyy ok"});
    SpellCheckWalk walk{checkerOver(kWords), {whole(project)}};

    REQUIRE(walk.advance().has_value());
    walk.replace("salut");
    REQUIRE(walk.advance().has_value()); // text 1, now current
    walk.replace("bien");
    REQUIRE(walk.advance().has_value()); // text 2, current: no gesture yet

    const auto corrections = walk.corrections();
    REQUIRE(corrections.size() == 2);
    CHECK(corrections[0].proposed == std::optional<std::string>{"salut ok"});
    CHECK(corrections[1].proposed == std::optional<std::string>{"bien ok"});
}

TEST_CASE("a gesture on the current text counts before the text is left", "[text][spell][walk]") {
    const Project project = projectOf({"qqq ok", "zzz"});
    SpellCheckWalk walk{checkerOver(kWords), {whole(project)}};

    REQUIRE(walk.advance().has_value());
    walk.replace("salut");

    const auto corrections = walk.corrections();
    REQUIRE(corrections.size() == 1);
    CHECK(corrections[0].proposed == std::optional<std::string>{"salut ok"});
    // Asking twice changes nothing.
    CHECK(walk.corrections().size() == 1);
}

TEST_CASE("replace all crosses texts and projects", "[text][spell][walk]") {
    const Project first = projectOf({"sallut ok", "ok"});
    const Project second = projectOf({"bien sallut", "ok zzz"});
    SpellCheckWalk walk{checkerOver(kWords), {whole(first), whole(second)}};

    SpellStop stop = expectStop(walk.advance());
    CHECK(stop.word == "sallut");
    walk.replaceAll("salut");

    // The second project's "sallut" is corrected silently: the walk stops on "zzz".
    stop = expectStop(walk.advance());
    CHECK(stop.project == &second);
    CHECK(stop.word == "zzz");
    walk.ignore();
    CHECK_FALSE(walk.advance().has_value());

    const auto corrections = walk.corrections();
    REQUIRE(corrections.size() == 2);
    CHECK(corrections[0].project == &first);
    CHECK(corrections[0].proposed == std::optional<std::string>{"salut ok"});
    CHECK(corrections[1].project == &second);
    CHECK(corrections[1].proposed == std::optional<std::string>{"bien salut"});
    // The silent replacement is remembered too, as in Gaupol; the list is
    // made unique when written.
    CHECK_FALSE(walk.checker().replacements().empty());
}

TEST_CASE("join and ignore all go through the walk", "[text][spell][walk]") {
    const Project project = projectOf({"ok sa lut qqq", "qqq"});
    SpellCheckWalk walk{checkerOver(kWords), {whole(project)}};

    SpellStop stop = expectStop(walk.advance());
    CHECK(stop.word == "sa");
    CHECK(walk.spaceAfter());
    walk.joinWithNext();
    // The joined "salut" is a word of the dictionary: the walk goes past it.
    stop = expectStop(walk.advance());
    CHECK(stop.word == "qqq");
    CHECK(stop.text == "ok salut qqq");
    walk.ignoreAll();
    CHECK_FALSE(walk.advance().has_value());
    CHECK(walk.corrections().size() == 1);
}

TEST_CASE("resuming with a typed text goes on from its start", "[text][spell][walk]") {
    const Project project = projectOf({"qqq", "bien"});
    SpellCheckWalk walk{checkerOver(kWords), {whole(project)}};

    REQUIRE(walk.advance().has_value());
    walk.resumeWithText("salut yyy");
    SpellStop stop = expectStop(walk.advance());
    CHECK(stop.word == "yyy");
    CHECK(stop.index == SubtitleIndex::fromValue(0));
    walk.replace("va");

    const auto corrections = walk.corrections();
    REQUIRE(corrections.size() == 1);
    CHECK(corrections[0].original == "qqq");
    CHECK(corrections[0].proposed == std::optional<std::string>{"salut va"});
}

TEST_CASE("accented text is walked and corrected by bytes", "[text][spell][walk]") {
    const Project project = projectOf({"été qqé ok"});
    SpellCheckWalk walk{checkerOver({"été", "ok", "évité"}), {whole(project)}};

    SpellStop stop = expectStop(walk.advance());
    CHECK(stop.word == "qqé");
    CHECK(stop.pos == 6);
    CHECK(stop.endPos == 10);
    walk.replace("évité");

    const auto corrections = walk.corrections();
    REQUIRE(corrections.size() == 1);
    CHECK(corrections[0].proposed == std::optional<std::string>{"été évité ok"});
}

TEST_CASE("the corrections go through one command per project", "[text][spell][walk]") {
    Project first = projectOf({"qqq", "ok"});
    Project second = projectOf({"ok", "zzz"});
    SpellCheckWalk walk{checkerOver(kWords), {whole(first), whole(second)}};

    REQUIRE(walk.advance().has_value());
    walk.replace("salut");
    REQUIRE(walk.advance().has_value());
    walk.replace("bien");
    CHECK_FALSE(walk.advance().has_value());

    const auto composed = applyCorrections(walk.corrections(), true);
    REQUIRE(composed.size() == 2);
    CHECK(composed[0].project == &first);
    CHECK(composed[1].project == &second);
    composed[0].command->apply(first);
    composed[1].command->apply(second);
    CHECK(first.subtitles()[0].mainText == "salut");
    CHECK(second.subtitles()[1].mainText == "bien");
}
