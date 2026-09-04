#pragma once

#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <filesystem>
#include <optional>
#include <string>

class QDialog;
class QWidget;

namespace subedit::gui {

/// Where to write, and in what shape.
///
/// **The four answers `Save As…` carries since phase 8.** The format was alone
/// until then, and the window could say nothing of the three the command line
/// had settled in phase 3 — an encoding, line endings, a mark. Giving it the
/// first without the other two would have left it half-way.
struct SaveTarget {
    std::filesystem::path path;
    core::SubtitleFormat format = core::SubtitleFormat::SubRip;

    /// The encoding to write in, its mark included.
    core::Encoding encoding = core::Encoding::utf8(core::ByteOrderMark::Absent);

    core::Newline newline = core::Newline::Lf;
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
    ///
    /// `directory` is where the question opens — le répertoire du dernier
    /// fichier ouvert ou enregistré, vide au premier lancement. Sans lui, la
    /// boîte s'ouvre là où le processus a été lancé, c'est-à-dire nulle part
    /// d'utile quand il vient d'un menu de bureau. Le chercheur de film reçoit
    /// le sien depuis toujours ; celui-ci l'a gagné avec #254.
    [[nodiscard]] virtual std::optional<std::filesystem::path>
    fileToOpen(const std::filesystem::path& directory) = 0;

    /// Which film to watch the document against, or nothing if the user
    /// changed their mind.
    ///
    /// `directory` is where the question opens — the directory of the subtitle
    /// file, since that is where the film almost always is, and it is the
    /// directory the naming convention has just looked through.
    [[nodiscard]] virtual std::optional<std::filesystem::path>
    videoToOpen(const std::filesystem::path& directory) = 0;

    /// Where to save and in what shape, or nothing if the user gave up.
    ///
    /// `current` is what the document carries now, so that the question opens
    /// on the file's own directory, format and line endings rather than on
    /// nowhere. `encoding` is asked for apart because it is the one answer the
    /// window may propose from elsewhere: a document that came from no file
    /// has none of its own, and the settings then remember what was written
    /// last.
    [[nodiscard]] virtual std::optional<SaveTarget> saveTarget(const core::SourceFile& current,
                                                               const core::Encoding& encoding) = 0;

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

    /// Says what an operation did, when the table cannot show it.
    ///
    /// Distinct from `reportFailure`, and not out of taste: one is a warning
    /// and the other a notice, they carry different icons, and a user who
    /// learns « 1 subtitle cleaned, 1 removed » has not been warned of
    /// anything.
    virtual void reportOutcome(const std::string& message) = 0;

    /// Says which widget the boxes are to sit over.
    ///
    /// **Told by the window, at its own construction**, and not given here at
    /// construction: a `Prompts` has to exist before the window that takes it,
    /// so the owner cannot be known then. Passing it later from `main` would
    /// work too, and was the shape that let it be forgotten — `subedit-gui`
    /// built its prompts on nothing, and no box ever sat over the window.
    ///
    /// Does nothing by default: a fake has no widget to sit over, and only
    /// `QtPrompts` has any use for the answer.
    virtual void ownedBy(QWidget* /*window*/) {}

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
