// The operations of `Tools`, without the window — ADR 0035, issue #486.
//
// `ProjectOperations` receives the page it is about and reads its target in
// that page's own selection; what belongs to the screen it asks of its `View`.
// These cases give it a page, a double of the view that records what it was
// asked, and prompts that answer the boxes. The window's own cases are
// unchanged: what they prove, this does too, one level down.

#include <subedit/core/config/duration_adjustment_settings.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/letter_case.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/duration_adjust_dialog.hpp>
#include <subedit/gui/project_operations.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/split_project_dialog.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QCheckBox>
#include <QDialog>
#include <QItemSelectionModel>
#include <QSpinBox>
#include <QString>
#include <QWidget>
#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::Document;
using subedit::core::Duration;
using subedit::core::InMemoryFileSystem;
using subedit::core::Project;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;
using subedit::gui::ProjectOperations;
using subedit::gui::ProjectPage;
using subedit::gui::ShiftDialog;
using subedit::test::FakePrompts;

constexpr const char* kFour = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                              "2\n00:00:03,000 --> 00:00:04,000\nDeux.\n\n"
                              "3\n00:00:05,000 --> 00:00:06,000\nTrois.\n\n"
                              "4\n00:00:07,000 --> 00:00:08,000\nQuatre.\n\n";

/// The window, as the operations see it.
class Desk final : public ProjectOperations::View {

public:
    QWidget parent;
    InMemoryFileSystem files;
    Document aimed = Document::Main;
    std::optional<Duration> length;
    std::optional<std::filesystem::path> toAppend;
    std::vector<std::string> announced;
    std::vector<std::pair<int, int>> selected;
    std::vector<Project> aside;
    std::size_t diagnosticsShown = 0;

    /// Answers every reading with an empty project, as no reader would.
    bool readsNothing = false;

    [[nodiscard]] QWidget* dialogParent() override { return &parent; }

    [[nodiscard]] Document targetDocument() const override { return aimed; }

    [[nodiscard]] std::optional<Duration> videoLength(const ProjectPage& /*page*/) const override {
        return length;
    }

    void announce(const std::string& message) override { announced.push_back(message); }

    [[nodiscard]] std::optional<std::filesystem::path> fileToAppend() override { return toAppend; }

    [[nodiscard]] std::expected<subedit::core::OpenedFile, std::string>
    read(const std::filesystem::path& path) override {
        if (readsNothing)
            return subedit::core::OpenedFile{};
        auto opened = subedit::core::openProject(files, path);
        if (!opened)
            return std::unexpected(path.string() + ": will not open");
        return std::move(*opened);
    }

    void showDiagnostics(std::span<const subedit::core::Diagnostic> diagnostics) override {
        diagnosticsShown += diagnostics.size();
    }

    void selectRows(int first, int last) override { selected.emplace_back(first, last); }

    void openAside(Project project) override { aside.push_back(std::move(project)); }
};

/// A page on four subtitles, the operations over it, and what answers them.
struct Bench {
    Desk desk;
    FakePrompts prompts;
    ProjectOperations operations{prompts, desk};
    std::unique_ptr<ProjectPage> page;

    Bench() {
        desk.files.addFile("/films/film.srt", kFour);
        auto opened = subedit::core::openProject(desk.files, "/films/film.srt");
        REQUIRE(opened.has_value());
        page = ProjectPage::make(std::move(opened->project));
    }

    [[nodiscard]] Timestamp startOf(std::size_t row) const {
        return page->session->project().subtitleAt(SubtitleIndex::fromValue(row)).start;
    }

    void select(int row) const {
        page->tableSelection->select(page->model->index(row, 0),
                                     QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

    /// Answers the shift box with `typed`.
    void shiftBy(const char* typed) {
        prompts.nextRun = true;
        prompts.fill = [typed](QDialog& dialog) {
            dynamic_cast<ShiftDialog&>(dialog).setTyped(QString::fromUtf8(typed));
        };
    }
};

} // namespace

TEST_CASE("an operation aims at the selection of the page it is given", "[gui][GUI-SHIFT-01]") {
    Bench bench;
    bench.select(1);
    bench.shiftBy("00:00:01,000");

    bench.operations.shift(*bench.page);

    CHECK(bench.startOf(0) == Timestamp::fromMilliseconds(1000));
    CHECK(bench.startOf(1) == Timestamp::fromMilliseconds(4000));
    CHECK(bench.startOf(2) == Timestamp::fromMilliseconds(5000));
    // Its box sits over the window the view names.
    CHECK(bench.prompts.lastDialog != nullptr);
    CHECK(bench.page->session->canUndo());
}

TEST_CASE("with nothing selected, an operation takes the whole file", "[gui][GUI-SHIFT-01]") {
    Bench bench;
    bench.shiftBy("00:00:01,000");

    bench.operations.shift(*bench.page);

    CHECK(bench.startOf(0) == Timestamp::fromMilliseconds(2000));
    CHECK(bench.startOf(3) == Timestamp::fromMilliseconds(8000));
}

TEST_CASE("a shift before the origin is refused and enters no history", "[gui][GUI-SHIFT-01]") {
    Bench bench;
    bench.shiftBy("-00:00:02,000");

    bench.operations.shift(*bench.page);

    REQUIRE(bench.prompts.failures.size() == 1);
    CHECK(bench.prompts.failures.front() ==
          "subtitle 1 would start before the origin, which no subtitle file can hold");
    CHECK_FALSE(bench.page->session->canUndo());
}

TEST_CASE("what passes the end of the film is said, and only for a move", "[gui][GUI-SHIFT-01]") {
    Bench bench;
    bench.desk.length = Duration::fromMilliseconds(7500);
    const subedit::core::Selection whole =
        subedit::core::Selection::all(bench.page->session->project());

    const std::string notice = bench.operations.applyQuietly(
        *bench.page,
        std::make_unique<subedit::core::ShiftCommand>(whole, Duration::fromMilliseconds(1000)),
        whole);
    CHECK_FALSE(notice.empty());
    CHECK(bench.prompts.outcomes.empty());

    // The same, through the road that says it: one box.
    bench.operations.apply(
        *bench.page,
        std::make_unique<subedit::core::ShiftCommand>(whole, Duration::fromMilliseconds(1000)),
        whole);
    CHECK(bench.prompts.outcomes.size() == 1);

    // Without a film, nothing to be past the end of.
    bench.desk.length.reset();
    bench.operations.apply(
        *bench.page,
        std::make_unique<subedit::core::ShiftCommand>(whole, Duration::fromMilliseconds(1000)),
        whole);
    CHECK(bench.prompts.outcomes.size() == 1);
}

TEST_CASE("an operation forgets where playback was placed", "[gui][GUI-PLAYER-02]") {
    Bench bench;
    bench.page->placedAt = 2;
    bench.shiftBy("00:00:01,000");

    bench.operations.shift(*bench.page);

    CHECK(bench.page->placedAt == -1);
}

TEST_CASE("the italic toggles the text aimed at, and says so in the status bar",
          "[gui][GUI-ITALIC-01]") {
    Bench bench;
    bench.select(0);

    bench.operations.toggleItalics(*bench.page);
    CHECK(bench.page->session->project().subtitleAt(SubtitleIndex::fromValue(0)).mainText ==
          "<i>Un.</i>");
    REQUIRE(bench.desk.announced.size() == 1);
    CHECK(bench.desk.announced.back() == subedit::core::noticeOfItalics(1, true));

    bench.operations.toggleItalics(*bench.page);
    CHECK(bench.page->session->project().subtitleAt(SubtitleIndex::fromValue(0)).mainText == "Un.");
}

TEST_CASE("an operation that changes nothing says so and enters no history", "[gui][GUI-CASE-01]") {
    Bench bench;
    bench.select(0);

    // « Un. » is already in sentence case.
    bench.operations.changeCase(*bench.page, subedit::core::LetterCase::Sentence);

    CHECK_FALSE(bench.page->session->canUndo());
    REQUIRE(bench.desk.announced.size() == 1);
    CHECK(bench.desk.announced.back() == subedit::core::nothingToChange());
}

TEST_CASE("the form of an adjustment is kept for the next one", "[gui][GUI-ADJUST-01]") {
    Bench bench;
    subedit::core::DurationAdjustmentSettings given;
    given.gapMilliseconds = 250;
    bench.operations.setDurationSettings(given);
    CHECK(bench.operations.durationSettings() == given);

    bench.prompts.nextRun = true;
    bench.prompts.fill = [](QDialog& dialog) {
        dynamic_cast<subedit::gui::DurationAdjustDialog&>(dialog).lengthenCheck()->setChecked(
            false);
    };
    bench.operations.adjustDurations(*bench.page);

    CHECK_FALSE(bench.operations.durationSettings().lengthen);
    // Said even when nothing moved.
    CHECK(bench.prompts.outcomes.size() == 1);
}

TEST_CASE("appending asks the view for the file, and selects what it wrote",
          "[gui][GUI-APPEND-01]") {
    Bench bench;

    // No file chosen: nothing is read, nothing is said.
    bench.operations.appendFile(*bench.page);
    CHECK(bench.page->session->project().count() == 4);

    bench.desk.toAppend = "/films/missing.srt";
    bench.operations.appendFile(*bench.page);
    CHECK(bench.prompts.failures.size() == 1);

    bench.desk.files.addFile("/films/more.srt", kFour);
    bench.desk.toAppend = "/films/more.srt";
    bench.operations.appendFile(*bench.page);

    CHECK(bench.page->session->project().count() == 8);
    REQUIRE(bench.desk.selected.size() == 1);
    CHECK(bench.desk.selected.front() == std::pair{4, 7});
    CHECK(bench.desk.announced.size() == 1);
}

TEST_CASE("splitting hands the tail to the view, and a cancel gives the selection back",
          "[gui][GUI-PSPLIT-01]") {
    Bench bench;
    bench.select(1);

    bench.prompts.nextRun = false;
    bench.prompts.fill = [](QDialog& dialog) {
        dynamic_cast<subedit::gui::SplitProjectDialog&>(dialog).subtitleBox()->setValue(3);
    };
    bench.operations.splitProject(*bench.page);
    CHECK(bench.page->session->project().count() == 4);
    CHECK_FALSE(bench.desk.selected.empty());
    const QModelIndexList rows = bench.page->tableSelection->selectedRows();
    REQUIRE(rows.size() == 1);
    CHECK(rows.front().row() == 1);

    bench.prompts.nextRun = true;
    bench.operations.splitProject(*bench.page);
    CHECK(bench.page->session->project().count() == 2);
    REQUIRE(bench.desk.aside.size() == 1);
    CHECK(bench.desk.aside.front().count() == 2);
    CHECK(bench.desk.announced.back() == subedit::core::noticeOfSplit(2));
}

TEST_CASE("a file with no subtitle appends nothing", "[gui][GUI-APPEND-01]") {
    // Unreachable from the window — every reader refuses a file with no
    // subtitle — and reachable here, through a view that reads anything.
    Bench bench;
    bench.desk.files.addFile("/films/empty.srt", "");
    bench.desk.toAppend = "/films/empty.srt";
    bench.desk.readsNothing = true;

    bench.operations.appendFile(*bench.page);

    CHECK(bench.page->session->project().count() == 4);
    CHECK_FALSE(bench.page->session->canUndo());
    CHECK(bench.desk.announced.empty());
    CHECK(bench.prompts.outcomes.empty());
}

TEST_CASE("two notices share one box, one to a line", "[gui][GUI-ADJUST-01]") {
    using subedit::gui::joinedNotices;
    CHECK(joinedNotices("done", "past") == "done\npast");
    CHECK(joinedNotices("", "past") == "past");
    CHECK(joinedNotices("done", "") == "done");
    CHECK(joinedNotices("", "").empty());
}
