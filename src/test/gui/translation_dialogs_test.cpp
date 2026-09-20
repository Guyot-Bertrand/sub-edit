// The two questions of a project that holds a translation — issue #432.
//
// **Tested without ever entering `exec()`**, as the other dialogs are: they are
// our own widgets, a test builds one, presses what a user would press and reads
// what comes out. Only the modal loop is out of reach, and it is behind
// `Prompts::run`.

#include <subedit/core/edit/translation.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/gui/open_translation_dialog.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/unsaved_documents_dialog.hpp>

#include <QAbstractButton>
#include <QCheckBox>
#include <QDialog>
#include <QPushButton>
#include <QRadioButton>
#include <catch2/catch_test_macros.hpp>

#include <array>

namespace {

using subedit::core::Document;
using subedit::core::TranslationMethod;
using subedit::gui::ModifiedDocument;
using subedit::gui::OpenTranslationDialog;
using subedit::gui::UnsavedChoice;
using subedit::gui::UnsavedDocumentsDialog;

/// The main document and the translation, both modified.
const std::array<ModifiedDocument, 2> kBoth = {
    ModifiedDocument{.document = Document::Main, .name = "film.srt"},
    ModifiedDocument{.document = Document::Translation, .name = "film.en.srt"},
};

} // namespace

TEST_CASE("the open dialog aligns by position unless told otherwise", "[gui][GUI-TRANS-01]") {
    // The default, because one line missing does not shift the ones after it —
    // which is what the eight cases of `paires/` are there to say.
    const OpenTranslationDialog dialog{QStringLiteral("film.en.srt")};

    CHECK(dialog.method() == TranslationMethod::Position);
    CHECK(dialog.positionButton()->isChecked());
}

TEST_CASE("the open dialog aligns by number when asked to", "[gui][GUI-TRANS-01]") {
    const OpenTranslationDialog dialog{QStringLiteral("film.en.srt")};

    dialog.numberButton()->setChecked(true);

    CHECK(dialog.method() == TranslationMethod::Number);
}

TEST_CASE("the open dialog names the file it is about", "[gui][GUI-TRANS-01]") {
    const OpenTranslationDialog dialog{QStringLiteral("film.en.srt")};

    CHECK(dialog.text().contains(QStringLiteral("film.en.srt")));
}

TEST_CASE("the closing question has one box for each modified document, all ticked",
          "[gui][GUI-CLOSE-01]") {
    const UnsavedDocumentsDialog dialog{kBoth};

    REQUIRE(dialog.boxes().size() == 2);
    CHECK(dialog.boxes().at(0)->isChecked());
    CHECK(dialog.boxes().at(1)->isChecked());
    CHECK(dialog.boxes().at(0)->text().contains(QStringLiteral("film.srt")));
    CHECK(dialog.boxes().at(1)->text().contains(QStringLiteral("film.en.srt")));
}

TEST_CASE("the list says which document is the translation", "[gui][GUI-CLOSE-01]") {
    const UnsavedDocumentsDialog dialog{kBoth};

    CHECK(dialog.boxes().at(1)->text().contains(QStringLiteral("Translation")));
    CHECK_FALSE(dialog.boxes().at(0)->text().contains(QStringLiteral("Translation")));
}

TEST_CASE("a file gone from the disk is listed as such", "[gui][GUI-CLOSE-01]") {
    // It counts as modified: it is the only copy of what the document holds.
    const std::array<ModifiedDocument, 2> gone = {
        ModifiedDocument{.document = Document::Main, .name = "film.srt", .missing = true},
        ModifiedDocument{.document = Document::Translation, .name = "film.en.srt"},
    };
    const UnsavedDocumentsDialog dialog{gone};

    CHECK(dialog.boxes().at(0)->text().contains(QStringLiteral("gone from the disk")));
    CHECK_FALSE(dialog.boxes().at(1)->text().contains(QStringLiteral("gone from the disk")));
}

TEST_CASE("saving answers with the documents that are still ticked", "[gui][GUI-CLOSE-01]") {
    const UnsavedDocumentsDialog dialog{kBoth};
    dialog.boxes().at(0)->setChecked(false);

    dialog.saveButton()->click();

    CHECK(dialog.choice() == UnsavedChoice::Save);
    REQUIRE(dialog.toSave().size() == 1);
    CHECK(dialog.toSave().front() == Document::Translation);
    CHECK(dialog.result() == QDialog::Accepted);
}

TEST_CASE("saving is out while nothing is ticked", "[gui][GUI-CLOSE-01]") {
    const UnsavedDocumentsDialog dialog{kBoth};

    dialog.boxes().at(0)->setChecked(false);
    dialog.boxes().at(1)->setChecked(false);

    CHECK_FALSE(dialog.saveButton()->isEnabled());

    dialog.boxes().at(1)->setChecked(true);
    CHECK(dialog.saveButton()->isEnabled());
}

TEST_CASE("closing without saving saves nothing, whatever is ticked", "[gui][GUI-CLOSE-01]") {
    const UnsavedDocumentsDialog dialog{kBoth};

    dialog.discardButton()->click();

    CHECK(dialog.choice() == UnsavedChoice::Discard);
    CHECK(dialog.toSave().empty());
    CHECK(dialog.result() == QDialog::Accepted);
}

TEST_CASE("giving up is the answer until another is given", "[gui][GUI-CLOSE-01]") {
    const UnsavedDocumentsDialog untouched{kBoth};
    CHECK(untouched.choice() == UnsavedChoice::Cancel);
    CHECK(untouched.toSave().empty());

    const UnsavedDocumentsDialog cancelled{kBoth};
    cancelled.cancelButton()->click();
    CHECK(cancelled.choice() == UnsavedChoice::Cancel);
    CHECK(cancelled.result() == QDialog::Rejected);
}
