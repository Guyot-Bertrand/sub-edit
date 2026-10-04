// The list of encodings, written to a stream.

#include <subedit/cli/encoding_listing.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::listEncodings;

TEST_CASE("the list is one name a line and says how many it wrote",
          "[cli][encodings][CLI-LISTENC-01]") {
    std::ostringstream out;

    const std::size_t written = listEncodings(out, false);

    CHECK(written > 50);
    CHECK_THAT(out.str(), ContainsSubstring("\nUTF-8\n"));
    std::size_t lines = 0;
    for (const char c : out.str()) {
        lines += c == '\n' ? 1 : 0;
    }
    CHECK(lines == written);
}

TEST_CASE("in json each name is an object with the schema and the command",
          "[cli][encodings][CLI-LISTENC-01]") {
    std::ostringstream out;

    CHECK(listEncodings(out, true) > 50);

    CHECK_THAT(
        out.str(),
        ContainsSubstring("{\"schema\":1,\"command\":\"list-encodings\",\"name\":\"UTF-8\"}\n"));
}
