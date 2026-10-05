#include <subedit/core/wording/correction.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

std::string_view reasonOf(FailureKind kind) {
    switch (kind) {
    case FailureKind::Untranslatable:
        return "cannot be translated";
    case FailureKind::CompileError:
        return "will not compile";
    case FailureKind::InvalidReplacement:
        return "has an invalid replacement";
    case FailureKind::TimedOut:
        return "timed out";
    case FailureKind::TooManyPasses:
        return "never settled";
    case FailureKind::TooLong:
        return "grew the text too long";
    }
    return {}; // unreachable: every enumerator is handled above
}

std::string_view reasonOf(PatternProblem problem) {
    switch (problem) {
    case PatternProblem::DirectoryUnreadable:
        return "directory cannot be read";
    case PatternProblem::FileUnreadable:
        return "file cannot be read";
    case PatternProblem::MalformedLine:
        return "malformed line";
    case PatternProblem::FieldOutsideRecord:
        return "field outside any pattern";
    case PatternProblem::UnknownField:
        return "unknown field";
    case PatternProblem::MissingField:
        return "missing field";
    case PatternProblem::InvalidValue:
        return "invalid value";
    case PatternProblem::MalformedActivation:
        break; // answered below, so that no unreachable line is left after the switch
    }
    return "malformed activation";
}

std::string describe(const PatternDiagnostic& diagnostic) {
    std::string text = diagnostic.file.filename().string();
    if (text.empty())
        text = diagnostic.file.string();
    if (diagnostic.line > 0)
        text += ", line " + std::to_string(diagnostic.line);
    text += " (" + std::string{reasonOf(diagnostic.problem)};
    if (!diagnostic.detail.empty())
        text += ": " + diagnostic.detail;
    return text + ")";
}

} // namespace subedit::core
