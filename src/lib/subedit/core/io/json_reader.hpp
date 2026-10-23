#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace subedit::core {

/// A JSON document, read.
///
/// **Only a reader, and a small one.** The project writes JSON by hand for its
/// own output (ADR 0038); what it needs to read is the output of a tool it
/// does not own — whisper.cpp writes a JSON file — and nothing else. It
/// accepts what RFC 8259 accepts, nesting up to a fixed depth, and refuses the
/// rest as a whole: a document that is not entirely valid is not read at all,
/// since a tool that wrote half a document was interrupted and what it wrote
/// cannot be trusted.
class JsonValue {

public:
    using Array = std::vector<JsonValue>;
    using Object = std::vector<std::pair<std::string, JsonValue>>;

    JsonValue() = default;

    explicit JsonValue(bool value) : m_value(value) {}

    explicit JsonValue(double value) : m_value(value) {}

    explicit JsonValue(std::string value) : m_value(std::move(value)) {}

    explicit JsonValue(Array value) : m_value(std::move(value)) {}

    explicit JsonValue(Object value) : m_value(std::move(value)) {}

    /// The member called `key` of an object — nothing for any other value, or
    /// when there is no such member. The first one wins when a key is repeated.
    [[nodiscard]] const JsonValue* find(std::string_view key) const;

    [[nodiscard]] const std::string* asString() const;
    [[nodiscard]] std::optional<double> asNumber() const;
    [[nodiscard]] std::optional<bool> asBool() const;
    [[nodiscard]] const Array* asArray() const;

private:
    std::variant<std::monostate, bool, double, std::string, Array, Object> m_value;
};

/// Reads `text` as one JSON document, or nothing when it is not entirely one.
[[nodiscard]] std::optional<JsonValue> parseJson(std::string_view text);

} // namespace subedit::core
