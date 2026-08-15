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

/// Reads the cases of `relative`, under the test corpus.
///
/// The format is one case per line, comments starting with `#`:
///
/// ```
/// une mention seule emporte le sous-titre | "[Bruit de pas]" | supprimé
/// une référence n'est pas un bruit        | "Voir [1] la note" | =
/// ```
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

/// Runs `transform` over every case and reports each failure by name.
///
/// One Catch2 assertion per case, so that a run says how many of the forty
/// broke and which — not that the first one did.
void checkTextCases(const std::vector<TextCase>& cases,
                    const std::function<std::optional<std::string>(const std::string&)>& transform);

} // namespace subedit::test
