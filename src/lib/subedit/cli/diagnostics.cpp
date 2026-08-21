#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/wording.hpp>

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
    std::string report =
        "line " + std::to_string(diagnostic.line) + ": " + std::string{nameOf(diagnostic.kind)};
    if (!diagnostic.detail.empty()) {
        report += " (\"" + boundedOf(diagnostic.detail) + "\")";
    }
    return report + ", " + std::string{nameOf(diagnostic.severity)};
}

void sayDiagnostics(const Reporter& reporter,
                    std::string_view path,
                    std::span<const core::Diagnostic> diagnostics) {
    for (const core::Diagnostic& diagnostic : diagnostics) {
        reporter.say(kDiagnosticLevel, std::string{path} + ": " + reportOf(diagnostic));
    }
}

} // namespace subedit::cli
