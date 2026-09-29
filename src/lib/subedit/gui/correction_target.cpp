#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/gui/correction_target.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/target.hpp>

namespace subedit::gui {

namespace {

[[nodiscard]] bool carries(const core::Project& project, core::Document document) {
    return document == core::Document::Main || project.translationFile().has_value();
}

} // namespace

std::vector<core::CorrectionTarget>
correctionTargetsOf(CorrectionScope scope,
                    core::Document document,
                    std::span<const std::unique_ptr<ProjectPage>> pages,
                    std::size_t currentPage) {
    std::vector<core::CorrectionTarget> targets;

    if (scope == CorrectionScope::AllProjects) {
        for (const std::unique_ptr<ProjectPage>& page : pages) {
            const core::Project& project = page->session->project();
            if (!carries(project, document))
                continue;
            targets.push_back(core::CorrectionTarget{.project = &project,
                                                     .selection = core::Selection::all(project),
                                                     .document = document});
        }
        return targets;
    }

    const ProjectPage& page = *pages[currentPage];
    const core::Project& project = page.session->project();
    if (!carries(project, document))
        return targets;

    const core::Selection selection = scope == CorrectionScope::Selection
                                          ? selectionOf(*page.tableSelection)
                                          : core::Selection::all(project);
    if (selection.count() == 0)
        return targets;

    targets.push_back(
        core::CorrectionTarget{.project = &project, .selection = selection, .document = document});
    return targets;
}

} // namespace subedit::gui
