#pragma once

// A replacement written the way Python's `re.sub` reads it, kept as what it is.
//
// Gaupol's replacements — `\1 \2`, `-\040`, `\1\060` — are templates for
// Python's `expand`. **Converting one is done once, at loading**, into pieces
// that are literal text or the number of a group: no engine's syntax is written
// into the data, and expanding is the same few lines whatever finds the matches.
// Decision D3 of the spec of phase 12.

#include <subedit/core/text/pattern_engine.hpp>

#include <cstddef>
#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

struct TemplateError {
    std::string reason;

    friend bool operator==(const TemplateError&, const TemplateError&) = default;
};

class ReplacementTemplate {

public:
    /// Reads `text` as Python's `re` reads a replacement.
    ///
    /// - `\1` to `\99` and `\g<n>` name a group, `\g<0>` the whole match, and
    ///   `\g<name>` the group `groupNamed` finds by that name;
    /// - **`\0` followed by digits, and three octal digits, are a character** —
    ///   `\040` is a space, `\060` a zero, which is how the shipped files write
    ///   what GKeyFile would otherwise repair;
    /// - `\n`, `\t`, `\r`, `\a`, `\b`, `\f`, `\v` and `\\` are what they say;
    /// - any other backslash followed by something that is not a letter stays as
    ///   it is, both characters, as Python leaves it.
    ///
    /// A group the pattern does not have, a letter Python has no escape for, an
    /// octal above `\377` and a trailing backslash are refused — Python raises,
    /// and the pattern is then disabled and named rather than applied.
    [[nodiscard]] static std::expected<ReplacementTemplate, TemplateError>
    parse(std::string_view text,
          std::size_t groupCount,
          const std::function<std::optional<int>(std::string_view)>& groupNamed);

    /// What `match` makes of the template in `text`. A group that took no part
    /// stands for nothing.
    [[nodiscard]] std::string expandedFor(std::string_view text, const Match& match) const;

private:
    struct Piece {
        /// The group this piece is, or `-1` for a literal.
        int group = -1;
        std::string literal;
    };

    std::vector<Piece> m_pieces;

    friend class TemplateReading;
};

} // namespace subedit::core
