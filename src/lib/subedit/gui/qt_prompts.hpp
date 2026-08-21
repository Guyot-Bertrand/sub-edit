#pragma once

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/gui/prompts.hpp>

#include <filesystem>
#include <optional>
#include <string>

class QDialog;
class QString;
class QWidget;

namespace subedit::gui {

/// What the file dialogs filter on, in the order they offer it.
[[nodiscard]] QString subtitleFilters();

/// Which format a chosen filter names.
///
/// **Exposed, and not because a test needed a keyhole.** It decides what a
/// « save as » writes, which is the one thing in this file that can be wrong
/// on its own — the rest is Qt opening a window. Behind the dialog it would
/// have been uncoverable along with everything else; here it is ordinary code
/// with an ordinary test.
[[nodiscard]] core::SubtitleFormat formatOfFilter(const QString& filter);

/// What a button of the unsaved-changes box means.
///
/// Exposed for the same reason. Anything that is not an explicit choice —
/// closing the box, pressing Escape — is `Cancel`, and that is a decision
/// rather than a default.
[[nodiscard]] UnsavedChoice choiceOf(int button);

/// The questions, asked of a human through Qt's modal dialogs.
///
/// **The one class of this library no test enters**, and it is why the seam
/// exists: each method is a single call to `QFileDialog` or `QMessageBox`,
/// both of which spin a nested event loop until somebody clicks. A test
/// reaching one would never return.
///
/// Everything worth deciding is therefore *not* here. What comes back — a
/// path, a format, a choice, or nothing at all — is decided by the window,
/// which a test drives through a fake of `Prompts`.
class QtPrompts final : public Prompts {

public:
    /// Builds prompts parented on `owner`, which the dialogs sit over.
    explicit QtPrompts(QWidget* owner) : m_owner(owner) {}

    [[nodiscard]] std::optional<std::filesystem::path> fileToOpen() override;

    [[nodiscard]] std::optional<SaveTarget> saveTarget(const core::SourceFile& current) override;

    [[nodiscard]] UnsavedChoice aboutUnsavedChanges() override;

    [[nodiscard]] bool run(QDialog& dialog) override;

    void reportFailure(const std::string& message) override;

private:
    QWidget* m_owner;
};

} // namespace subedit::gui
