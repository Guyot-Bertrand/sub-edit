// `config list|show|set|unset`, run against an in-memory file system.

#include <subedit/cli/configuring.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/config/operation_settings.hpp>
#include <subedit/core/config/settings.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::ExitCode;
using subedit::cli::listSettingKeys;
using subedit::cli::Reporter;
using subedit::cli::setSetting;
using subedit::cli::showDefaultSettings;
using subedit::cli::showSettings;
using subedit::cli::unsetSetting;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;

namespace {

const std::filesystem::path kPath = "/c/subedit/settings.conf";

} // namespace

TEST_CASE("config list names every catalogued key and no value", "[config]") {
    std::ostringstream out;
    std::ostringstream err;
    const Reporter reporter = Reporter{err, 1}.withTextOutput(out);

    CHECK(listSettingKeys(reporter) == ExitCode::Success);

    for (const auto& key : subedit::core::operationKeys())
        CHECK_THAT(out.str(), ContainsSubstring(std::string{key.key} + "\n"));
    CHECK_THAT(out.str(),
               ContainsSubstring("duration-adjust.speed\n"
                                 "    Reading speed the durations are brought to.\n"
                                 "    Accepts: a number of characters per second, above 0, "
                                 "decimals allowed\n"
                                 "    Default: 15\n"
                                 "    Used by: subedit-cli adjust; Adjust Durations…\n"));
    CHECK_THAT(out.str(), !ContainsSubstring(" = "));
    CHECK(err.str().empty());
}

TEST_CASE("config list in json writes one record per key", "[config]") {
    std::ostringstream records;
    std::ostringstream err;
    const Reporter reporter = Reporter{err, 1}.withRecords(records).forCommand("config");

    CHECK(listSettingKeys(reporter) == ExitCode::Success);

    const std::string text = records.str();
    CHECK(static_cast<std::size_t>(std::ranges::count(text, '\n')) ==
          subedit::core::operationKeys().size());
    CHECK_THAT(text,
               ContainsSubstring(
                   R"({"schema":1,"command":"config","key":"duration-adjust.speed",)"
                   R"("description":"Reading speed the durations are brought to.",)"
                   R"("accepts":"a number of characters per second, above 0, decimals allowed",)"
                   R"("default":"15","used_by":"subedit-cli adjust; Adjust Durations…",)"
                   R"("surface":"both"})"));
    CHECK_THAT(text, ContainsSubstring(R"("key":"correction.mentions.enabled",)"));
    CHECK_THAT(text, ContainsSubstring(R"("surface":"window-only"})"));
}

TEST_CASE("config show says where it looked and where each value comes from", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(files
                .writeFile(
                    kPath,
                    "duration-adjust.speed = 12\ntable.columns = nonsense\ngeneral.theme = dark\n")
                .has_value());
    std::ostringstream out;
    std::ostringstream err;
    const Reporter reporter = Reporter{err, 1}.withTextOutput(out);

    CHECK(showSettings(files, kPath, reporter) == ExitCode::Success);

    const std::string text = out.str();
    CHECK(text.starts_with("File: /c/subedit/settings.conf (found)\n"));
    CHECK_THAT(text, ContainsSubstring("\nduration-adjust.speed = 12   [file]   (default 15)\n"));
    CHECK_THAT(text, ContainsSubstring("\nduration-adjust.gap-ms = 0   [default]\n"));
    CHECK_THAT(text, ContainsSubstring("\ncorrection.mentions.enabled = "));
    CHECK_THAT(text, !ContainsSubstring("table."));
    CHECK_THAT(text, !ContainsSubstring("general.theme"));
    CHECK_THAT(text, !ContainsSubstring("window."));
}

TEST_CASE("config show on a missing file says not found and shows defaults", "[config]") {
    InMemoryFileSystem files;
    std::ostringstream out;
    std::ostringstream err;
    const Reporter reporter = Reporter{err, 1}.withTextOutput(out);

    CHECK(showSettings(files, kPath, reporter) == ExitCode::Success);

    const std::string text = out.str();
    CHECK(text.starts_with("File: /c/subedit/settings.conf (not found)\n"));
    CHECK_THAT(text, ContainsSubstring("\nduration-adjust.speed = 15   [default]\n"));
    CHECK_THAT(text, !ContainsSubstring("[file]"));
    CHECK(files.fileCount() == 0);
}

TEST_CASE("config show in json writes the file first, then one record per key", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "duration-adjust.speed = 12\n").has_value());
    std::ostringstream records;
    std::ostringstream err;
    const Reporter reporter = Reporter{err, 1}.withRecords(records).forCommand("config");

    CHECK(showSettings(files, kPath, reporter) == ExitCode::Success);

    const std::string text = records.str();
    CHECK(text.starts_with(
        R"({"schema":1,"command":"config","file":"/c/subedit/settings.conf","exists":true})"
        "\n"));
    CHECK(static_cast<std::size_t>(std::ranges::count(text, '\n')) ==
          subedit::core::operationKeys().size() + 1);
    CHECK_THAT(text,
               ContainsSubstring(R"({"schema":1,"command":"config","key":"duration-adjust.speed",)"
                                 R"("value":"12","source":"file","default":"15",)"
                                 R"("used_by":"subedit-cli adjust; Adjust Durations…",)"
                                 R"("surface":"both"})"));
    CHECK_THAT(text,
               ContainsSubstring(R"("key":"duration-adjust.gap-ms","value":"0",)"
                                 R"("source":"default","default":"0",)"));
}

TEST_CASE("config show with no file shows the defaults and says why", "[config]") {
    std::ostringstream out;
    std::ostringstream err;
    const Reporter reporter = Reporter{err, 1}.withTextOutput(out);

    CHECK(showDefaultSettings(reporter) == ExitCode::Success);

    const std::string text = out.str();
    CHECK(text.starts_with("File: none (--no-config)\n"));
    CHECK_THAT(text, ContainsSubstring("\nduration-adjust.speed = 15   [default]\n"));
    CHECK_THAT(text, !ContainsSubstring("[file]"));
    CHECK(err.str().empty());
}

TEST_CASE("config show with no file in json says the file is null", "[config]") {
    std::ostringstream records;
    std::ostringstream err;
    const Reporter reporter = Reporter{err, 1}.withRecords(records).forCommand("config");

    CHECK(showDefaultSettings(reporter) == ExitCode::Success);

    const std::string text = records.str();
    CHECK(text.starts_with(R"({"schema":1,"command":"config","file":null,"exists":false})"
                           "\n"));
    CHECK(static_cast<std::size_t>(std::ranges::count(text, '\n')) ==
          subedit::core::operationKeys().size() + 1);
    CHECK_THAT(text,
               ContainsSubstring(R"("key":"duration-adjust.speed","value":"15",)"
                                 R"("source":"default",)"));
}

TEST_CASE("config set writes the value and keeps the display preferences", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "window.maximised = true\n").has_value());
    std::ostringstream err;
    const Reporter reporter{err, 1};

    CHECK(setSetting(files, kPath, "duration-adjust.speed", "12", reporter) == ExitCode::Success);

    const auto read = subedit::core::readSettings(files, kPath);
    CHECK(read.settings.durationAdjustment.charactersPerSecond == 12.0);
    CHECK(read.settings.maximised);
    CHECK(err.str() == "duration-adjust.speed = 12\n");
}

TEST_CASE("config set says nothing when quiet", "[config]") {
    InMemoryFileSystem files;
    std::ostringstream err;
    const Reporter reporter{err, 0};

    CHECK(setSetting(files, kPath, "duration-adjust.speed", "12", reporter) == ExitCode::Success);
    CHECK(err.str().empty());
}

TEST_CASE("config set creates the file and its directory", "[config]") {
    InMemoryFileSystem files;
    const std::filesystem::path path = "/new/dir/settings.conf";
    std::ostringstream err;
    const Reporter reporter{err, 1};

    CHECK(setSetting(files, path, "duration-adjust.gap-ms", "250", reporter) == ExitCode::Success);

    REQUIRE(files.contentOf(path).has_value());
    CHECK_THAT(*files.contentOf(path), ContainsSubstring("\nduration-adjust.gap-ms = 250\n"));
    REQUIRE(files.directoriesAsked().size() == 1);
    CHECK(files.directoriesAsked().front() == std::filesystem::path{"/new/dir"});
}

TEST_CASE("config set with a bad value is a usage error and writes nothing", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "duration-adjust.speed = 12\n").has_value());
    std::ostringstream err;
    const Reporter reporter{err, 1};

    CHECK(setSetting(files, kPath, "duration-adjust.speed", "abc", reporter) == ExitCode::Usage);

    CHECK(*files.readFile(kPath) == "duration-adjust.speed = 12\n");
    CHECK_FALSE(err.str().empty());
}

TEST_CASE("config set refuses a display key and an unknown key", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "window.maximised = true\n").has_value());
    std::ostringstream err;
    const Reporter reporter{err, 0};

    CHECK(setSetting(files, kPath, "window.maximised", "false", reporter) == ExitCode::Usage);
    CHECK(setSetting(files, kPath, "no-such.key", "1", reporter) == ExitCode::Usage);

    CHECK(*files.readFile(kPath) == "window.maximised = true\n");
    // A refusal is said even when quiet.
    CHECK_THAT(err.str(), ContainsSubstring("window.maximised"));
    CHECK_THAT(err.str(), ContainsSubstring("no-such.key"));
}

TEST_CASE("config set on an unreadable file fails and writes nothing", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "window.maximised = true\n").has_value());
    files.failNextRead(FileErrorKind::PermissionDenied);
    std::ostringstream err;
    const Reporter reporter{err, 1};

    CHECK(setSetting(files, kPath, "duration-adjust.speed", "12", reporter) == ExitCode::AllFailed);

    CHECK(*files.readFile(kPath) == "window.maximised = true\n");
    CHECK(err.str() ==
          "/c/subedit/settings.conf: cannot be opened: permission denied; nothing written\n");
}

TEST_CASE("config set whose write fails says why", "[config]") {
    InMemoryFileSystem files;
    files.failNextWrite(FileErrorKind::PermissionDenied);
    std::ostringstream err;
    const Reporter reporter{err, 1};

    CHECK(setSetting(files, kPath, "duration-adjust.speed", "12", reporter) == ExitCode::AllFailed);

    CHECK_FALSE(files.contentOf(kPath).has_value());
    CHECK(err.str() == "/c/subedit/settings.conf: cannot be written: permission denied\n");
}

TEST_CASE("config set rewrites an unreadable value as its default, said at level 2", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "duration-adjust.gap-ms = soon\n").has_value());
    std::ostringstream err;
    const Reporter reporter{err, 2};

    CHECK(setSetting(files, kPath, "duration-adjust.speed", "12", reporter) == ExitCode::Success);

    CHECK(err.str() == "settings: duration-adjust.gap-ms: cannot read 'soon', default written\n"
                       "duration-adjust.speed = 12\n");
    CHECK_THAT(*files.contentOf(kPath), ContainsSubstring("\n#duration-adjust.gap-ms = 0\n"));
}

TEST_CASE("config unset restores the default and leaves it commented", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "duration-adjust.speed = 12\nwindow.maximised = true\n")
                .has_value());
    std::ostringstream err;
    const Reporter reporter{err, 1};

    CHECK(unsetSetting(files, kPath, "duration-adjust.speed", reporter) == ExitCode::Success);

    CHECK_THAT(*files.contentOf(kPath), ContainsSubstring("\n#duration-adjust.speed = 15\n"));
    const auto read = subedit::core::readSettings(files, kPath);
    CHECK(read.settings.durationAdjustment.charactersPerSecond ==
          subedit::core::Settings{}.durationAdjustment.charactersPerSecond);
    CHECK(read.settings.maximised);
    CHECK(err.str() == "duration-adjust.speed reset to its default (15)\n");
}

TEST_CASE("config unset on a key already at its default succeeds", "[config]") {
    InMemoryFileSystem files;
    std::ostringstream err;
    const Reporter reporter{err, 1};

    CHECK(unsetSetting(files, kPath, "duration-adjust.gap-ms", reporter) == ExitCode::Success);

    REQUIRE(files.contentOf(kPath).has_value());
    CHECK_THAT(*files.contentOf(kPath), ContainsSubstring("\n#duration-adjust.gap-ms = 0\n"));
    CHECK(err.str() == "duration-adjust.gap-ms reset to its default (0)\n");
}

TEST_CASE("config unset refuses a key that is not an operation key", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "window.maximised = true\n").has_value());
    std::ostringstream err;
    const Reporter reporter{err, 1};

    CHECK(unsetSetting(files, kPath, "general.theme", reporter) == ExitCode::Usage);

    CHECK(*files.readFile(kPath) == "window.maximised = true\n");
}
