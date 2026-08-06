#pragma once

#include <compare>
#include <cstdint>

namespace subedit::core {

/// A position expressed in whole video frames, or a count of frames.
///
/// Frames are never the internal representation of a position — `Timestamp`
/// is — but they are what frame-based formats store and what the user reads
/// when working against a video. Conversion always goes through an explicit
/// `FrameRate`, so the frame count is a view, not a second source of truth.
///
/// Unlike time, frames have no sub-unit: the same type serves as a position
/// and as a count, and adding two of them is meaningful.
class Frame {

public:
    /// Builds a frame from a signed frame number.
    [[nodiscard]] static constexpr Frame fromNumber(std::int64_t number) { return Frame{number}; }

    /// Builds frame zero.
    [[nodiscard]] static constexpr Frame zero() { return Frame{0}; }

    /// Returns the signed frame number.
    [[nodiscard]] constexpr std::int64_t number() const { return m_number; }

    constexpr Frame& operator+=(Frame other) {
        m_number += other.m_number;
        return *this;
    }

    constexpr Frame& operator-=(Frame other) {
        m_number -= other.m_number;
        return *this;
    }

    [[nodiscard]] friend constexpr Frame operator+(Frame left, Frame right) {
        return Frame{left.m_number + right.m_number};
    }

    [[nodiscard]] friend constexpr Frame operator-(Frame left, Frame right) {
        return Frame{left.m_number - right.m_number};
    }

    [[nodiscard]] friend constexpr Frame operator-(Frame value) { return Frame{-value.m_number}; }

    [[nodiscard]] friend constexpr bool operator==(Frame, Frame) = default;

    [[nodiscard]] friend constexpr auto operator<=>(Frame, Frame) = default;

private:
    explicit constexpr Frame(std::int64_t number) : m_number(number) {}

    std::int64_t m_number;
};

} // namespace subedit::core
