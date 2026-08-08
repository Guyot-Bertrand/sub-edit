#pragma once

#include <subedit/core/time/ratio.hpp>

#include <array>
#include <cstdint>
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
/// A `Ratio` with two invariants of its own: both terms are strictly positive,
/// and neither passes `kLargestTerm`. The first says that a rate is a rate; the
/// second is what makes the conversions below total, since every product they
/// form then holds in a `std::int64_t`.
class FrameRate {

public:
    /// The largest either term may reach.
    ///
    /// A billion frames per second is not a video, and the bound has to sit
    /// somewhere for `millisecondsPerFrame` and `conversionTo` to be unable to
    /// overflow. One billion leaves both of them a factor of nine.
    static constexpr std::int64_t kLargestTerm = 1000000000;

    /// Builds one of the standard frame rates.
    ///
    /// Explicit on purpose: a `StandardFrameRate` names a choice offered to the
    /// user, a `FrameRate` is a number used in arithmetic, and the two should
    /// not silently stand in for one another.
    explicit constexpr FrameRate(StandardFrameRate standard) : FrameRate(exactValueOf(standard)) {}

    /// Builds an arbitrary frame rate, or nothing if either term is not
    /// strictly positive, or passes `kLargestTerm`.
    [[nodiscard]] static constexpr std::optional<FrameRate> create(std::int64_t numerator,
                                                                   std::int64_t denominator) {
        if (numerator <= 0 || denominator <= 0)
            return std::nullopt;
        if (numerator > kLargestTerm || denominator > kLargestTerm)
            return std::nullopt;

        return FrameRate{ratioOf(numerator, denominator)};
    }

    /// Returns the rate itself, as a factor.
    [[nodiscard]] constexpr Ratio framesPerSecond() const { return m_framesPerSecond; }

    /// Returns the numerator of the reduced fraction, in frames.
    [[nodiscard]] constexpr std::int64_t numerator() const { return m_framesPerSecond.numerator(); }

    /// Returns the denominator of the reduced fraction, in seconds.
    [[nodiscard]] constexpr std::int64_t denominator() const {
        return m_framesPerSecond.denominator();
    }

    /// Returns how long one frame lasts, in milliseconds, exactly.
    ///
    /// Never a whole number of milliseconds for the NTSC rates: a frame at
    /// 24000/1001 lasts 1001/24 of one. That is the reason a position is
    /// computed from the rational and rounded once, at the end.
    ///
    /// Held rather than derived on call: a reader converts once per subtitle,
    /// and reducing the fraction each time measured 3.7 times the cost of the
    /// conversion itself. A frame rate is built once per project.
    [[nodiscard]] constexpr Ratio millisecondsPerFrame() const { return m_millisecondsPerFrame; }

    /// Returns how many frames one millisecond spans, exactly.
    [[nodiscard]] constexpr Ratio framesPerMillisecond() const { return m_framesPerMillisecond; }

    /// Returns the factor carrying a position timed at this rate to the same
    /// position timed at `destination`.
    ///
    /// Naming the direction is the point. The inverse is just as valid a ratio
    /// and looks the same at the call site; applied by mistake it drifts the
    /// file the other way — some seven seconds over a feature film between 25
    /// and 23.976 — with nothing to say so until someone plays it.
    [[nodiscard]] constexpr Ratio conversionTo(FrameRate destination) const {
        return ratioOf(numerator() * destination.denominator(),
                       denominator() * destination.numerator());
    }

    /// Two rates are equal when they are the same number of frames per second.
    /// The two derived ratios follow from that one, and comparing them again
    /// would only ask the same question twice.
    [[nodiscard]] friend constexpr bool operator==(FrameRate left, FrameRate right) {
        return left.m_framesPerSecond == right.m_framesPerSecond;
    }

private:
    static constexpr std::int64_t kMillisecondsPerSecond = 1000;

    explicit constexpr FrameRate(Ratio framesPerSecond)
        : m_framesPerSecond(framesPerSecond),
          m_millisecondsPerFrame(ratioOf(kMillisecondsPerSecond * framesPerSecond.denominator(),
                                         framesPerSecond.numerator())),
          m_framesPerMillisecond(ratioOf(framesPerSecond.numerator(),
                                         kMillisecondsPerSecond * framesPerSecond.denominator())) {}

    /// Builds a ratio from terms this class has already established as valid.
    ///
    /// Strictly positive and bounded by `kLargestTerm`, so `Ratio`'s fallible
    /// factory would answer a question that is already settled. `FrameRate` is
    /// a friend of `Ratio` for this one reason.
    [[nodiscard]] static constexpr Ratio ratioOf(std::int64_t numerator, std::int64_t denominator) {
        return Ratio{Ratio::reduce(numerator, denominator)};
    }

    [[nodiscard]] static constexpr Ratio exactValueOf(StandardFrameRate standard) {
        switch (standard) {
        case StandardFrameRate::Fps23976:
            return ratioOf(24000, 1001);
        case StandardFrameRate::Fps24:
            return ratioOf(24, 1);
        case StandardFrameRate::Fps25:
            return ratioOf(25, 1);
        case StandardFrameRate::Fps29970:
            return ratioOf(30000, 1001);
        case StandardFrameRate::Fps30:
            return ratioOf(30, 1);
        case StandardFrameRate::Fps50:
            return ratioOf(50, 1);
        case StandardFrameRate::Fps59940:
            return ratioOf(60000, 1001);
        case StandardFrameRate::Fps60:
            return ratioOf(60, 1);
        }
        // The switch above covers every enumerator, and the compiler checks
        // that it does. Only a cast from an out-of-range integer could land
        // here, which is a programming error rather than a case to handle:
        // inventing a frame rate for it would hide the bug.
        std::unreachable();
    }

    Ratio m_framesPerSecond;
    Ratio m_millisecondsPerFrame;
    Ratio m_framesPerMillisecond;
};

} // namespace subedit::core
