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

#include <subedit/core/config/settings.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/encoding.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::ByteOrderMark;
using subedit::core::Encoding;
using subedit::core::FileError;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::InsertPlacement;
using subedit::core::readSettings;
using subedit::core::renderSettings;
using subedit::core::Settings;
using subedit::core::SettingsRead;
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

// ## Option à son défaut : réécrite commentée

TEST_CASE("an option at its default is written back commented out", "[config]") {
    const std::string written = renderSettings(Settings{});

    CHECK_THAT(written, ContainsSubstring("#window.geometry = "));
    CHECK_THAT(written, ContainsSubstring("#window.maximised = false"));
    CHECK_THAT(written, ContainsSubstring("#table.columns = "));
    CHECK_THAT(written, ContainsSubstring("#window.table-share = "));
    CHECK_THAT(written, ContainsSubstring("#file.directory = "));
    CHECK_THAT(written, ContainsSubstring("#general.theme = system"));
    CHECK_THAT(written, ContainsSubstring("#edit.insert-placement = below"));
    CHECK_THAT(written, ContainsSubstring("#file.write-encoding = UTF-8"));
    CHECK_THAT(written, ContainsSubstring("#file.write-bom = false"));
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
