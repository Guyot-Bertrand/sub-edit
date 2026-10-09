#include <subedit/core/i18n/plural_expression.hpp>

#include <array>
#include <cctype>
#include <cstddef>
#include <optional>
#include <utility>

namespace subedit::core {

namespace {

using Kind = PluralNode::Kind;

/// A header is a few dozen characters; a megabyte of parentheses is an attack.
constexpr std::size_t kMaxLength = 1024;
/// The tree is evaluated by recursion, so its size bounds the depth.
constexpr std::size_t kMaxNodes = 256;
constexpr int kDeepestNesting = 32;
constexpr std::uint64_t kBase = 10;

struct Operator {
    std::string_view text;
    Kind kind;
    int level;
};

// Longest spelling first, so that `<=` is not read as `<` and then a stray `=`.
constexpr std::array<Operator, 13> kOperators{{
    {.text = "||", .kind = Kind::Or, .level = 1},
    {.text = "&&", .kind = Kind::And, .level = 2},
    {.text = "==", .kind = Kind::Equal, .level = 3},
    {.text = "!=", .kind = Kind::NotEqual, .level = 3},
    {.text = "<=", .kind = Kind::LessOrEqual, .level = 4},
    {.text = ">=", .kind = Kind::GreaterOrEqual, .level = 4},
    {.text = "<", .kind = Kind::Less, .level = 4},
    {.text = ">", .kind = Kind::Greater, .level = 4},
    {.text = "+", .kind = Kind::Add, .level = 5},
    {.text = "-", .kind = Kind::Subtract, .level = 5},
    {.text = "*", .kind = Kind::Multiply, .level = 6},
    {.text = "/", .kind = Kind::Divide, .level = 6},
    {.text = "%", .kind = Kind::Modulo, .level = 6},
}};
constexpr int kLowestLevel = 1;
constexpr int kHighestLevel = 6;

class Parser {

public:
    explicit Parser(std::string_view text) : m_text(text) {}

    // The nodes and the index of the root.
    std::expected<std::pair<std::vector<PluralNode>, std::uint32_t>, std::string> run() {
        if (m_text.size() > kMaxLength) {
            return std::unexpected("the expression is too long");
        }
        const auto root = ternary(0);
        if (!root) {
            return std::unexpected(m_error);
        }
        skipSpaces();
        if (m_position != m_text.size()) {
            return std::unexpected("unexpected text after the expression");
        }
        return std::pair{std::move(m_nodes), *root};
    }

private:
    std::vector<PluralNode> m_nodes;
    std::string m_error;

    std::optional<std::uint32_t> fail(std::string reason) {
        if (m_error.empty()) {
            m_error = std::move(reason);
        }
        return std::nullopt;
    }

    std::optional<std::uint32_t> add(const PluralNode& node) {
        if (m_nodes.size() >= kMaxNodes) {
            return fail("the expression is too large");
        }
        m_nodes.push_back(node);
        return static_cast<std::uint32_t>(m_nodes.size() - 1);
    }

    void skipSpaces() {
        while (m_position < m_text.size() &&
               std::isspace(static_cast<unsigned char>(m_text[m_position])) != 0) {
            ++m_position;
        }
    }

    bool accept(std::string_view token) {
        skipSpaces();
        if (m_text.substr(m_position, token.size()) != token) {
            return false;
        }
        m_position += token.size();
        return true;
    }

    // `a ? b : c`, right-associative.
    std::optional<std::uint32_t> ternary(int depth) {
        if (depth > kDeepestNesting) {
            return fail("the expression is nested too deeply");
        }
        const auto condition = binary(kLowestLevel, depth);
        if (!condition) {
            return std::nullopt;
        }
        if (!accept("?")) {
            return condition;
        }
        const auto yes = ternary(depth + 1);
        if (!yes) {
            return std::nullopt;
        }
        if (!accept(":")) {
            return fail("a `?` without its `:`");
        }
        const auto no = ternary(depth + 1);
        if (!no) {
            return std::nullopt;
        }
        return add({.kind = Kind::Conditional, .left = *condition, .right = *yes, .other = *no});
    }

    // The operator at the cursor if it belongs to `level`; the cursor moves past it.
    std::optional<Kind> operatorOf(int level) {
        skipSpaces();
        for (const auto& candidate : kOperators) {
            if (m_text.substr(m_position, candidate.text.size()) != candidate.text) {
                continue;
            }
            if (candidate.level != level) {
                // A longer operator of another level would have matched first: `<` is never
                // a prefix of `<=` here because `<=` is listed before it.
                return std::nullopt;
            }
            m_position += candidate.text.size();
            return candidate.kind;
        }
        return std::nullopt;
    }

    // Left-associative levels, from `||` (1) to `*` `/` `%` (6).
    std::optional<std::uint32_t> binary(int level, int depth) {
        if (level > kHighestLevel) {
            return unary(depth);
        }
        auto left = binary(level + 1, depth);
        if (!left) {
            return std::nullopt;
        }
        while (const auto kind = operatorOf(level)) {
            const auto right = binary(level + 1, depth);
            if (!right) {
                return std::nullopt;
            }
            left = add({.kind = *kind, .left = *left, .right = *right});
            if (!left) {
                return std::nullopt;
            }
        }
        return left;
    }

    std::optional<std::uint32_t> unary(int depth) {
        skipSpaces();
        if (m_position < m_text.size() && m_text[m_position] == '!' &&
            m_text.substr(m_position, 2) != "!=") {
            ++m_position;
            if (depth > kDeepestNesting) {
                return fail("the expression is nested too deeply");
            }
            const auto operand = unary(depth + 1);
            if (!operand) {
                return std::nullopt;
            }
            return add({.kind = Kind::Not, .left = *operand});
        }
        return primary(depth);
    }

    std::optional<std::uint32_t> primary(int depth) {
        skipSpaces();
        if (m_position >= m_text.size()) {
            return fail("the expression ends too soon");
        }
        const char c = m_text[m_position];
        if (c == '(') {
            ++m_position;
            const auto inner = ternary(depth + 1);
            if (!inner) {
                return std::nullopt;
            }
            if (!accept(")")) {
                return fail("a `(` without its `)`");
            }
            return inner;
        }
        if (c == 'n') {
            ++m_position;
            return add({.kind = Kind::Variable});
        }
        if (std::isdigit(static_cast<unsigned char>(c)) != 0) {
            std::uint64_t value = 0;
            while (m_position < m_text.size() &&
                   std::isdigit(static_cast<unsigned char>(m_text[m_position])) != 0) {
                const auto digit = static_cast<std::uint64_t>(m_text[m_position] - '0');
                if (value > (UINT64_MAX - digit) / kBase) {
                    return fail("a number is too large");
                }
                value = value * kBase + digit;
                ++m_position;
            }
            return add({.kind = Kind::Number, .value = value});
        }
        return fail(std::string("unexpected character `") + c + "`");
    }

    std::string_view m_text;
    std::size_t m_position = 0;
};

std::uint64_t applyBinary(Kind kind, std::uint64_t left, std::uint64_t right) {
    switch (kind) {
    case Kind::Multiply:
        return left * right;
    case Kind::Divide:
        return right == 0 ? 0 : left / right;
    case Kind::Modulo:
        return right == 0 ? 0 : left % right;
    case Kind::Add:
        return left + right;
    case Kind::Subtract:
        return left - right;
    case Kind::Less:
        return left < right ? 1 : 0;
    case Kind::Greater:
        return left > right ? 1 : 0;
    case Kind::LessOrEqual:
        return left <= right ? 1 : 0;
    case Kind::GreaterOrEqual:
        return left >= right ? 1 : 0;
    case Kind::Equal:
        return left == right ? 1 : 0;
    default: // Kind::NotEqual: the only operator left, so that the switch has no dead branch.
        return left != right ? 1 : 0;
    }
}

std::uint64_t
evaluateNode(const std::vector<PluralNode>& nodes, std::uint32_t index, std::uint64_t n) {
    const PluralNode& node = nodes[index];
    const auto left = [&] { return evaluateNode(nodes, node.left, n); };
    const auto right = [&] { return evaluateNode(nodes, node.right, n); };

    switch (node.kind) {
    case Kind::Number:
        return node.value;
    case Kind::Variable:
        return n;
    case Kind::Not:
        return left() == 0 ? 1 : 0;
    case Kind::And:
        return left() != 0 && right() != 0 ? 1 : 0;
    case Kind::Or:
        return left() != 0 || right() != 0 ? 1 : 0;
    case Kind::Conditional:
        return left() != 0 ? right() : evaluateNode(nodes, node.other, n);
    default:
        return applyBinary(node.kind, left(), right());
    }
}

} // namespace

std::expected<PluralExpression, std::string> PluralExpression::parse(std::string_view text) {
    Parser parser(text);
    auto parsed = parser.run();
    if (!parsed) {
        return std::unexpected(parsed.error());
    }
    return PluralExpression(std::move(parsed->first), parsed->second);
}

PluralExpression PluralExpression::germanic() {
    return *parse("n != 1");
}

std::uint64_t PluralExpression::evaluate(std::uint64_t n) const {
    return evaluateNode(m_nodes, m_root, n);
}

} // namespace subedit::core
