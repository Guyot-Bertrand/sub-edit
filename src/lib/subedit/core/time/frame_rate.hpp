#pragma once

#include <array>
#include <cstdint>
#include <numeric>
#include <optional>
#include <utility>

namespace subedit::core {

/// The eight frame rates offered by the interface.
///
/// Three of them are the NTSC rates, defined as a whole rate divided by 1.001.
/// They have no exact representation as a floating-point number, which is why
/// `FrameRate` keeps a rational and not a `double`.
enum class StandardFrameRate {
    Fps23976, ///< 24000/1001, cinema transferred to NTSC video
    Fps24,    ///< cinema
    Fps25,    ///< PAL and SECAM video
    Fps29970, ///< 30000/1001, NTSC video
    Fps30,
    Fps50,    ///< PAL and SECAM at double rate
    Fps59940, ///< 60000/1001
    Fps60,
};

/// The standard frame rates, in the order the interface offers them.
inline constexpr std::array<StandardFrameRate, 8> kStandardFrameRates = {
    StandardFrameRate::Fps23976,
    StandardFrameRate::Fps24,
    StandardFrameRate::Fps25,
    StandardFrameRate::Fps29970,
    StandardFrameRate::Fps30,
    StandardFrameRate::Fps50,
    StandardFrameRate::Fps59940,
    StandardFrameRate::Fps60,
};

/// A frame rate, as an exact rational number of frames per second.
///
/// Stored reduced to lowest terms, so that two rates written differently but
/// equal in value compare equal. Both terms are strictly positive, an
/// invariant the factory function is there to guarantee.
class FrameRate {

public:
    /// Builds one of the standard frame rates.
    ///
    /// Explicit on purpose: a `StandardFrameRate` names a choice offered to the
    /// user, a `FrameRate` is a number used in arithmetic, and the two should
    /// not silently stand in for one another.
    explicit constexpr FrameRate(StandardFrameRate standard) : FrameRate(exactValueOf(standard)) {}

    /// Builds an arbitrary frame rate, or nothing if either term is not
    /// strictly positive.
    [[nodiscard]] static constexpr std::optional<FrameRate> create(std::int64_t numerator,
                                                                   std::int64_t denominator) {
        if (numerator <= 0 || denominator <= 0)
            return std::nullopt;
        return FrameRate{Terms{.numerator = numerator, .denominator = denominator}};
    }

    /// Returns the numerator of the reduced fraction, in frames.
    [[nodiscard]] constexpr std::int64_t numerator() const { return m_numerator; }

    /// Returns the denominator of the reduced fraction, in seconds.
    [[nodiscard]] constexpr std::int64_t denominator() const { return m_denominator; }

    [[nodiscard]] friend constexpr bool operator==(FrameRate, FrameRate) = default;

private:
    struct Terms {
        std::int64_t numerator;
        std::int64_t denominator;
    };

    explicit constexpr FrameRate(Terms terms)
        : m_numerator(terms.numerator / std::gcd(terms.numerator, terms.denominator)),
          m_denominator(terms.denominator / std::gcd(terms.numerator, terms.denominator)) {}

    [[nodiscard]] static constexpr Terms exactValueOf(StandardFrameRate standard) {
        switch (standard) {
        case StandardFrameRate::Fps23976:
            return {.numerator = 24000, .denominator = 1001};
        case StandardFrameRate::Fps24:
            return {.numerator = 24, .denominator = 1};
        case StandardFrameRate::Fps25:
            return {.numerator = 25, .denominator = 1};
        case StandardFrameRate::Fps29970:
            return {.numerator = 30000, .denominator = 1001};
        case StandardFrameRate::Fps30:
            return {.numerator = 30, .denominator = 1};
        case StandardFrameRate::Fps50:
            return {.numerator = 50, .denominator = 1};
        case StandardFrameRate::Fps59940:
            return {.numerator = 60000, .denominator = 1001};
        case StandardFrameRate::Fps60:
            return {.numerator = 60, .denominator = 1};
        }
        // The switch above covers every enumerator, and the compiler checks
        // that it does. Only a cast from an out-of-range integer could land
        // here, which is a programming error rather than a case to handle:
        // inventing a frame rate for it would hide the bug.
        std::unreachable();
    }

    std::int64_t m_numerator;
    std::int64_t m_denominator;
};

} // namespace subedit::core
