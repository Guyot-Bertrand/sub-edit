#include <subedit/core/config/theme.hpp>
#include <subedit/gui/theme.hpp>

#include <QApplication>
#include <QColor>
#include <QPalette>

#include <utility>

namespace subedit::gui {

namespace {

/// The two grounds, and the rest follows from them.
///
/// These are not colours chosen to please: they are the ones the table needs
/// for the four anomaly tints to stay readable over them. The contrast is
/// checked by a test rather than assumed.
constexpr QColor kLightWindow{239, 239, 239};
constexpr QColor kLightBase{255, 255, 255};
constexpr QColor kLightText{16, 16, 16};

constexpr QColor kDarkWindow{45, 45, 45};
constexpr QColor kDarkBase{30, 30, 30};
constexpr QColor kDarkText{232, 232, 232};

/// The blue of the selection, the same in both palettes.
///
/// A selection that changed hue with the theme would be one landmark fewer: it
/// is the one element the eye looks for without naming it.
constexpr QColor kHighlight{53, 110, 190};
constexpr QColor kHighlightText{255, 255, 255};

/// The grey of what is out, far enough from the text to look out.
constexpr QColor kLightDisabled{130, 130, 130};
constexpr QColor kDarkDisabled{120, 120, 120};

[[nodiscard]] QPalette
paletteOf(const QColor& window, const QColor& base, const QColor& text, const QColor& disabled) {
    QPalette palette;

    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, base);
    // Every other row, when the table alternates them: near enough the ground
    // not to stripe it, far enough to be seen.
    palette.setColor(QPalette::AlternateBase, window);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, window);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::ToolTipBase, base);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::Highlight, kHighlight);
    palette.setColor(QPalette::HighlightedText, kHighlightText);

    palette.setColor(QPalette::Disabled, QPalette::Text, disabled);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabled);

    return palette;
}

} // namespace

QPalette paletteFor(core::Theme theme) {
    switch (theme) {
    case core::Theme::System:
        return QApplication::palette();
    case core::Theme::Light:
        return paletteOf(kLightWindow, kLightBase, kLightText, kLightDisabled);
    case core::Theme::Dark:
        return paletteOf(kDarkWindow, kDarkBase, kDarkText, kDarkDisabled);
    }
    std::unreachable();
}

void applyTheme(core::Theme theme) {
    if (theme == core::Theme::System)
        return;

    QApplication::setPalette(paletteFor(theme));
}

} // namespace subedit::gui
