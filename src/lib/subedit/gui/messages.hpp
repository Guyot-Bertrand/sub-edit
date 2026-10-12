#pragma once

// What the window calls to speak — spec 15, D1 and D2, issue #664.
//
// **The core's catalogue, in Qt's string type, and nothing more.** The window and the command
// line read one catalogue through one reader (ADR 0042): `QObject::tr` would be a second
// mechanism, and a second set of catalogues to keep in step. These functions bear the core's
// names so that the extraction (`src/scripts/update-pot.sh`) and the ratchet
// (`check-translatable-strings.py`) see a window message exactly as they see a core one.
//
// **Accelerators are written `&`** in the message itself, as Qt reads them, so a translation
// chooses its own letter. **Keyboard shortcuts are never messages**: they stay the Qt key
// sequences in every language (D3).

#include <subedit/core/i18n/messages.hpp>

#include <QByteArray>
#include <QString>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace subedit::gui {

/// The translation of `text`, or `text`.
[[nodiscard]] QString translate(std::string_view text);

/// The translation of `text` in `context`, or `text` — for two identical English words that
/// translate differently, and for nothing else.
[[nodiscard]] QString translateIn(std::string_view context, std::string_view text);

/// The form that `n` selects. A sentence with a count is written whole, never assembled
/// around an `s`.
[[nodiscard]] QString
translatePlural(std::string_view singular, std::string_view plural, std::uint64_t n);

/// `%1`..`%9` of `text` replaced by `arguments`, by the core's own substitution: one pass, so
/// an argument that contains `%2` is not expanded again, and a translation may reorder them.
/// `QString::arg` is not used on translated text (D2).
template<typename... Arguments>
    requires(std::same_as<Arguments, QString> && ...)
[[nodiscard]] QString substitute(const QString& text, const Arguments&... arguments) {
    const QByteArray bytes = text.toUtf8();
    // The UTF-8 of each argument lives until the end of the full expression that calls the
    // lambda, which is as long as the core needs the views of it.
    return [&bytes](const auto&... utf8) {
        return QString::fromStdString(core::substitute(
            std::string_view{bytes.constData(), static_cast<std::size_t>(bytes.size())},
            {std::string_view{utf8.constData(), static_cast<std::size_t>(utf8.size())}...}));
    }(arguments.toUtf8()...);
}

} // namespace subedit::gui
