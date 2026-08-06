#pragma once

#include <subedit/core/model/document.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <string>

namespace subedit::core {

/// One subtitle: when it appears, when it leaves, and what it says.
///
/// It carries **both texts and a single pair of positions**, which states
/// directly that a translation has no timing of its own.
///
/// **`end >= start` is not an invariant.** This is an assumed exception to the
/// principle of unrepresentable invalid states, justified by ADR 0008: a real
/// file may contain an incoherent subtitle, and the user has to see it to fix
/// it. Refusing to hold the anomaly would mean refusing to open the file. The
/// anomaly produces a diagnostic, not a rejection.
struct Subtitle {
    Timestamp start = Timestamp::origin();
    Timestamp end = Timestamp::origin();

    /// Raw text, tags of the format included — see ADR 0009.
    std::string mainText{};
    std::string translationText{};

    FormatExtras extras{};

    /// Returns how long the subtitle stays on screen.
    ///
    /// Derived from the two positions, never stored: a stored duration is one
    /// more thing to keep in step with them. Negative when the end precedes
    /// the start, which is how that anomaly stays visible.
    [[nodiscard]] Duration duration() const { return end - start; }

    /// Returns the text of `document`.
    [[nodiscard]] const std::string& text(Document document) const {
        return document == Document::Main ? mainText : translationText;
    }

    /// Returns the text of `document`, for writing.
    [[nodiscard]] std::string& text(Document document) {
        return document == Document::Main ? mainText : translationText;
    }

    friend bool operator==(const Subtitle&, const Subtitle&) = default;
};

} // namespace subedit::core
