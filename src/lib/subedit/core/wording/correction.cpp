#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/wording/correction.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

std::string_view patternName(std::string_view englishName) {
    // Not a literal: the msgid is the file's own text, which the template takes from the
    // shipped patterns (`update-pot.sh`).
    return translate(englishName);
}

std::string_view patternName(const CorrectionPattern& pattern) {
    return patternName(pattern.name);
}

std::string_view patternDescription(const CorrectionPattern& pattern) {
    return translate(pattern.description);
}

std::string_view reasonOf(FailureKind kind) {
    switch (kind) {
    case FailureKind::Untranslatable:
        return translate("cannot be translated");
    case FailureKind::CompileError:
        return translate("will not compile");
    case FailureKind::InvalidReplacement:
        return translate("has an invalid replacement");
    case FailureKind::TimedOut:
        return translate("timed out");
    case FailureKind::TooManyPasses:
        return translate("never settled");
    case FailureKind::TooLong:
        return translate("grew the text too long");
    }
    return {}; // unreachable: every enumerator is handled above
}

std::string_view reasonOf(PatternProblem problem) {
    switch (problem) {
    case PatternProblem::DirectoryUnreadable:
        return translate("directory cannot be read");
    case PatternProblem::FileUnreadable:
        return translate("file cannot be read");
    case PatternProblem::MalformedLine:
        return translate("malformed line");
    case PatternProblem::FieldOutsideRecord:
        return translate("field outside any pattern");
    case PatternProblem::UnknownField:
        return translate("unknown field");
    case PatternProblem::MissingField:
        return translate("missing field");
    case PatternProblem::InvalidValue:
        return translate("invalid value");
    case PatternProblem::MalformedActivation:
        break; // answered below, so that no unreachable line is left after the switch
    }
    return translate("malformed activation");
}

std::string describe(const PatternDiagnostic& diagnostic) {
    std::string text = diagnostic.file.filename().string();
    if (text.empty())
        text = diagnostic.file.string();
    if (diagnostic.line > 0)
        // TRANSLATORS: %1 is a file name, %2 a line number in it.
        text = substitute(translate("%1, line %2"), {text, std::to_string(diagnostic.line)});
    text += " (" + std::string{reasonOf(diagnostic.problem)};
    if (!diagnostic.detail.empty())
        text += ": " + diagnostic.detail;
    return text + ")";
}

} // namespace subedit::core
