// What `Save As…` adds under the file listing — issue #299.
//
// A widget of ours, so driven without a click and without an event loop: what
// a test cannot reach is `exec()`, and `exec()` alone.

#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/gui/save_shape.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLineEdit>
#include <QVBoxLayout>
#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace {

using subedit::core::ByteOrderMark;
using subedit::core::Encoding;
using subedit::core::EncodingRefusal;
using subedit::core::Newline;
using subedit::core::SubtitleFormat;
using subedit::gui::addSaveShapeTo;
using subedit::gui::kOfferedEncodings;
using subedit::gui::saveDialogFor;
using subedit::gui::SaveShape;
using subedit::gui::SaveTarget;
using subedit::gui::targetOf;

/// The encoding of that name, or a failed test.
[[nodiscard]] Encoding named(const char* name) {
    const std::expected<Encoding, EncodingRefusal> encoding =
        Encoding::create(name, ByteOrderMark::Absent);
    if (!encoding.has_value()) {
        FAIL("ICU ne connaît pas cet encodage");
        return Encoding::utf8(ByteOrderMark::Absent);
    }
    return *encoding;
}

} // namespace

TEST_CASE("the shape opens on what the file carries", "[gui][GUI-ENC-02]") {
    // The defaults are the file's own: writing it back otherwise without being
    // asked would lose what the reading kept.
    const SaveShape shape{named("windows-1252"), Newline::CrLf};

    CHECK(shape.encoding() == named("windows-1252"));
    CHECK(shape.newline() == Newline::CrLf);
    CHECK_FALSE(shape.wantsByteOrderMark());
}

TEST_CASE("a mark the file carried is proposed again", "[gui][GUI-ENC-02]") {
    const SaveShape shape{Encoding::utf8(ByteOrderMark::Present), Newline::Lf};

    CHECK(shape.wantsByteOrderMark());
    CHECK(shape.encoding() == Encoding::utf8(ByteOrderMark::Present));
}

TEST_CASE("every encoding offered is one ICU can write", "[gui][GUI-ENC-02]") {
    // The list is short and written here — D2 of the scoping — but it is not a
    // second truth: every one of its entries has to exist in ICU, or the menu
    // would offer an encoding nothing would write.
    for (const auto& offered : kOfferedEncodings) {
        INFO("encodage : " << offered.charset);
        CHECK(Encoding::create(offered.charset, ByteOrderMark::Absent).has_value());
    }
}

TEST_CASE("an encoding that is not on the list is typed", "[gui][GUI-ENC-02]") {
    // « Other… » is what keeps the short list from being a ceiling: the set of
    // encodings is ICU's, and the list offers a part of it.
    const SaveShape shape{Encoding::utf8(ByteOrderMark::Absent), Newline::Lf};

    shape.encodingBox()->setCurrentIndex(shape.encodingBox()->count() - 1);
    // Typed under any of its names — ICU knows them all.
    shape.otherName()->setText(QStringLiteral("cp1257"));

    CHECK(shape.encoding() == named("windows-1257"));
}

TEST_CASE("the name field shows itself only for the other entry", "[gui][GUI-ENC-02]") {
    SaveShape shape{Encoding::utf8(ByteOrderMark::Absent), Newline::Lf};
    shape.show();

    CHECK_FALSE(shape.otherName()->isVisible());

    shape.encodingBox()->setCurrentIndex(shape.encodingBox()->count() - 1);

    CHECK(shape.otherName()->isVisible());
}

TEST_CASE("a name nobody knows is no encoding at all", "[gui][GUI-ENC-02]") {
    // Answering nothing rather than falling back to UTF-8: writing a file in an
    // encoding nobody named is not something to settle on one's own.
    const SaveShape shape{Encoding::utf8(ByteOrderMark::Absent), Newline::Lf};

    shape.encodingBox()->setCurrentIndex(shape.encodingBox()->count() - 1);
    shape.otherName()->setText(QStringLiteral("klingon-1"));

    CHECK_FALSE(shape.encoding().has_value());
}

TEST_CASE("a file opened in an encoding the list does not offer opens on the other field",
          "[gui][GUI-ENC-02]") {
    const SaveShape shape{named("cp1257"), Newline::Lf};

    // The name ICU gives it, and not the one typed to reach it: that is the one
    // the report and the settings file carry.
    CHECK(shape.otherName()->text() == QStringLiteral("windows-1257"));
    CHECK(shape.encoding() == named("cp1257"));
}

TEST_CASE("the mark is offered only where one exists", "[gui][GUI-ENC-02]") {
    // A byte order mark exists for the Unicode encodings and for no other.
    // Greyed rather than hidden: a greyed box says why the choice is not on
    // offer.
    const SaveShape shape{Encoding::utf8(ByteOrderMark::Present), Newline::Lf};

    CHECK(shape.markBox()->isEnabled());

    shape.encodingBox()->setCurrentIndex(
        shape.encodingBox()->findText(QStringLiteral("windows-1252"), Qt::MatchStartsWith));

    CHECK_FALSE(shape.markBox()->isEnabled());
    CHECK_FALSE(shape.wantsByteOrderMark());
}

TEST_CASE("the line endings offered are the three the core knows", "[gui][GUI-ENC-02]") {
    const SaveShape shape{Encoding::utf8(ByteOrderMark::Absent), Newline::Lf};

    REQUIRE(shape.newlineBox()->count() == 3);
    shape.newlineBox()->setCurrentIndex(2);
    CHECK(shape.newline() == Newline::Cr);
}

TEST_CASE("the save dialog opens on the file and its shape", "[gui][GUI-ENC-02]") {
    // What `Save As…` shows, built without being opened: `exec()` alone is out
    // of a test's reach, and it is the only thing that is.
    const subedit::core::SourceFile current{.path = std::filesystem::path{"/films/film.srt"},
                                            .newline = Newline::CrLf};

    const std::unique_ptr<QFileDialog> dialog =
        saveDialogFor(current, named("windows-1252"), nullptr);

    REQUIRE(dialog != nullptr);
    CHECK(dialog->acceptMode() == QFileDialog::AcceptSave);
    const auto* shape = dialog->findChild<const SaveShape*>();
    REQUIRE(shape != nullptr);
    CHECK(shape->encoding() == named("windows-1252"));
    CHECK(shape->newline() == Newline::CrLf);
}

TEST_CASE("what the save dialog was filled with is read back", "[gui][GUI-ENC-02]") {
    const std::unique_ptr<QFileDialog> dialog =
        saveDialogFor(subedit::core::SourceFile{}, Encoding::utf8(ByteOrderMark::Absent), nullptr);
    dialog->selectFile(QStringLiteral("/films/copie.vtt"));
    dialog->selectNameFilter(QStringLiteral("WebVTT (*.vtt)"));
    auto* shape = dialog->findChild<SaveShape*>();
    REQUIRE(shape != nullptr);
    shape->newlineBox()->setCurrentIndex(1);
    shape->markBox()->setChecked(true);

    const std::expected<SaveTarget, std::string> target = targetOf(*dialog);

    REQUIRE(target.has_value());
    // The name and not the whole path: a file dialog resolves what it is given
    // against a directory that depends on the machine.
    CHECK(target->path.filename() == std::filesystem::path{"copie.vtt"});
    CHECK(target->format == SubtitleFormat::WebVtt);
    CHECK(target->newline == Newline::CrLf);
    CHECK(target->encoding == Encoding::utf8(ByteOrderMark::Present));
}

TEST_CASE("a name nobody knows writes no file at all", "[gui][GUI-ENC-02]") {
    // The refusal carries the name that was typed: that is what the window will
    // show, and the only thing the user can correct.
    const std::unique_ptr<QFileDialog> dialog =
        saveDialogFor(subedit::core::SourceFile{}, Encoding::utf8(ByteOrderMark::Absent), nullptr);
    dialog->selectFile(QStringLiteral("/films/copie.srt"));
    auto* shape = dialog->findChild<SaveShape*>();
    REQUIRE(shape != nullptr);
    shape->encodingBox()->setCurrentIndex(shape->encodingBox()->count() - 1);
    shape->otherName()->setText(QStringLiteral("klingon-1"));

    const std::expected<SaveTarget, std::string> target = targetOf(*dialog);

    REQUIRE_FALSE(target.has_value());
    CHECK(target.error() == "no encoding is named \"klingon-1\"");
}

TEST_CASE("an encoding that writes its own mark writes no file either", "[gui][GUI-ENC-02]") {
    // The same refusal as the command line, in the same words: the model
    // refuses, not each surface for itself. And the mark box can do nothing
    // about it — what the converter writes does not switch off.
    const std::unique_ptr<QFileDialog> dialog =
        saveDialogFor(subedit::core::SourceFile{}, Encoding::utf8(ByteOrderMark::Absent), nullptr);
    dialog->selectFile(QStringLiteral("/films/copie.srt"));
    auto* shape = dialog->findChild<SaveShape*>();
    REQUIRE(shape != nullptr);
    shape->encodingBox()->setCurrentIndex(shape->encodingBox()->count() - 1);
    shape->otherName()->setText(QStringLiteral("UTF-16"));

    // The box greys out, there being no encoding whose mark we know.
    CHECK_FALSE(shape->markBox()->isEnabled());

    const std::expected<SaveTarget, std::string> target = targetOf(*dialog);

    REQUIRE_FALSE(target.has_value());
    CHECK(target.error() ==
          "\"UTF-16\" writes a byte order mark of its own; name the byte order, as UTF-16LE and "
          "UTF-16BE do");
}

TEST_CASE("the fields go into the columns the dialog already uses", "[gui][GUI-ENC-02]") {
    // **The defect of issue #321**: the widget carried a layout of its own, so
    // its labels landed in the same column as the dialog's and its fields two
    // columns too far left. What this case tests is the column, the only thing
    // that means anything without a screen.
    QFileDialog dialog;
    dialog.setOption(QFileDialog::DontUseNativeDialog);
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    auto* grid = dynamic_cast<QGridLayout*>(dialog.layout());
    REQUIRE(grid != nullptr);
    const int before = grid->rowCount();
    auto* shape = addSaveShapeTo(dialog, Encoding::utf8(ByteOrderMark::Absent), Newline::Lf);

    REQUIRE(grid->rowCount() == before + 4);

    // The dialog's own field column: the one its file name lives in, row 2 of
    // its grid.
    int row = 0;
    int column = 0;
    int rowSpan = 0;
    int columnSpan = 0;
    grid->getItemPosition(
        grid->indexOf(grid->itemAtPosition(2, 1)->widget()), &row, &column, &rowSpan, &columnSpan);
    const int fields = column;

    for (QWidget* field : {static_cast<QWidget*>(shape->encodingBox()),
                           static_cast<QWidget*>(shape->newlineBox()),
                           static_cast<QWidget*>(shape->markBox()),
                           static_cast<QWidget*>(shape->otherName())}) {
        INFO("champ : " << field->metaObject()->className());
        REQUIRE(grid->indexOf(field) >= 0);
        grid->getItemPosition(grid->indexOf(field), &row, &column, &rowSpan, &columnSpan);
        CHECK(column == fields);
    }
}

TEST_CASE("a host that lays out otherwise still gets the fields", "[gui][GUI-ENC-02]") {
    // The fallback, and it is reachable — which is why the function takes a
    // widget and not a file dialog. The day Qt lays its dialog out otherwise, a
    // misplaced field beats an absent one.
    QWidget host;
    // On the stack, and installed by its constructor: a layout given a parent
    // widget belongs to it, and this one unhooks itself first, being destroyed
    // before the widget it was declared after.
    QVBoxLayout column{&host};

    auto* shape = addSaveShapeTo(host, Encoding::utf8(ByteOrderMark::Absent), Newline::Lf);

    REQUIRE(shape != nullptr);
    REQUIRE(host.layout() == &column);
    CHECK(column.indexOf(shape) >= 0);
}

TEST_CASE("a host with no layout at all keeps the shape reachable", "[gui][GUI-ENC-02]") {
    // Neither grid nor layout: the shape exists all the same, and `targetOf`
    // finds it. Without that, a save would write without knowing into what.
    QWidget host;

    auto* shape = addSaveShapeTo(host, Encoding::utf8(ByteOrderMark::Absent), Newline::Lf);

    REQUIRE(shape != nullptr);
    CHECK(host.findChild<const SaveShape*>() == shape);
}

TEST_CASE("the shape sits inside the file dialog", "[gui][GUI-ENC-02]") {
    // Photographed and tested in the same place: what the window shows is what
    // this test drives, `exec()` apart.
    QFileDialog dialog;
    dialog.setOption(QFileDialog::DontUseNativeDialog);

    const SaveShape* shape =
        addSaveShapeTo(dialog, Encoding::utf8(ByteOrderMark::Absent), Newline::Lf);

    REQUIRE(shape != nullptr);
    CHECK(shape->parent() == &dialog);
    CHECK(dialog.findChild<const SaveShape*>() == shape);
}
