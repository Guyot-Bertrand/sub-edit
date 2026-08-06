#pragma once

#include <compare>
#include <cstddef>

namespace subedit::core {

/// The position of a subtitle in a project, counted from zero.
///
/// A distinct type rather than a bare `std::size_t`, because two numberings
/// coexist: the zero-based position used by the code, and the one-based number
/// the user reads on screen and in a SubRip file. Confusing them is the
/// classic off-by-one of a subtitle editor, and the conversion lives here so
/// that it is written once instead of at every site.
class SubtitleIndex {

public:
    /// Builds an index from a zero-based position.
    [[nodiscard]] static constexpr SubtitleIndex fromValue(std::size_t value) {
        return SubtitleIndex{value};
    }

    /// Builds an index from the one-based number the user reads.
    [[nodiscard]] static constexpr SubtitleIndex fromNumber(std::size_t number) {
        return SubtitleIndex{number - 1};
    }

    /// Returns the zero-based position.
    [[nodiscard]] constexpr std::size_t value() const { return m_value; }

    /// Returns the one-based number shown to the user.
    [[nodiscard]] constexpr std::size_t number() const { return m_value + 1; }

    [[nodiscard]] friend constexpr bool operator==(SubtitleIndex, SubtitleIndex) = default;

    [[nodiscard]] friend constexpr auto operator<=>(SubtitleIndex, SubtitleIndex) = default;

private:
    explicit constexpr SubtitleIndex(std::size_t value) : m_value(value) {}

    std::size_t m_value;
};

} // namespace subedit::core
