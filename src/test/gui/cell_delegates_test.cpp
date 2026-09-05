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
#include <QScrollBar>
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

[[nodiscard]] Project threeSubtitles() {
    Project project;
    project.setSubtitles({at(1000, 2500, "Une seule ligne."),
                          at(3000, 4500, "Premiere ligne.\nSeconde ligne."),
                          at(5000, 6500, "Une.\nDeux.\nTrois.")});
    return project;
}

/// The option a view hands a delegate, with a widget behind it.
///
/// **The widget is what carries the style**, and the room a `QPlainTextEdit`
/// keeps around its lines is read from it. An option without one would measure
/// the frame at zero and prove nothing about the editor.
[[nodiscard]] QStyleOptionViewItem viewedFrom(const QWidget& widget) {
    QStyleOptionViewItem option;
    option.initFrom(&widget);
    option.widget = &widget;
    return option;
}

/// Wide enough that no line of these fixtures could ever wrap.
constexpr int kWideEnough = 600;

/// Tall enough that the parent never squeezes an editor it holds.
constexpr int kTallEnough = 400;

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

TEST_CASE("a row is as tall as the lines its subtitle carries", "[gui][GUI-EDIT-01]") {
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};
    const TextDelegate delegate;
    const QWidget widget;
    const QStyleOptionViewItem option = viewedFrom(widget);

    const int one = delegate.sizeHint(option, model.index(0, 4)).height();
    const int two = delegate.sizeHint(option, model.index(1, 4)).height();
    const int three = delegate.sizeHint(option, model.index(2, 4)).height();

    // **The steps are equal, and that is the claim.** A height that merely grew
    // would be satisfied by anything; what says a line is being counted rather
    // than guessed at is that the second step matches the first, and that both
    // measure one line spacing of the font.
    CHECK(two > one);
    CHECK(three - two == two - one);
    CHECK(two - one == option.fontMetrics.lineSpacing());
}

TEST_CASE("the editor of a multiline subtitle has no scrollbar", "[gui][GUI-EDIT-01]") {
    // **The measurement that decided the margin** — issue #322. An editor given
    // exactly what the style measures for its text keeps a vertical scrollbar
    // with two arrows and almost no travel, and the line the height was meant
    // to show is the line it hides. Nothing but opening one settles it, which
    // is why this test exists rather than a comment.
    //
    // **The parent is shown, and that is not a detail.** A `QPlainTextEdit`
    // that was never shown keeps its viewport at the size it was born with,
    // whatever it is resized to — so the same test on a hidden widget answers
    // about the birth and not about the height.
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};
    const TextDelegate delegate;
    QWidget parent;
    parent.resize(kWideEnough, kTallEnough);
    parent.show();
    QCoreApplication::processEvents();
    const QStyleOptionViewItem option = viewedFrom(parent);

    for (const int row : {0, 1, 2}) {
        INFO("ligne : " << row);
        const QModelIndex cell = model.index(row, 4);
        const std::unique_ptr<QWidget> editor{delegate.createEditor(&parent, option, cell)};
        delegate.setEditorData(editor.get(), cell);

        auto* field = qobject_cast<QPlainTextEdit*>(editor.get());
        REQUIRE(field != nullptr);
        field->show();
        field->resize(kWideEnough, delegate.sizeHint(option, cell).height());
        QCoreApplication::processEvents();

        // A scrollbar with nowhere to go is one Qt does not show; the range is
        // what says so, and it says it whether the bar is painted or not.
        CHECK(field->verticalScrollBar()->maximum() == 0);
    }
}

TEST_CASE("the editor asks for the height of its text", "[gui][GUI-EDIT-01]") {
    // **Found by a screenshot, and it is the only place a persistent editor is
    // used.** A table that sizes a row to its contents asks a persistent editor
    // how tall it wants to be, and a plain `QPlainTextEdit` answers with a
    // default of several lines whatever it holds — a two-line subtitle opened a
    // cell two hundred pixels tall. What it must answer is what its row already
    // measures.
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};
    const TextDelegate delegate;
    QWidget parent;
    parent.resize(kWideEnough, kTallEnough);
    parent.show();
    QCoreApplication::processEvents();
    const QStyleOptionViewItem option = viewedFrom(parent);

    for (const int row : {0, 1, 2}) {
        INFO("ligne : " << row);
        const QModelIndex cell = model.index(row, 4);
        const std::unique_ptr<QWidget> editor{delegate.createEditor(&parent, option, cell)};
        delegate.setEditorData(editor.get(), cell);
        QCoreApplication::processEvents();

        CHECK(editor->sizeHint().height() == delegate.sizeHint(option, cell).height());
    }
}

TEST_CASE("the editor grows with a line typed into it", "[gui][GUI-EDIT-01]") {
    // The one case the height of a row cannot cover: the row is two lines tall,
    // the editor with it, and a `Shift+Enter` makes a third. The row catches up
    // when the edit is validated; until then this is what keeps the new line
    // in view.
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};
    const TextDelegate delegate;
    QWidget parent;
    parent.resize(kWideEnough, kTallEnough);
    parent.show();
    QCoreApplication::processEvents();
    const QStyleOptionViewItem option = viewedFrom(parent);

    const QModelIndex cell = model.index(1, 4);
    const std::unique_ptr<QWidget> editor{delegate.createEditor(&parent, option, cell)};
    delegate.setEditorData(editor.get(), cell);
    auto* field = qobject_cast<QPlainTextEdit*>(editor.get());
    REQUIRE(field != nullptr);
    field->show();
    field->resize(kWideEnough, delegate.sizeHint(option, cell).height());
    QCoreApplication::processEvents();
    const int twoLines = field->height();

    field->setPlainText(QStringLiteral("Premiere ligne.\nSeconde ligne.\nTroisieme ligne."));
    QCoreApplication::processEvents();

    CHECK(field->height() > twoLines);
    CHECK(field->verticalScrollBar()->maximum() == 0);
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
