#pragma once

#include <subedit/core/model/subtitle_format.hpp>

#include <filesystem>
#include <optional>
#include <string>

class QDialog;

namespace subedit::core {
struct SourceFile;
} // namespace subedit::core

namespace subedit::gui {

/// Where to write, and as what.
struct SaveTarget {
    std::filesystem::path path;
    core::SubtitleFormat format = core::SubtitleFormat::SubRip;
};

/// What to do about a document that differs from its file.
enum class UnsavedChoice {
    Save,    ///< write it, then go on
    Discard, ///< go on and lose the changes
    Cancel,  ///< do not go on at all
};

/// The questions the window has to ask a human, and nothing else.
///
/// **A seam, and it exists for one reason: `QDialog::exec` cannot be
/// tested.** It spins a nested event loop until somebody clicks, so a test
/// that reaches it never returns. Behind this interface, the production
/// implementation is one modal call per method and nothing more; a fake in the
/// tests answers what the scenario calls for, which is what makes « the user
/// cancelled » and « the user chose to discard » reachable at all — and those
/// are the paths where the mistakes live.
///
/// It asks; it decides nothing. Reading a file, writing it, and giving up are
/// the window's business, whatever the answer.
class Prompts {

public:
    virtual ~Prompts() = default;

    /// Which file to open, or nothing if the user changed their mind.
    [[nodiscard]] virtual std::optional<std::filesystem::path> fileToOpen() = 0;

    /// Where to save and in which format, or nothing if the user gave up.
    ///
    /// `current` is what the document carries now, so that the question opens
    /// on the file's own directory and format rather than on nowhere.
    [[nodiscard]] virtual std::optional<SaveTarget> saveTarget(const core::SourceFile& current) = 0;

    /// What to do with changes that were never written.
    [[nodiscard]] virtual UnsavedChoice aboutUnsavedChanges() = 0;

    /// Shows `dialog` and says whether it was accepted.
    ///
    /// **One method for every dialog this project writes itself**, and that is
    /// the whole difference with the file and message boxes above. Those
    /// belong to Qt: nothing of them could be reached but the call. Ours are
    /// ordinary widgets — a test builds one, fills its fields and reads what it
    /// makes of them, without ever entering an event loop. What cannot be
    /// tested is `exec()`, and `exec()` alone, so that is all this hides.
    [[nodiscard]] virtual bool run(QDialog& dialog) = 0;

    /// Says that something could not be done, and why.
    ///
    /// The one method that asks nothing. It is here rather than in the window
    /// because it is a modal box like the others, and leaving it out would put
    /// one `exec()` back where no test can go.
    virtual void reportFailure(const std::string& message) = 0;

protected:
    Prompts() = default;
    Prompts(const Prompts&) = default;
    Prompts(Prompts&&) = default;
    Prompts& operator=(const Prompts&) = default;
    Prompts& operator=(Prompts&&) = default;
};

} // namespace subedit::gui
