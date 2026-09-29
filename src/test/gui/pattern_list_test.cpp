// The pattern-list widget of a task page — issue #505, task 8. One tick box
// per pattern *name* a cascade holds: several records may share a name
// (`CorrectionPattern::name`'s own comment), and share a single box.

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/pattern_list.hpp>

#include <QCheckBox>
#include <QString>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace {

using subedit::core::CorrectionSettings;
using subedit::core::InMemoryFileSystem;
using subedit::core::PatternCatalogue;
using subedit::core::PatternKind;
using subedit::core::readPatternCatalogue;
using subedit::gui::PatternList;

const std::filesystem::path kShipped = "/patterns";

/// Two common-error records under `Zyyy`: "Letter I" (enabled by default) and
/// "Musical notes" (`Enabled=False` in the `.conf`, the mechanism the shipped
/// files use for a record disabled out of the box).
PatternCatalogue twoRecords() {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Letter I\nClasses=OCR;\nPattern=a\n"
                  "\n[Common Error Pattern]\nName=Musical notes\nClasses=OCR;\nPattern=b\n");
    files.addFile(kShipped / "Zyyy.common-error.conf",
                  R"(<patterns><pattern name="Musical notes" enabled="false"/></patterns>)");
    return readPatternCatalogue(files, kShipped, {});
}

/// The same name, "Repeated", held by two records of different codes: one at
/// `Zyyy` and one, more specific, at `Latn-en` — neither names `Policy=Replace`,
/// so the cascade of `Latn-en` holds both records rather than one replacing
/// the other. This is the case a single shared box must cover: toggling it
/// writes one `PatternActivation` per record, each keeping its own code.
PatternCatalogue sharedNameAcrossCodes() {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Repeated\nClasses=OCR;\nPattern=a\n");
    files.addFile(kShipped / "Latn-en.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Repeated\nClasses=OCR;\nPattern=b\n");
    return readPatternCatalogue(files, kShipped, {});
}

} // namespace

TEST_CASE("each box opens on the shipped default when settings carry no override",
          "[gui][pattern-list]") {
    const PatternCatalogue catalogue = twoRecords();
    PatternList list{catalogue, PatternKind::CommonError};

    list.setCode("Zyyy", CorrectionSettings{});

    CHECK(list.activations().empty()); // nothing overridden yet: every box agrees with its default
}

TEST_CASE("a box shows the shipped default of the record it stands for", "[gui][pattern-list]") {
    const PatternCatalogue catalogue = twoRecords();
    PatternList list{catalogue, PatternKind::CommonError};
    list.setCode("Zyyy", CorrectionSettings{});

    auto* onByDefault = list.findChild<QCheckBox*>(QString::fromStdString("Letter I"));
    auto* offByDefault = list.findChild<QCheckBox*>(QString::fromStdString("Musical notes"));

    REQUIRE(onByDefault != nullptr);
    REQUIRE(offByDefault != nullptr);
    CHECK(onByDefault->isChecked());
    CHECK_FALSE(offByDefault->isChecked());
}

TEST_CASE("unchecking a box on by default produces one activation, disabled",
          "[gui][pattern-list]") {
    const PatternCatalogue catalogue = twoRecords();
    PatternList list{catalogue, PatternKind::CommonError};
    list.setCode("Zyyy", CorrectionSettings{});

    auto* box = list.findChild<QCheckBox*>(QString::fromStdString("Letter I"));
    REQUIRE(box != nullptr);
    box->toggle();

    const auto activations = list.activations();
    REQUIRE(activations.size() == 1);
    CHECK(activations[0].name == "Letter I");
    CHECK(activations[0].code == "Zyyy");
    CHECK_FALSE(activations[0].enabled);
}

TEST_CASE("toggling a box back to its default drops the activation again", "[gui][pattern-list]") {
    const PatternCatalogue catalogue = twoRecords();
    PatternList list{catalogue, PatternKind::CommonError};
    list.setCode("Zyyy", CorrectionSettings{});

    auto* box = list.findChild<QCheckBox*>(QString::fromStdString("Letter I"));
    REQUIRE(box != nullptr);
    box->toggle();
    box->toggle();

    CHECK(list.activations().empty());
}

TEST_CASE("an override in settings opens the box it names, and an untouched box leaves it be",
          "[gui][pattern-list]") {
    const PatternCatalogue catalogue = twoRecords();
    PatternList list{catalogue, PatternKind::CommonError};

    CorrectionSettings settings;
    settings.patternActivations.push_back(subedit::core::PatternActivation{
        .kind = PatternKind::CommonError, .code = "Zyyy", .name = "Letter I", .enabled = false});
    const auto opened = settings.patternActivations;
    list.setCode("Zyyy", settings);

    auto* box = list.findChild<QCheckBox*>(QString::fromStdString("Letter I"));
    REQUIRE(box != nullptr);
    CHECK_FALSE(box->isChecked());

    // Left where the override put it, the box is not the user's word: it
    // neither reports the override again nor erases it, so folding — any
    // number of times — keeps exactly the one entry it opened on.
    CHECK(list.activations().empty());
    CHECK(list.shownRecords().empty());
    list.foldInto(settings.patternActivations);
    list.foldInto(settings.patternActivations);
    CHECK(settings.patternActivations == opened);
}

TEST_CASE("a name shared by two records of different codes shows a single box",
          "[gui][pattern-list]") {
    const PatternCatalogue catalogue = sharedNameAcrossCodes();
    PatternList list{catalogue, PatternKind::CommonError};

    list.setCode("Latn-en", CorrectionSettings{});

    const auto boxes = list.findChildren<QCheckBox*>(QString::fromStdString("Repeated"));
    CHECK(boxes.size() == 1);
}

TEST_CASE("unchecking a box shared by two records writes one activation per record",
          "[gui][pattern-list]") {
    const PatternCatalogue catalogue = sharedNameAcrossCodes();
    PatternList list{catalogue, PatternKind::CommonError};
    list.setCode("Latn-en", CorrectionSettings{});

    auto* box = list.findChild<QCheckBox*>(QString::fromStdString("Repeated"));
    REQUIRE(box != nullptr);
    box->toggle();

    auto activations = list.activations();
    REQUIRE(activations.size() == 2);
    std::ranges::sort(activations, {}, [](const auto& one) { return one.code; });
    CHECK(activations[0].code == "Latn-en");
    CHECK(activations[0].name == "Repeated");
    CHECK_FALSE(activations[0].enabled);
    CHECK(activations[1].code == "Zyyy");
    CHECK(activations[1].name == "Repeated");
    CHECK_FALSE(activations[1].enabled);
}

TEST_CASE("a box shared by two records opens checked when either record is enabled",
          "[gui][pattern-list]") {
    const PatternCatalogue catalogue = sharedNameAcrossCodes();
    PatternList list{catalogue, PatternKind::CommonError};

    CorrectionSettings settings;
    // Only the more specific record is turned off; the other keeps its
    // shipped default (on). The shared box still opens checked: it stands
    // for "any of them enabled".
    settings.patternActivations.push_back(subedit::core::PatternActivation{
        .kind = PatternKind::CommonError, .code = "Latn-en", .name = "Repeated", .enabled = false});
    list.setCode("Latn-en", settings);

    auto* box = list.findChild<QCheckBox*>(QString::fromStdString("Repeated"));
    REQUIRE(box != nullptr);
    CHECK(box->isChecked());
}

TEST_CASE("a box shared by two records opens checked even when the first one seen is off",
          "[gui][pattern-list]") {
    const PatternCatalogue catalogue = sharedNameAcrossCodes();
    PatternList list{catalogue, PatternKind::CommonError};

    CorrectionSettings settings;
    // The less specific record — `Zyyy`, seen first in the cascade — is the
    // one turned off this time: the shared box's `isChecked()` reads false
    // going into the second record, so only *its* own default (still on) can
    // decide the box, rather than short-circuiting on the first record's.
    settings.patternActivations.push_back(subedit::core::PatternActivation{
        .kind = PatternKind::CommonError, .code = "Zyyy", .name = "Repeated", .enabled = false});
    list.setCode("Latn-en", settings);

    auto* box = list.findChild<QCheckBox*>(QString::fromStdString("Repeated"));
    REQUIRE(box != nullptr);
    CHECK(box->isChecked());
}

TEST_CASE("setCode rebuilds the boxes for the newly requested cascade", "[gui][pattern-list]") {
    const PatternCatalogue catalogue = twoRecords();
    PatternList list{catalogue, PatternKind::CommonError};
    list.setCode("Zyyy", CorrectionSettings{});
    REQUIRE(list.findChild<QCheckBox*>(QString::fromStdString("Letter I")) != nullptr);

    list.setCode("Zyyy", CorrectionSettings{});

    // Rebuilding did not duplicate the boxes.
    CHECK(list.findChildren<QCheckBox*>(QString::fromStdString("Letter I")).size() == 1);
}

namespace {

/// Two records of the *same* code sharing a name — "Repeated", twice in
/// `Zyyy`, neither `Policy=Replace`: adjacent ranks, one box, and one key
/// (kind, code, name) between them.
PatternCatalogue sharedNameSameCode() {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Repeated\nClasses=OCR;\nPattern=a\n"
                  "\n[Common Error Pattern]\nName=Repeated\nClasses=OCR;\nPattern=b\n");
    return readPatternCatalogue(files, kShipped, {});
}

} // namespace

TEST_CASE("a name shared by two records of the same code writes a single activation",
          "[gui][pattern-list]") {
    const PatternCatalogue catalogue = sharedNameSameCode();
    REQUIRE(catalogue.cascade(PatternKind::CommonError, "Zyyy").size() == 2);
    PatternList list{catalogue, PatternKind::CommonError};
    list.setCode("Zyyy", CorrectionSettings{});

    CHECK(list.findChildren<QCheckBox*>(QString::fromStdString("Repeated")).size() == 1);

    auto* box = list.findChild<QCheckBox*>(QString::fromStdString("Repeated"));
    REQUIRE(box != nullptr);
    box->toggle();
    CHECK(list.shownRecords().size() == 2); // both records stand behind the one box

    // Both records carry the same key: one override covers both, and
    // folding it twice over does not pile up copies.
    const auto activations = list.activations();
    REQUIRE(activations.size() == 1);
    CHECK(activations[0].code == "Zyyy");
    CHECK_FALSE(activations[0].enabled);

    std::vector<subedit::core::PatternActivation> folded;
    list.foldInto(folded);
    list.foldInto(folded);
    CHECK(folded == activations);

    // Reopened on what was folded, as a task page does after every fold, the
    // box turned back to its default erases the one override again.
    CorrectionSettings reopened;
    reopened.patternActivations = folded;
    list.setCode("Zyyy", reopened);
    box = list.findChild<QCheckBox*>(QString::fromStdString("Repeated"));
    REQUIRE(box != nullptr);
    REQUIRE_FALSE(box->isChecked());
    box->toggle(); // back to its default
    list.foldInto(folded);
    CHECK(folded.empty());
}

TEST_CASE("folding a box turned back to its default erases the override it opened on",
          "[gui][pattern-list]") {
    const PatternCatalogue catalogue = twoRecords();
    PatternList list{catalogue, PatternKind::CommonError};

    CorrectionSettings settings;
    settings.patternActivations.push_back(subedit::core::PatternActivation{
        .kind = PatternKind::CommonError, .code = "Zyyy", .name = "Letter I", .enabled = false});
    list.setCode("Zyyy", settings);

    auto* box = list.findChild<QCheckBox*>(QString::fromStdString("Letter I"));
    REQUIRE(box != nullptr);
    REQUIRE_FALSE(box->isChecked());
    box->setChecked(true); // "Letter I" is on by default: this *is* its default again

    list.foldInto(settings.patternActivations);

    CHECK(settings.patternActivations.empty());
}

namespace {
const PatternCatalogue& shippedPatterns() {
    static const PatternCatalogue catalogue = [] {
        const subedit::core::RealFileSystem files;
        return readPatternCatalogue(files, SUBEDIT_PATTERNS_DIR, {});
    }();
    return catalogue;
}
} // namespace

TEST_CASE("an untouched shared box leaves a parent code's override alone when folded",
          "[gui][pattern-list]") {
    // The shipped `Latn` and `Latn-en` both hold "Space between number and
    // unit", neither replacing the other: under `Latn-en` one box stands for
    // both records.
    const PatternCatalogue& catalogue = shippedPatterns();
    PatternList list{catalogue, PatternKind::CommonError};
    CorrectionSettings settings;

    list.setCode("Latn", settings);
    auto* box = list.findChild<QCheckBox*>(QString::fromStdString("Space between number and unit"));
    REQUIRE(box != nullptr);
    REQUIRE(box->isChecked());
    box->setChecked(false);
    list.foldInto(settings.patternActivations);

    const subedit::core::PatternActivation latnOff{.kind = PatternKind::CommonError,
                                                   .code = "Latn",
                                                   .name = "Space between number and unit",
                                                   .enabled = false};
    REQUIRE(settings.patternActivations == std::vector{latnOff});

    list.setCode("Latn-en", settings);
    REQUIRE(list.findChildren<QCheckBox*>(QString::fromStdString("Space between number and unit"))
                .size() == 1);
    box = list.findChild<QCheckBox*>(QString::fromStdString("Space between number and unit"));
    REQUIRE(box != nullptr);
    REQUIRE(box->isChecked()); // the `Latn-en` record is still on

    list.foldInto(settings.patternActivations);

    CHECK(settings.patternActivations == std::vector{latnOff});
}
