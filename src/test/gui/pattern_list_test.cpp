// The pattern-list widget of a task page — issue #505, task 8. One tick box
// per pattern *name* a cascade holds: several records may share a name
// (`CorrectionPattern::name`'s own comment), and share a single box.

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/pattern_list.hpp>

#include <QCheckBox>
#include <QString>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>

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

TEST_CASE("an override in settings opens the box it names, and stays reported",
          "[gui][pattern-list]") {
    const PatternCatalogue catalogue = twoRecords();
    PatternList list{catalogue, PatternKind::CommonError};

    CorrectionSettings settings;
    settings.patternActivations.push_back(subedit::core::PatternActivation{
        .kind = PatternKind::CommonError, .code = "Zyyy", .name = "Letter I", .enabled = false});
    list.setCode("Zyyy", settings);

    auto* box = list.findChild<QCheckBox*>(QString::fromStdString("Letter I"));
    REQUIRE(box != nullptr);
    CHECK_FALSE(box->isChecked());

    // `activations()` compares against the record's *shipped* default, not
    // against the settings `setCode` opened on — so a box left where an
    // existing override put it still disagrees with the shipped default
    // ("Letter I" is on by default) and is reported, unchanged.
    const auto activations = list.activations();
    REQUIRE(activations.size() == 1);
    CHECK(activations[0].name == "Letter I");
    CHECK(activations[0].code == "Zyyy");
    CHECK_FALSE(activations[0].enabled);
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

TEST_CASE("setCode rebuilds the boxes for the newly requested cascade", "[gui][pattern-list]") {
    const PatternCatalogue catalogue = twoRecords();
    PatternList list{catalogue, PatternKind::CommonError};
    list.setCode("Zyyy", CorrectionSettings{});
    REQUIRE(list.findChild<QCheckBox*>(QString::fromStdString("Letter I")) != nullptr);

    list.setCode("Zyyy", CorrectionSettings{});

    // Rebuilding did not duplicate the boxes.
    CHECK(list.findChildren<QCheckBox*>(QString::fromStdString("Letter I")).size() == 1);
}
