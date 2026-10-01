#include <subedit/cli/json.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>

using subedit::cli::cleanedUtf8;
using subedit::cli::Json;

TEST_CASE("scalars are written compactly", "[cli][json]") {
    CHECK(Json{}.dump() == "null");
    CHECK(Json{nullptr}.dump() == "null");
    CHECK(Json{true}.dump() == "true");
    CHECK(Json{false}.dump() == "false");
    CHECK(Json{0}.dump() == "0");
    CHECK(Json{std::int64_t{-7001}}.dump() == "-7001");
    CHECK(Json{std::size_t{4000}}.dump() == "4000");
    CHECK(Json{"text"}.dump() == "\"text\"");
}

TEST_CASE("quotes, backslashes and control characters are escaped", "[cli][json]") {
    CHECK(Json{"a\"b"}.dump() == R"("a\"b")");
    CHECK(Json{"a\\b"}.dump() == R"("a\\b")");
    CHECK(Json{"a\nb\r\tc"}.dump() == R"("a\nb\r\tc")");
    CHECK(Json{"\b\f"}.dump() == R"("\b\f")");
    CHECK(Json{std::string{"a\x01z", 3}}.dump() == "\"a\\u0001z\"");
    CHECK(Json{std::string{"\x1f"}}.dump() == "\"\\u001f\"");
    // DEL is not a control character for JSON.
    CHECK(Json{"\x7f"}.dump() == "\"\x7f\"");
    // The slash needs no escape.
    CHECK(Json{"a/b"}.dump() == "\"a/b\"");
}

TEST_CASE("other characters are written as the UTF-8 they are", "[cli][json]") {
    CHECK(Json{"Réplique à la mémoire"}.dump() == "\"Réplique à la mémoire\"");
    CHECK(Json{"日本語"}.dump() == "\"日本語\"");
    // Outside the Basic Multilingual Plane: one character, not a surrogate pair.
    CHECK(Json{"\xF0\x9F\x98\x80"}.dump() == "\"\xF0\x9F\x98\x80\"");
}

TEST_CASE("an object keeps its keys in the order they were added", "[cli][json]") {
    Json object = Json::object();
    object.set("z", 1)
        .set("a", Json::array().push(true).push("x").push(Json{}))
        .set("m", Json::object());

    CHECK(object.dump() == R"({"z":1,"a":[true,"x",null],"m":{}})");
    CHECK(Json::array().dump() == "[]");
}

TEST_CASE("a key is escaped like a value", "[cli][json]") {
    CHECK(Json::object().set("a\"b", 1).dump() == R"({"a\"b":1})");
}

TEST_CASE("valid UTF-8 is left as it is", "[cli][json]") {
    const auto cleaned = cleanedUtf8("é日\xF0\x9F\x98\x80 ok");

    CHECK(cleaned.text == "é日\xF0\x9F\x98\x80 ok");
    CHECK_FALSE(cleaned.replaced);
}

TEST_CASE("invalid UTF-8 is replaced byte by byte, and said", "[cli][json]") {
    // A lone continuation byte, a truncated sequence, an overlong form, a
    // surrogate, and what lies past U+10FFFF.
    for (const char* bad :
         {"\x80", "\xE2\x82", "\xC0\xAF", "\xED\xA0\x80", "\xF4\x90\x80\x80", "\xFF"}) {
        const auto cleaned = cleanedUtf8(std::string{"a"} + bad + "b");
        CHECK(cleaned.replaced);
        CHECK(cleaned.text.starts_with("a\xEF\xBF\xBD"));
        CHECK(cleaned.text.ends_with("b"));
    }
    CHECK(cleanedUtf8("a\xFF"
                      "b")
              .text == "a\xEF\xBF\xBD"
                       "b");
}

TEST_CASE("a text that is not UTF-8 is written as valid JSON", "[cli][json]") {
    CHECK(Json{"caf\xE9"}.dump() == "\"caf\xEF\xBF\xBD\"");
}
