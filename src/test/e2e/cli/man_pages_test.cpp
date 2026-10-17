// The man pages of the command line, as the build lays them out — issue #683, spec 15, D9.
//
// **The pages are generated from the manual**, by `src/scripts/md-to-man.py`, into
// `<build>/share/man/[<language>/]man1`: the layout an installation has, so what is read here is
// what `cmake --install` regenerates under the prefix (`cmake/ManPages.cmake`). The test compares
// them with the binary, never with the manual: a converter that dropped a table, or a manual that
// forgot an option, gives a page `--help` contradicts, and that is what a reader of `man` would
// meet.
//
// `check-installation.sh` asks the same of the installed pages, through `check-cli-manual.py`;
// this is the test that cites the requirement.

#include <catch2/catch_test_macros.hpp>

#include <cctype>
#include <filesystem>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "cli_run.hpp"

using subedit::e2e::CliRun;
using subedit::e2e::contentOf;
using subedit::e2e::invoke;
using subedit::e2e::invokeProgram;
using subedit::e2e::Locale;

namespace {

namespace fs = std::filesystem;

/// The languages whose pages the build generates, under `share/man`: English in `man1`, every
/// other in `<language>/man1`. French is the one that ships (`src/po/LINGUAS`).
const std::vector<std::string> kPageDirectories{"man1", "fr/man1"};

/// The lines of a section of a help (`Options:`, `Subcommands:`), as `check-cli-manual.py`
/// reads them: from its title to the first blank line after a line of it.
std::vector<std::string> section(const std::string& help, std::string_view title) {
    std::vector<std::string> lines;
    bool inside = false;
    std::size_t start = 0;
    while (start < help.size()) {
        std::size_t end = help.find('\n', start);
        if (end == std::string::npos)
            end = help.size();
        const std::string line = help.substr(start, end - start);
        start = end + 1;
        if (!line.empty() && line.front() != ' ' && line.back() == ':') {
            inside = line == title;
            continue;
        }
        if (!inside)
            continue;
        if (line.find_first_not_of(' ') == std::string::npos) {
            if (!lines.empty())
                break;
            continue;
        }
        lines.push_back(line);
    }
    return lines;
}

/// The long options of a help's `Options:`, `--help` excepted — every subcommand has its own.
std::set<std::string> longOptions(const std::string& help) {
    std::set<std::string> options;
    for (const std::string& line : section(help, "Options:")) {
        // The names column only: the descriptions quote other words.
        const std::size_t first = line.find_first_not_of(' ');
        const std::size_t gap = line.find("  ", first);
        const std::string names = line.substr(first, gap == std::string::npos ? gap : gap - first);
        std::size_t at = names.find("--");
        while (at != std::string::npos) {
            std::size_t end = at + 2;
            while (end < names.size() &&
                   (std::islower(static_cast<unsigned char>(names[end])) != 0 ||
                    std::isdigit(static_cast<unsigned char>(names[end])) != 0 || names[end] == '-'))
                ++end;
            if (at == 0 || names[at - 1] == ',' || names[at - 1] == ' ')
                options.insert(names.substr(at, end - at));
            at = names.find("--", end);
        }
    }
    options.erase("--help");
    return options;
}

std::vector<std::string> subcommands() {
    const CliRun help = invoke({"--help"});
    std::vector<std::string> names;
    for (const std::string& line : section(help.output, "Subcommands:")) {
        const std::size_t first = line.find_first_not_of(' ');
        names.push_back(line.substr(first, line.find(' ', first) - first));
    }
    return names;
}

/// An option as troff writes it, `\-\-by`, and not the start of a longer one (`\-\-by\-…`).
bool namesOption(const std::string& page, const std::string& option) {
    std::string escaped;
    for (const char character : option) {
        if (character == '-')
            escaped += '\\';
        escaped += character;
    }
    std::size_t at = page.find(escaped);
    while (at != std::string::npos) {
        const std::size_t after = at + escaped.size();
        const bool longer =
            after < page.size() && (std::isalnum(static_cast<unsigned char>(page[after])) != 0 ||
                                    (page[after] == '\\' && page.compare(after, 2, "\\-") == 0));
        if (!longer)
            return true;
        at = page.find(escaped, after);
    }
    return false;
}

fs::path pages(const std::string& directory) {
    return fs::path{SUBEDIT_MAN_DIR} / directory;
}

} // namespace

TEST_CASE("every subcommand has its man page in every language, with the options of its help",
          "[e2e][man][i18n][CLI-I18N-04]") {
    const std::vector<std::string> names = subcommands();
    REQUIRE(names.size() > 1);
    const std::set<std::string> global = longOptions(invoke({"--help"}).output);
    REQUIRE(global.contains("--format"));

    for (const std::string& directory : kPageDirectories) {
        INFO("pages of " << pages(directory).string());
        const std::string top = contentOf(pages(directory) / "subedit-cli.1");
        REQUIRE_FALSE(top.empty());
        for (const std::string& option : global) {
            INFO("global option " << option);
            CHECK(namesOption(top, option));
        }

        for (const std::string& name : names) {
            INFO("subcommand " << name);
            const std::string page = contentOf(pages(directory) / ("subedit-cli-" + name + ".1"));
            REQUIRE_FALSE(page.empty());
            CHECK(page.find(".TH SUBEDIT-CLI-") != std::string::npos);
            for (const std::string& option : longOptions(invoke({name, "--help"}).output)) {
                INFO("option " << option);
                CHECK(namesOption(page, option));
            }
        }

        // And no page for a subcommand the binary does not have: one left behind by a removal.
        std::set<std::string> expected{"subedit-cli.1"};
        for (const std::string& name : names)
            expected.insert("subedit-cli-" + name + ".1");
        std::set<std::string> found;
        for (const fs::directory_entry& entry : fs::directory_iterator(pages(directory)))
            found.insert(entry.path().filename().string());
        CHECK(found == expected);
    }
}

TEST_CASE("the French pages are written in French", "[e2e][man][i18n][CLI-I18N-04]") {
    const std::string page = contentOf(pages("fr/man1") / "subedit-cli-shift.1");
    CHECK(page.find(".SH \"NOM\"") != std::string::npos);
    CHECK(page.find(".SH \"VOIR AUSSI\"") != std::string::npos);
    CHECK(page.find("\"Commandes utilisateur\"") != std::string::npos);
    CHECK(contentOf(pages("man1") / "subedit-cli-shift.1").find(".SH \"NAME\"") !=
          std::string::npos);
}

TEST_CASE("man renders every page without a single warning", "[e2e][man][i18n][CLI-I18N-04]") {
    // `man` comes from the build, as the binaries do: a test that looked it up would pass or fail
    // with the `PATH` of whoever runs it.
    const std::string man = SUBEDIT_MAN_PROGRAM;
    INFO("man was not found at configuration; ./src/scripts/setup-toolchain.sh installs it");
    REQUIRE_FALSE(man.empty());

    for (const std::string& directory : kPageDirectories) {
        for (const fs::directory_entry& entry : fs::directory_iterator(pages(directory))) {
            // A UTF-8 terminal, and the ASCII one `C` gives, where a character with no glyph
            // draws nothing — the generated page carries a fallback for each.
            for (const char* locale : {"C.UTF-8", "C"}) {
                INFO(entry.path().string() << " under " << locale);
                const CliRun run = invokeProgram(man,
                                                 {"-l", "--warnings=w", entry.path().string()},
                                                 Locale{.all = std::string{locale}});
                CHECK(run.exitCode == 0);
                CHECK(run.errors.empty());
                CHECK_FALSE(run.output.empty());
            }
        }
    }
}
