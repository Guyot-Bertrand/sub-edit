#pragma once

#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/operation_dialog.hpp>

#include <QString>

#include <cstddef>
#include <functional>
#include <optional>

namespace subedit::core {
class Project;
}

class QLabel;
class QLineEdit;
class QSpinBox;

namespace subedit::gui {

/// A reference as the dialog reads it: a subtitle number, and where its start
/// belongs.
///
/// **Not `core::TransformReference`, and not for want of trying.** That header
/// drags `Selection`, whose iterator pulls the C++20 library headers `moc`
/// cannot parse — the table model carries the same scar. Turning this into the
/// core's own type is one line in the window, which is where the command is
/// built anyway.
struct TypedReference {
    int number = 1;

    /// The origin, and not a member left to chance: `Timestamp` has no public
    /// default constructor that would lay a value down, so a `TypedReference`
    /// built with no initialiser carried an indeterminate position. Spotted by
    /// the analysis of the headers, which the gate had never done — issue
    /// #269.
    core::Timestamp target = core::Timestamp::origin();

    friend bool operator==(const TypedReference&, const TypedReference&) = default;
};

/// What the dialog shows of a subtitle it is asked to anchor: where it starts
/// now, and what it says.
///
/// Text, not core values, for the reason `TypedReference` is not the core's:
/// the window turns the project's subtitle into this where the command is
/// built anyway.
struct AnchorView {
    QString start;
    QString text;
};

/// The subtitle of `project` that `number` names, as the dialog shows it, or
/// nothing for a number outside the file.
///
/// A function of its own, and not a lambda in the window, so that the bound is
/// checked where it can be tested: `Project::subtitleAt` throws past the last
/// subtitle, and a throw out of a slot ends the application.
[[nodiscard]] std::optional<AnchorView> anchorIn(const core::Project& project, int number);

/// Looks a subtitle up by the number the user typed; nothing for a number
/// outside the file.
using AnchorLookup = std::function<std::optional<AnchorView>(int number)>;

/// Asks where two subtitles belong, and moves everything else with them.
///
/// The correction a whole file needs when it was timed against another cut:
/// say where the first subtitle really starts and where a late one really
/// starts, and the affine correction between them follows.
///
/// **Two references on one subtitle define no transform** — the core refuses
/// it, and this dialog refuses to be validated before the user finds out.
class TransformDialog final : public OperationDialog {
    Q_OBJECT

public:
    /// Two counts, and they are not the same one.
    ///
    /// `targetCount` is what the operation would touch — the selection, or the
    /// whole file — and it is what the dialog says it applies to.
    /// `subtitleCount` bounds the two indices, and it is the file: a reference
    /// is a subtitle number, so it may name a line the selection leaves out,
    /// and one outside the file corrects nothing.
    ///
    /// They were one parameter until the review of the phase, which is how the
    /// label came to name the file while the operation touched the selection.
    ///
    /// `lookup` is what lets the dialog say which subtitle a number is: its
    /// current start, and its text. Without it the two rows stay empty, which
    /// is the dialog a test builds when it only cares about what was typed.
    TransformDialog(std::size_t targetCount,
                    std::size_t subtitleCount,
                    AnchorLookup lookup = {},
                    QWidget* parent = nullptr);

    /// The current start shown for the first, or second, reference.
    [[nodiscard]] QString firstCurrent() const;
    [[nodiscard]] QString secondCurrent() const;

    /// The text shown for the first, or second, reference.
    [[nodiscard]] QString firstText() const;
    [[nodiscard]] QString secondText() const;

    [[nodiscard]] std::optional<TypedReference> first() const;

    [[nodiscard]] std::optional<TypedReference> second() const;

    [[nodiscard]] bool isComplete() const override;

    /// Fills both references, as a user would.
    void setTyped(int firstNumber,
                  const QString& firstTarget,
                  int secondNumber,
                  const QString& secondTarget) const;

private:
    /// One reference: what is typed, and what the number stands for.
    struct Row {
        QSpinBox* number;
        QLineEdit* current;
        QLineEdit* target;
        QLabel* text;
    };

    [[nodiscard]] static std::optional<TypedReference> referenceOf(const Row& row);

    [[nodiscard]] Row makeRow(std::size_t subtitleCount);

    /// Shows the subtitle `row.number` names, and offers its current start as
    /// the new one — the user changes what they mean to change.
    void refresh(const Row& row);

    AnchorLookup m_lookup;
    Row m_first;
    Row m_second;
};

} // namespace subedit::gui
