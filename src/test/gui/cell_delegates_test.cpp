// The editing delegates — issue #129.
//
// They are tested outside any view, by handing them the event: what we want to
// know is what a delegate makes of a key, and a view would add nothing to that
// question but the luck of a focus. Whether the table puts them on the right
// columns is another question, and it is in `main_window_test.cpp`.

#include <subedit/core/edit/session.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/cell_delegates.hpp>
#include <subedit/gui/line_length_display.hpp>
#include <subedit/gui/subtitle_editor.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QCoreApplication>
#include <QFontMetrics>
#include <QImage>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPainter>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QSignalSpy>
#include <QStyleOptionViewItem>
#include <QTextBlock>
#include <QTextLayout>
#include <QValidator>
#include <QWidget>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace {

using subedit::core::CharacterLineMeasure;
using subedit::core::InMemoryFileSystem;
using subedit::core::MarkupVocabulary;
using subedit::core::openSpellChecker;
using subedit::core::Project;
using subedit::core::Session;
using subedit::core::SpellChecker;
using subedit::core::Subtitle;
using subedit::core::Timestamp;
using subedit::core::WordList;
using subedit::core::WordListSpellProvider;
using subedit::gui::DurationDelegate;
using subedit::gui::LengthMeasures;
using subedit::gui::LineLengthDisplay;
using subedit::gui::PositionDelegate;
using subedit::gui::smallerFont;
using subedit::gui::SubtitleEditor;
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

/// A checker that knows `un` and `bon`, and nothing else.
[[nodiscard]] std::shared_ptr<const SpellChecker> smallChecker() {
    WordList list;
    list.words = {"un", "bon"};
    WordListSpellProvider provider;
    provider.add("fr", std::move(list));
    const InMemoryFileSystem files;
    return std::make_shared<const SpellChecker>(
        openSpellChecker(provider, "fr", files, "/none.repl").value());
}

/// The words a text edit underlines as misspelt, in order.
[[nodiscard]] std::vector<QString> underlinedIn(const QPlainTextEdit& field) {
    std::vector<QString> words;
    for (QTextBlock block = field.document()->firstBlock(); block.isValid(); block = block.next()) {
        for (const QTextLayout::FormatRange& range : block.layout()->formats()) {
            if (range.format.underlineStyle() == QTextCharFormat::SpellCheckUnderline)
                words.push_back(block.text().mid(range.start, range.length));
        }
    }
    return words;
}

/// Every text is as long as 7, whatever it says — so that a number on screen
/// can only have come from this measure.
class SevenMeasure final : public subedit::core::LineMeasure {

public:
    [[nodiscard]] double lengthOf(std::string_view /*text*/) const override { return 7.0; }
};

[[nodiscard]] LineLengthDisplay characters() {
    return {.measure = std::make_shared<const CharacterLineMeasure>(),
            .vocabulary = MarkupVocabulary::Html};
}

[[nodiscard]] LineLengthDisplay sevens() {
    return {.measure = std::make_shared<const SevenMeasure>(),
            .vocabulary = MarkupVocabulary::Html};
}

/// A project with one subtitle of this text.
[[nodiscard]] Project projectOf(const char* text) {
    Project project;
    project.setSubtitles({at(1000, 2500, text)});
    return project;
}

constexpr int kCellWidth = 300;
constexpr int kCellHeight = 60;

/// What a delegate paints for a cell, on white.
[[nodiscard]] QImage
paintedCell(const TextDelegate& delegate, const QWidget& widget, const QModelIndex& cell) {
    QImage image{kCellWidth, kCellHeight, QImage::Format_RGB32};
    image.fill(Qt::white);
    QStyleOptionViewItem option = viewedFrom(widget);
    option.rect = QRect{0, 0, kCellWidth, kCellHeight};
    QPainter painter{&image};
    delegate.paint(&painter, option, cell);
    painter.end();
    return image;
}

/// The rightmost column that holds ink, or -1.
[[nodiscard]] int inkReach(const QImage& image) {
    for (int x = image.width() - 1; x >= 0; --x) {
        for (int y = 0; y < image.height(); ++y) {
            if (image.pixel(x, y) != qRgb(255, 255, 255))
                return x;
        }
    }
    return -1;
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

TEST_CASE("the duration field refuses a sign at the keyboard", "[gui][GUI-DURATION-01]") {
    // A start may lie before the video; a length may not be negative. The
    // shape is otherwise that of a position.
    Session session{oneSubtitle()};
    const SubtitleTableModel model{session};
    const DurationDelegate delegate;
    QWidget parent;
    const std::unique_ptr<QWidget> editor{
        delegate.createEditor(&parent, QStyleOptionViewItem{}, model.index(0, 3))};
    delegate.setEditorData(editor.get(), model.index(0, 3));

    auto* field = qobject_cast<QLineEdit*>(editor.get());
    REQUIRE(field != nullptr);
    REQUIRE(field->validator() != nullptr);

    const auto judge = [&field](const char* typed) {
        QString text = QString::fromUtf8(typed);
        int position = 0;
        return field->validator()->validate(text, position);
    };

    CHECK(field->hasAcceptableInput());
    CHECK(judge("00:00:02,500") == QValidator::Acceptable);
    CHECK(judge("0:02.5") == QValidator::Acceptable);
    CHECK(judge("-0:01,000") == QValidator::Invalid);
    CHECK(judge("longtemps") == QValidator::Invalid);
}

TEST_CASE("GUI-SPELL-04: an unknown word is underlined in the editor, a known one is not",
          "[gui][GUI-SPELL-04]") {
    Session session{oneSubtitle()};
    const SubtitleTableModel model{session};
    TextDelegate delegate;
    delegate.setSpellCheckerSource([] { return smallChecker(); });
    QWidget parent;
    const std::unique_ptr<QWidget> editor{
        delegate.createEditor(&parent, QStyleOptionViewItem{}, model.index(0, 4))};
    auto* field = qobject_cast<QPlainTextEdit*>(editor.get());
    REQUIRE(field != nullptr);

    field->setPlainText(QStringLiteral("un bon xyzzy"));
    QCoreApplication::processEvents();

    CHECK(underlinedIn(*field) == std::vector<QString>{QStringLiteral("xyzzy")});
}

TEST_CASE("GUI-SPELL-04: the underline counts UTF-16 units, not bytes", "[gui][GUI-SPELL-04]") {
    Session session{oneSubtitle()};
    const SubtitleTableModel model{session};
    TextDelegate delegate;
    delegate.setSpellCheckerSource([] { return smallChecker(); });
    QWidget parent;
    const std::unique_ptr<QWidget> editor{
        delegate.createEditor(&parent, QStyleOptionViewItem{}, model.index(0, 4))};
    auto* field = qobject_cast<QPlainTextEdit*>(editor.get());
    REQUIRE(field != nullptr);

    field->setPlainText(QStringLiteral("un été bon\nun bonn"));
    QCoreApplication::processEvents();

    CHECK(underlinedIn(*field) ==
          std::vector<QString>{QStringLiteral("été"), QStringLiteral("bonn")});
}

TEST_CASE("GUI-SPELL-04: the underline follows the typing", "[gui][GUI-SPELL-04]") {
    Session session{oneSubtitle()};
    const SubtitleTableModel model{session};
    TextDelegate delegate;
    delegate.setSpellCheckerSource([] { return smallChecker(); });
    QWidget parent;
    const std::unique_ptr<QWidget> editor{
        delegate.createEditor(&parent, QStyleOptionViewItem{}, model.index(0, 4))};
    auto* field = qobject_cast<QPlainTextEdit*>(editor.get());
    REQUIRE(field != nullptr);

    field->setPlainText(QStringLiteral("un bo"));
    CHECK(underlinedIn(*field) == std::vector<QString>{QStringLiteral("bo")});

    field->moveCursor(QTextCursor::End);
    field->insertPlainText(QStringLiteral("n"));
    CHECK(underlinedIn(*field).empty());

    field->insertPlainText(QStringLiteral("x"));
    CHECK(underlinedIn(*field) == std::vector<QString>{QStringLiteral("bonx")});
}

TEST_CASE("GUI-SPELL-04: without a checker the editor underlines nothing and does not fail",
          "[gui][GUI-SPELL-04]") {
    Session session{oneSubtitle()};
    const SubtitleTableModel model{session};
    QWidget parent;

    SECTION("no source at all") {
        const TextDelegate delegate;
        const std::unique_ptr<QWidget> editor{
            delegate.createEditor(&parent, QStyleOptionViewItem{}, model.index(0, 4))};
        auto* field = qobject_cast<QPlainTextEdit*>(editor.get());
        REQUIRE(field != nullptr);
        field->setPlainText(QStringLiteral("xyzzy"));
        QCoreApplication::processEvents();
        CHECK(underlinedIn(*field).empty());
    }

    SECTION("a source that answers null") {
        TextDelegate delegate;
        delegate.setSpellCheckerSource([] { return std::shared_ptr<const SpellChecker>{}; });
        const std::unique_ptr<QWidget> editor{
            delegate.createEditor(&parent, QStyleOptionViewItem{}, model.index(0, 4))};
        auto* field = qobject_cast<QPlainTextEdit*>(editor.get());
        REQUIRE(field != nullptr);
        field->setPlainText(QStringLiteral("xyzzy"));
        QCoreApplication::processEvents();
        CHECK(underlinedIn(*field).empty());
    }
}

// ## The length of each line — issue #526, GUI-EDIT-04

TEST_CASE("GUI-EDIT-04: a cell shows the length after each line, and only when asked",
          "[gui][GUI-EDIT-04]") {
    Session session{projectOf("Bonjour")};
    const SubtitleTableModel model{session};
    const QWidget widget;
    const QModelIndex cell = model.index(0, 4);

    const TextDelegate bare;
    TextDelegate measured;
    measured.setLengthSources([] { return std::optional{sevens()}; }, {});
    TextDelegate off;
    off.setLengthSources([] { return std::optional<LineLengthDisplay>{}; }, {});

    const QImage without = paintedCell(bare, widget, cell);
    const QImage with = paintedCell(measured, widget, cell);

    // The number sits to the right of the text, where nothing was drawn.
    CHECK(inkReach(with) > inkReach(without));
    // A source that answers nothing paints the cell as no source does.
    CHECK(paintedCell(off, widget, cell) == without);
}

TEST_CASE("GUI-EDIT-04: each line of a cell is drawn on its own row, with its own length",
          "[gui][GUI-EDIT-04]") {
    Session session{projectOf("Un\nDeux")};
    const SubtitleTableModel model{session};
    const QWidget widget;
    const QModelIndex cell = model.index(0, 4);
    TextDelegate measured;
    measured.setLengthSources([] { return std::optional{sevens()}; }, {});

    const QImage image = paintedCell(measured, widget, cell);

    // The lowest ink is more than a line spacing down: the second line has a
    // row of its own.
    int lowest = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixel(x, y) != qRgb(255, 255, 255))
                lowest = y;
        }
    }
    CHECK(lowest > QFontMetrics{widget.font()}.lineSpacing());
}

TEST_CASE("GUI-EDIT-04: the cell is as wide as its line and its length", "[gui][GUI-EDIT-04]") {
    Session session{projectOf("Bonjour")};
    const SubtitleTableModel model{session};
    const QWidget widget;
    const QStyleOptionViewItem option = viewedFrom(widget);
    const QModelIndex cell = model.index(0, 4);

    const TextDelegate bare;
    TextDelegate measured;
    measured.setLengthSources([] { return std::optional{sevens()}; }, {});

    const QSize plain = bare.sizeHint(option, cell);
    const QSize longer = measured.sizeHint(option, cell);

    // Four pixels and "[7]" in the smaller type — and not a pixel of height.
    const QFontMetrics small{smallerFont(option.font)};
    CHECK(longer.width() - plain.width() == 4 + small.horizontalAdvance(QStringLiteral("[7]")));
    CHECK(longer.height() == plain.height());
}

TEST_CASE("GUI-EDIT-04: the length is the one the measure gives, tags not counted",
          "[gui][GUI-EDIT-04]") {
    const QWidget widget;
    const QStyleOptionViewItem option = viewedFrom(widget);
    const QFontMetrics small{smallerFont(option.font)};

    // The width the length adds is the width of its own digits, so it tells
    // which number was written: "[2]" for the text without its tags, "[9]"
    // for the same text read as if a bracket were a letter.
    Session tagged{projectOf("<i>ab</i>")};
    const SubtitleTableModel taggedModel{tagged};
    Session plainText{projectOf("<i>ab</i>")};
    const SubtitleTableModel plainModel{plainText};

    TextDelegate chars;
    chars.setLengthSources([] { return std::optional{characters()}; }, {});
    TextDelegate untagged;
    untagged.setLengthSources(
        [] {
            return std::optional{
                LineLengthDisplay{.measure = std::make_shared<const CharacterLineMeasure>(),
                                  .vocabulary = MarkupVocabulary::None}};
        },
        {});

    const TextDelegate bare;
    const int base = bare.sizeHint(option, taggedModel.index(0, 4)).width();
    // "<i>ab</i>" in HTML is two characters; with no vocabulary, nine.
    CHECK(chars.sizeHint(option, taggedModel.index(0, 4)).width() - base ==
          4 + small.horizontalAdvance(QStringLiteral("[2]")));
    CHECK(untagged.sizeHint(option, plainModel.index(0, 4)).width() - base ==
          4 + small.horizontalAdvance(QStringLiteral("[9]")));
}

TEST_CASE("GUI-EDIT-04: a blank line and an empty cell carry no length", "[gui][GUI-EDIT-04]") {
    Session session{projectOf("")};
    const SubtitleTableModel model{session};
    const QWidget widget;
    const QStyleOptionViewItem option = viewedFrom(widget);
    const QModelIndex cell = model.index(0, 4);

    const TextDelegate bare;
    TextDelegate measured;
    measured.setLengthSources([] { return std::optional{sevens()}; }, {});

    CHECK(measured.sizeHint(option, cell) == bare.sizeHint(option, cell));
    CHECK(paintedCell(measured, widget, cell) == paintedCell(bare, widget, cell));
}

TEST_CASE("GUI-EDIT-04: the two units measure differently, and the cache follows the font",
          "[gui][GUI-EDIT-04]") {
    LengthMeasures measures;
    const QFont font;

    const auto characterMeasure = measures.measureFor(subedit::core::LengthUnit::Characters, font);
    CHECK(characterMeasure->lengthOf("café") == 4.0);
    // The same object as long as nothing changed: this is what the cache is
    // bounded to.
    CHECK(measures.measureFor(subedit::core::LengthUnit::Characters, font) == characterMeasure);

    const auto ems = measures.measureFor(subedit::core::LengthUnit::Ems, font);
    CHECK(ems != characterMeasure);
    CHECK(ems->lengthOf("abcdefghijklmnopqrstuvwxyz") == Catch::Approx(26 * 0.55));

    QFont bigger = font;
    bigger.setPointSizeF(font.pointSizeF() + 6);
    CHECK(measures.measureFor(subedit::core::LengthUnit::Ems, bigger) != ems);
    // A measure handed out earlier is still good.
    CHECK(ems->lengthOf("abcdefghijklmnopqrstuvwxyz") == Catch::Approx(26 * 0.55));
}

TEST_CASE("GUI-EDIT-04: the editor's margin has a length for each line", "[gui][GUI-EDIT-04]") {
    Session session{projectOf("Bonjour\n\nà toi")};
    const SubtitleTableModel model{session};
    TextDelegate delegate;
    delegate.setLengthSources({}, [] { return std::optional{characters()}; });
    QWidget parent;
    parent.resize(kWideEnough, kTallEnough);
    parent.show();
    const QModelIndex cell = model.index(0, 4);

    const std::unique_ptr<QWidget> editor{delegate.createEditor(&parent, viewedFrom(parent), cell)};
    delegate.setEditorData(editor.get(), cell);
    auto* field = dynamic_cast<SubtitleEditor*>(editor.get());
    REQUIRE(field != nullptr);

    // One per line, the empty one included, accents counted once.
    CHECK(field->gutterLengths() == std::vector<int>{7, 0, 5});
    CHECK(field->gutterWidth() > 0);
}

TEST_CASE("GUI-EDIT-04: the margin follows the typing and is as wide as its largest number",
          "[gui][GUI-EDIT-04]") {
    Session session{projectOf("a")};
    const SubtitleTableModel model{session};
    TextDelegate delegate;
    delegate.setLengthSources({}, [] { return std::optional{characters()}; });
    QWidget parent;
    const QModelIndex cell = model.index(0, 4);
    const std::unique_ptr<QWidget> editor{delegate.createEditor(&parent, viewedFrom(parent), cell)};
    auto* field = dynamic_cast<SubtitleEditor*>(editor.get());
    REQUIRE(field != nullptr);

    field->setPlainText(QStringLiteral("abc"));
    CHECK(field->gutterLengths() == std::vector<int>{3});
    const int oneDigit = field->gutterWidth();

    field->setPlainText(QStringLiteral("abc\n<i>défghijklm</i>"));
    CHECK(field->gutterLengths() == std::vector<int>{3, 10});
    CHECK(field->gutterWidth() > oneDigit);

    // An empty text draws nothing, as Gaupol's does.
    field->setPlainText(QString{});
    CHECK(field->gutterLengths().empty());
}

TEST_CASE("GUI-EDIT-04: an editor with no source has no margin", "[gui][GUI-EDIT-04]") {
    Session session{projectOf("abc")};
    const SubtitleTableModel model{session};
    QWidget parent;
    const QModelIndex cell = model.index(0, 4);

    SECTION("no source") {
        const TextDelegate delegate;
        const std::unique_ptr<QWidget> editor{
            delegate.createEditor(&parent, viewedFrom(parent), cell)};
        delegate.setEditorData(editor.get(), cell);
        auto* field = dynamic_cast<SubtitleEditor*>(editor.get());
        REQUIRE(field != nullptr);
        CHECK(field->gutterWidth() == 0);
        CHECK(field->gutterLengths().empty());
    }

    SECTION("a source that answers nothing") {
        TextDelegate delegate;
        delegate.setLengthSources({}, [] { return std::optional<LineLengthDisplay>{}; });
        const std::unique_ptr<QWidget> editor{
            delegate.createEditor(&parent, viewedFrom(parent), cell)};
        delegate.setEditorData(editor.get(), cell);
        auto* field = dynamic_cast<SubtitleEditor*>(editor.get());
        REQUIRE(field != nullptr);
        CHECK(field->gutterWidth() == 0);
    }
}
