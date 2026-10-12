// `config list|show|set|unset`: the operation settings, read and changed
// through the real binary — spec 17, decision D7.
//
// **Every run that touches a file names it with `--config`**, inside a
// `Scratch`: the user's settings file is never the one a test writes, and the
// harness has moved the configuration home anyway.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cctype>
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::contentOf;
using subedit::e2e::invoke;
using subedit::e2e::Scratch;
using subedit::e2e::writeFile;

namespace {

/// A strict JSON reader that only says whether `text` is one value, whole.
///
/// The harness has no JSON library, and a check by substring would let a blank
/// line or a truncated record through: this is the least that proves each line
/// of the output parses.
class JsonCheck {

public:
    explicit JsonCheck(std::string_view text) : m_text{text} {}

    /// Whether the whole text is exactly one JSON object.
    [[nodiscard]] bool isObject() {
        skipSpace();
        if (peek() != '{')
            return false;
        if (!value())
            return false;
        skipSpace();
        return m_at == m_text.size();
    }

private:
    [[nodiscard]] char peek() const { return m_at < m_text.size() ? m_text[m_at] : '\0'; }

    void skipSpace() {
        while (m_at < m_text.size() && (m_text[m_at] == ' ' || m_text[m_at] == '\t' ||
                                        m_text[m_at] == '\r' || m_text[m_at] == '\n'))
            ++m_at;
    }

    [[nodiscard]] bool literal(std::string_view word) {
        if (m_text.substr(m_at, word.size()) != word)
            return false;
        m_at += word.size();
        return true;
    }

    [[nodiscard]] bool string() {
        if (peek() != '"')
            return false;
        ++m_at;
        while (m_at < m_text.size()) {
            const char c = m_text[m_at++];
            if (c == '"')
                return true;
            if (static_cast<unsigned char>(c) < 0x20)
                return false;
            if (c == '\\') {
                if (m_at >= m_text.size())
                    return false;
                const char escaped = m_text[m_at++];
                if (escaped == 'u') {
                    for (int i = 0; i < 4; ++i) {
                        if (m_at >= m_text.size() ||
                            std::isxdigit(static_cast<unsigned char>(m_text[m_at])) == 0)
                            return false;
                        ++m_at;
                    }
                } else if (std::string_view{"\"\\/bfnrt"}.find(escaped) == std::string_view::npos) {
                    return false;
                }
            }
        }
        return false;
    }

    [[nodiscard]] bool number() {
        const std::size_t start = m_at;
        if (peek() == '-')
            ++m_at;
        std::size_t digits = 0;
        while (std::isdigit(static_cast<unsigned char>(peek())) != 0) {
            ++m_at;
            ++digits;
        }
        if (digits == 0)
            return false;
        if (peek() == '.') {
            ++m_at;
            if (std::isdigit(static_cast<unsigned char>(peek())) == 0)
                return false;
            while (std::isdigit(static_cast<unsigned char>(peek())) != 0)
                ++m_at;
        }
        return m_at > start;
    }

    [[nodiscard]] bool members(char close, bool keyed) {
        ++m_at;
        skipSpace();
        if (peek() == close) {
            ++m_at;
            return true;
        }
        while (true) {
            skipSpace();
            if (keyed) {
                if (!string())
                    return false;
                skipSpace();
                if (peek() != ':')
                    return false;
                ++m_at;
            }
            if (!value())
                return false;
            skipSpace();
            if (peek() == close) {
                ++m_at;
                return true;
            }
            if (peek() != ',')
                return false;
            ++m_at;
        }
    }

    [[nodiscard]] bool value() {
        skipSpace();
        switch (peek()) {
        case '{':
            return members('}', true);
        case '[':
            return members(']', false);
        case '"':
            return string();
        case 't':
            return literal("true");
        case 'f':
            return literal("false");
        case 'n':
            return literal("null");
        default:
            return number();
        }
    }

    std::string_view m_text;
    std::size_t m_at = 0;
};

/// The lines of `text`, each without its newline. A text that does not end
/// with one gives its last piece as a line too.
std::vector<std::string> linesOf(const std::string& text) {
    std::vector<std::string> lines;
    std::size_t start = 0;
    for (std::size_t at = text.find('\n'); at != std::string::npos; at = text.find('\n', start)) {
        lines.push_back(text.substr(start, at - start));
        start = at + 1;
    }
    if (start < text.size())
        lines.push_back(text.substr(start));
    return lines;
}

/// The same four subtitles as `adjust_test.cpp` — see that file's header.
const std::string kInput = "1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n"
                           "2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n"
                           "3\n00:00:14,200 --> 00:00:14,300\nabc\n\n"
                           "4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n";

/// The end of each subtitle of a SubRip file.
std::vector<std::string> endsOf(const std::string& srt) {
    std::vector<std::string> ends;
    for (std::size_t at = srt.find(" --> "); at != std::string::npos;
         at = srt.find(" --> ", at + 1)) {
        ends.push_back(srt.substr(at + 5, 12));
    }
    return ends;
}

} // namespace

TEST_CASE("config list works with no file and no option", "[e2e][config]") {
    const CliRun run = invoke({"config", "list"});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("duration-adjust.speed\n"));
    CHECK_THAT(run.output, ContainsSubstring("    Accepts: "));
    // No file is read for a list, and none is announced.
    CHECK_THAT(run.errors, !ContainsSubstring("settings:"));
}

TEST_CASE("config list ignores --config and --no-config", "[e2e][config]") {
    const Scratch scratch;
    const std::string file = scratch.of("settings.conf");

    const CliRun plain = invoke({"config", "list"});
    const CliRun named = invoke({"--config", file, "config", "list"});
    const CliRun disabled = invoke({"--no-config", "config", "list"});

    CHECK(named.exitCode == 0);
    CHECK(disabled.exitCode == 0);
    CHECK(named.output == plain.output);
    CHECK(disabled.output == plain.output);
    CHECK_FALSE(std::filesystem::exists(file));
}

TEST_CASE("config set then show then unset round-trips through the file", "[e2e][config]") {
    const Scratch scratch;
    const std::string file = scratch.of("settings.conf");

    const CliRun set = invoke({"--config", file, "config", "set", "duration-adjust.speed", "12"});
    REQUIRE(set.exitCode == 0);
    CHECK(set.output.empty());
    CHECK(set.errors == "duration-adjust.speed = 12\n");
    CHECK_THAT(contentOf(file), ContainsSubstring("duration-adjust.speed = 12\n"));

    const CliRun shown = invoke({"--config", file, "config", "show"});
    CHECK(shown.exitCode == 0);
    CHECK_THAT(shown.output, ContainsSubstring("File: " + file + " (found)\n"));
    CHECK_THAT(shown.output,
               ContainsSubstring("\nduration-adjust.speed = 12   [file]   (default 15)\n"));

    const CliRun unset = invoke({"--config", file, "config", "unset", "duration-adjust.speed"});
    REQUIRE(unset.exitCode == 0);
    CHECK(unset.errors == "duration-adjust.speed reset to its default (15)\n");

    const CliRun again = invoke({"--config", file, "config", "show"});
    CHECK(again.exitCode == 0);
    CHECK_THAT(again.output, ContainsSubstring("\nduration-adjust.speed = 15   [default]\n"));
}

TEST_CASE("config show on a missing file says not found and writes nothing", "[e2e][config]") {
    const Scratch scratch;
    const std::string file = scratch.of("settings.conf");

    const CliRun run = invoke({"--config", file, "config", "show"});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("File: " + file + " (not found)\n"));
    CHECK_THAT(run.output, ContainsSubstring("\nduration-adjust.speed = 15   [default]\n"));
    CHECK_FALSE(std::filesystem::exists(file));
}

TEST_CASE("config show --no-config shows the defaults and reads no file", "[e2e][config]") {
    const CliRun run = invoke({"--no-config", "config", "show"});

    CHECK(run.exitCode == 0);
    CHECK(run.output.starts_with("File: none (--no-config)\n"));
    CHECK_THAT(run.output, ContainsSubstring("\nduration-adjust.speed = 15   [default]\n"));
    CHECK_THAT(run.output, !ContainsSubstring("[file]"));
}

TEST_CASE("config set with a bad value exits with the usage code", "[e2e][config]") {
    const Scratch scratch;
    const std::string file = scratch.of("settings.conf");

    for (const std::string& value : std::vector<std::string>{"fast", "0", "-3"}) {
        INFO(value);
        const CliRun run =
            invoke({"--config", file, "config", "set", "duration-adjust.speed", value});

        CHECK(run.exitCode == 1);
        CHECK(run.output.empty());
        CHECK_THAT(run.errors, ContainsSubstring("duration-adjust.speed"));
        CHECK_FALSE(std::filesystem::exists(file));
    }
}

TEST_CASE("config set refuses a display preference", "[e2e][config]") {
    const Scratch scratch;
    const std::string file =
        writeFile(scratch, "settings.conf", "general.theme = dark\nduration-adjust.speed = 12\n");

    for (const std::vector<std::string>& line : std::vector<std::vector<std::string>>{
             {"--config", file, "config", "set", "general.theme", "light"},
             {"--config", file, "config", "unset", "window.maximised"}}) {
        INFO(line[4]);
        const CliRun run = invoke(line);

        CHECK(run.exitCode == 1);
        CHECK_THAT(run.errors, ContainsSubstring(line[4] + " is a display preference"));
    }
    // Refused, so not rewritten.
    CHECK(contentOf(file) == "general.theme = dark\nduration-adjust.speed = 12\n");
}

TEST_CASE("config set refuses an unknown key and points to config list", "[e2e][config]") {
    const Scratch scratch;
    const std::string file = scratch.of("settings.conf");

    const CliRun set = invoke({"--config", file, "config", "set", "no-such.key", "1"});
    const CliRun unset = invoke({"--config", file, "config", "unset", "no-such.key"});

    for (const CliRun& run : {set, unset}) {
        CHECK(run.exitCode == 1);
        CHECK_THAT(run.errors,
                   ContainsSubstring("unknown setting no-such.key; subedit-cli config list "
                                     "names them"));
    }
    CHECK_FALSE(std::filesystem::exists(file));
}

TEST_CASE("config set and unset with --no-config are usage errors", "[e2e][config]") {
    for (const std::vector<std::string>& line : std::vector<std::vector<std::string>>{
             {"--no-config", "config", "set", "duration-adjust.speed", "12"},
             {"--no-config", "config", "unset", "duration-adjust.speed"}}) {
        INFO(line[2]);
        const CliRun run = invoke(line);

        CHECK(run.exitCode == 1);
        CHECK(run.output.empty());
        CHECK(run.errors == "--no-config has nothing to write to\n");
    }
}

TEST_CASE("an empty --config name is a usage error, not the user's file", "[e2e][config]") {
    for (const std::vector<std::string>& line : std::vector<std::vector<std::string>>{
             {"--config", "", "config", "set", "duration-adjust.speed", "12"},
             {"--config", "", "config", "show"}}) {
        INFO(line[3]);
        const CliRun run = invoke(line);

        CHECK(run.exitCode == 1);
        CHECK(run.output.empty());
        CHECK(run.errors == "--config needs a file name\n");
    }
}

TEST_CASE("config set refuses a number of lines out of 1 to 1000", "[e2e][config]") {
    const Scratch scratch;
    const std::string file = scratch.of("settings.conf");

    for (const char* value : {"0", "-2", "1001", "100000"}) {
        INFO(value);
        const CliRun run =
            invoke({"--config", file, "config", "set", "correction.line-break.max-lines", value});

        CHECK(run.exitCode == 1);
        CHECK_THAT(run.errors, ContainsSubstring("correction.line-break.max-lines"));
    }
    CHECK_FALSE(std::filesystem::exists(file));
    CHECK(invoke({"--config", file, "config", "set", "correction.line-break.max-lines", "1000"})
              .exitCode == 0);
}

TEST_CASE("config needs one of its subcommands", "[e2e][config]") {
    const CliRun bare = invoke({"config"});
    CHECK(bare.exitCode == 1);

    const CliRun missing = invoke({"config", "set", "duration-adjust.speed"});
    CHECK(missing.exitCode == 1);
    CHECK_THAT(missing.errors, ContainsSubstring("value"));
}

TEST_CASE("config list and show --format json are one object a line", "[e2e][config][json]") {
    const Scratch scratch;
    const std::string file = writeFile(scratch, "settings.conf", "duration-adjust.speed = 12\n");

    for (const std::vector<std::string>& line : std::vector<std::vector<std::string>>{
             {"--format", "json", "config", "list"},
             {"--format", "json", "--config", file, "config", "show"},
             {"--format", "json", "--no-config", "config", "show"}}) {
        INFO(line[line.size() - 2] + " " + line.back());
        const CliRun run = invoke(line);

        CHECK(run.exitCode == 0);
        REQUIRE(run.output.ends_with("\n"));
        const std::vector<std::string> lines = linesOf(run.output);
        REQUIRE(lines.size() > 1);
        for (const std::string& one : lines) {
            INFO(one);
            CHECK_FALSE(one.empty());
            CHECK(JsonCheck{one}.isObject());
            CHECK(one.starts_with(R"({"schema":1,"command":"config",)"));
        }
    }

    const CliRun shown = invoke({"--format", "json", "--config", file, "config", "show"});
    CHECK(shown.output.starts_with(R"({"schema":1,"command":"config","file":")" + file +
                                   R"(","exists":true})"
                                   "\n"));
    CHECK_THAT(shown.output,
               ContainsSubstring(R"("key":"duration-adjust.speed","value":"12","source":"file",)"));

    const CliRun none = invoke({"--format", "json", "--no-config", "config", "show"});
    CHECK(none.output.starts_with(R"({"schema":1,"command":"config","file":null,"exists":false})"
                                  "\n"));
}

TEST_CASE("a value set by config set reaches adjust", "[e2e][config][adjust]") {
    const Scratch scratch;
    const std::string file = scratch.of("settings.conf");
    const std::string input = writeFile(scratch, "in/d.srt", kInput);

    for (const std::vector<std::string>& setting :
         std::vector<std::vector<std::string>>{{"duration-adjust.lengthen", "false"},
                                               {"duration-adjust.shorten", "false"},
                                               {"duration-adjust.minimum-ms", "1500"}}) {
        INFO(setting[0]);
        REQUIRE(invoke({"--config", file, "config", "set", setting[0], setting[1]}).exitCode == 0);
    }

    const CliRun run =
        invoke({"--config", file, "adjust", "--output-dir", scratch.of("out"), input});

    REQUIRE(run.exitCode == 0);
    // The speed moves no end and the minimum is 1.5 s: case B2 of adjust_test.cpp.
    CHECK(endsOf(contentOf(scratch.of("out/d.srt"))) ==
          std::vector<std::string>{"00:00:01,500", "00:00:14,000", "00:00:15,000", "00:00:16,500"});
}

TEST_CASE("config set and unset write a bare file name in the working directory", "[e2e][config]") {
    const Scratch scratch;

    // The parent of "s.conf" is empty: no directory is to be made for it.
    const CliRun set = invoke(
        {"--config", "s.conf", "config", "set", "duration-adjust.speed", "12"}, scratch.path());
    INFO(set.errors);
    REQUIRE(set.exitCode == 0);
    CHECK_THAT(contentOf(scratch.of("s.conf")), ContainsSubstring("12"));

    const CliRun unset =
        invoke({"--config", "s.conf", "config", "unset", "duration-adjust.speed"}, scratch.path());
    INFO(unset.errors);
    CHECK(unset.exitCode == 0);
}
