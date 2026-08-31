#pragma once

#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>

namespace subedit::core {

/// An exact rational factor, signed and reduced.
///
/// Not a quantity but what multiplies one: neither a position nor a length,
/// which is why it is a type of its own rather than a `Timestamp` used as a
/// coefficient.
///
/// A factor held as a `double` would be enough for none of the uses: the ratio
/// of two NTSC frame rates has no exact binary representation, and the error
/// would land on every position of the file.
///
/// Stored reduced, with a strictly positive denominator. Two ratios written
/// differently but equal in value then compare equal, the sign lives in one
/// place, and `scale` may divide without asking about signs.
class Ratio {

public:
    /// Builds a ratio, or nothing if it would not be one.
    ///
    /// Refused: a null denominator, and either term equal to the smallest
    /// `std::int64_t` — that value has no opposite in its own type, which both
    /// the reduction and the move of the sign need.
    [[nodiscard]] static constexpr std::optional<Ratio> create(std::int64_t numerator,
                                                               std::int64_t denominator) {
        constexpr std::int64_t kSmallest = std::numeric_limits<std::int64_t>::min();
        if (denominator == 0 || numerator == kSmallest || denominator == kSmallest)
            return std::nullopt;

        return Ratio{reduce(numerator, denominator)};
    }

    /// Builds the neutral factor, which leaves every value untouched.
    [[nodiscard]] static constexpr Ratio one() {
        return Ratio{Reduced{.numerator = 1, .denominator = 1}};
    }

    /// Returns the numerator of the reduced fraction, which carries the sign.
    [[nodiscard]] constexpr std::int64_t numerator() const { return m_numerator; }

    /// Returns the denominator of the reduced fraction, strictly positive.
    [[nodiscard]] constexpr std::int64_t denominator() const { return m_denominator; }

    /// Multiplies `value` by the exact rational and rounds the result once, to
    /// the nearest integer with halves away from zero.
    ///
    /// A single rounding is the whole point: computing in two steps, or going
    /// through an intermediate grid such as the frames, quantises the value
    /// twice and lands up to half a step away from the exact answer.
    ///
    /// **Exact on the domain that means something.** A position stays under
    /// `3.6 × 10⁸` milliseconds and a reduced ratio keeps a numerator of the
    /// same order, so the intermediate product holds in a `std::int64_t` with
    /// room to spare. Past that the result saturates at the bounds of the type
    /// rather than wrapping around: an answer that is defined and ordered like
    /// the exact one, for an input that was already meaningless — the bound is
    /// some 292 million years.
    [[nodiscard]] constexpr std::int64_t scale(std::int64_t value) const {
        return roundedDivide(saturatedProduct(value, m_numerator), m_denominator);
    }

    [[nodiscard]] friend constexpr bool operator==(Ratio, Ratio) = default;

private:
    /// `FrameRate` is a `Ratio` with two invariants of its own — terms
    /// strictly positive, and bounded so that its conversions cannot overflow.
    /// Having established them, it builds its ratios through the constructor
    /// below rather than through the fallible factory, which would ask it to
    /// handle a failure its own invariants exclude.
    friend class FrameRate;

    /// Two terms that have been through `reduce`.
    ///
    /// A type of its own rather than a pair of integers, so that the private
    /// constructor cannot be handed raw terms by mistake: what it takes is
    /// reduced, and the compiler holds that promise.
    struct Reduced {
        std::int64_t numerator;
        std::int64_t denominator;
    };

    explicit constexpr Ratio(Reduced terms)
        : m_numerator(terms.numerator), m_denominator(terms.denominator) {}

    /// Divides both terms by their greatest common divisor and moves the sign
    /// to the numerator.
    ///
    /// `std::gcd(0, d)` is `d`, so a null numerator reduces to `0/1` rather
    /// than keeping the denominator it was built with. Neither term is the
    /// smallest `std::int64_t`, which `create` has already refused, so the
    /// negation below is defined.
    [[nodiscard]] static constexpr Reduced reduce(std::int64_t numerator,
                                                  std::int64_t denominator) {
        const std::int64_t divisor = std::gcd(numerator, denominator);
        const std::int64_t sign = denominator < 0 ? -1 : 1;

        return {.numerator = sign * numerator / divisor,
                .denominator = sign * denominator / divisor};
    }

    /// Multiplies, stopping at the bounds of the type instead of overflowing.
    ///
    /// The test is made before the multiplication and never overflows itself:
    /// dividing a bound by a non-null operand is always defined.
    ///
    /// The two-power guard is not decoration. Every real conversion takes the
    /// first branch — a position is under `2³¹` milliseconds, some 25 days, and
    /// a reduced frame-rate term is far below it — and the division-based test
    /// costs about as much as the whole conversion. Measured on the frame
    /// conversion: 1.9 times the cost without the guard, 1.4 with.
    [[nodiscard]] static constexpr std::int64_t saturatedProduct(std::int64_t left,
                                                                 std::int64_t right) {
        constexpr std::int64_t kLargest = std::numeric_limits<std::int64_t>::max();
        constexpr std::int64_t kSmallest = std::numeric_limits<std::int64_t>::min();
        constexpr std::int64_t kSafe = std::int64_t{1} << 31;

        if (left > -kSafe && left < kSafe && right > -kSafe && right < kSafe)
            return left * right;

        if (left == 0 || right == 0)
            return 0;

        const bool positive = (left > 0) == (right > 0);

        // Les quatre cas de signe, écrits à plat. C'était un ternaire dans un
        // ternaire, et la table de vérité était juste ; elle ne se relisait
        // simplement pas deux fois de la même façon.
        bool overflows = false;
        if (left > 0 && right > 0)
            overflows = left > kLargest / right;
        else if (left > 0)
            overflows = right < kSmallest / left;
        else if (right > 0)
            overflows = left < kSmallest / right;
        else
            overflows = left < kLargest / right;

        if (overflows)
            return positive ? kLargest : kSmallest;

        return left * right;
    }

    /// Divides, rounding to the nearest integer with halves away from zero.
    ///
    /// `divisor` is the denominator, strictly positive by construction — which
    /// is why this stays inside `Ratio`. As a free function it would be a
    /// precondition held by a comment; here it is held by the type.
    ///
    /// Symmetry around zero is the point. Round-half-to-even, which Gaupol
    /// inherits from Python, gives different answers for a value and its
    /// opposite, so shifting a subtitle by the same amount on either side of
    /// the origin would not give matching results.
    ///
    /// The halving test is written against `divisor - distance` rather than
    /// `2 * distance`, which would overflow for a denominator past half the
    /// range. Both terms of that subtraction are in `[0, divisor]`.
    [[nodiscard]] static constexpr std::int64_t roundedDivide(std::int64_t dividend,
                                                              std::int64_t divisor) {
        const std::int64_t quotient = dividend / divisor;
        const std::int64_t remainder = dividend % divisor;
        if (remainder == 0)
            return quotient;

        const std::int64_t distance = remainder < 0 ? -remainder : remainder;
        if (distance < divisor - distance)
            return quotient;

        return dividend > 0 ? quotient + 1 : quotient - 1;
    }

    std::int64_t m_numerator;
    std::int64_t m_denominator;
};

} // namespace subedit::core
