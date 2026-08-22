#pragma once

#include <subedit/gui/operation_dialog.hpp>

#include <cstddef>

namespace subedit::gui {

/// Asks whether to remove the hearing-impaired mentions of the target.
///
/// **Nothing to fill in**, which is the whole of what distinguishes it from
/// the other three: the operation has no parameter, so the dialog is a
/// confirmation and the target label is its content. It is a dialog rather
/// than a message box because the target is the question — « on these four
/// subtitles, or on all of them? » — and that sentence is already written
/// once, in `OperationDialog`.
class HearingImpairedDialog final : public OperationDialog {
    Q_OBJECT

public:
    explicit HearingImpairedDialog(std::size_t targetCount, QWidget* parent = nullptr);

    /// Always: there is nothing to type, so there is nothing to get wrong.
    [[nodiscard]] bool isComplete() const override { return true; }
};

} // namespace subedit::gui
