#include <subedit/core/text/correction_pattern.hpp>

#include <array>
#include <cstddef>
#include <string_view>
#include <type_traits>
#include <variant>

namespace subedit::core {

std::string_view fileExtensionOf(PatternKind kind) {
    // In the order of `PatternKind`, which `CorrectionPattern::kind` also relies on.
    static constexpr std::array<std::string_view, 4> kExtensions{
        "common-error", "capitalization", "hearing-impaired", "line-break"};
    return kExtensions[static_cast<std::size_t>(kind)];
}

PatternKind CorrectionPattern::kind() const {
    // The alternatives of `fields` are declared in the order of `PatternKind`,
    // which is what this cast relies on.
    static_assert(std::variant_size_v<decltype(fields)> == 4);
    static_assert(
        std::is_same_v<std::variant_alternative_t<0, decltype(fields)>, CommonErrorFields>);
    static_assert(
        std::is_same_v<std::variant_alternative_t<1, decltype(fields)>, CapitalizationFields>);
    static_assert(
        std::is_same_v<std::variant_alternative_t<2, decltype(fields)>, HearingImpairedFields>);
    static_assert(std::is_same_v<std::variant_alternative_t<3, decltype(fields)>, LineBreakFields>);
    return static_cast<PatternKind>(fields.index());
}

} // namespace subedit::core
