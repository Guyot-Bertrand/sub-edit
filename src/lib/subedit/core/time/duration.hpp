#pragma once

#include <subedit/core/time/ratio.hpp>

#include <compare>
#include <cstdint>

namespace subedit::core {

/// A signed length of time, counted in whole milliseconds.
///
/// A duration is not a position: only `Timestamp` is. The two are distinct
/// types so that adding one to the other is a compile error rather than a
/// timing bug found by the user.
class Duration {

public:
    /// Builds a duration from a signed millisecond count.
    [[nodiscard]] static constexpr Duration fromMilliseconds(std::int64_t count) {
        return Duration{count};
    }

    /// Builds the null duration.
    [[nodiscard]] static constexpr Duration zero() { return Duration{0}; }

    /// Returns the signed millisecond count.
    [[nodiscard]] constexpr std::int64_t milliseconds() const { return m_milliseconds; }

    /// Returns this duration multiplied by `factor`, rounded once.
    ///
    /// A named method and not an operator: scaling is a rounding decision, and
    /// none of them is taken implicitly.
    [[nodiscard]] constexpr Duration scaledBy(Ratio factor) const {
        return Duration{factor.scale(m_milliseconds)};
    }

    constexpr Duration& operator+=(Duration other) {
        m_milliseconds += other.m_milliseconds;
        return *this;
    }

    constexpr Duration& operator-=(Duration other) {
        m_milliseconds -= other.m_milliseconds;
        return *this;
    }

    [[nodiscard]] friend constexpr Duration operator+(Duration left, Duration right) {
        return Duration{left.m_milliseconds + right.m_milliseconds};
    }

    [[nodiscard]] friend constexpr Duration operator-(Duration left, Duration right) {
        return Duration{left.m_milliseconds - right.m_milliseconds};
    }

    [[nodiscard]] friend constexpr Duration operator-(Duration value) {
        return Duration{-value.m_milliseconds};
    }

    [[nodiscard]] friend constexpr Duration operator*(Duration value, std::int64_t factor) {
        return Duration{value.m_milliseconds * factor};
    }

    [[nodiscard]] friend constexpr Duration operator*(std::int64_t factor, Duration value) {
        return value * factor;
    }

    [[nodiscard]] friend constexpr bool operator==(Duration, Duration) = default;

    [[nodiscard]] friend constexpr auto operator<=>(Duration, Duration) = default;

private:
    explicit constexpr Duration(std::int64_t count) : m_milliseconds(count) {}

    std::int64_t m_milliseconds;
};

} // namespace subedit::core
