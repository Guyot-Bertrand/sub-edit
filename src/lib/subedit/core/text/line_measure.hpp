#pragma once

// How long a line of text is, for the line-breaker's limits — decision D5 of
// the spec of phase 12.
//
// **An interface with two implementations, one on each side of the noyau's
// boundary.** This file holds the noyau's own, in characters; `gui` holds the
// other, in ems — the font-based unit Gaupol shows, `QFontMetricsF` calibrated
// the way Gaupol calibrates it. The line-breaker asks only this interface,
// never which one it was given: swapping units is swapping the object passed
// in, nothing in the algorithm changes.

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace subedit::core {

namespace detail {

/// Hashes a `std::string` the same way as the `std::string_view` a lookup is
/// given, so `CachedLineMeasure` never has to build a `std::string` just to
/// ask its table whether it already knows one.
struct TransparentStringHash {
    // The standard's own name for the tag that makes a hash transparent —
    // `std::unordered_map::find` looks for it by this exact spelling, so it
    // cannot follow the project's own naming convention.
    // NOLINTNEXTLINE(readability-identifier-naming)
    using is_transparent = void;

    [[nodiscard]] std::size_t operator()(std::string_view text) const noexcept {
        return std::hash<std::string_view>{}(text);
    }
};

} // namespace detail

/// How long a piece of text is, in whatever unit a caller cares about.
///
/// **Tags are a caller's business, not this one's**: `lengthOf` is handed
/// visible text, already stripped of markup by whoever read it — the
/// line-breaker, through its own reading of `format`. A measure that had to
/// know about tags would be a measure that knew about subtitle formats, and
/// this one counts characters or pixels, nothing else.
class LineMeasure {

public:
    virtual ~LineMeasure() = default;

    /// The length of `text` — `\n` counts like any other character, since
    /// nothing here treats a line as more than a string.
    [[nodiscard]] virtual double lengthOf(std::string_view text) const = 0;

protected:
    LineMeasure() = default;
    LineMeasure(const LineMeasure&) = default;
    LineMeasure(LineMeasure&&) = default;
    LineMeasure& operator=(const LineMeasure&) = default;
    LineMeasure& operator=(LineMeasure&&) = default;
};

/// Gaupol's `get_char_length`: the number of Unicode code points.
class CharacterLineMeasure final : public LineMeasure {

public:
    [[nodiscard]] double lengthOf(std::string_view text) const override;
};

/// A `LineMeasure` that remembers what it was already asked.
///
/// **Injected, not built in** — decision D5: the line-breaker takes whatever
/// `LineMeasure` it is given, and a caller wraps one in this to have it
/// remember. The table lives as long as this object does, which a caller
/// keeps to the time of one operation on a document — a table that outlived
/// it could answer a length from a font nobody uses any more.
class CachedLineMeasure final : public LineMeasure {

public:
    /// `underlying` must outlive this object.
    explicit CachedLineMeasure(const LineMeasure& underlying);

    [[nodiscard]] double lengthOf(std::string_view text) const override;

private:
    const LineMeasure* m_underlying;
    mutable std::unordered_map<std::string, double, detail::TransparentStringHash, std::equal_to<>>
        m_cache;
};

} // namespace subedit::core
