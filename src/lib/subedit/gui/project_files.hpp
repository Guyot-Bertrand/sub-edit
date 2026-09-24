#pragma once

#include <subedit/core/edit/translation.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/translation_file.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

class QWidget;

namespace subedit::core {
class FileSystem;
enum class Document;
} // namespace subedit::core

namespace subedit::gui {

class Prompts;
struct ModifiedDocument;
struct ProjectPage;

/// Opening, saving and the question of unsaved changes — ADR 0034, issue #460.
///
/// **It holds the directory of the last file opened or saved**, and nothing of
/// a project: every gesture receives the page it is about. The tabs stay the
/// window's — a file already open, a tab of its own, a film given to a tab —
/// and this reaches them through its `View` only where a question has to be
/// asked over the project it concerns.
class ProjectFiles final {

public:
    /// What the files ask of the window.
    class View {

    public:
        virtual ~View() = default;

        /// The open projects, in the order of their tabs.
        [[nodiscard]] virtual int projectCount() const = 0;

        [[nodiscard]] virtual ProjectPage& project(int index) = 0;

        /// The index of the project on screen.
        [[nodiscard]] virtual int shownProject() const = 0;

        /// Brings the tab of project `index` forward, so that a question about
        /// one of its documents opens over it — and only then: a document
        /// with a file is written behind its tab (#461).
        virtual void show(int index) = 0;

        /// `document` of `page` has just been written; `moved` when it was
        /// written somewhere new, which renames it.
        virtual void saved(ProjectPage& page, core::Document document, bool moved) = 0;

        /// Says in the status bar what a gesture did, when nothing is wrong.
        virtual void announce(const std::string& message) = 0;

        /// What the boxes this opens sit over.
        [[nodiscard]] virtual QWidget* dialogParent() = 0;

    protected:
        View() = default;
        View(const View&) = default;
        View(View&&) = default;
        View& operator=(const View&) = default;
        View& operator=(View&&) = default;
    };

    /// A translation read and the way to align it, both chosen by the user.
    struct ChosenTranslation {
        core::TranslationFile read;
        core::TranslationMethod method;
    };

    /// What a drop holds, sorted: subtitle files in the order they came, and
    /// films.
    struct Dropped {
        std::vector<std::filesystem::path> subtitles;
        std::vector<std::filesystem::path> films;
    };

    /// All three must outlive this.
    ProjectFiles(core::FileSystem& files, Prompts& prompts, View& view);

    /// Writes `document`, asking where if it has never been anywhere.
    [[nodiscard]] bool save(ProjectPage& page, core::Document document);

    /// Asks where and in what shape to write `document`, then writes it.
    [[nodiscard]] bool saveAs(ProjectPage& page, core::Document document);

    /// Whether `document` differs from its file. **The translation only counts
    /// while the project has one**: a translation that was undone away has no
    /// file to differ from.
    [[nodiscard]] static bool isModified(const ProjectPage& page, core::Document document);

    /// The documents of `page` a closing would lose, in the order the window
    /// shows them. **A file gone from the disk counts as modified**, and is
    /// marked as such: what the window holds is then the only copy of it.
    [[nodiscard]] std::vector<ModifiedDocument> modifiedDocuments(const ProjectPage& page) const;

    /// Whether the documents of project `index` may be lost — asked, and saved
    /// if the user says so.
    ///
    /// **One question however many documents are modified.** Nothing modified
    /// goes on; one asks what it always asked; two ask through the list, with a
    /// box each.
    [[nodiscard]] bool mayDiscard(int index);

    /// The same question for every open project at once.
    [[nodiscard]] bool mayDiscardAll();

    /// `Projects ▸ Save All`: every modified document, tab by tab, until one is
    /// given up. **Only a document with no file brings its tab forward**, for
    /// the box that asks its name; the tab shown at the start is shown again
    /// at the end.
    void saveAll();

    /// Asks which translation to open for `page` and how to align it, having
    /// first asked about the one it replaces; reads it. Nothing when the user
    /// gave up or the file will not open — which is then said.
    [[nodiscard]] std::optional<ChosenTranslation> chooseTranslation(ProjectPage& page);

    /// Asks which file to open, from the remembered directory.
    [[nodiscard]] std::optional<std::filesystem::path> askFileToOpen();

    /// Reads `path`, and remembers its directory if it opens; says why it will
    /// not otherwise, in the words `Open…` uses.
    [[nodiscard]] std::expected<core::OpenedFile, std::string>
    read(const std::filesystem::path& path);

    /// The project that already holds `path`, however the path is spelled.
    [[nodiscard]] std::optional<int> indexOf(const std::filesystem::path& path);

    /// Sorts what was dropped, by the name alone.
    [[nodiscard]] static Dropped sort(std::span<const std::filesystem::path> paths);

    [[nodiscard]] const std::filesystem::path& lastDirectory() const { return m_lastDirectory; }

    void setLastDirectory(std::filesystem::path directory);

private:
    /// The question itself, over `modified` — `owners[i]` being the project
    /// that `modified[i]` belongs to.
    [[nodiscard]] bool mayDiscard(const std::vector<ModifiedDocument>& modified,
                                  const std::vector<int>& owners);

    /// Whether the translation of `page` may be replaced — asked before the
    /// file is, as for the main document.
    [[nodiscard]] bool mayReplaceTranslation(ProjectPage& page);

    /// `save` for `document` of project `index`, the tab brought forward only
    /// when a name has to be asked — the box then says which project it is.
    /// Whether it did so goes in `shown`.
    [[nodiscard]] bool saveIn(int index, core::Document document, bool& shown);

    void rememberDirectoryOf(const std::filesystem::path& file);

    core::FileSystem* m_files;
    Prompts* m_prompts;
    View* m_view;

    /// Where the "open" box opens: the directory of the last file opened or
    /// saved, empty at the first launch.
    std::filesystem::path m_lastDirectory;
};

} // namespace subedit::gui
