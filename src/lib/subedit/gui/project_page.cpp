#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <memory>
#include <span>
#include <utility>

namespace subedit::gui {

// Out of line, where `Session` and `SubtitleTableModel` are complete types:
// the default constructor and destructor `unique_ptr` needs are trivial, but
// a header that only forward-declares the two cannot instantiate them itself.
ProjectPage::ProjectPage() = default;
ProjectPage::~ProjectPage() = default;

std::unique_ptr<ProjectPage> ProjectPage::make(core::Project project,
                                               std::span<const core::Diagnostic> diagnostics) {
    // A session rebuilt rather than reset: a history carries what was done to
    // one file, and it has nothing to say about the next. Everything else starts
    // at its own default — no match, no target, no video, nothing placed.
    auto page = std::make_unique<ProjectPage>();
    page->session = std::make_unique<core::Session>(std::move(project));
    page->model = std::make_unique<SubtitleTableModel>(*page->session);
    page->tableSelection = std::make_unique<QItemSelectionModel>(page->model.get());
    page->diagnostics.assign(diagnostics.begin(), diagnostics.end());
    return page;
}

bool isBlank(const ProjectPage& page) {
    const core::Session& session = *page.session;
    const core::Project& project = session.project();
    return !project.sourceFile().path.has_value() && !project.translationFile().has_value() &&
           project.count() == 0 && !session.canUndo() && !session.canRedo() &&
           !project.video().has_value() && !session.hasUnsavedChanges(core::Document::Main) &&
           !session.hasUnsavedChanges(core::Document::Translation);
}

} // namespace subedit::gui
