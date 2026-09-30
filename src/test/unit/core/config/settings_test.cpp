// The persisted configuration — issue #240, decision of ADR 0022.
//
// **The six cases of the ADR's table have one test case each**, and they are
// the heart of the ticket: what is decided is not what the configuration keeps
// but what it does when the file does not say what one expects. A configuration
// is a comfort; its failing must cost the comfort and nothing else.
//
// Everything goes through `InMemoryFileSystem`: not one of these cases touches
// a real location, and it is the seam of the ADR that guarantees it — the
// configuration receives its path rather than looking for one.
//
// **No requirement tag here**, and for the reason of #154:
// `check-requirements.sh` reads the window and end-to-end binaries, so a
// `GUI-CONFIG-0N` written in a core test would be invisible to it — decoration
// rather than traceability. The three promises are cited where the window is
// put to the test, in `window_settings_test.cpp`.

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/config/duration_adjustment_settings.hpp>
#include <subedit/core/config/settings.hpp>
#include <subedit/core/config/spell_check_settings.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/encoding.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::ByteOrderMark;
using subedit::core::CorrectionSettings;
using subedit::core::DurationAdjustmentSettings;
using subedit::core::Encoding;
using subedit::core::FileError;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::InsertPlacement;
using subedit::core::PatternActivation;
using subedit::core::PatternKind;
using subedit::core::readSettings;
using subedit::core::renderSettings;
using subedit::core::Settings;
using subedit::core::SettingsRead;
using subedit::core::SpellCheckDocument;
using subedit::core::SpellCheckSettings;
using subedit::core::SpellCheckTarget;
using subedit::core::Theme;
using subedit::core::WindowGeometry;
using subedit::core::writeSettings;

constexpr const char* kPath = "/config/subedit/settings.conf";

[[nodiscard]] SettingsRead readOf(const std::string& content) {
    InMemoryFileSystem files;
    files.addFile(kPath, content);
    return readSettings(files, kPath);
}

/// A configuration with nothing at its default, for the cases that need a
/// round trip.
[[nodiscard]] Settings chosen() {
    return Settings{.geometry = WindowGeometry{.x = 40, .y = 60, .width = 1440, .height = 900},
                    .maximised = true,
                    .columnWidths = {50, 120, 120, 120},
                    .tableShare = 63,
                    .lastDirectory = std::filesystem::path{"/films/quai"},
                    .theme = Theme::Dark,
                    .insertPlacement = InsertPlacement::Above};
}

} // namespace

// ## A missing file: every default, and it is no error

TEST_CASE("a missing file gives every default, reporting nothing", "[config]") {
    const InMemoryFileSystem files;

    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings == Settings{});
    CHECK(read.diagnostics.empty());
    // The first launch has nothing to report: no setting was lost.
    CHECK_FALSE(read.unreadable.has_value());
}

// ## An unreadable file: every default, and a diagnostic

TEST_CASE("an unreadable file gives every default, and says so", "[config]") {
    InMemoryFileSystem files;
    files.addFile(kPath, "window.maximised = true\n");
    files.failNextRead(FileErrorKind::PermissionDenied);

    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings == Settings{});
    REQUIRE(read.unreadable.has_value());
    // `value_or` rather than a dereference: the static analysis does not read
    // the `REQUIRE` above as a guard, and the default of a `FileError` is
    // `NotFound`, which the comparison refuses.
    CHECK(read.unreadable.value_or(FileError{}).kind == FileErrorKind::PermissionDenied);
}

// ## An unknown key: ignored on reading, absent on rewriting

TEST_CASE("an unknown key is ignored without a word", "[config]") {
    // A file written by a version that knew more of them is not a faulty file:
    // it is the failure mode tolerance chooses.
    const SettingsRead read = readOf("window.maximised = true\n"
                                     "window.opacity = 0.5\n"
                                     "table.font = DejaVu Sans\n");

    CHECK(read.settings.maximised);
    CHECK(read.diagnostics.empty());
}

TEST_CASE("an unknown key is not written back", "[config]") {
    InMemoryFileSystem files;
    files.addFile(kPath, "window.opacity = 0.5\n");

    const SettingsRead read = readSettings(files, kPath);
    REQUIRE(writeSettings(files, kPath, read.settings).has_value());

    CHECK(files.contentOf(kPath).value_or("").find("opacity") == std::string::npos);
}

// ## An unreadable value: the default is kept, a diagnostic is produced

TEST_CASE("an unreadable value leaves the default in place, and says so", "[config]") {
    const SettingsRead read = readOf("window.geometry = plus tard\n");

    CHECK_FALSE(read.settings.geometry.has_value());
    REQUIRE(read.diagnostics.size() == 1);
    CHECK(read.diagnostics.front().key == "window.geometry");
    CHECK(read.diagnostics.front().value == "plus tard");
}

TEST_CASE("an unreadable value does not carry off the other options", "[config]") {
    // The diagnostic accompanies the default rather than replacing the file:
    // what could be read still reads.
    const SettingsRead read = readOf("window.geometry = 0,0,0,0\n"
                                     "window.maximised = true\n");

    CHECK_FALSE(read.settings.geometry.has_value());
    CHECK(read.settings.maximised);
    CHECK(read.diagnostics.size() == 1);
}

TEST_CASE("column widths are refused in any number but four", "[config]") {
    // Three widths for four settable columns: guessing which one is missing
    // would be guessing what the user wanted.
    CHECK(readOf("table.columns = 50,120,120\n").diagnostics.size() == 1);
    CHECK(readOf("table.columns = 50,120,120,120,700\n").diagnostics.size() == 1);
    CHECK(readOf("table.columns = 50,120,120,120\n").diagnostics.empty());
}

TEST_CASE("the two boolean values read, and they alone", "[config]") {
    // `false` as much as `true`: it is the default, so nobody writes it of
    // their own accord — and a value no test reads is a value one does not know
    // reads at all.
    CHECK(readOf("window.maximised = true\n").settings.maximised);
    CHECK_FALSE(readOf("window.maximised = false\n").settings.maximised);
    CHECK(readOf("window.maximised = false\n").diagnostics.empty());
}

TEST_CASE("a null or negative column width is unreadable", "[config]") {
    // A column zero wide is a column one would not find again, and a negative
    // width does not exist: neither is a width the user laid down.
    CHECK(readOf("table.columns = 50,0,120,120\n").diagnostics.size() == 1);
    CHECK(readOf("table.columns = 50,-10,120,120\n").diagnostics.size() == 1);
}

TEST_CASE("a line without an equals sign is ignored", "[config]") {
    // It is not an option, so it is not an unreadable option: nothing is
    // reported, and the rest of the file reads.
    const SettingsRead read = readOf("ceci n'est pas une option\n"
                                     "window.maximised = true\n");

    CHECK(read.settings.maximised);
    CHECK(read.diagnostics.empty());
}

TEST_CASE("anything trailing after a number makes it unreadable", "[config]") {
    // "12 pixels" is not a twelve followed by noise: it is a value that could
    // not be read, and accepting it would be accepting anything at all.
    CHECK(readOf("window.maximised = oui\n").diagnostics.size() == 1);
    CHECK(readOf("table.columns = 50,120,120,120 px\n").diagnostics.size() == 1);
}

// ## The three options that came with #254 and #241

TEST_CASE("the three themes read, and nothing else", "[config]") {
    CHECK(readOf("general.theme = system\n").settings.theme == Theme::System);
    CHECK(readOf("general.theme = light\n").settings.theme == Theme::Light);
    CHECK(readOf("general.theme = dark\n").settings.theme == Theme::Dark);

    // A theme nobody knows leaves the one that does nothing.
    const SettingsRead unknown = readOf("general.theme = solarized\n");
    CHECK(unknown.settings.theme == Theme::System);
    CHECK(unknown.diagnostics.size() == 1);
}

TEST_CASE("the three themes are written back as they read", "[config]") {
    // The round trip of all three, and not only of the one picked in the other
    // cases: a value that writes and does not read back would be a preference
    // lost at the next restart.
    for (const Theme theme : {Theme::System, Theme::Light, Theme::Dark}) {
        InMemoryFileSystem files;
        REQUIRE(writeSettings(files, kPath, Settings{.theme = theme}).has_value());

        CHECK(readSettings(files, kPath).settings.theme == theme);
    }
}

// ## The side of an insertion, which came with #242

TEST_CASE("the two sides of an insertion read, and nothing else", "[config]") {
    CHECK(readOf("edit.insert-placement = above\n").settings.insertPlacement ==
          InsertPlacement::Above);
    CHECK(readOf("edit.insert-placement = below\n").settings.insertPlacement ==
          InsertPlacement::Below);

    // A side nobody knows leaves Gaupol's, which is ours.
    const SettingsRead unknown = readOf("edit.insert-placement = sideways\n");
    CHECK(unknown.settings.insertPlacement == InsertPlacement::Below);
    CHECK(unknown.diagnostics.size() == 1);
}

// ## The encoding last written, which came with #299

TEST_CASE("the encoding last written reads, under any of its names", "[config]") {
    // ICU's name, aliases included: what the file carries is the one it
    // settles on, and reading it back under another name gives the same
    // value.
    CHECK(readOf("file.write-encoding = windows-1252\n").settings.writeEncoding ==
          Encoding::create("cp1252", ByteOrderMark::Absent).value());

    // A name ICU cannot convert is an unreadable setting: the default stays,
    // and it is reported like the others.
    const SettingsRead unknown = readOf("file.write-encoding = klingon-1\n");
    CHECK_FALSE(unknown.settings.writeEncoding.has_value());
    CHECK(unknown.diagnostics.size() == 1);
}

TEST_CASE("the mark travels with the encoding it belongs to", "[config]") {
    // **Two keys for one value** — issue #317. `-sig` stopped being a name in
    // #315, so the file keeps the name apart from the mark the way the command
    // line keeps `--encoding` apart from `--bom`. What is checked here is that
    // the two go back together: `UTF-8` chosen with its mark came back without
    // it, from one session to the next.
    const SettingsRead marked = readOf("file.write-encoding = UTF-8\nfile.write-bom = true\n");
    CHECK(marked.settings.writeEncoding == Encoding::utf8(ByteOrderMark::Present));

    // **The order of the two lines is nobody's to command** — a file edited by
    // hand puts them where it likes — so the mark goes on once all of it is
    // read.
    const SettingsRead reversed = readOf("file.write-bom = true\nfile.write-encoding = UTF-8\n");
    CHECK(reversed.settings.writeEncoding == Encoding::utf8(ByteOrderMark::Present));

    // Without the key, no mark: that is what seven versions wrote.
    CHECK(readOf("file.write-encoding = UTF-8\n").settings.writeEncoding ==
          Encoding::utf8(ByteOrderMark::Absent));

    // A mark with no encoding puts nothing anywhere: the encoding carries it.
    CHECK_FALSE(readOf("file.write-bom = true\n").settings.writeEncoding.has_value());
}

TEST_CASE("the encoding last written is written back as it read", "[config]") {
    InMemoryFileSystem files;
    const std::expected<Encoding, subedit::core::EncodingRefusal> central =
        Encoding::create("windows-1250", ByteOrderMark::Absent);
    REQUIRE(central.has_value());
    REQUIRE(writeSettings(files, kPath, Settings{.writeEncoding = *central}).has_value());

    CHECK(readSettings(files, kPath).settings.writeEncoding == *central);
}

TEST_CASE("an encoding chosen with its mark is found again with it", "[config]") {
    InMemoryFileSystem files;
    const Encoding marked = Encoding::utf8(ByteOrderMark::Present);
    REQUIRE(writeSettings(files, kPath, Settings{.writeEncoding = marked}).has_value());

    CHECK(readSettings(files, kPath).settings.writeEncoding == marked);
}

TEST_CASE("the two sides are written back as they read", "[config]") {
    for (const InsertPlacement placement : {InsertPlacement::Above, InsertPlacement::Below}) {
        InMemoryFileSystem files;
        REQUIRE(writeSettings(files, kPath, Settings{.insertPlacement = placement}).has_value());

        CHECK(readSettings(files, kPath).settings.insertPlacement == placement);
    }
}

TEST_CASE("the share given to the table refuses both its extremes", "[config]") {
    // Neither zero nor a hundred: a table of no height, or a video band of no
    // height, is a window one could no longer reopen otherwise than by deleting
    // its configuration file.
    CHECK(readOf("window.table-share = 0\n").diagnostics.size() == 1);
    CHECK(readOf("window.table-share = 100\n").diagnostics.size() == 1);
    CHECK(readOf("window.table-share = 1\n").settings.tableShare == 1);
    CHECK(readOf("window.table-share = 99\n").settings.tableShare == 99);
    CHECK(readOf("window.table-share = deux tiers\n").diagnostics.size() == 1);
}

TEST_CASE("a relative directory is unreadable, an absolute one is not", "[config]") {
    // A relative path is relative to a working directory nobody knows: it is
    // not a path to complete on a hunch.
    CHECK(readOf("file.directory = ../films\n").diagnostics.size() == 1);
    CHECK_FALSE(readOf("file.directory = ../films\n").settings.lastDirectory.has_value());

    CHECK(readOf("file.directory = /films/quai\n").settings.lastDirectory ==
          std::filesystem::path{"/films/quai"});
}

// ## A missing option: the default, without it being a special case

TEST_CASE("a missing option is worth its default", "[config]") {
    const SettingsRead read = readOf("window.maximised = true\n");

    CHECK(read.settings.maximised);
    CHECK_FALSE(read.settings.geometry.has_value());
    CHECK(read.settings.columnWidths.empty());
    CHECK(read.diagnostics.empty());
}

// ## The columns: their order and those taken away — issue #442

TEST_CASE("an order of the columns is every column once, by name", "[config]") {
    using subedit::core::TableColumn;

    const SettingsRead read = readOf("table.order = text, number,start,end,duration,translation\n");

    CHECK(read.diagnostics.empty());
    CHECK(read.settings.columnOrder == std::vector<TableColumn>{TableColumn::Text,
                                                                TableColumn::Number,
                                                                TableColumn::Start,
                                                                TableColumn::End,
                                                                TableColumn::Duration,
                                                                TableColumn::Translation});
}

TEST_CASE("an order that misses a column, repeats one or names none is refused", "[config]") {
    CHECK(readOf("table.order = number,start,end,duration,text\n").diagnostics.size() == 1);
    CHECK(
        readOf("table.order = number,number,end,duration,text,translation\n").diagnostics.size() ==
        1);
    CHECK(readOf("table.order = number,start,end,duration,text,notes\n").diagnostics.size() == 1);
    CHECK(readOf("table.order = number,start,end,duration,text,notes\n")
              .settings.columnOrder.empty());
}

TEST_CASE("the hidden columns are among the four that may be hidden", "[config]") {
    using subedit::core::TableColumn;

    CHECK(readOf("table.hidden = start,duration\n").settings.hiddenColumns ==
          std::vector<TableColumn>{TableColumn::Start, TableColumn::Duration});
    CHECK(readOf("table.hidden =\n").settings.hiddenColumns.empty());
    CHECK(readOf("table.hidden =\n").diagnostics.empty());
    // The text can never be taken away, and the translation follows a rule of
    // its own.
    CHECK(readOf("table.hidden = text\n").diagnostics.size() == 1);
    CHECK(readOf("table.hidden = translation\n").diagnostics.size() == 1);
    CHECK(readOf("table.hidden = start,start\n").diagnostics.size() == 1);
}

TEST_CASE("the order and the hidden columns go round the file", "[config]") {
    using subedit::core::TableColumn;
    Settings settings;
    settings.columnOrder = {TableColumn::Number,
                            TableColumn::Text,
                            TableColumn::Start,
                            TableColumn::End,
                            TableColumn::Duration,
                            TableColumn::Translation};
    settings.hiddenColumns = {TableColumn::Duration};

    const std::string written = renderSettings(settings);
    CHECK_THAT(written,
               ContainsSubstring("\ntable.order = number,text,start,end,duration,translation\n"));
    CHECK_THAT(written, ContainsSubstring("\ntable.hidden = duration\n"));

    const SettingsRead read = readOf(written);
    CHECK(read.diagnostics.empty());
    CHECK(read.settings.columnOrder == settings.columnOrder);
    CHECK(read.settings.hiddenColumns == settings.hiddenColumns);
}

// ## Option à son défaut : réécrite commentée

TEST_CASE("an option at its default is written back commented out", "[config]") {
    const std::string written = renderSettings(Settings{});

    CHECK_THAT(written, ContainsSubstring("#window.geometry = "));
    CHECK_THAT(written, ContainsSubstring("#window.maximised = false"));
    CHECK_THAT(written, ContainsSubstring("#table.columns = "));
    CHECK_THAT(written,
               ContainsSubstring("#table.order = number,start,end,duration,text,translation"));
    CHECK_THAT(written, ContainsSubstring("#table.hidden = "));
    CHECK_THAT(written, ContainsSubstring("#window.table-share = "));
    CHECK_THAT(written, ContainsSubstring("#file.directory = "));
    CHECK_THAT(written, ContainsSubstring("#general.theme = system"));
    CHECK_THAT(written, ContainsSubstring("#edit.insert-placement = below"));
    CHECK_THAT(written, ContainsSubstring("#file.write-encoding = UTF-8"));
    CHECK_THAT(written, ContainsSubstring("#file.write-bom = false"));
    CHECK_THAT(written, ContainsSubstring("#search.regex = false"));
    CHECK_THAT(written, ContainsSubstring("#search.ignore-case = true"));
    CHECK_THAT(written, ContainsSubstring("#duration-adjust.speed = 15\n"));
    CHECK_THAT(written, ContainsSubstring("#duration-adjust.lengthen = true"));
    CHECK_THAT(written, ContainsSubstring("#duration-adjust.shorten = false"));
    CHECK_THAT(written, ContainsSubstring("#duration-adjust.minimum-enabled = true"));
    CHECK_THAT(written, ContainsSubstring("#duration-adjust.minimum-ms = 1500"));
    CHECK_THAT(written, ContainsSubstring("#duration-adjust.maximum-enabled = false"));
    CHECK_THAT(written, ContainsSubstring("#duration-adjust.maximum-ms = 6000"));
    CHECK_THAT(written, ContainsSubstring("#duration-adjust.gap-enabled = true"));
    CHECK_THAT(written, ContainsSubstring("#duration-adjust.gap-ms = 0"));
}

TEST_CASE("an option that was set is written back bare", "[config]") {
    const std::string written = renderSettings(chosen());

    CHECK_THAT(written, ContainsSubstring("\nwindow.geometry = 40,60,1440,900\n"));
    CHECK_THAT(written, ContainsSubstring("\nwindow.maximised = true\n"));
    CHECK_THAT(written, ContainsSubstring("\ntable.columns = 50,120,120,120\n"));
    CHECK_THAT(written, ContainsSubstring("\nwindow.table-share = 63\n"));
    CHECK_THAT(written, ContainsSubstring("\nfile.directory = /films/quai\n"));
    CHECK_THAT(written, ContainsSubstring("\ngeneral.theme = dark\n"));
    CHECK_THAT(written, ContainsSubstring("\nedit.insert-placement = above\n"));
}

// **What writing an option commented out buys**, and the reason it is no
// affectation: an option never touched is not frozen at the value of the day it
// was written. Reading back what was just written therefore gives the defaults
// again, and not frozen values.
TEST_CASE("an option at its default reads back as the default after a rewrite", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(writeSettings(files, kPath, Settings{}).has_value());

    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings == Settings{});
    CHECK(read.diagnostics.empty());
}

TEST_CASE("what was set reads back unchanged", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(writeSettings(files, kPath, chosen()).has_value());

    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings == chosen());
    CHECK(read.diagnostics.empty());
}

// ## What writing asks of the file system

TEST_CASE("writing makes the directory nobody made", "[config]") {
    // At the first launch, `~/.config/subedit` does not exist.
    InMemoryFileSystem files;

    CHECK(writeSettings(files, kPath, chosen()).has_value());
    CHECK(files.contentOf(kPath).has_value());
}

TEST_CASE("a refused write is returned, and stops nothing", "[config]") {
    InMemoryFileSystem files;
    files.failNextWrite(FileErrorKind::PermissionDenied);

    CHECK_FALSE(writeSettings(files, kPath, chosen()).has_value());
}

TEST_CASE("the written file explains itself", "[config]") {
    // It is meant to be opened in an editor: without these lines, a reader
    // coming across commented-out options takes them for leftovers.
    const std::string written = renderSettings(Settings{});

    CHECK(written.starts_with("# subedit settings."));
    CHECK_THAT(written, ContainsSubstring("commented out"));
}

// ## The two options of a search, which came with #384

TEST_CASE("the two options of a search read, and are kept", "[config]") {
    const SettingsRead read = readOf("search.regex = true\nsearch.ignore-case = false\n");
    CHECK(read.settings.search.regex);
    CHECK_FALSE(read.settings.search.ignoreCase);
    CHECK(read.diagnostics.empty());

    // Unreadable, the default stays — Gaupol's: plain text, the case ignored.
    const SettingsRead unreadable = readOf("search.ignore-case = peut-etre\n");
    CHECK(unreadable.settings.search.ignoreCase);
    CHECK(unreadable.diagnostics.size() == 1);

    InMemoryFileSystem files;
    const subedit::core::SearchOptions chosenOptions{.regex = true, .ignoreCase = false};
    REQUIRE(writeSettings(files, kPath, Settings{.search = chosenOptions}).has_value());
    CHECK(readSettings(files, kPath).settings.search == chosenOptions);
    CHECK_THAT(renderSettings(Settings{.search = chosenOptions}),
               ContainsSubstring("\nsearch.regex = true"));
}

// ## The form of `Adjust Durations…`, which came with #409

namespace {

/// A form with nothing at its default, and a speed that needs a decimal point.
[[nodiscard]] DurationAdjustmentSettings chosenAdjustment() {
    return DurationAdjustmentSettings{.charactersPerSecond = 12.5,
                                      .lengthen = false,
                                      .shorten = true,
                                      .minimumEnabled = false,
                                      .minimumMilliseconds = 2000,
                                      .maximumEnabled = true,
                                      .maximumMilliseconds = 4250,
                                      .gapEnabled = false,
                                      .gapMilliseconds = 80};
}

} // namespace

TEST_CASE("the form of the duration adjustment is kept across sessions", "[config]") {
    InMemoryFileSystem files;
    const Settings written{.durationAdjustment = chosenAdjustment()};

    REQUIRE(writeSettings(files, kPath, written).has_value());
    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings.durationAdjustment == chosenAdjustment());
    CHECK(read.diagnostics.empty());
}

TEST_CASE("a file that does not mention the duration adjustment gives its defaults", "[config]") {
    const SettingsRead read = readOf("window.maximised = true\n");

    CHECK(read.settings.durationAdjustment == DurationAdjustmentSettings{});
    CHECK(read.diagnostics.empty());
}

TEST_CASE("the reading speed is written in its shortest form, and read back exactly", "[config]") {
    const std::string fifteen = renderSettings(Settings{});
    CHECK_THAT(fifteen, ContainsSubstring("#duration-adjust.speed = 15\n"));

    for (const double speed : {12.5, 0.1, 22.75, 99.0}) {
        InMemoryFileSystem files;
        const Settings written{.durationAdjustment = {.charactersPerSecond = speed}};
        REQUIRE(writeSettings(files, kPath, written).has_value());
        CHECK(readSettings(files, kPath).settings.durationAdjustment.charactersPerSecond == speed);
    }

    CHECK_THAT(renderSettings(Settings{.durationAdjustment = chosenAdjustment()}),
               ContainsSubstring("\nduration-adjust.speed = 12.5\n"));
}

TEST_CASE("a value of the duration adjustment that cannot be read leaves its default", "[config]") {
    // A speed of zero or less is not a speed, a negative duration is not a
    // duration, and neither is text: each is named, and the default stays.
    for (const char* line : {"duration-adjust.speed = fast\n",
                             "duration-adjust.speed = 0\n",
                             "duration-adjust.speed = -3\n",
                             "duration-adjust.speed = nan\n",
                             "duration-adjust.speed = inf\n",
                             "duration-adjust.lengthen = maybe\n",
                             "duration-adjust.minimum-ms = -1\n",
                             "duration-adjust.minimum-ms = 1.5\n",
                             "duration-adjust.maximum-ms = -6000\n",
                             "duration-adjust.gap-ms = soon\n",
                             "duration-adjust.gap-ms = -80\n"}) {
        const SettingsRead read = readOf(line);

        CHECK(read.settings.durationAdjustment == DurationAdjustmentSettings{});
        CHECK(read.diagnostics.size() == 1);
    }

    // One bad value does not spare the good ones around it.
    const SettingsRead mixed =
        readOf("duration-adjust.speed = 0\nduration-adjust.minimum-ms = 2000\n");
    CHECK(mixed.settings.durationAdjustment.charactersPerSecond == 15.0);
    CHECK(mixed.settings.durationAdjustment.minimumMilliseconds == 2000);
    REQUIRE(mixed.diagnostics.size() == 1);
    CHECK(mixed.diagnostics.front().key == "duration-adjust.speed");
}

TEST_CASE("a duration of zero is a duration, and a speed above ninety-nine is read", "[config]") {
    // Zero milliseconds is a real minimum and a real gap. The speed's upper
    // bound is the dialog's business: what the file says is kept, and the box
    // clamps what it shows.
    const SettingsRead read =
        readOf("duration-adjust.minimum-ms = 0\nduration-adjust.speed = 120\n");

    CHECK(read.settings.durationAdjustment.minimumMilliseconds == 0);
    CHECK(read.settings.durationAdjustment.charactersPerSecond == 120.0);
    CHECK(read.diagnostics.empty());
}

// ## `Correct Texts…` — issue #504, decision D8

namespace {

[[nodiscard]] CorrectionSettings chosenCorrection() {
    return CorrectionSettings{
        .mentions = {.enabled = true, .code = "Latn-en"},
        .commonErrors = {.enabled = false, .code = "Latn-fr"},
        .capitalization = {.enabled = false, .code = "Latn"},
        .lineBreak = {.enabled = true, .code = "Latn-en-US"},
        .human = false,
        .ocr = false,
        .soundInBrackets = true,
        .soundInParentheses = true,
        .patternActivations = {PatternActivation{.kind = PatternKind::CommonError,
                                                 .code = "Latn-en",
                                                 .name = "I majuscule",
                                                 .enabled = false},
                               PatternActivation{.kind = PatternKind::HearingImpaired,
                                                 .code = "Latn-en",
                                                 .name = "Speaker before a colon",
                                                 .enabled = true},
                               PatternActivation{.kind = PatternKind::Capitalization,
                                                 .code = "Latn",
                                                 .name = "Sentence start",
                                                 .enabled = false},
                               PatternActivation{.kind = PatternKind::LineBreak,
                                                 .code = "Latn-en",
                                                 .name = "Dialogue dash",
                                                 .enabled = false}},
        .lineBreakMaxLength = 32.5,
        .lineBreakMaxLines = 2,
        .lineBreakInEms = false,
        .lineBreakSkipOnLength = false,
        .lineBreakSkipMaxLength = 30.5,
        .lineBreakSkipOnLines = false,
        .lineBreakSkipMaxLines = 4,
        .joinSplitEnabled = true,
        .joinWords = false,
        .splitWords = true,
        .spellLanguage = "fr_FR",
        .removeBlankSubtitles = false,
    };
}

} // namespace

TEST_CASE("the correction assistant's settings are kept across sessions", "[config]") {
    InMemoryFileSystem files;
    const Settings written{.correction = chosenCorrection()};

    REQUIRE(writeSettings(files, kPath, written).has_value());
    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings.correction == chosenCorrection());
    CHECK(read.diagnostics.empty());
}

TEST_CASE("a file that does not mention the correction assistant gives its defaults", "[config]") {
    const SettingsRead read = readOf("window.maximised = true\n");

    CHECK(read.settings.correction == CorrectionSettings{});
    CHECK(read.diagnostics.empty());
}

TEST_CASE("an option of the correction assistant at its default is written back commented out",
          "[config]") {
    const std::string rendered = renderSettings(Settings{});

    CHECK_THAT(rendered, ContainsSubstring("#correction.common-errors.enabled = true\n"));
    CHECK_THAT(rendered, ContainsSubstring("#correction.human = true\n"));
    CHECK_THAT(rendered, ContainsSubstring("#correction.activations = \n"));

    CHECK_THAT(renderSettings(Settings{.correction = chosenCorrection()}),
               ContainsSubstring("\ncorrection.human = false\n"));
}

TEST_CASE("a value of the correction assistant that cannot be read leaves its default",
          "[config]") {
    for (const char* line : {"correction.mentions.enabled = maybe\n",
                             "correction.mentions.code = \n",
                             "correction.human = perhaps\n",
                             "correction.line-break.max-length = wide\n",
                             "correction.line-break.max-lines = two\n",
                             "correction.line-break.in-ems = maybe\n",
                             "correction.line-break.skip-max-length = wide\n",
                             "correction.line-break.skip-max-lines = two\n",
                             "correction.join-split.enabled = maybe\n",
                             "correction.join-split.language = French\n"}) {
        const SettingsRead read = readOf(line);

        CHECK(read.settings.correction == CorrectionSettings{});
        CHECK(read.diagnostics.size() == 1);
    }
}

TEST_CASE("an activation naming a pattern that no longer exists changes nothing it could find",
          "[config]") {
    // Decision D2: a name a shipped file no longer carries is ignored, like
    // every other setting ADR 0022 keeps — this settings layer does not read
    // the catalogue at all, so there is nothing to compare against, and the
    // entry is simply kept, unread by anything that would act on it.
    const SettingsRead read =
        readOf("correction.activations = common-error:Latn-xx:Gone now:false\n");

    REQUIRE(read.settings.correction.patternActivations.size() == 1);
    CHECK(read.settings.correction.patternActivations.front().name == "Gone now");
    CHECK(read.diagnostics.empty());
}

TEST_CASE("an activation entry short of its four fields is not read", "[config]") {
    for (const char* entry :
         {"common-error", "common-error:Latn-en", "common-error:Latn-en:Name"}) {
        const SettingsRead read = readOf(std::string{"correction.activations = "} + entry + "\n");

        CHECK(read.settings.correction.patternActivations.empty());
        REQUIRE(read.diagnostics.size() == 1);
        CHECK(read.diagnostics.front().key == "correction.activations");
    }
}

TEST_CASE("an empty activation list reads as no activation at all", "[config]") {
    const SettingsRead read = readOf("correction.activations = \n");

    CHECK(read.settings.correction.patternActivations.empty());
    CHECK(read.diagnostics.empty());
}

TEST_CASE("one unreadable entry of the activation list refuses the whole list", "[config]") {
    // The same all-or-nothing rule a column order already keeps: a list is
    // one option, and a list half read is a list nobody asked for.
    const SettingsRead read =
        readOf("correction.activations = common-error:Latn-en:Ligature ff:true,not-a-kind:Zyyy:X:"
               "true\n");

    CHECK(read.settings.correction.patternActivations.empty());
    REQUIRE(read.diagnostics.size() == 1);
    CHECK(read.diagnostics.front().key == "correction.activations");
}

TEST_CASE("the activation list round-trips through the file, in order", "[config]") {
    InMemoryFileSystem files;
    const Settings written{.correction = chosenCorrection()};
    REQUIRE(writeSettings(files, kPath, written).has_value());

    CHECK(readSettings(files, kPath).settings.correction.patternActivations ==
          chosenCorrection().patternActivations);
}

// ## `Check Spelling…` — issue #509

TEST_CASE("the spell check's settings are kept across sessions", "[config]") {
    InMemoryFileSystem files;
    const Settings written{.spellCheck = {.language = "fr_FR",
                                          .target = SpellCheckTarget::AllProjects,
                                          .document = SpellCheckDocument::Translation,
                                          .inlineCheck = true}};

    REQUIRE(writeSettings(files, kPath, written).has_value());
    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings.spellCheck == written.spellCheck);
    CHECK(read.settings.spellCheck.inlineCheck);
    CHECK(read.diagnostics.empty());
}

TEST_CASE("a file that does not mention the spell check gives its defaults", "[config]") {
    const SettingsRead read = readOf("window.maximised = true\n");

    CHECK(read.settings.spellCheck == SpellCheckSettings{});
    CHECK(read.settings.spellCheck.target == SpellCheckTarget::CurrentProject);
    CHECK(read.settings.spellCheck.document == SpellCheckDocument::Main);
    CHECK_FALSE(read.settings.spellCheck.inlineCheck); // GUI-SPELL-04: off, as in Gaupol
    CHECK(read.diagnostics.empty());
}

TEST_CASE("the spell check's options at their default are written back commented out", "[config]") {
    const std::string rendered = renderSettings(Settings{});

    CHECK_THAT(rendered, ContainsSubstring("#spell-check.language = \n"));
    CHECK_THAT(rendered, ContainsSubstring("#spell-check.target = current-project\n"));
    CHECK_THAT(rendered, ContainsSubstring("#spell-check.document = main\n"));
    CHECK_THAT(rendered, ContainsSubstring("#spell-check.inline = false\n"));

    CHECK_THAT(renderSettings(Settings{.spellCheck = {.target = SpellCheckTarget::Selection}}),
               ContainsSubstring("\nspell-check.target = selection\n"));
}

TEST_CASE("every named value of the spell check's target is read", "[config]") {
    const std::vector<std::pair<const char*, SpellCheckTarget>> targets = {
        {"spell-check.target = selection\n", SpellCheckTarget::Selection},
        {"spell-check.target = current-project\n", SpellCheckTarget::CurrentProject},
        {"spell-check.target = all-projects\n", SpellCheckTarget::AllProjects},
    };
    for (const auto& [line, target] : targets) {
        const SettingsRead read = readOf(line);

        CHECK(read.settings.spellCheck.target == target);
        CHECK(read.diagnostics.empty());
    }
}

TEST_CASE("every named value of the spell check's document is read", "[config]") {
    const std::vector<std::pair<const char*, SpellCheckDocument>> documents = {
        {"spell-check.document = main\n", SpellCheckDocument::Main},
        {"spell-check.document = translation\n", SpellCheckDocument::Translation},
    };
    for (const auto& [line, document] : documents) {
        const SettingsRead read = readOf(line);

        CHECK(read.settings.spellCheck.document == document);
        CHECK(read.diagnostics.empty());
    }
}

TEST_CASE("a value of the spell check that cannot be read leaves its default", "[config]") {
    for (const char* line : {"spell-check.language = French\n",
                             "spell-check.target = everything\n",
                             "spell-check.document = notes\n",
                             "spell-check.inline = maybe\n"}) {
        const SettingsRead read = readOf(line);

        CHECK(read.settings.spellCheck == SpellCheckSettings{});
        REQUIRE(read.diagnostics.size() == 1);
        CHECK_FALSE(read.diagnostics.front().value.empty());
    }
}
