#pragma once

#include <subedit/core/config/search_options.hpp>
#include <subedit/core/model/selection.hpp>

#include <QMetaObject>

#include <memory>
#include <optional>
#include <vector>

class QWidget;

namespace subedit::core {
class Command;
class SearchPattern;
enum class Document;
} // namespace subedit::core

namespace subedit::gui {

struct ProjectPage;
class SearchDialog;

/// `Find and Replace…`: the dialog, the target, finding and replacing in one
/// project or in all of them — ADR 0034, issue #460.
///
/// **It holds the dialog and the two options**, and nothing of a project: the
/// match found and the target captured are the page's, and it reaches the
/// pages through its `View`.
class ProjectSearch final {

public:
    /// What the search asks of the window: the projects, which one is shown,
    /// and the gestures that belong to the screen.
    class View {

    public:
        virtual ~View() = default;

        /// The open projects, in the order of their tabs.
        [[nodiscard]] virtual int projectCount() const = 0;

        [[nodiscard]] virtual ProjectPage& project(int index) = 0;

        /// The index of the project on screen.
        [[nodiscard]] virtual int shownProject() const = 0;

        /// Brings the tab of project `index` forward.
        virtual void show(int index) = 0;

        /// The text the current cell aims at.
        [[nodiscard]] virtual core::Document targetDocument() const = 0;

        /// Whether two texts are on screen — the search box names the one it
        /// looks in only then.
        [[nodiscard]] virtual bool twoTexts() const = 0;

        /// What the selection of the project on screen makes the target.
        [[nodiscard]] virtual core::Selection selectionTarget() const = 0;

        /// Selects `row` of the project on screen, as a move to a match and not
        /// as a new target.
        virtual void moveTo(int row) = 0;

        /// Applies `command` to the project on screen, one entry of its history.
        virtual void apply(std::unique_ptr<core::Command> command,
                           const core::Selection& target) = 0;

    protected:
        View() = default;
        View(const View&) = default;
        View(View&&) = default;
        View& operator=(const View&) = default;
        View& operator=(View&&) = default;
    };

    /// `view` and `parent` must outlive this; the dialog, once made, belongs to
    /// `parent`.
    ProjectSearch(View& view, QWidget* parent);

    /// Cuts what the dialog was told to call: the dialog belongs to `parent`
    /// and may outlive this.
    ~ProjectSearch();

    ProjectSearch(const ProjectSearch&) = delete;
    ProjectSearch& operator=(const ProjectSearch&) = delete;
    ProjectSearch(ProjectSearch&&) = delete;
    ProjectSearch& operator=(ProjectSearch&&) = delete;

    /// Opens the dialog, or brings it back to the front.
    ///
    /// **The same dialog every time**, kept from one opening to the next with
    /// what was typed in it: finding again is the common case.
    void open();

    /// The dialog once it has been opened, and nothing before.
    [[nodiscard]] SearchDialog* dialog() const { return m_dialog; }

    [[nodiscard]] core::SearchOptions options() const { return m_options; }

    /// Takes the two options a session left, and shows them if the dialog is
    /// open.
    void setOptions(core::SearchOptions options);

    /// Says in the dialog which text it looks in, and whether « all open
    /// projects » can be ticked; forgets the match of the project on screen if
    /// the text it aims at has changed.
    ///
    /// **A match is a place in one text.** The translation of a subtitle may
    /// read exactly what the main text does, so a match kept across a change of
    /// column would be replaced there without the user having seen it found.
    void refresh();

    /// Finds the next or the previous match, and moves the table to it.
    void find(bool forward);

    /// Replaces the match last found, then finds the next one.
    void replace();

    /// Replaces every match of the target — or of every project.
    void replaceAll();

private:
    [[nodiscard]] std::optional<core::SearchPattern> pattern();

    /// The target of the search under way in the project on screen, captured
    /// at its first gesture and kept until the user selects otherwise.
    [[nodiscard]] core::Selection target();

    /// The search of `find` when « All open projects » is ticked: the project
    /// on screen first, then the others in the order of their tabs —
    /// `GUI-SEARCH-04`.
    void findAcrossProjects(bool forward, const core::SearchPattern& pattern);

    /// `Replace All` over every open project, one entry of history in each one
    /// it touches.
    void replaceAllAcrossProjects(const core::SearchPattern& pattern);

    [[nodiscard]] ProjectPage& shown();

    View* m_view;
    QWidget* m_parent;
    SearchDialog* m_dialog = nullptr;
    std::vector<QMetaObject::Connection> m_connections;
    core::SearchOptions m_options;
};

} // namespace subedit::gui
