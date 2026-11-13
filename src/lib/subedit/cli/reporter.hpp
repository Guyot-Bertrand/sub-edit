#pragma once

// Telling the user what is happening, at the level of detail they asked for.

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>

namespace subedit::cli {

class Json;

/// What a line of narration is about: the closed list of `docs/specs/cli-narration.md` (D3).
///
/// **A key is a constant, never translated**, like the name of a subcommand: a script relies on
/// `dest:` whatever the language. Only the value of a sentence is translated. A key is added
/// with the table of the spec, never here alone.
enum class Key {
    Source,
    Translation,
    Media,
    Format,
    Encoding,
    Bom,
    LineEndings,
    BytesRead,
    Diagnostic,
    Alignment,
    Progress,
    Warning,
    Error,
    Result,
    BytesWritten,
    Dest,
    Tail,
    Settings,
    Walk,
    Summary,
};

/// The name `key` is written under — `line-endings` for `Key::LineEndings`.
[[nodiscard]] std::string_view nameOf(Key key);

/// The four levels of narration, and the stream they go to.
///
/// **Narration is never the result.** It goes to standard error, so that a
/// caller piping the result of a subcommand receives the result and nothing
/// else — and so that silence can be asked for without losing what the command
/// was invoked to produce.
class Reporter {

public:
    /// `level` is 0 for `--quiet`, 1 by default, 2 for `-vv`, 3 for `-vvv`.
    Reporter(std::ostream& errors, int level) : m_errors{&errors}, m_level{level} {}

    [[nodiscard]] int level() const { return m_level; }

    /// The same reporter, also writing one record per file to `out`.
    ///
    /// **Records are the result, narration is the account** — the split the
    /// class keeps for a human holds for a script: `out` is standard output, and
    /// what `tell` writes stays on standard error whatever the level, unchanged.
    [[nodiscard]] Reporter withRecords(std::ostream& out) const;

    /// The same reporter, also able to write the text of a result to `out`.
    ///
    /// Standard output when the surface is a terminal, and used for the one
    /// thing a text run puts there: what `--dry-run` would change. A reporter
    /// that records writes none of it — the records are the result then.
    [[nodiscard]] Reporter withTextOutput(std::ostream& out) const;

    /// Writes `text` as the result, when the form is text. `text` carries its
    /// own line ends; nothing is added.
    void result(std::string_view text) const;

    /// The same reporter, knowing which subcommand is speaking.
    [[nodiscard]] Reporter forCommand(std::string command) const;

    /// The same reporter, speaking of one element of the batch: the `position`-th of `count`,
    /// counted from one, whose input is `source`.
    ///
    /// **The element is identified by its position, not by its path** — a path may hold any
    /// character, a position nothing to escape, and it is the order the records are written in.
    ///
    /// **Its `source` line is written by the reporter**, just before the first line the element
    /// writes, at whatever level that line is: an element begins with its input, and under
    /// `--quiet` an error never comes without it. Every reporter derived from this one shares
    /// that, so that the source is said once. A caller writes `Key::Source` only for a second
    /// input of the same element (`append`).
    [[nodiscard]] Reporter
    forItem(std::size_t position, std::size_t count, std::string source) const;

    /// Writes `value` under `key` when the asked-for level reaches `atLeast`, as
    /// `[subedit] [<command>] [<i>/<n>] <key>: <value>`.
    ///
    /// A `-` stands for a command not yet known, and for the element of a line about the whole
    /// run. **A sentence that holds line ends is one line per physical line**, each with the
    /// prefix and the key; **a path is one line whatever it holds**, its line ends written `\n`.
    void tell(int atLeast, Key key, std::string_view value) const;

    /// Writes `value` under `Key::Error`, whatever the level, `--quiet` included.
    ///
    /// A command that fails in silence leaves its exit code as the only clue, and turns every
    /// incident into an investigation. "Nothing" means nothing of what recounts; what raises the
    /// alarm stays.
    void error(std::string_view value) const;

    /// Whether this reporter speaks of the element whose input is `file`: a sentence about it
    /// need not name it, its `source` line does.
    [[nodiscard]] bool speaksOf(std::string_view file) const {
        return m_item != nullptr && m_item->source == file;
    }

    /// Whether a record is wanted for each file (`--format json`).
    [[nodiscard]] bool recording() const { return m_records != nullptr; }

    /// The name the records are written under.
    [[nodiscard]] const std::string& command() const { return m_command; }

    /// Writes `record` as one line, when records are wanted.
    void record(const Json& record) const;

private:
    /// The element a reporter speaks of, shared by every reporter derived from it.
    struct Item {
        std::size_t position = 0;
        std::size_t count = 0;
        std::string source;
        bool announced = false;
    };

    void write(Key key, std::string_view value) const;
    void writeLine(Key key, std::string_view value) const;

    std::ostream* m_errors;
    int m_level;
    std::ostream* m_records = nullptr;
    std::ostream* m_text = nullptr;
    std::string m_command;
    std::shared_ptr<Item> m_item;
};

} // namespace subedit::cli
