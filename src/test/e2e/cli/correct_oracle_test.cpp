// The cases Gaupol's own code was confronted with, played by the command line.
//
// `data/motifs/attendus/` holds, for each type of pattern, what Gaupol does with
// a text — written by an oracle that is Gaupol's code, never by this program. The
// cases that name a **cascade** — `cascade Latn-en corrige …` — are the ones a
// command line can play whole: a code, the activation the shipped `.conf` files
// give, and nothing else. They are played here through the real binary, and what
// the binary writes is compared to what the oracle says.
//
// The cases of a single record (`Latn:5`) are the core's own unit tests'; the
// line-break ones are those of `correct --tasks line-break`.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "cli_run.hpp"

using subedit::e2e::CliRun;
using subedit::e2e::contentOf;
using subedit::e2e::corpus;
using subedit::e2e::invoke;
using subedit::e2e::Scratch;
using subedit::e2e::writeFile;

namespace {

/// One line of a `.cas`: where it applies, what it announces, what it says.
struct OracleCase {
    std::string code;
    bool everything = false; // `tous`: every pattern of the type ticked
    std::string label;
    std::string input;
    std::optional<std::string> expected; // nothing: the subtitle goes
    bool unchanged = false;              // `=`
    int line = 0;
};

/// A quoted text of a `.cas`, its escapes undone: `\"`, `\\`, `\n`, `\t`.
[[nodiscard]] std::string unquoted(const std::string& field) {
    std::string out;
    for (std::size_t i = 1; i + 1 < field.size(); ++i) {
        if (field[i] != '\\') {
            out += field[i];
            continue;
        }
        ++i;
        if (field[i] == 'n') {
            out += '\n';
        } else if (field[i] == 't') {
            out += '\t';
        } else {
            out += field[i];
        }
    }
    return out;
}

/// The cascade cases of `file`, those a command line plays: no record alone, no
/// line-break limits.
[[nodiscard]] std::vector<OracleCase> cascadeCasesOf(const std::string& relative) {
    std::vector<OracleCase> cases;
    std::istringstream lines{contentOf(corpus(relative))};
    std::string line;
    int number = 0;
    while (std::getline(lines, line)) {
        ++number;
        if (!line.starts_with("cascade ")) {
            continue;
        }
        const std::size_t dash = line.find(" — ");
        REQUIRE(dash != std::string::npos);
        std::istringstream head{line.substr(0, dash)};
        std::string word;
        std::string code;
        head >> word >> code;
        std::vector<std::string> rest;
        while (head >> word) {
            rest.push_back(word);
        }
        // `42/2 coupe` is a line break; `corrige` and `intact` are ours.
        if (rest.empty() || rest.back() == "coupe" || rest.front().find('/') != std::string::npos) {
            continue;
        }

        // « label | "input" | "expected" » — the label holds no bar.
        const std::string tail = line.substr(dash + std::string{" — "}.size());
        const std::size_t first = tail.find(" | ");
        const std::size_t second = tail.find(" | ", first + 1);
        OracleCase one;
        one.code = code;
        one.everything = std::ranges::find(rest, "tous") != rest.end();
        one.label = tail.substr(0, first);
        one.input = unquoted(tail.substr(first + 3, second - first - 3));
        const std::string expected = tail.substr(second + 3);
        one.unchanged = expected == "=";
        one.expected = expected == "supprimé" || one.unchanged ? std::nullopt
                                                               : std::optional{unquoted(expected)};
        one.line = number;
        cases.push_back(std::move(one));
    }
    return cases;
}

/// A SubRip file of `texts`, each lasting a second.
[[nodiscard]] std::string srt(const std::vector<std::string>& texts) {
    std::string out;
    for (std::size_t i = 0; i < texts.size(); ++i) {
        const auto second = [](std::size_t n) {
            return std::string{"00:00:"} + (n < 10 ? "0" : "") + std::to_string(n) + ",000";
        };
        out += std::to_string(i + 1) + "\n" + second((2 * i) + 1) + " --> " + second((2 * i) + 2) +
               "\n" + texts[i] + "\n\n";
    }
    return out;
}

/// The texts of the blocks of a SubRip file, in order.
[[nodiscard]] std::vector<std::string> textsOf(const std::string& file) {
    std::vector<std::string> texts;
    std::size_t at = 0;
    while (at < file.size()) {
        std::size_t end = file.find("\n\n", at);
        if (end == std::string::npos) {
            end = file.size();
        }
        const std::string block = file.substr(at, end - at);
        const std::size_t firstEnd = block.find('\n');
        const std::size_t secondEnd = block.find('\n', firstEnd + 1);
        texts.push_back(secondEnd == std::string::npos ? std::string{}
                                                       : block.substr(secondEnd + 1));
        at = end + 2;
    }
    return texts;
}

/// The names of the patterns of `type` that the cascade of `code` reads, read
/// from the shipped files themselves — what « all of them ticked » comes to.
[[nodiscard]] std::vector<std::string> namesOf(const std::string& type, const std::string& code) {
    std::set<std::string> names;
    std::string prefix;
    std::istringstream parts{code};
    std::string part;
    while (std::getline(parts, part, '-')) {
        if (!prefix.empty()) {
            prefix += '-';
        }
        prefix += part;
        std::string path{SUBEDIT_PATTERNS_DIR};
        path += '/';
        path += prefix;
        path += '.';
        path += type;
        std::ifstream file{path};
        std::string line;
        while (std::getline(file, line)) {
            if (line.starts_with("Name=")) {
                names.insert(line.substr(5));
            }
        }
    }
    return {names.begin(), names.end()};
}

void confront(const std::string& relative, const std::string& task, const std::string& type) {
    const std::vector<OracleCase> cases = cascadeCasesOf(relative);
    REQUIRE_FALSE(cases.empty());

    const Scratch scratch;
    for (std::size_t at = 0; at < cases.size();) {
        // A case that continues — « [1/2] », « [2/2] » — is one file of two
        // subtitles: the capital of the second depends on the first.
        const std::size_t last = cases[at].label.ends_with("[1/2]") ? at + 1 : at;
        std::vector<std::string> inputs;
        std::vector<std::string> expected;
        for (std::size_t k = at; k <= last; ++k) {
            inputs.push_back(cases[k].input);
            expected.push_back(cases[k].unchanged ? cases[k].input
                                                  : cases[k].expected.value_or(""));
        }
        const bool removed = !cases[at].unchanged && !cases[at].expected.has_value();

        INFO(relative << ":" << cases[at].line << " " << cases[at].label);
        const std::string in = writeFile(scratch, "in.srt", srt(inputs));
        const std::string out = scratch.of("out.srt");
        std::vector<std::string> args{
            "--quiet", "correct", "--tasks", task, "--code", cases[at].code};
        if (cases[at].everything) {
            for (const std::string& name : namesOf(type, cases[at].code)) {
                args.emplace_back("--enable");
                args.push_back(name);
            }
        }
        args.insert(args.end(), {"--output", out, in});

        const CliRun run = invoke(args);
        REQUIRE(run.exitCode == 0);
        CHECK(textsOf(contentOf(out)) == (removed ? std::vector<std::string>{} : expected));
        at = last + 1;
    }
}

} // namespace

TEST_CASE("the common-error cascades behave as the oracle says", "[e2e][patterns]") {
    confront("motifs/attendus/common-error.cas", "common-errors", "common-error");
}

TEST_CASE("the capitalization cascades behave as the oracle says", "[e2e][patterns]") {
    confront("motifs/attendus/capitalization.cas", "capitalization", "capitalization");
}

TEST_CASE("the hearing-impaired cascades, all ticked, behave as the oracle says",
          "[e2e][patterns]") {
    confront("motifs/attendus/hearing-impaired.cas", "mentions", "hearing-impaired");
}
