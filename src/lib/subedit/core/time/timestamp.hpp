#pragma once

#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

/// The character separating the seconds from their decimals.
enum class DecimalMark {
    Comma,  ///< `00:00:01,500`, as SubRip writes it
    Period, ///< `00:00:01.500`, as WebVTT writes it
};

/// Whether the hours field is written.
enum class HourField {
    Always,  ///< `HH:MM:SS`
    Omitted, ///< `MM:SS`, unless the position reaches one hour
};

/// A position on the timeline, counted in whole signed milliseconds from the
/// start of the video.
///
/// Positions before the origin are valid: shifting a subtitle backwards may
/// legitimately take it there, and refusing to represent it would turn an
/// editing operation into a special case.
class Timestamp {

public:
    /// Builds a position from a signed millisecond count.
    [[nodiscard]] static constexpr Timestamp fromMilliseconds(std::int64_t count) {
        return Timestamp{count};
    }

    /// Builds the position of the start of the video.
    [[nodiscard]] static constexpr Timestamp origin() { return Timestamp{0}; }

    /// Builds the position at which `frame` starts, at `rate`.
    ///
    /// The result is rounded to the nearest millisecond, halves away from
    /// zero. This round trip is exact: converting the result back with the
    /// same rate gives `frame` again, for any rate up to 60 frames per second.
    [[nodiscard]] static constexpr Timestamp fromFrame(Frame frame, FrameRate rate) {
        return Timestamp{roundedDivide(frame.number() * rate.denominator() * kMillisecondsPerSecond,
                                       rate.numerator())};
    }

    /// Reads a position from a timestamp, or nothing if the text is not one.
    ///
    /// Deliberately permissive, as the files in the wild require: fields of
    /// one or two digits, one to three decimals or none at all, hours
    /// optional, either decimal mark, an optional leading sign, surrounding
    /// blanks ignored. Out-of-range minutes and seconds are refused, so that
    /// the caller can report the line rather than invent a position for it.
    [[nodiscard]] static std::optional<Timestamp> parse(std::string_view text);

    /// Returns the signed millisecond count.
    [[nodiscard]] constexpr std::int64_t milliseconds() const { return m_milliseconds; }

    /// Writes the position as `HH:MM:SS,mmm`, or `MM:SS,mmm` when the hours
    /// are omitted and the position stays below one hour.
    ///
    /// Positions before the origin carry a leading sign, and the magnitude
    /// saturates at `99:59:59,999` — a constraint of the file formats, which
    /// leaves the position itself untouched.
    [[nodiscard]] std::string format(DecimalMark mark, HourField hours = HourField::Always) const;

    /// Returns the frame this position falls in, at `rate`.
    ///
    /// Rounded to the nearest frame, halves away from zero. Sub-frame
    /// precision is lost, so converting the result back does not give this
    /// position again — inherent to the coarser grid, not a defect.
    [[nodiscard]] constexpr Frame toFrame(FrameRate rate) const {
        return Frame::fromNumber(roundedDivide(m_milliseconds * rate.numerator(),
                                               rate.denominator() * kMillisecondsPerSecond));
    }

    constexpr Timestamp& operator+=(Duration shift) {
        m_milliseconds += shift.milliseconds();
        return *this;
    }

    constexpr Timestamp& operator-=(Duration shift) {
        m_milliseconds -= shift.milliseconds();
        return *this;
    }

    [[nodiscard]] friend constexpr Duration operator-(Timestamp end, Timestamp start) {
        return Duration::fromMilliseconds(end.m_milliseconds - start.m_milliseconds);
    }

    [[nodiscard]] friend constexpr Timestamp operator+(Timestamp position, Duration shift) {
        return Timestamp{position.m_milliseconds + shift.milliseconds()};
    }

    [[nodiscard]] friend constexpr Timestamp operator+(Duration shift, Timestamp position) {
        return position + shift;
    }

    [[nodiscard]] friend constexpr Timestamp operator-(Timestamp position, Duration shift) {
        return Timestamp{position.m_milliseconds - shift.milliseconds()};
    }

    [[nodiscard]] friend constexpr bool operator==(Timestamp, Timestamp) = default;

    [[nodiscard]] friend constexpr auto operator<=>(Timestamp, Timestamp) = default;

private:
    static constexpr std::int64_t kMillisecondsPerSecond = 1000;
    static constexpr std::int64_t kSecondsPerMinute = 60;
    static constexpr std::int64_t kMinutesPerHour = 60;
    static constexpr std::int64_t kMillisecondsPerMinute =
        kMillisecondsPerSecond * kSecondsPerMinute;
    static constexpr std::int64_t kMillisecondsPerHour = kMillisecondsPerMinute * kMinutesPerHour;

    /// The largest magnitude the two-digit hours field can hold.
    static constexpr std::int64_t kWritableHours = 100;
    static constexpr std::int64_t kMaxWritableMilliseconds =
        kWritableHours * kMillisecondsPerHour - 1;

    explicit constexpr Timestamp(std::int64_t count) : m_milliseconds(count) {}

    /// Divides, rounding to the nearest integer with halves away from zero.
    ///
    /// `divisor` is strictly positive: it always comes from a `FrameRate`,
    /// whose two terms are positive by construction.
    ///
    /// Symmetry around zero is the point. Round-half-to-even, which Gaupol
    /// inherits from Python, gives different answers for a value and its
    /// opposite, so shifting a subtitle by the same amount on either side of
    /// the origin would not give matching results.
    [[nodiscard]] static constexpr std::int64_t roundedDivide(std::int64_t dividend,
                                                              std::int64_t divisor) {
        const std::int64_t quotient = dividend / divisor;
        const std::int64_t remainder = dividend % divisor;
        if (remainder == 0)
            return quotient;

        const std::int64_t distanceFromQuotient = remainder < 0 ? -remainder : remainder;
        if (2 * distanceFromQuotient < divisor)
            return quotient;

        return dividend > 0 ? quotient + 1 : quotient - 1;
    }

    std::int64_t m_milliseconds;
};

} // namespace subedit::core
