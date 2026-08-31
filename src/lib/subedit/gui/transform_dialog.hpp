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

    /// L'origine, et non un membre laissé au hasard : `Timestamp` n'a pas de
    /// constructeur par défaut public qui poserait une valeur, et un
    /// `TypedReference` construit sans initialisateur portait donc une position
    /// indéterminée. Repéré par l'analyse des en-têtes, que la porte n'avait
    /// jamais faite — issue #269.
    core::Timestamp target = core::Timestamp::origin();

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
    TransformDialog(std::size_t targetCount, std::size_t subtitleCount, QWidget* parent = nullptr);

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
