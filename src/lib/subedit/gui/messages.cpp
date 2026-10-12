#include <subedit/core/i18n/messages.hpp>
#include <subedit/gui/messages.hpp>

#include <QString>

#include <cstdint>
#include <string_view>

namespace subedit::gui {

namespace {

[[nodiscard]] QString fromUtf8(std::string_view text) {
    return QString::fromUtf8(text.data(), static_cast<qsizetype>(text.size()));
}

} // namespace

QString translate(std::string_view text) {
    return fromUtf8(core::translate(text));
}

QString translateIn(std::string_view context, std::string_view text) {
    return fromUtf8(core::translateIn(context, text));
}

QString translatePlural(std::string_view singular, std::string_view plural, std::uint64_t n) {
    return fromUtf8(core::translatePlural(singular, plural, n));
}

} // namespace subedit::gui
