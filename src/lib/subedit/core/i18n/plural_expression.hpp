#pragma once

// The `plural=` expression of a catalogue's `Plural-Forms` header — ADR 0042.
//
// `n != 1` for English, `n > 1` for French, a three-way ternary for Russian or Polish: the
// grammar is the small subset of C that gettext allows, and nothing else. It is read from a
// file the program does not control, so the parser refuses what is too long or too deep, and
// the evaluation can neither divide by zero nor overflow into undefined behaviour.

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// One node of an expression tree, kept in a flat vector. An implementation detail of
/// `PluralExpression`, public only so that the parser can build it.
struct PluralNode {
    enum class Kind : std::uint8_t {
        Number,
        Variable, ///< `n`
        Not,
        Multiply,
        Divide,
        Modulo,
        Add,
        Subtract,
        Less,
        Greater,
        LessOrEqual,
        GreaterOrEqual,
        Equal,
        NotEqual,
        And,
        Or,
        Conditional, ///< `left ? right : other`
    };

    Kind kind = Kind::Number;
    std::uint32_t left = 0;
    std::uint32_t right = 0;
    std::uint32_t other = 0;
    std::uint64_t value = 0;
};

class PluralExpression {

public:
    /// Parses the text after `plural=`, without the closing `;`. The reason is a sentence for a
    /// diagnostic.
    [[nodiscard]] static std::expected<PluralExpression, std::string> parse(std::string_view text);

    /// `n != 1`: the rule of English, and of every language a catalogue does not say otherwise
    /// about.
    [[nodiscard]] static PluralExpression germanic();

    /// The index of the plural form that `n` selects. A division by zero yields zero, and the
    /// arithmetic wraps rather than overflows.
    [[nodiscard]] std::uint64_t evaluate(std::uint64_t n) const;

private:
    PluralExpression(std::vector<PluralNode> nodes, std::uint32_t root)
        : m_nodes(std::move(nodes)), m_root(root) {}

    std::vector<PluralNode> m_nodes;
    std::uint32_t m_root = 0;
};

} // namespace subedit::core
