#pragma once

// Writing JSON, and nothing else.
//
// **There is nothing to read**, which is why this is a hundred lines and not a
// dependency (ADR 0038, ADR 0004): the command line only ever *writes* JSON.
// The output is RFC 8259 — quotes, backslashes and control characters escaped,
// every other character, the accented ones and those outside the Basic
// Multilingual Plane included, written as the UTF-8 it already is.

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace subedit::cli {

/// A text made valid UTF-8, and whether that cost anything.
struct CleanedText {
    std::string text;

    /// True when a byte that was not valid UTF-8 had to be replaced by U+FFFD.
    bool replaced = false;
};

/// `text` with every invalid UTF-8 sequence replaced by U+FFFD.
///
/// **A path on Linux is bytes**, and bytes are not always UTF-8; JSON strings
/// are Unicode. The fidelity of such a path is not promised, and the caller says
/// so (`path-not-utf8`) rather than writing a document no parser accepts.
[[nodiscard]] CleanedText cleanedUtf8(std::string_view text);

/// A JSON value, built to be written.
///
/// **Numbers are integers, and that is the type's choice and not a convention**:
/// there is no way to put a floating point number in one. Two outputs compared
/// byte for byte then depend on no rounding.
///
/// An object keeps its keys **in the order they were added** — the output must
/// be deterministic, and a hash order would not be.
class Json {

public:
    using Array = std::vector<Json>;
    using Object = std::vector<std::pair<std::string, Json>>;

    Json() = default; // null

    Json(std::nullptr_t) {}

    Json(bool value) : m_value{value} {}

    Json(std::int64_t value) : m_value{value} {}

    Json(int value) : m_value{static_cast<std::int64_t>(value)} {}

    Json(std::size_t value) : m_value{static_cast<std::int64_t>(value)} {}

    /// A text, cleaned: an invalid byte becomes U+FFFD rather than invalid JSON.
    Json(std::string_view text) : m_value{cleanedUtf8(text).text} {}

    Json(const std::string& text) : Json{std::string_view{text}} {}

    Json(const char* text) : Json{std::string_view{text}} {}

    [[nodiscard]] static Json array() {
        Json json;
        json.m_value = Array{};
        return json;
    }

    [[nodiscard]] static Json object() {
        Json json;
        json.m_value = Object{};
        return json;
    }

    /// Appends to an array.
    Json& push(Json value);

    /// Adds a key to an object, after the ones already there.
    Json& set(std::string key, Json value);

    /// The compact form: no whitespace, one line, no end of line.
    [[nodiscard]] std::string dump() const;

private:
    std::variant<std::nullptr_t, bool, std::int64_t, std::string, Array, Object> m_value;
};

} // namespace subedit::cli
