// Reading the JSON a tool wrote.

#include <subedit/core/io/json_reader.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

using subedit::core::JsonValue;
using subedit::core::parseJson;

namespace {

/// The document `text` is, or a failed requirement.
JsonValue read(std::string_view text) {
    const std::optional<JsonValue> parsed = parseJson(text);
    REQUIRE(parsed.has_value());
    return parsed.value_or(JsonValue{});
}

} // namespace

TEST_CASE("an object is read with its members", "[json]") {
    const JsonValue document =
        read(R"({"name": "tiny", "size": 75.5, "multilingual": true, "none": null})");

    REQUIRE(document.find("name") != nullptr);
    CHECK(*document.find("name")->asString() == "tiny");
    CHECK(document.find("size")->asNumber() == 75.5);
    CHECK(document.find("multilingual")->asBool() == true);
    CHECK(document.find("none") != nullptr);
    CHECK(document.find("absent") == nullptr);
}

TEST_CASE("an array is read in order, and an empty one is empty", "[json]") {
    const JsonValue document = read(R"({"a": [1, 2, 3], "b": [], "c": {}})");

    const auto* numbers = document.find("a")->asArray();
    REQUIRE(numbers != nullptr);
    REQUIRE(numbers->size() == 3);
    CHECK((*numbers)[2].asNumber() == 3);
    CHECK(document.find("b")->asArray()->empty());
    CHECK(document.find("c")->find("x") == nullptr);
}

TEST_CASE("the accessors say nothing for a value of another kind", "[json]") {
    const JsonValue document = read(R"("text")");

    CHECK(document.asString() != nullptr);
    CHECK_FALSE(document.asNumber().has_value());
    CHECK_FALSE(document.asBool().has_value());
    CHECK(document.asArray() == nullptr);
    CHECK(document.find("x") == nullptr);
}

TEST_CASE("the first member wins when a key is repeated", "[json]") {
    const JsonValue document = read(R"({"k": "first", "k": "second"})");

    CHECK(*document.find("k")->asString() == "first");
}

TEST_CASE("the escapes of a string are read", "[json]") {
    const JsonValue document = read(R"("q\" b\\ s\/ \b\f\n\r\t e\u00e9 \u20ac \ud83d\ude00")");

    CHECK(*document.asString() == "q\" b\\ s/ \b\f\n\r\t e\xC3\xA9 \xE2\x82\xAC \xF0\x9F\x98\x80");
}

TEST_CASE("a one-byte and a two-byte code point are encoded", "[json]") {
    const JsonValue document = read(R"("\u0041\u00ff")");

    CHECK(*document.asString() == "A\xC3\xBF");
}

TEST_CASE("numbers are read with a sign, a fraction and an exponent", "[json]") {
    const JsonValue document = read("[-1, 0.25, 1e3, -2.5E-1]");

    const auto& items = *document.asArray();
    CHECK(items[0].asNumber() == -1);
    CHECK(items[1].asNumber() == 0.25);
    CHECK(items[2].asNumber() == 1000);
    CHECK(items[3].asNumber() == -0.25);
}

TEST_CASE("whitespace around everything is allowed", "[json]") {
    CHECK(parseJson(" \n\t{ \"a\" :\r\n [ 1 , 2 ] } \n").has_value());
}

TEST_CASE("a document that is not entirely valid is not read", "[json]") {
    CHECK_FALSE(parseJson("").has_value());
    CHECK_FALSE(parseJson("{").has_value());
    CHECK_FALSE(parseJson("{\"a\": 1").has_value());
    CHECK_FALSE(parseJson("{\"a\" 1}").has_value());
    CHECK_FALSE(parseJson("{a: 1}").has_value());
    CHECK_FALSE(parseJson("{\"a\": }").has_value());
    CHECK_FALSE(parseJson("[1 2]").has_value());
    CHECK_FALSE(parseJson("[1,").has_value());
    CHECK_FALSE(parseJson("{\"a\": 1} x").has_value());
    CHECK_FALSE(parseJson("tru").has_value());
    CHECK_FALSE(parseJson("nul").has_value());
    CHECK_FALSE(parseJson("fals").has_value());
    CHECK_FALSE(parseJson("\"open").has_value());
    CHECK_FALSE(parseJson("\"bad \\q escape\"").has_value());
    CHECK_FALSE(parseJson("\"ends with a backslash\\").has_value());
}

TEST_CASE("a number that is not one is refused", "[json]") {
    CHECK_FALSE(parseJson("+1").has_value());
    CHECK_FALSE(parseJson("nan").has_value());
    CHECK_FALSE(parseJson("-").has_value());
    CHECK_FALSE(parseJson("abc").has_value());
}

TEST_CASE("a broken unicode escape is refused", "[json]") {
    CHECK_FALSE(parseJson("\"\\u12\"").has_value());
    CHECK_FALSE(parseJson("\"\\u12zz\"").has_value());
    CHECK_FALSE(parseJson("\"\\ud83d\"").has_value());
    CHECK_FALSE(parseJson("\"\\ud83dx\"").has_value());
    CHECK_FALSE(parseJson("\"\\ud83d\\u0041\"").has_value());
    CHECK_FALSE(parseJson("\"\\ude00\"").has_value());
}

TEST_CASE("nesting beyond the limit is refused rather than followed", "[json]") {
    CHECK(parseJson(std::string(60, '[') + std::string(60, ']')).has_value());
    CHECK_FALSE(parseJson(std::string(200, '[') + std::string(200, ']')).has_value());
}
