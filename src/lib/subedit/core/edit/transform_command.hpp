#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/ratio.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <optional>
#include <utility>
#include <vector>

namespace subedit::core {

class Project;

/// A subtitle, and where the user wants its start to land.
struct TransformReference {
    SubtitleIndex index;
    Timestamp target;
};

/// Corrects the positions of a selection from two reference points.
///
/// The user picks two subtitles and says where each of their starts belongs;
/// everything in between and around follows, affinely:
///
/// ```
/// t′ = round( (t − x₁) × r ) + y₁      with r = (y₂ − y₁) / (x₂ − x₁)
/// ```
///
/// Written this way rather than as `round(r × t) + constant`, which is
/// Gaupol's form, for two reasons. **One rounding**, and above all **the two
/// references land exactly where they were asked to**: `(x₂ − x₁) × r` is
/// `y₂ − y₁` exactly, by construction of `r`. Gaupol's form rounds the
/// coefficient and the constant separately and does not guarantee it.
///
/// `r` is a `Ratio`, so it is exact. A null denominator — two references on
/// one subtitle, or on two subtitles that start together — defines no
/// transform, and `create` refuses it.
///
/// **What it retains to undo itself is the previous positions.** The operation
/// rounds, so its inverse cannot be a second calculation: transforming back
/// would land a millisecond off. That is the price the shift does not pay.
class TransformCommand final : public Command {

public:
    /// Builds the transform, or nothing if the two references define none.
    ///
    /// The previous positions are captured now, from `project` as it stands.
    /// A reference outside `selection` still defines the correction, but its
    /// own start does not move — the caller decides what it is transforming.
    [[nodiscard]] static std::optional<TransformCommand> create(const Project& project,
                                                                Selection selection,
                                                                TransformReference first,
                                                                TransformReference second);

    void apply(Project& project) override;

    void revert(Project& project) override;

    [[nodiscard]] CommandKind kind() const override { return CommandKind::Transform; }

    /// Reports a change of positions on the selected subtitles.
    [[nodiscard]] std::vector<Change> describe() const override {
        return {Change{.kind = ChangeKind::Positions, .subtitles = m_selection}};
    }

private:
    /// The two ends of one subtitle, as they stood before the transform.
    struct Positions {
        Timestamp start;
        Timestamp end;
    };

    TransformCommand(Selection selection,
                     Ratio ratio,
                     Timestamp origin,
                     Timestamp target,
                     std::vector<Positions> previous)
        : m_selection(std::move(selection)),
          m_ratio(ratio),
          m_origin(origin),
          m_target(target),
          m_previous(std::move(previous)) {}

    /// Returns where `position` lands.
    [[nodiscard]] Timestamp transformed(Timestamp position) const {
        return m_target + (position - m_origin).scaledBy(m_ratio);
    }

    Selection m_selection;
    Ratio m_ratio;

    /// The first reference: `x₁`, the position distances are measured from,
    /// and `y₁`, where it lands.
    Timestamp m_origin;
    Timestamp m_target;

    std::vector<Positions> m_previous;
};

} // namespace subedit::core
