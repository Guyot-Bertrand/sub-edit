// The editing delegates — issue #129.
//
// They are tested outside any view, by handing them the event: what we want to
// know is what a delegate makes of a key, and a view would add nothing to that
// question but the luck of a focus. Whether the table puts them on the right
// columns is another question, and it is in `main_window_test.cpp`.

#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/cell_delegates.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QCoreApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSignalSpy>
#include <QStyleOptionViewItem>
#include <QValidator>
#include <QWidget>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <memory>

namespace {

using subedit::core::Project;
using subedit::core::Session;
using subedit::core::Subtitle;
using subedit::core::Timestamp;
using subedit::gui::PositionDelegate;
using subedit::gui::SubtitleTableModel;
using subedit::gui::TextDelegate;

[[nodiscard]] Subtitle at(std::int64_t start, std::int64_t end, const char* text) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end),
                    .mainText = text};
}

[[nodiscard]] Project oneSubtitle() {
    Project project;
    project.setSubtitles({at(1000, 2500, "Un.")});
    return project;
}

/// The keystroke a delegate receives, assembled by hand.
[[nodiscard]] QKeyEvent pressing(int key, Qt::KeyboardModifiers modifiers = Qt::NoModifier) {
    return QKeyEvent{QEvent::KeyPress, key, modifiers};
}

} // namespace

TEST_CASE("the text delegate edits in a multiline field", "[gui][GUI-EDIT-01]") {
    Session session{oneSubtitle()};
    const SubtitleTableModel model{session};
    const TextDelegate delegate;
    QWidget parent;

    const std::unique_ptr<QWidget> editor{
        delegate.createEditor(&parent, QStyleOptionViewItem{}, model.index(0, 4))};
    delegate.setEditorData(editor.get(), model.index(0, 4));

    auto* field = qobject_cast<QPlainTextEdit*>(editor.get());
    REQUIRE(field != nullptr);
    CHECK(field->toPlainText().toStdString() == "Un.");
}

TEST_CASE("enter validates what was typed in a text cell", "[gui][GUI-EDIT-01]") {
    Session session{oneSubtitle()};
    SubtitleTableModel model{session};
    TextDelegate delegate;
    QWidget parent;
    const std::unique_ptr<QWidget> editor{
        delegate.createEditor(&parent, QStyleOptionViewItem{}, model.index(0, 4))};
    auto* field = qobject_cast<QPlainTextEdit*>(editor.get());
    REQUIRE(field != nullptr);
    field->setPlainText(QStringLiteral("Autre chose."));

    const QSignalSpy committed{&delegate, &TextDelegate::commitData};
    QKeyEvent enter = pressing(Qt::Key_Return);
    const bool swallowed = delegate.eventFilter(editor.get(), &enter);

    // Swallowed, and that is what counts: let through, it would amount to a
    // line break in the validated text.
    CHECK(swallowed);
    CHECK(committed.count() == 1);

    delegate.setModelData(editor.get(), &model, model.index(0, 4));
    CHECK(model.data(model.index(0, 4), Qt::DisplayRole).toString().toStdString() ==
          "Autre chose.");
}

TEST_CASE("shift and enter make a line break rather than a validation", "[gui][GUI-EDIT-01]") {
    Session session{oneSubtitle()};
    const SubtitleTableModel model{session};
    TextDelegate delegate;
    QWidget parent;
    const std::unique_ptr<QWidget> editor{
        delegate.createEditor(&parent, QStyleOptionViewItem{}, model.index(0, 4))};

    const QSignalSpy committed{&delegate, &TextDelegate::commitData};
    QKeyEvent enter = pressing(Qt::Key_Return, Qt::ShiftModifier);
    const bool swallowed = delegate.eventFilter(editor.get(), &enter);

    // Handed back to the editor, which makes the line break of it.
    CHECK_FALSE(swallowed);
    CHECK(committed.count() == 0);
}

TEST_CASE("escape leaves a text cell as it was", "[gui][GUI-EDIT-01]") {
    Session session{oneSubtitle()};
    const SubtitleTableModel model{session};
    TextDelegate delegate;
    QWidget parent;
    const std::unique_ptr<QWidget> editor{
        delegate.createEditor(&parent, QStyleOptionViewItem{}, model.index(0, 4))};
    qobject_cast<QPlainTextEdit*>(editor.get())->setPlainText(QStringLiteral("jamais validé"));

    const QSignalSpy committed{&delegate, &TextDelegate::commitData};
    const QSignalSpy closed{&delegate, &TextDelegate::closeEditor};
    QKeyEvent escape = pressing(Qt::Key_Escape);
    delegate.eventFilter(editor.get(), &escape);
    QCoreApplication::processEvents();

    CHECK(committed.count() == 0);
    CHECK(closed.count() == 1);
    CHECK(model.data(model.index(0, 4), Qt::DisplayRole).toString().toStdString() == "Un.");
}

TEST_CASE("the position delegate opens on what the cell shows", "[gui][GUI-EDIT-02]") {
    Session session{oneSubtitle()};
    const SubtitleTableModel model{session};
    const PositionDelegate delegate;
    QWidget parent;

    const std::unique_ptr<QWidget> editor{
        delegate.createEditor(&parent, QStyleOptionViewItem{}, model.index(0, 1))};
    delegate.setEditorData(editor.get(), model.index(0, 1));

    auto* field = qobject_cast<QLineEdit*>(editor.get());
    REQUIRE(field != nullptr);
    CHECK(field->text().toStdString() == "00:00:01,000");
}

TEST_CASE("the position field refuses what could never be a position", "[gui][GUI-EDIT-02]") {
    // The constraint is the one of the shape, and it stops there: the bounds of
    // the fields belong to `Timestamp::parse`, which refuses seventy minutes
    // without the shape having anything to say about it.
    Session session{oneSubtitle()};
    const SubtitleTableModel model{session};
    const PositionDelegate delegate;
    QWidget parent;
    const std::unique_ptr<QWidget> editor{
        delegate.createEditor(&parent, QStyleOptionViewItem{}, model.index(0, 1))};

    auto* field = qobject_cast<QLineEdit*>(editor.get());
    REQUIRE(field != nullptr);
    REQUIRE(field->validator() != nullptr);

    const auto judge = [&field](const char* typed) {
        QString text = QString::fromUtf8(typed);
        int position = 0;
        return field->validator()->validate(text, position);
    };

    CHECK(judge("00:00:01,000") == QValidator::Acceptable);
    CHECK(judge("1:02.5") == QValidator::Acceptable);
    CHECK(judge("-0:01,000") == QValidator::Acceptable);
    CHECK(judge("bientôt") == QValidator::Invalid);
}
