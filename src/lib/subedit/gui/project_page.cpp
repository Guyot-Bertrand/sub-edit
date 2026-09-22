#include <subedit/core/edit/session.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

namespace subedit::gui {

// Out of line, where `Session` and `SubtitleTableModel` are complete types:
// the default constructor and destructor `unique_ptr` needs are trivial, but
// a header that only forward-declares the two cannot instantiate them itself.
ProjectPage::ProjectPage() = default;
ProjectPage::~ProjectPage() = default;

} // namespace subedit::gui
