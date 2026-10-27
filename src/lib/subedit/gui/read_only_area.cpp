#include <subedit/gui/read_only_area.hpp>

#include <QFrame>
#include <QPalette>
#include <QPlainTextEdit>

namespace subedit::gui {

void styleAsReadOnly(QPlainTextEdit& area) {
    area.setReadOnly(true);
    area.setFocusPolicy(Qt::ClickFocus);
    area.setFrameShape(QFrame::StyledPanel);
    area.setFrameShadow(QFrame::Sunken);
    area.viewport()->setBackgroundRole(QPalette::AlternateBase);
    area.viewport()->setAutoFillBackground(true);
}

} // namespace subedit::gui
