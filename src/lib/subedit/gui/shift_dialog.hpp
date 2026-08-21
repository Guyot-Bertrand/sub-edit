#pragma once

#include <subedit/core/time/duration.hpp>
#include <subedit/gui/operation_dialog.hpp>

#include <cstddef>
#include <optional>

class QLineEdit;
class QString;

namespace subedit::gui {

/// Asks by how much to move the target, forwards or backwards.
///
/// One signed duration, typed as a timestamp is: `00:00:02,500` moves forward,
/// `-0:01,250` moves back. Read by `Timestamp::parse`, so the same permissive
/// spelling a cell accepts works here.
class ShiftDialog final : public OperationDialog {
    Q_OBJECT

public:
    explicit ShiftDialog(std::size_t targetCount, QWidget* parent = nullptr);

    /// How much to move by, or nothing if what was typed is not a duration.
    [[nodiscard]] std::optional<core::Duration> shift() const;

    [[nodiscard]] bool isComplete() const override;

    /// Types `text` into the field, as a user would.
    void setTyped(const QString& text);

private:
    QLineEdit* m_by;
};

} // namespace subedit::gui
