#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/wording/analysis.hpp>

#include <cstddef>

namespace subedit::cli {

namespace {

/// How much of a detail is worth reading before it stops being context.
///
/// Counted in bytes and cut on a byte, which can split a character in two —
/// acceptable here and nowhere else: this is a debugging line, truncated on
/// purpose, and the alternative is to walk the UTF-8 for a string the reader
/// will not finish anyway.
constexpr std::size_t kLongestDetail = 80;

std::string boundedOf(const std::string& detail) {
    if (detail.size() <= kLongestDetail) {
        return detail;
    }
    return detail.substr(0, kLongestDetail) + "…";
}

} // namespace

std::string reportOf(const core::Diagnostic& diagnostic) {
    // A diagnostic about the whole file has no line to name, and "line 0"
    // would name a place that is not there — see `kWholeFile`.
    const std::string severity{nameOf(diagnostic.severity)};
    const std::string_view kind = nameOf(diagnostic.kind);
    const std::string line = std::to_string(diagnostic.line);
    if (diagnostic.detail.empty()) {
        return diagnostic.line == core::kWholeFile
                   // TRANSLATORS: %1 is what a reading met, such as « ignored line », %2 how
                   // serious it is.
                   ? core::substitute(core::translate("%1, %2"), {kind, severity})
                   // TRANSLATORS: %1 is a line number, %2 what a reading met on it, %3 how
                   // serious it is.
                   : core::substitute(core::translate("line %1: %2, %3"), {line, kind, severity});
    }
    const std::string detail = boundedOf(diagnostic.detail);
    return diagnostic.line == core::kWholeFile
               // TRANSLATORS: %1 is what a reading met, %2 an excerpt of the file, %3 how serious
               // it is.
               ? core::substitute(core::translate("%1 (\"%2\"), %3"), {kind, detail, severity})
               // TRANSLATORS: %1 is a line number, %2 what a reading met on it, %3 an excerpt
               // of the file, %4 how serious it is.
               : core::substitute(core::translate("line %1: %2 (\"%3\"), %4"),
                                  {line, kind, detail, severity});
}

void sayDiagnostics(const Reporter& reporter,
                    std::string_view path,
                    std::span<const core::Diagnostic> diagnostics) {
    for (const core::Diagnostic& diagnostic : diagnostics) {
        reporter.say(kDiagnosticLevel, std::string{path} + ": " + reportOf(diagnostic));
    }
}

} // namespace subedit::cli
