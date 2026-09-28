#pragma once

// Decomposing pattern codes into script, language, country — issue #505, task 5.
//
// A code is `Script[-language[-COUNTRY]]`, or `Zyyy` for every script. There
// is no ISO-15924/639/3166 registry here: the three combos (script, language,
// country) are populated from what the catalogue actually carries, decomposed
// and cascading.

#include <subedit/core/text/correction_pattern.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {
class PatternCatalogue;
} // namespace subedit::core

namespace subedit::gui {

/// A code's three parts — `Script[-language[-COUNTRY]]`, or all empty for the
/// bare `Zyyy`.
struct PatternCodeParts {
    std::string script = "";
    std::string language = "";
    std::string country = "";

    friend bool operator==(const PatternCodeParts&, const PatternCodeParts&) = default;
};

[[nodiscard]] PatternCodeParts splitCode(std::string_view code);

/// `Zyyy` for an empty script — the one code that names no script at all.
[[nodiscard]] std::string joinCode(const PatternCodeParts& parts);

/// The scripts the catalogue carries records of `kind` under, `Zyyy` first —
/// **not an external locale list**: what is not in the catalogue is not
/// offered, since nothing would come of choosing it.
[[nodiscard]] std::vector<std::string> scriptsOf(const core::PatternCatalogue& catalogue,
                                                 core::PatternKind kind);

/// The languages the catalogue carries under `script`, for `kind`.
[[nodiscard]] std::vector<std::string> languagesOf(const core::PatternCatalogue& catalogue,
                                                   core::PatternKind kind,
                                                   std::string_view script);

/// The countries the catalogue carries under `script`-`language`, for `kind`.
[[nodiscard]] std::vector<std::string> countriesOf(const core::PatternCatalogue& catalogue,
                                                   core::PatternKind kind,
                                                   std::string_view script,
                                                   std::string_view language);

} // namespace subedit::gui
