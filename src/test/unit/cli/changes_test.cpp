// The list of what a subcommand of text would change, and the two forms it takes.

#include <subedit/cli/changes.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

using subedit::cli::changesOf;
using subedit::cli::TextChange;
using subedit::cli::textOf;
using subedit::core::Document;

TEST_CASE("a change reads as its number, the text before and the text after",
          "[cli][changes][CLI-DRYRUN-04]") {
    const std::vector<TextChange> changes{
        {.subtitle = 2, .document = Document::Main, .before = "Hello [cough]", .after = "Hello"}};

    CHECK(textOf("a.srt", changes) == "a.srt: subtitle 2\n- Hello [cough]\n+ Hello\n");
}

TEST_CASE("every line of a text has its own prefix", "[cli][changes][CLI-DRYRUN-04]") {
    const std::vector<TextChange> changes{{.subtitle = 5,
                                           .document = Document::Main,
                                           .before = "One [a\nb] two",
                                           .after = "One\ntwo"}};

    CHECK(textOf("a.srt", changes) == "a.srt: subtitle 5\n- One [a\n- b] two\n+ One\n+ two\n");
}

TEST_CASE("a removed subtitle says so and has no text after", "[cli][changes][CLI-DRYRUN-04]") {
    const std::vector<TextChange> changes{
        {.subtitle = 1, .document = Document::Main, .before = "[door]"}};

    CHECK(textOf("a.srt", changes) == "a.srt: subtitle 1 (removed)\n- [door]\n");
}

TEST_CASE("the translation is named, and an empty text keeps its line",
          "[cli][changes][CLI-DRYRUN-04]") {
    const std::vector<TextChange> changes{
        {.subtitle = 3, .document = Document::Translation, .before = "[porte]", .after = ""}};

    CHECK(textOf("a.srt", changes) == "a.srt: subtitle 3 (translation)\n- [porte]\n+ \n");
}

TEST_CASE("no change is no text", "[cli][changes][CLI-DRYRUN-04]") {
    CHECK(textOf("a.srt", {}).empty());
}

TEST_CASE("the same changes are the json, after being null for a removal",
          "[cli][changes][CLI-DRYRUN-05]") {
    const std::vector<TextChange> changes{
        {.subtitle = 1, .document = Document::Main, .before = "[door]"},
        {.subtitle = 2, .document = Document::Translation, .before = "a\"b", .after = "c\nd"}};

    CHECK(changesOf(changes).dump() ==
          R"([{"subtitle":1,"document":"main","before":"[door]","after":null},)"
          R"({"subtitle":2,"document":"translation","before":"a\"b","after":"c\nd"}])");
    CHECK(changesOf({}).dump() == "[]");
}
