#pragma once

#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/operation_dialog.hpp>

#include <cstddef>
#include <optional>

class QLineEdit;
class QSpinBox;
class QString;

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
    core::Timestamp target;

    friend bool operator==(const TypedReference&, const TypedReference&) = default;
};

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
    /// `subtitleCount` bounds the two indices: a reference outside the file
    /// corrects nothing.
    explicit TransformDialog(std::size_t subtitleCount, QWidget* parent = nullptr);

    [[nodiscard]] std::optional<TypedReference> first() const;

    [[nodiscard]] std::optional<TypedReference> second() const;

    [[nodiscard]] bool isComplete() const override;

    /// Fills both references, as a user would.
    void setTyped(int firstNumber,
                  const QString& firstTarget,
                  int secondNumber,
                  const QString& secondTarget);

private:
    [[nodiscard]] static std::optional<TypedReference> referenceOf(const QSpinBox& number,
                                                                   const QLineEdit& target);

    QSpinBox* m_firstNumber;
    QLineEdit* m_firstTarget;
    QSpinBox* m_secondNumber;
    QLineEdit* m_secondTarget;
};

} // namespace subedit::gui
