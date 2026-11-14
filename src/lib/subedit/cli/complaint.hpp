#pragma once

// The parser's complaints, said in the user's language.
//
// CLI11 words its diagnostics in English, inside the library, and offers no
// catalogue to replace them. What it does give is a closed set of exception
// types, each built from one or two fixed English templates around the names
// and values the user typed. src/exe/cli tells which type was thrown — it is
// the only place that knows CLI11 — and this file reads the names and values
// back out of the template and puts them in the same sentence, translated.
//
// The English msgids are CLI11's own sentences, word for word: in English
// nothing changes, for the scripts and the manual that quote them. The one
// exception is the upper bound of `CLI::NonNegativeNumber`, the largest
// `double`, which CLI11 prints and no reader can use.
//
// A template this file does not recognise is returned as CLI11 wrote it: an
// English sentence is better than a sentence with a hole in it, and the
// templates the tool can raise today are all recognised (`complaint_test`).

#include <string>
#include <string_view>

namespace subedit::cli {

/// Which of the parser's exceptions a complaint came from: one per type the tool's
/// command line can raise. `Other` for the rest — `RequiresError` and
/// `ConversionError` among them, which no option declares a way to raise today —
/// returned untouched.
enum class ComplaintKind {
    Unexpected, ///< an argument nothing takes (`ExtrasError`)
    Required,   ///< a required argument or subcommand missing (`RequiredError`)
    Excludes,   ///< two options that cannot go together (`ExcludesError`)
    Count,      ///< too many or too few values (`ArgumentMismatch`)
    Validation, ///< a value a check refused (`ValidationError`)
    Other,
};

/// `said`, the parser's English message, in the user's language.
[[nodiscard]] std::string complaintIn(ComplaintKind kind, std::string_view said);

/// The line that follows every complaint, in the user's language.
[[nodiscard]] std::string helpHint();

} // namespace subedit::cli
