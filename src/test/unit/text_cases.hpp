#pragma once

// A table of text cases, read from a file rather than compiled in.

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace subedit::test {

/// One case: a name, a text to transform, and what it must become.
///
/// `line` is where the case was written, so that a failure sends the reader to
/// the case and not to the loop that ran it.
struct TextCase {
    std::string name;
    int line = 0;
    std::string input;

    /// What the text must become, or nothing when the subtitle itself goes.
    ///
    /// The distinction is not decoration: phase 4 decided that a subtitle whose
    /// text a mention emptied is **removed**, and a corpus that could only say
    /// « empty text » would be unable to hold that. It is written `supprimé`.
    std::optional<std::string> expected;
};

/// One case of a replacement: a text, what is looked for, what goes in its
/// place, and what the text must become.
///
/// **Five fields where `TextCase` has three**, and they are a different corpus
/// rather than an option on the same one: a transformation takes a text and a
/// replacement takes three things. A file is one or the other, and its reader
/// says which.
struct ReplacementCase {
    std::string name;
    int line = 0;
    std::string input;
    std::string pattern;
    std::string replacement;

    /// What the text must become. Never nothing: a replacement rewrites a
    /// subtitle, it never takes one away, so `supprimé` is refused here.
    std::string expected;
};

/// Reads the cases of `relative`, under the test corpus.
///
/// The format is one case per line, comments starting with `#`:
///
/// ```
/// <what the case shows> | "<the text given>" | "<the text expected>"
/// ```
///
/// The cases of the corpus name themselves in French, like every other piece of
/// test data; only the two words the format reserves are fixed — `=` and
/// `supprimé`, described below.
///
/// **The two texts are quoted**, which is what lets the columns be aligned
/// without changing the data: a space outside the quotes is layout, a space
/// inside is text. Several cases turn on exactly that — a mention removed from
/// the middle of a line leaves two spaces behind, and whether it should is the
/// question. `=` in place of the expected means the text is left alone; it is
/// its own spelling so that the cases which must change nothing can be counted
/// at a glance, and `supprimé` says the subtitle does not survive the
/// transformation at all.
///
/// Escapes inside a text: `\"`, `\\`, `\n`, `\t`. The tabulation earns its place:
/// the references of phase 4 tolerate one inside their brackets, and a corpus
/// that could not write it could not say so. A pipe needs none — outside the
/// quotes it separates, inside it is a character like any other.
///
/// Throws `std::runtime_error` naming the file and the line it could not read.
/// A corpus that does not load is not a corpus that passes.
[[nodiscard]] std::vector<TextCase> textCasesOf(const std::string& relative);

/// Reads the replacement cases of `relative`, under the test corpus.
///
/// Same shape as above with two fields more, and the same escapes:
///
/// ```
/// <what the case shows> | "<the text>" | "<looked for>" | "<put in place>" | "<expected>"
/// ```
///
/// `=` still means « the text is left alone », which is what a pattern that
/// matches nothing gives. `supprimé` is refused, and so is an empty pattern:
/// what it would match is a question no case should have to answer.
///
/// Throws `std::runtime_error` naming the file and the line it could not read.
[[nodiscard]] std::vector<ReplacementCase> replacementCasesOf(const std::string& relative);

/// Runs `transform` over every case and reports each failure by name.
///
/// One Catch2 assertion per case, so that a run says how many of the forty
/// broke and which — not that the first one did.
void checkTextCases(const std::vector<TextCase>& cases,
                    const std::function<std::optional<std::string>(const std::string&)>& transform);

/// Runs `replace` over every case and reports each failure by name.
void checkReplacementCases(
    const std::vector<ReplacementCase>& cases,
    const std::function<std::string(const std::string&, const std::string&, const std::string&)>&
        replace);

} // namespace subedit::test
