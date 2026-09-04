#include <subedit/cli/encoding_grammar.hpp>
#include <subedit/core/model/encoding.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <expected>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::encodingNamed;
using subedit::core::ByteOrderMark;
using subedit::core::Encoding;

TEST_CASE("an encoding is named as ICU names it, or as anyone names it", "[cli][grammar]") {
    // The set is ICU's, and so are the aliases: one encoding, several spellings,
    // one value.
    CHECK(encodingNamed("utf-8") == Encoding::utf8(ByteOrderMark::Absent));
    CHECK(encodingNamed("cp1252") == encodingNamed("windows-1252"));
    CHECK(encodingNamed("latin1") == encodingNamed("iso-8859-1"));
}

TEST_CASE("the name kept is the one a report will show", "[cli][grammar]") {
    const std::expected<Encoding, std::string> western = encodingNamed("cp1252");

    REQUIRE(western.has_value());
    CHECK(western->charset() == "windows-1252");
}

TEST_CASE("an encoding names no mark", "[cli][grammar]") {
    // A mark is what a file carries, not what an option names: `--bom` and
    // `--no-bom` are how one is asked for.
    const std::expected<Encoding, std::string> utf8 = encodingNamed("utf-8");

    REQUIRE(utf8.has_value());
    CHECK(utf8->byteOrderMark() == ByteOrderMark::Absent);
}

TEST_CASE("a name nothing can convert is refused, and quoted back", "[cli][grammar]") {
    // Refused while the user is still being asked something, rather than at the
    // moment of reading a file.
    const std::expected<Encoding, std::string> unknown = encodingNamed("klingon-1");

    REQUIRE_FALSE(unknown.has_value());
    CHECK_THAT(unknown.error(), ContainsSubstring("klingon-1"));
}

TEST_CASE("an empty name is not an encoding", "[cli][grammar]") {
    // ICU reads an empty name as « the encoding of the locale », which is the
    // one answer this phase refuses to give in silence.
    CHECK_FALSE(encodingNamed("").has_value());
}
