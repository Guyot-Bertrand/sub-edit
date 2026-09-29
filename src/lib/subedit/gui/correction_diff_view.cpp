#include <subedit/gui/correction_diff_view.hpp>

namespace subedit::gui {

QString correctionDiffHtml(const std::vector<core::DiffSpan>& spans) {
    QString html;
    for (const core::DiffSpan& span : spans) {
        const QString escaped = QString::fromStdString(span.text).toHtmlEscaped();
        html += span.changed ? "<b>" + escaped + "</b>" : escaped;
    }
    return html;
}

} // namespace subedit::gui
