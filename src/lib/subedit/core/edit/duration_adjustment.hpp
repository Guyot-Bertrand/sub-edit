#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/time/duration.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

namespace subedit::core {

class Project;
class Selection;

/// Gaupol's default reading speed, in visible characters per second.
inline constexpr double kDefaultReadingSpeed = 15.0;

/// Gaupol's default minimum duration, in milliseconds.
inline constexpr std::int64_t kDefaultMinimumMilliseconds = 1500;

/// Gaupol's default maximum duration, in milliseconds — off by default, and
/// this is only what a switched-off field shows.
inline constexpr std::int64_t kDefaultMaximumMilliseconds = 6000;

/// The reading speed a duration is measured against, and which way it may move
/// an end.
///
/// Built through `create`, which refuses what is not a speed: the constructor
/// is private, so a value of this type is always strictly positive.
class ReadingSpeed {

public:
    /// Builds a reading speed, or nothing if `charactersPerSecond` is zero or
    /// less.
    [[nodiscard]] static std::optional<ReadingSpeed>
    create(double charactersPerSecond, bool lengthen, bool shorten);

    /// Visible characters per second. Strictly positive.
    [[nodiscard]] double charactersPerSecond() const { return m_charactersPerSecond; }

    /// Moves the end later when the text needs more time to be read.
    [[nodiscard]] bool lengthen() const { return m_lengthen; }

    /// Moves the end earlier when the text is on screen longer than it needs.
    [[nodiscard]] bool shorten() const { return m_shorten; }

    friend bool operator==(const ReadingSpeed&, const ReadingSpeed&) = default;

private:
    ReadingSpeed(double charactersPerSecond, bool lengthen, bool shorten)
        : m_charactersPerSecond(charactersPerSecond), m_lengthen(lengthen), m_shorten(shorten) {}

    double m_charactersPerSecond;
    bool m_lengthen;
    bool m_shorten;
};

/// The four constraints of `adjust_durations` — decision D2 of the phase-10
/// spec.
///
/// **An inactive constraint is absent, not zero.** Gaupol tests `minimum and …`,
/// which is false for zero, so a minimum of zero switches the minimum off by
/// accident; its gap, tested by `is not None`, does not. Here every constraint
/// is an option, and zero means zero.
///
/// The defaults are Gaupol's: a speed of fifteen characters a second that only
/// lengthens, a minimum of 1.5 s, no maximum, and a gap of zero.
struct DurationConstraints {
    std::optional<ReadingSpeed> speed = ReadingSpeed::create(kDefaultReadingSpeed, true, false);
    std::optional<Duration> minimum = Duration::fromMilliseconds(kDefaultMinimumMilliseconds);
    std::optional<Duration> maximum{};
    std::optional<Duration> gap = Duration::zero();

    /// Tells whether any constraint is active, and so whether adjusting could
    /// do anything at all.
    [[nodiscard]] bool isAny() const {
        const bool reads = speed.has_value() && (speed->lengthen() || speed->shorten());
        return reads || minimum.has_value() || maximum.has_value() || gap.has_value();
    }

    friend bool operator==(const DurationConstraints&, const DurationConstraints&) = default;
};

/// How many subtitles ended up violating each constraint once the others had
/// their say.
///
/// **This is what the adjustment sacrifices, and it is said** — ADR 0008. Gaupol
/// violates in silence; a subtitle whose next one starts too soon cannot have
/// both its minimum and its gap, and the order decides which one gives way.
///
/// **The maximum has no count, because it always holds.** It is applied after
/// the speed and the minimum, and only the gap comes after it — which can only
/// bring an end earlier. A minimum above the maximum is given up to it, and
/// counted as a minimum.
struct SacrificedConstraints {
    std::size_t speed = 0;
    std::size_t minimum = 0;
    std::size_t gap = 0;

    [[nodiscard]] bool isAny() const { return speed + minimum + gap > 0; }

    friend bool operator==(const SacrificedConstraints&, const SacrificedConstraints&) = default;
};

/// What adjusting will do, read before it is applied.
struct DurationAdjustment {
    /// Nothing when no end moves.
    std::unique_ptr<Command> command{};

    /// How many ends move.
    std::size_t adjusted = 0;

    /// What no end could satisfy, counted over the whole target — including
    /// the subtitles whose end did not move.
    SacrificedConstraints sacrificed{};
};

/// Builds the adjustment of the subtitles of `selection` to `constraints`.
///
/// **Gaupol's order, and only the end moves.** Reading speed, then minimum,
/// then maximum, then the gap to the next subtitle; each one sets the end, and
/// the last one applied wins. The gap comes last because it is the only
/// defensible winner: two subtitles on screen at once is a fault the viewer
/// sees, a subtitle a little short is not. The start is never touched — moving
/// it would move the subtitle, which is the work of a shift.
///
/// **The length of a text is counted without its tags**, in characters and not
/// in bytes, line breaks included — `get_text_length`: reading speed is about
/// what the viewer reads.
///
/// **The last subtitle of the project has no next one, and no bound for it.**
/// Gaupol gives it 360 000 s; a number that means « no constraint » is written
/// here as the absence it is. The next subtitle is the next in the project,
/// selected or not.
///
/// One entry in the history, however many ends move.
[[nodiscard]] DurationAdjustment adjustDurations(const Project& project,
                                                 const Selection& selection,
                                                 const DurationConstraints& constraints);

} // namespace subedit::core
