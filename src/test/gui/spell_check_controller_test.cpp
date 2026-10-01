// `SpellCheckController` — issue #509, D6: opens the settings window and the
// spell-check window, applies what the walk corrected through the ordinary
// `Session::apply` road, one history entry per project, and saves the
// replacement list.

#include <subedit/core/config/spell_check_settings.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/translation.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/spell_replacements.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/spell_check_controller.hpp>
#include <subedit/gui/spell_check_dialog.hpp>
#include <subedit/gui/spell_check_settings_dialog.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QComboBox>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QRadioButton>
#include <QWidget>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <span>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::AttachedTranslation;
using subedit::core::attachTranslation;
using subedit::core::InMemoryFileSystem;
using subedit::core::noDictionaryFor;
using subedit::core::noticeOfCorrection;
using subedit::core::openProject;
using subedit::core::Project;
using subedit::core::SourceFile;
using subedit::core::SpellCheckDocument;
using subedit::core::SpellCheckSettings;
using subedit::core::SpellCheckTarget;
using subedit::core::SpellProvider;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;
using subedit::core::TranslationMethod;
using subedit::core::WordList;
using subedit::core::WordListSpellProvider;
using subedit::gui::ProjectPage;
using subedit::gui::SpellCheckController;
using subedit::gui::SpellCheckDialog;
using subedit::gui::SpellCheckSettingsDialog;
using subedit::test::FakePrompts;

/// A directory that removes itself — the harness a test that writes goes
/// through, never a bare name and never a resolved configuration location.
class ScratchDirectory {

public:
    ScratchDirectory() {
        static int serial = 0;
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path = std::filesystem::temp_directory_path() /
                 ("subedit-spell-" + std::to_string(stamp) + "-" + std::to_string(++serial));
        std::filesystem::create_directories(m_path);
    }

    ScratchDirectory(const ScratchDirectory&) = delete;
    ScratchDirectory(ScratchDirectory&&) = delete;
    ScratchDirectory& operator=(const ScratchDirectory&) = delete;
    ScratchDirectory& operator=(ScratchDirectory&&) = delete;

    ~ScratchDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(m_path, ignored);
    }

    [[nodiscard]] const std::filesystem::path& path() const { return m_path; }

private:
    std::filesystem::path m_path;
};

[[nodiscard]] std::shared_ptr<WordListSpellProvider> frenchProvider() {
    WordList list;
    list.words = {"ok", "salut", "bien", "va", "bonjour"};
    list.suggestions = {{"qqq", {"salut", "bien"}}};
    auto provider = std::make_shared<WordListSpellProvider>();
    provider->add("fr", std::move(list));
    return provider;
}

[[nodiscard]] subedit::core::OpenedFile openedOf(const std::vector<std::string>& texts) {
    std::string content;
    int number = 1;
    for (const std::string& text : texts) {
        const int second = number;
        content += std::to_string(number++) + "\n00:00:0" + std::to_string(second) +
                   ",000 --> 00:00:0" + std::to_string(second) + ",500\n" + text + "\n\n";
    }
    InMemoryFileSystem files;
    files.addFile("/film.srt", content);
    auto opened = openProject(files, "/film.srt");
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

[[nodiscard]] std::unique_ptr<ProjectPage> pageOf(const std::vector<std::string>& texts) {
    return ProjectPage::make(std::move(openedOf(texts).project));
}

/// A page whose only subtitle carries `translation` as its translation.
[[nodiscard]] std::unique_ptr<ProjectPage> pageWithTranslation(const std::string& main,
                                                               const std::string& translation) {
    subedit::core::OpenedFile opened = openedOf({main});
    const std::vector<Subtitle> lines{Subtitle{.start = Timestamp::fromMilliseconds(1000),
                                               .end = Timestamp::fromMilliseconds(1500),
                                               .mainText = translation}};
    const AttachedTranslation attached =
        attachTranslation(opened.project, lines, SourceFile{}, TranslationMethod::Position);
    attached.command->apply(opened.project);
    return ProjectPage::make(std::move(opened.project));
}

[[nodiscard]] const std::string& mainTextOf(const ProjectPage& page, unsigned row) {
    return page.session->project().subtitleAt(SubtitleIndex::fromValue(row)).mainText;
}

class Desk final : public SpellCheckController::View {
public:
    QWidget parent;
    std::vector<std::unique_ptr<ProjectPage>> pages_;
    std::size_t shown = 0;
    std::shared_ptr<const SpellProvider> provider = frenchProvider();
    std::filesystem::path configDirectory;
    std::vector<std::string> announced;
    std::vector<std::pair<const Project*, SubtitleIndex>> revealed;

    [[nodiscard]] QWidget* dialogParent() override { return &parent; }

    [[nodiscard]] std::span<const std::unique_ptr<ProjectPage>> pages() const override {
        return pages_;
    }

    [[nodiscard]] std::size_t shownProject() const override { return shown; }

    [[nodiscard]] const SpellProvider* spellProvider() const override { return provider.get(); }

    [[nodiscard]] std::filesystem::path spellConfigDirectory() const override {
        return configDirectory;
    }

    void announce(const std::string& message) override { announced.push_back(message); }

    void reveal(const Project& project, SubtitleIndex index) override {
        revealed.emplace_back(&project, index);
    }
};

/// The user clicks `Replace` `times` times, and closes the window.
[[nodiscard]] auto replacing(int times) {
    return [times](QDialog& dialog) {
        auto& window = dynamic_cast<SpellCheckDialog&>(dialog);
        for (int done = 0; done < times && !window.walkFinished(); ++done)
            window.replaceButton()->click();
    };
}

const SpellCheckSettings kFrench{.language = "fr"};

} // namespace

TEST_CASE("GUI-SPELL-01: replacing a word applies through the history, one entry per project",
          "[gui][spell-check-controller][GUI-SPELL-01]") {
    const ScratchDirectory scratch;
    Desk desk;
    desk.configDirectory = scratch.path();
    desk.pages_.push_back(pageOf({"ok qqq"}));
    desk.pages_.push_back(pageOf({"qqq bien", "va qqq"}));
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = replacing(3);
    SpellCheckController controller{prompts, desk};
    controller.setSettings({.language = "fr", .target = SpellCheckTarget::AllProjects});

    controller.openCheck();

    CHECK(mainTextOf(*desk.pages_[0], 0) == "ok salut");
    CHECK(mainTextOf(*desk.pages_[1], 0) == "salut bien");
    CHECK(mainTextOf(*desk.pages_[1], 1) == "va salut");
    // One gesture undoes a whole project, and only one entry was made.
    REQUIRE(desk.pages_[1]->session->canUndo());
    (void)desk.pages_[1]->session->undo();
    CHECK(mainTextOf(*desk.pages_[1], 0) == "qqq bien");
    CHECK(mainTextOf(*desk.pages_[1], 1) == "va qqq");
    CHECK_FALSE(desk.pages_[1]->session->canUndo());
    CHECK(mainTextOf(*desk.pages_[0], 0) == "ok salut");

    REQUIRE(desk.announced.size() == 1);
    CHECK(desk.announced[0] == noticeOfCorrection(3, 0));
    // Every word reached was revealed, in the project that holds it.
    REQUIRE(desk.revealed.size() == 3);
    CHECK(desk.revealed[0].first == &desk.pages_[0]->session->project());
    CHECK(desk.revealed[1].first == &desk.pages_[1]->session->project());
    CHECK(desk.revealed[2].second == SubtitleIndex::fromValue(1));
}

TEST_CASE("closing in the middle applies what was done and nothing more",
          "[gui][spell-check-controller][GUI-SPELL-01]") {
    const ScratchDirectory scratch;
    Desk desk;
    desk.configDirectory = scratch.path();
    desk.pages_.push_back(pageOf({"qqq ok", "qqq"}));
    FakePrompts prompts;
    prompts.nextRun = false; // the window is closed, not accepted
    prompts.fill = replacing(1);
    SpellCheckController controller{prompts, desk};
    controller.setSettings(kFrench);

    controller.openCheck();

    CHECK(mainTextOf(*desk.pages_[0], 0) == "salut ok");
    CHECK(mainTextOf(*desk.pages_[0], 1) == "qqq");
    CHECK(desk.pages_[0]->session->canUndo());
    REQUIRE(desk.announced.size() == 1);
    CHECK(desk.announced[0] == noticeOfCorrection(1, 0));
}

TEST_CASE("closing with nothing done changes nothing and leaves no history",
          "[gui][spell-check-controller]") {
    const ScratchDirectory scratch;
    Desk desk;
    desk.configDirectory = scratch.path();
    desk.pages_.push_back(pageOf({"qqq"}));
    FakePrompts prompts;
    SpellCheckController controller{prompts, desk};
    controller.setSettings(kFrench);

    controller.openCheck();

    CHECK(mainTextOf(*desk.pages_[0], 0) == "qqq");
    CHECK_FALSE(desk.pages_[0]->session->canUndo());
    CHECK(prompts.runAsked == 1);
}

TEST_CASE("the replacement list is written when the window closes",
          "[gui][spell-check-controller]") {
    const ScratchDirectory scratch;
    Desk desk;
    desk.configDirectory = scratch.path();
    desk.pages_.push_back(pageOf({"qqq"}));
    FakePrompts prompts;
    prompts.fill = replacing(1);
    SpellCheckController controller{prompts, desk};
    controller.setSettings(kFrench);

    controller.openCheck();

    const std::filesystem::path file = subedit::core::spellReplacementFile(scratch.path(), "fr");
    CHECK(std::filesystem::exists(file));
    CHECK(prompts.failures.empty());
}

TEST_CASE("nothing is written when nothing was replaced", "[gui][spell-check-controller]") {
    const ScratchDirectory scratch;
    Desk desk;
    desk.configDirectory = scratch.path();
    desk.pages_.push_back(pageOf({"ok bien"}));
    FakePrompts prompts;
    SpellCheckController controller{prompts, desk};
    controller.setSettings(kFrench);

    controller.openCheck();

    CHECK_FALSE(std::filesystem::exists(subedit::core::spellReplacementFile(scratch.path(), "fr")));
    // No misspelt word: the window is not even shown.
    CHECK(prompts.runAsked == 0);
    REQUIRE(desk.announced.size() == 1);
    CHECK(desk.announced[0] == noticeOfCorrection(0, 0));
}

TEST_CASE("the selection target walks the selected rows only", "[gui][spell-check-controller]") {
    const ScratchDirectory scratch;
    Desk desk;
    desk.configDirectory = scratch.path();
    desk.pages_.push_back(pageOf({"qqq", "qqq", "qqq"}));
    ProjectPage& page = *desk.pages_[0];
    page.tableSelection->select(page.model->index(1, 0),
                                QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    FakePrompts prompts;
    prompts.fill = replacing(5);
    SpellCheckController controller{prompts, desk};
    controller.setSettings({.language = "fr", .target = SpellCheckTarget::Selection});

    controller.openCheck();

    CHECK(mainTextOf(page, 0) == "qqq");
    CHECK(mainTextOf(page, 1) == "salut");
    CHECK(mainTextOf(page, 2) == "qqq");
}

TEST_CASE("an empty selection says there is nothing to check", "[gui][spell-check-controller]") {
    Desk desk;
    desk.pages_.push_back(pageOf({"qqq"}));
    FakePrompts prompts;
    SpellCheckController controller{prompts, desk};
    controller.setSettings({.language = "fr", .target = SpellCheckTarget::Selection});

    controller.openCheck();

    CHECK(prompts.runAsked == 0);
    REQUIRE(desk.announced.size() == 1);
    CHECK(desk.announced[0] == "Nothing to check.");
    CHECK(mainTextOf(*desk.pages_[0], 0) == "qqq");
}

TEST_CASE("the translation document is checked in the projects that carry one",
          "[gui][spell-check-controller]") {
    const ScratchDirectory scratch;
    Desk desk;
    desk.configDirectory = scratch.path();
    desk.pages_.push_back(pageOf({"qqq"})); // no translation: left out
    desk.pages_.push_back(pageWithTranslation("qqq", "qqq ok"));
    FakePrompts prompts;
    prompts.fill = replacing(5);
    SpellCheckController controller{prompts, desk};
    controller.setSettings({.language = "fr",
                            .target = SpellCheckTarget::AllProjects,
                            .document = SpellCheckDocument::Translation});

    controller.openCheck();

    CHECK(mainTextOf(*desk.pages_[0], 0) == "qqq");
    const Subtitle& second =
        desk.pages_[1]->session->project().subtitleAt(SubtitleIndex::fromValue(0));
    CHECK(second.mainText == "qqq");
    CHECK(second.translationText == "salut ok");
}

TEST_CASE("a project without a translation has nothing to check in one",
          "[gui][spell-check-controller]") {
    Desk desk;
    desk.pages_.push_back(pageOf({"qqq"}));
    FakePrompts prompts;
    SpellCheckController controller{prompts, desk};
    controller.setSettings({.language = "fr", .document = SpellCheckDocument::Translation});

    controller.openCheck();

    CHECK(prompts.runAsked == 0);
    REQUIRE(desk.announced.size() == 1);
    CHECK(desk.announced[0] == "Nothing to check.");
}

TEST_CASE("GUI-SPELL-02: no dictionary for the language switches the check off and says why",
          "[gui][spell-check-controller][GUI-SPELL-02]") {
    Desk desk;
    desk.pages_.push_back(pageOf({"qqq"}));
    FakePrompts prompts;
    SpellCheckController controller{prompts, desk};

    controller.setSettings(kFrench);
    CHECK(controller.spellCheckAvailable());
    CHECK(controller.unavailableReason().empty());

    controller.setSettings({.language = "de"});
    CHECK_FALSE(controller.spellCheckAvailable());
    CHECK(controller.unavailableReason() == noDictionaryFor("de"));

    desk.provider = nullptr;
    controller.setSettings(kFrench);
    CHECK_FALSE(controller.spellCheckAvailable());
    CHECK(controller.unavailableReason() == noDictionaryFor("fr"));
}

TEST_CASE("the settings window keeps what it was given only when accepted",
          "[gui][spell-check-controller]") {
    Desk desk;
    desk.pages_.push_back(pageOf({"qqq"}));
    FakePrompts prompts;
    prompts.fill = [](QDialog& dialog) {
        auto& settings = dynamic_cast<SpellCheckSettingsDialog&>(dialog);
        settings.apply({.language = "fr", .target = SpellCheckTarget::AllProjects});
    };
    SpellCheckController controller{prompts, desk};

    prompts.nextRun = false;
    controller.configure();
    CHECK(controller.settings() == SpellCheckSettings{});

    prompts.nextRun = true;
    controller.configure();
    CHECK(controller.settings().language == "fr");
    CHECK(controller.settings().target == SpellCheckTarget::AllProjects);
}

TEST_CASE("the settings window offers the translation when any project carries one",
          "[gui][spell-check-controller]") {
    Desk desk;
    desk.pages_.push_back(pageOf({"qqq"}));
    desk.pages_.push_back(pageWithTranslation("qqq", "qqq"));
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& settings = dynamic_cast<SpellCheckSettingsDialog&>(dialog);
        CHECK(settings.translationRadio()->isEnabled());
        settings.translationRadio()->setChecked(true);
    };
    SpellCheckController controller{prompts, desk};

    controller.configure();

    CHECK(controller.settings().document == SpellCheckDocument::Translation);
}

TEST_CASE("a window with no dictionary provider says so and opens nothing",
          "[gui][spell-check-controller]") {
    Desk desk;
    desk.provider = nullptr;
    desk.pages_.push_back(pageOf({"qqq"}));
    FakePrompts prompts;
    SpellCheckController controller{prompts, desk};
    controller.setSettings(kFrench);

    controller.openCheck();

    CHECK(prompts.runAsked == 0);
    REQUIRE(desk.announced.size() == 1);
    CHECK(desk.announced[0] == noDictionaryFor("fr"));
}

TEST_CASE("a language the provider does not offer says so and opens nothing",
          "[gui][spell-check-controller]") {
    Desk desk;
    desk.pages_.push_back(pageOf({"qqq"}));
    FakePrompts prompts;
    SpellCheckController controller{prompts, desk};
    controller.setSettings({.language = "de"});

    controller.openCheck();

    CHECK(prompts.runAsked == 0);
    REQUIRE(desk.announced.size() == 1);
    CHECK(desk.announced[0] == noDictionaryFor("de"));
}

TEST_CASE("a project closed while the window was open is left out of what is applied",
          "[gui][spell-check-controller]") {
    const ScratchDirectory scratch;
    Desk desk;
    desk.configDirectory = scratch.path();
    desk.pages_.push_back(pageOf({"qqq"}));
    desk.pages_.push_back(pageOf({"qqq"}));
    std::unique_ptr<ProjectPage> closed; // kept alive: the walk still points at it
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [&](QDialog& dialog) {
        auto& window = dynamic_cast<SpellCheckDialog&>(dialog);
        window.replaceButton()->click();
        window.replaceButton()->click();
        closed = std::move(desk.pages_[0]);
        desk.pages_.erase(desk.pages_.begin());
    };
    SpellCheckController controller{prompts, desk};
    controller.setSettings({.language = "fr", .target = SpellCheckTarget::AllProjects});

    controller.openCheck();

    REQUIRE(desk.pages_.size() == 1);
    CHECK(mainTextOf(*desk.pages_[0], 0) == "salut");
    CHECK(mainTextOf(*closed, 0) == "qqq");
    CHECK(desk.pages_[0]->session->canUndo());
}

TEST_CASE("a replacement list that cannot be written is reported, and the text still applied",
          "[gui][spell-check-controller]") {
    const ScratchDirectory scratch;
    // A regular file where the directory should be: nothing can be created under it.
    const std::filesystem::path blocker = scratch.path() / "config";
    { std::ofstream{blocker} << "not a directory"; }
    Desk desk;
    desk.configDirectory = blocker;
    desk.pages_.push_back(pageOf({"qqq"}));
    FakePrompts prompts;
    prompts.fill = replacing(1);
    SpellCheckController controller{prompts, desk};
    controller.setSettings(kFrench);

    controller.openCheck();

    CHECK(mainTextOf(*desk.pages_[0], 0) == "salut");
    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures[0].starts_with("Could not save the replacements for fr: "));
}
