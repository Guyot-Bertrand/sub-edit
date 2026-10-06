// The shared options, read without a process: the range, the reading choices, the
// translation, and the order `prepare` reads them in.

#include <subedit/cli/options.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>
#include <vector>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::DestinationOptions;
using subedit::cli::Inputs;
using subedit::cli::pairingOf;
using subedit::cli::Preparation;
using subedit::cli::prepare;
using subedit::cli::prepareWriting;
using subedit::cli::rangeOf;
using subedit::cli::readingWith;
using subedit::cli::Reporter;
using subedit::cli::TranslationOptions;
using subedit::core::InMemoryFileSystem;

namespace {

const std::string kOne = "1\n00:00:01,000 --> 00:00:02,000\none\n\n";

Inputs inputs(std::vector<std::string> paths) {
    return Inputs{.paths = std::move(paths), .roots = {}};
}

} // namespace

TEST_CASE("a range is nothing when it was not given, and names its option when it is wrong",
          "[cli][options][CLI-RANGE-01]") {
    CHECK_FALSE(rangeOf("").value().has_value());
    CHECK(rangeOf("2-4").value().has_value());

    const auto refused = rangeOf("5-2");
    REQUIRE_FALSE(refused.has_value());
    CHECK_THAT(refused.error(), ContainsSubstring("--range"));
}

TEST_CASE("a frame rate that names nothing is refused, and one that does is carried",
          "[cli][options]") {
    CHECK_FALSE(readingWith(std::nullopt, "").value().frameRate.has_value());
    CHECK(readingWith(std::nullopt, "25").value().frameRate.has_value());

    const auto refused = readingWith(std::nullopt, "banana");
    REQUIRE_FALSE(refused.has_value());
    CHECK_THAT(refused.error(), ContainsSubstring("--frame-rate"));
}

TEST_CASE("the document and the translation file come together, on a subcommand that has both",
          "[cli][options][CLI-TRANS-02]") {
    const Inputs one = inputs({"a.srt"});

    SECTION("--document translation needs -t") {
        const TranslationOptions options{.document = "translation"};
        const auto refused = pairingOf(options, true, one);
        REQUIRE_FALSE(refused.has_value());
        CHECK_THAT(refused.error(), ContainsSubstring("use -t"));
    }

    SECTION("-t needs --document translation") {
        const TranslationOptions options{.file = "fr.srt", .document = "main"};
        const auto refused = pairingOf(options, true, one);
        REQUIRE_FALSE(refused.has_value());
        CHECK_THAT(refused.error(), ContainsSubstring("use --document translation"));
    }

    SECTION("both, and it is the translation that is targeted") {
        const TranslationOptions options{
            .file = "fr.srt", .document = "translation", .alignMethod = "number"};
        const auto pairing = pairingOf(options, true, one);
        REQUIRE(pairing.has_value());
        REQUIRE(pairing->has_value());
        const subedit::cli::Pairing chosen = pairing->value_or(subedit::cli::Pairing{});
        CHECK(chosen.translation == "fr.srt");
        CHECK(chosen.method == subedit::core::TranslationMethod::Number);
    }

    SECTION("neither is no pairing, and the main document is the target") {
        const auto pairing = pairingOf(TranslationOptions{.document = "main"}, true, one);
        REQUIRE(pairing.has_value());
        CHECK_FALSE(pairing->has_value());
    }
}

TEST_CASE("an alignment method needs a translation file", "[cli][options][CLI-TRANS-05]") {
    const auto refused =
        pairingOf(TranslationOptions{.alignMethod = "position"}, false, inputs({"a.srt"}));

    REQUIRE_FALSE(refused.has_value());
    CHECK_THAT(refused.error(), ContainsSubstring("--align-method needs a translation file"));
}

TEST_CASE("a translation file goes with one input given by name", "[cli][options][CLI-TRANS-03]") {
    const TranslationOptions options{.file = "fr.srt", .document = "translation"};

    const auto batch = pairingOf(options, true, inputs({"a.srt", "b.srt"}));
    const auto walked = pairingOf(
        options, true, Inputs{.paths = {"films/a.srt"}, .roots = {std::filesystem::path{"films"}}});

    REQUIRE_FALSE(batch.has_value());
    CHECK_THAT(batch.error(), ContainsSubstring("one invocation per pair"));
    CHECK_FALSE(walked.has_value());
}

TEST_CASE("prepare reads the range, then the inputs, then the translation, and the first refusal "
          "wins",
          "[cli][options][CLI-USAGE-03]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kOne);
    files.addFile("b.srt", kOne);
    std::ostringstream errors;
    const Reporter reporter{errors, 1};

    // The inputs and the translation are wrong too: the range is said.
    const std::vector<std::string> two{"a.srt", "b.srt"};
    const std::string badRange = "5-2";
    const TranslationOptions translation{.file = "fr.srt", .document = "translation"};
    const auto range =
        prepare(files,
                reporter,
                Preparation{.files = two, .range = &badRange, .translation = &translation});
    REQUIRE_FALSE(range.has_value());
    CHECK_THAT(range.error(), ContainsSubstring("--range"));

    // Mend the range: now it is the translation that is said — two inputs and -t.
    const std::string goodRange = "1-2";
    const auto pairing =
        prepare(files,
                reporter,
                Preparation{.files = two, .range = &goodRange, .translation = &translation});
    REQUIRE_FALSE(pairing.has_value());
    CHECK_THAT(pairing.error(), ContainsSubstring("one invocation per pair"));
}

TEST_CASE("prepareWriting judges the destination last, against the inputs",
          "[cli][options][CLI-USAGE-03]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kOne);
    std::ostringstream errors;
    const Reporter reporter{errors, 1};
    const std::vector<std::string> one{"a.srt"};

    const DestinationOptions none{};
    const auto refused = prepareWriting(files, reporter, Preparation{.files = one}, none);
    REQUIRE_FALSE(refused.has_value());
    CHECK_THAT(refused.error(), ContainsSubstring("no destination given"));

    const DestinationOptions named{.output = "out.srt"};
    const auto prepared = prepareWriting(files, reporter, Preparation{.files = one}, named);
    REQUIRE(prepared.has_value());
    CHECK(prepared->inputs.paths == one);
    CHECK_FALSE(prepared->range.has_value());
    CHECK_FALSE(prepared->pairing.has_value());
}

TEST_CASE("prepare says a directory without --recursive, and carries a translation it accepted",
          "[cli][options][CLI-BATCH-08][CLI-TRANS-02]") {
    InMemoryFileSystem files;
    files.addFile("films/a.srt", kOne);
    files.addFile("a.srt", kOne);
    std::ostringstream errors;
    const Reporter reporter{errors, 1};

    const std::vector<std::string> directory{"films"};
    const auto refused = prepare(files, reporter, Preparation{.files = directory});
    REQUIRE_FALSE(refused.has_value());
    CHECK_THAT(refused.error(), ContainsSubstring("is a directory"));

    // The same refusal reaches a subcommand that writes, whatever it was given after.
    const auto writing =
        prepareWriting(files, reporter, Preparation{.files = directory}, DestinationOptions{});
    REQUIRE_FALSE(writing.has_value());
    CHECK_THAT(writing.error(), ContainsSubstring("is a directory"));

    const std::vector<std::string> one{"a.srt"};
    const TranslationOptions translation{.file = "fr.srt", .document = "translation"};
    const auto prepared =
        prepare(files, reporter, Preparation{.files = one, .translation = &translation});
    REQUIRE(prepared.has_value());
    CHECK(prepared->pairing.value_or(subedit::cli::Pairing{}).translation == "fr.srt");
}
