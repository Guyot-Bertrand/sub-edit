#pragma once

#include <subedit/core/model/document.hpp>
#include <subedit/core/text/correction_run.hpp>

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace subedit::gui {

struct ProjectPage;

/// The three targets D8 names for the correction assistant's first page.
enum class CorrectionScope {
    Selection,      ///< the rows selected in the page on screen
    CurrentProject, ///< the whole file of the page on screen
    AllProjects,    ///< every open project, whole
};

/// Resolves `scope` and `document` into the concrete `CorrectionTarget`s
/// `proposeCorrections` takes — the window's job, never `proposeCorrections`'
/// own (issue #504's own doc comment on `CorrectionTarget`).
///
/// A project with no translation of its own is left out when `document` is
/// `Translation`: proposing corrections to an empty column that only follows
/// the main text would report every line of it as "changed" from nothing.
[[nodiscard]] std::vector<core::CorrectionTarget>
correctionTargetsOf(CorrectionScope scope,
                    core::Document document,
                    std::span<const std::unique_ptr<ProjectPage>> pages,
                    std::size_t currentPage);

} // namespace subedit::gui
