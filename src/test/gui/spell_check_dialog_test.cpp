// The spell-check window — issue #509, D6: Gaupol's dialog, driven by a walk
// over a dictionary written in the test.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/spell_check_walk.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/spell_check_dialog.hpp>

#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QTextCursor>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

using subedit::core::CorrectionTarget;
using subedit::core::Document;
using subedit::core::InMemoryFileSystem;
using subedit::core::openSpellChecker;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::SpellCheckWalk;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;
using subedit::core::WordList;
using subedit::core::WordListSpellProvider;
using subedit::gui::SpellCheckDialog;

/// A project, a dictionary and a walk over one; the dialog is made by each test.
struct Fixture {
    Project project;
    std::shared_ptr<std::vector<std::string>> personal;
    std::unique_ptr<SpellCheckWalk> walk;

    explicit Fixture(const std::vector<std::string>& texts) {
        std::vector<Subtitle> subtitles;
        std::int64_t start = 0;
        for (const std::string& text : texts) {
            subtitles.push_back({.start = Timestamp::fromMilliseconds(start),
                                 .end = Timestamp::fromMilliseconds(start + 900),
                                 .mainText = text});
            start += 1000;
        }
        project.setSubtitles(std::move(subtitles));

        WordList list;
        list.words = {"ok", "salut", "bien", "va", "bonjour"};
        list.suggestions = {{"qqq", {"salut", "bien"}}, {"bnjour", {"bonjour"}}};
        personal = list.personal;
        WordListSpellProvider provider;
        provider.add("fr", std::move(list));
        const InMemoryFileSystem files;
        walk = std::make_unique<SpellCheckWalk>(
            openSpellChecker(provider, "fr", files, "/none.repl").value(),
            std::vector<CorrectionTarget>{{.project = &project,
                                           .selection = Selection::all(project),
                                           .document = Document::Main}});
    }
};

QString selectedText(const SpellCheckDialog& dialog) {
    return dialog.textView()->textCursor().selectedText();
}

} // namespace

TEST_CASE("starting shows the first unknown word, selected, with its suggestions",
          "[gui][spell-check-dialog]") {
    Fixture fixture{{"ok qqq bien"}};
    SpellCheckDialog dialog{*fixture.walk};
    // Not a QSignalSpy: its arguments go through QVariant, which a `SpellStop`
    // (with no default) does not fit.
    std::vector<subedit::core::SpellStop> stops;
    QObject::connect(&dialog,
                     &SpellCheckDialog::stopped,
                     [&stops](const subedit::core::SpellStop& stop) { stops.push_back(stop); });

    dialog.start();

    CHECK(dialog.textView()->toPlainText() == "ok qqq bien");
    CHECK(selectedText(dialog) == "qqq");
    REQUIRE(dialog.suggestionList()->count() == 2);
    CHECK(dialog.suggestionList()->item(0)->text() == "salut");
    // Gaupol selects the first suggestion, which fills the replacement.
    CHECK(dialog.replacementField()->text() == "salut");
    CHECK(dialog.replaceButton()->isEnabled());
    CHECK(dialog.addButton()->isEnabled());
    CHECK_FALSE(dialog.saveButton()->isEnabled());

    REQUIRE(stops.size() == 1);
    CHECK(stops.front().project == &fixture.project);
    CHECK(stops.front().index == SubtitleIndex::fromValue(0));
}

TEST_CASE("selecting another suggestion fills the replacement", "[gui][spell-check-dialog]") {
    Fixture fixture{{"ok qqq"}};
    SpellCheckDialog dialog{*fixture.walk};
    dialog.start();

    dialog.suggestionList()->setCurrentRow(1);

    CHECK(dialog.replacementField()->text() == "bien");
}

TEST_CASE("typing in the replacement refreshes the suggestions", "[gui][spell-check-dialog]") {
    Fixture fixture{{"ok qqq"}};
    SpellCheckDialog dialog{*fixture.walk};
    dialog.start();

    dialog.replacementField()->clear();
    emit dialog.replacementField()->textEdited(QString{});
    CHECK(dialog.suggestionList()->count() == 0);
    CHECK_FALSE(dialog.replaceButton()->isEnabled());

    dialog.replacementField()->setText(QStringLiteral("bnjour"));
    emit dialog.replacementField()->textEdited(QStringLiteral("bnjour"));

    REQUIRE(dialog.suggestionList()->count() == 1);
    CHECK(dialog.suggestionList()->item(0)->text() == "bonjour");
    // The suggestions changed, and the field did not follow: one is still to be chosen.
    CHECK(dialog.replacementField()->text() == "bnjour");
    CHECK(dialog.replaceButton()->isEnabled());
}

TEST_CASE("GUI-SPELL-01: ignore moves on to the next word",
          "[gui][spell-check-dialog][GUI-SPELL-01]") {
    Fixture fixture{{"qqq ok qqq"}};
    SpellCheckDialog dialog{*fixture.walk};
    dialog.start();

    dialog.ignoreButton()->click();

    REQUIRE(dialog.stop() != nullptr);
    CHECK(dialog.stop()->pos == 7);
    CHECK_FALSE(dialog.walkFinished());
}

TEST_CASE("GUI-SPELL-01: ignore all skips the later instances",
          "[gui][spell-check-dialog][GUI-SPELL-01]") {
    Fixture fixture{{"qqq ok qqq", "ok zzz"}};
    SpellCheckDialog dialog{*fixture.walk};
    dialog.start();

    dialog.ignoreAllButton()->click();

    REQUIRE(dialog.stop() != nullptr);
    CHECK(dialog.stop()->word == "zzz");
}

TEST_CASE("GUI-SPELL-01: add puts the word in the personal list",
          "[gui][spell-check-dialog][GUI-SPELL-01]") {
    Fixture fixture{{"ok qqq"}};
    SpellCheckDialog dialog{*fixture.walk};
    dialog.start();

    dialog.addButton()->click();

    REQUIRE(fixture.personal->size() == 1);
    CHECK(fixture.personal->front() == "qqq");
    CHECK(dialog.walkFinished());
}

TEST_CASE("GUI-SPELL-01: replace puts the replacement in the text",
          "[gui][spell-check-dialog][GUI-SPELL-01]") {
    Fixture fixture{{"ok qqq zzz"}};
    SpellCheckDialog dialog{*fixture.walk};
    dialog.start();

    dialog.replacementField()->setText(QStringLiteral("bien"));
    dialog.replaceButton()->click();

    REQUIRE(dialog.stop() != nullptr);
    CHECK(dialog.stop()->text == "ok bien zzz");
    CHECK(dialog.stop()->word == "zzz");
    CHECK(dialog.textView()->toPlainText() == "ok bien zzz");
}

TEST_CASE("GUI-SPELL-01: replace all reaches the later texts",
          "[gui][spell-check-dialog][GUI-SPELL-01]") {
    Fixture fixture{{"ok qqq", "qqq va zzz"}};
    SpellCheckDialog dialog{*fixture.walk};
    dialog.start();

    dialog.replaceAllButton()->click();

    // "salut", the first suggestion, replaced the word here and in the next text.
    REQUIRE(dialog.stop() != nullptr);
    CHECK(dialog.stop()->word == "zzz");
    CHECK(dialog.stop()->text == "salut va zzz");
    CHECK(dialog.stop()->index == SubtitleIndex::fromValue(1));
}

TEST_CASE("GUI-SPELL-01: join with previous and next need a space on that side",
          "[gui][spell-check-dialog][GUI-SPELL-01]") {
    Fixture fixture{{"qqq ok", "ok qqq", "ok qqq ok"}};
    SpellCheckDialog dialog{*fixture.walk};

    dialog.start(); // "qqq" starts the text: nothing before it
    CHECK_FALSE(dialog.joinWithPreviousButton()->isEnabled());
    CHECK(dialog.joinWithNextButton()->isEnabled());

    dialog.ignoreButton()->click(); // "qqq" ends the text: nothing after it
    CHECK(dialog.joinWithPreviousButton()->isEnabled());
    CHECK_FALSE(dialog.joinWithNextButton()->isEnabled());

    dialog.ignoreButton()->click(); // "qqq" between two words
    CHECK(dialog.joinWithPreviousButton()->isEnabled());
    CHECK(dialog.joinWithNextButton()->isEnabled());
}

TEST_CASE("GUI-SPELL-01: join with previous makes a compound the walk then finds correct",
          "[gui][spell-check-dialog][GUI-SPELL-01]") {
    Fixture fixture{{"ok bon jour"}};
    SpellCheckDialog dialog{*fixture.walk};
    dialog.start();
    CHECK(dialog.stop()->word == "bon");
    dialog.ignoreButton()->click();
    REQUIRE(dialog.stop() != nullptr);
    CHECK(dialog.stop()->word == "jour");

    dialog.joinWithPreviousButton()->click();

    CHECK(dialog.walkFinished());
    const auto corrections = fixture.walk->corrections();
    REQUIRE(corrections.size() == 1);
    CHECK(corrections.front().proposed == "ok bonjour");
}

TEST_CASE("GUI-SPELL-01: join with next removes the space after the word",
          "[gui][spell-check-dialog][GUI-SPELL-01]") {
    Fixture fixture{{"ok bon jour"}};
    SpellCheckDialog dialog{*fixture.walk};
    dialog.start();

    dialog.joinWithNextButton()->click();

    CHECK(dialog.walkFinished());
    const auto corrections = fixture.walk->corrections();
    REQUIRE(corrections.size() == 1);
    CHECK(corrections.front().proposed == "ok bonjour");
}

TEST_CASE("GUI-SPELL-01: editing the text allows saving and greys the other gestures",
          "[gui][spell-check-dialog][GUI-SPELL-01]") {
    Fixture fixture{{"ok qqq ok"}};
    SpellCheckDialog dialog{*fixture.walk};
    dialog.start();

    dialog.textView()->appendPlainText(QStringLiteral(" zzz"));

    CHECK(dialog.saveButton()->isEnabled());
    for (QPushButton* button : {dialog.addButton(),
                                dialog.ignoreButton(),
                                dialog.ignoreAllButton(),
                                dialog.replaceButton(),
                                dialog.replaceAllButton(),
                                dialog.joinWithPreviousButton(),
                                dialog.joinWithNextButton()})
        CHECK_FALSE(button->isEnabled());
}

TEST_CASE("GUI-SPELL-01: save and resume checks the edited text from its start",
          "[gui][spell-check-dialog][GUI-SPELL-01]") {
    Fixture fixture{{"ok qqq ok"}};
    SpellCheckDialog dialog{*fixture.walk};
    dialog.start();

    dialog.textView()->setPlainText(QStringLiteral("zzz ok qqq"));
    dialog.saveButton()->click();

    REQUIRE(dialog.stop() != nullptr);
    CHECK(dialog.stop()->text == "zzz ok qqq");
    CHECK(dialog.stop()->word == "zzz");
    CHECK(selectedText(dialog) == "zzz");
    CHECK_FALSE(dialog.saveButton()->isEnabled());
    CHECK(dialog.ignoreButton()->isEnabled());
}

TEST_CASE("the end of the walk greys the buttons and empties the window",
          "[gui][spell-check-dialog]") {
    Fixture fixture{{"ok qqq"}};
    SpellCheckDialog dialog{*fixture.walk};
    const QSignalSpy finished{&dialog, &SpellCheckDialog::finishedWalking};
    dialog.start();

    dialog.ignoreButton()->click();

    CHECK(dialog.walkFinished());
    CHECK(finished.count() == 1);
    CHECK_FALSE(dialog.grid()->isEnabled());
    CHECK(dialog.stop() == nullptr);
    CHECK(dialog.textView()->toPlainText().isEmpty());
    CHECK(dialog.replacementField()->text().isEmpty());
    CHECK(dialog.suggestionList()->count() == 0);
    CHECK_FALSE(dialog.ignoreButton()->isEnabled());
}

TEST_CASE("a walk with nothing unknown finishes at once", "[gui][spell-check-dialog]") {
    Fixture fixture{{"ok bien"}};
    SpellCheckDialog dialog{*fixture.walk};

    dialog.start();

    CHECK(dialog.walkFinished());
}

TEST_CASE("the dialog counts the selected word in characters, not bytes",
          "[gui][spell-check-dialog]") {
    Fixture fixture{{"été qqq"}};
    SpellCheckDialog dialog{*fixture.walk};
    dialog.start();

    // "été" is a word the dictionary does not know, the first stop.
    CHECK(selectedText(dialog) == "été");
}
