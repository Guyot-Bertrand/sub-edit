// The light and dark theme — issue #241, decision D3.
//
// **What makes the theme testable is its shape.** Qt 6.4 has no colour scheme
// API at all, so "system" can read nothing; but because light and dark are
// palettes *we lay down*, a test can lay them down too and read what it gets. A
// reading of the desktop would be neither testable nor reproducible.
//
// **The readability of the anomaly tints is checked and not assumed.** The
// model wants them translucent so as to follow the ground; a colour readable on
// white is not necessarily readable on near-black, and nothing said so.

#include <subedit/core/config/theme.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/theme.hpp>

#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QPalette>
#include <QVariant>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace {

using subedit::core::Project;
using subedit::core::Session;
using subedit::core::Subtitle;
using subedit::core::Theme;
using subedit::core::Timestamp;
using subedit::gui::applyTheme;
using subedit::gui::paletteFor;
using subedit::gui::SubtitleTableModel;

/// Puts the palette of the application back as it was.
///
/// `applyTheme` touches a global state of the process, and the cases that
/// follow did not ask to inherit it. Given back in a destructor rather than at
/// the end of a case: an assertion that fails must not leave the binary painted
/// dark for all the rest.
class PaletteRestored {
public:
    PaletteRestored() : m_held(QApplication::palette()) {}

    PaletteRestored(const PaletteRestored&) = delete;
    PaletteRestored& operator=(const PaletteRestored&) = delete;
    PaletteRestored(PaletteRestored&&) = delete;
    PaletteRestored& operator=(PaletteRestored&&) = delete;

    ~PaletteRestored() { QApplication::setPalette(m_held); }

private:
    QPalette m_held;
};

/// The relative luminance of a colour, as the contrast formula defines it.
[[nodiscard]] double luminanceOf(const QColor& colour) {
    const auto channel = [](double value) {
        return value <= 0.03928 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
    };
    return (0.2126 * channel(static_cast<double>(colour.redF()))) +
           (0.7152 * channel(static_cast<double>(colour.greenF()))) +
           (0.0722 * channel(static_cast<double>(colour.blueF())));
}

/// The contrast ratio between two opaque colours, from 1 to 21.
[[nodiscard]] double contrastOf(const QColor& one, const QColor& other) {
    const double bright = std::max(luminanceOf(one), luminanceOf(other));
    const double dim = std::min(luminanceOf(one), luminanceOf(other));
    return (bright + 0.05) / (dim + 0.05);
}

/// The colour one sees when `wash` is painted over `under`.
[[nodiscard]] QColor washedOver(const QColor& wash, const QColor& under) {
    // `QColor` works in `float`; the contrast formula in `double`. The
    // compositing is therefore done in Qt's type, and the conversion is written
    // rather than suffered.
    const float alpha = wash.alphaF();
    const auto mix = [alpha](float top, float bottom) {
        return (top * alpha) + (bottom * (1 - alpha));
    };
    return QColor::fromRgbF(mix(wash.redF(), under.redF()),
                            mix(wash.greenF(), under.greenF()),
                            mix(wash.blueF(), under.blueF()));
}

[[nodiscard]] Subtitle from(std::int64_t start, std::int64_t end) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end),
                    .mainText = "x"};
}

/// A document carrying all three anomalies at once, and so all three tints.
[[nodiscard]] Project damaged() {
    Project project;
    project.setSubtitles({from(1000, 2000),
                          // finit avant de commencer
                          from(3000, 2500),
                          // overlaps the previous one
                          from(2400, 5000),
                          // starts before the previous one
                          from(2000, 6000)});
    return project;
}

/// The tints the table really lays down, read through the model.
[[nodiscard]] std::vector<QColor> tintsOf(const SubtitleTableModel& model) {
    std::vector<QColor> tints;
    for (int row = 0; row < model.rowCount({}); ++row) {
        const QVariant painted = model.data(model.index(row, 1), Qt::BackgroundRole);
        if (painted.isValid())
            tints.push_back(painted.value<QBrush>().color());
    }
    return tints;
}

} // namespace

TEST_CASE("dark is dark, and light is light", "[gui][theme][GUI-THEME-02]") {
    // The ground, and not the name: it is what all the rest is read on.
    CHECK(luminanceOf(paletteFor(Theme::Dark).base().color()) < 0.1);
    CHECK(luminanceOf(paletteFor(Theme::Light).base().color()) > 0.8);
}

TEST_CASE("the system palette is the one already in place", "[gui][theme]") {
    // Answering the current palette is the most honest way to say "nothing":
    // the one that would be laid down is the one that is.
    const PaletteRestored restored;
    QPalette peculiar;
    peculiar.setColor(QPalette::Base, QColor{7, 8, 9});
    QApplication::setPalette(peculiar);

    CHECK(paletteFor(Theme::System).base().color() == QColor{7, 8, 9});
}

TEST_CASE("every palette reads its text against its background", "[gui][theme]") {
    // Four and a half to one: the threshold a text has to hold to be read.
    for (const Theme theme : {Theme::Light, Theme::Dark}) {
        const QPalette palette = paletteFor(theme);
        CHECK(contrastOf(palette.text().color(), palette.base().color()) > 4.5);
        CHECK(contrastOf(palette.windowText().color(), palette.window().color()) > 4.5);
        CHECK(contrastOf(palette.highlightedText().color(), palette.highlight().color()) > 4.5);
    }
}

// **"System" lays down no palette at all**, and that is the heart of decision
// D3: we ship the two themes we know how to lay down, and we do not invent a
// reading of the desktop Qt 6.4 does not allow.
TEST_CASE("system applies nothing", "[gui][theme]") {
    const PaletteRestored restored;
    QPalette peculiar;
    peculiar.setColor(QPalette::Base, QColor{1, 2, 3});
    QApplication::setPalette(peculiar);

    applyTheme(Theme::System);

    CHECK(QApplication::palette().base().color() == QColor{1, 2, 3});
}

TEST_CASE("dark applies its palette to the application", "[gui][theme][GUI-THEME-02]") {
    const PaletteRestored restored;

    applyTheme(Theme::Dark);

    CHECK(QApplication::palette().base().color() == paletteFor(Theme::Dark).base().color());
}

// **The point the issue asked to check rather than assume.** The model tints
// the time columns with a translucent wash, on purpose, so that the window
// follows the palette of the desktop. A colour readable on white is not
// necessarily readable on near-black, and it is the kind of thing that shows
// only when looked at.
TEST_CASE("anomaly tints keep the text readable, light and dark alike", "[gui][theme]") {
    Session session{damaged()};
    const SubtitleTableModel model{session};
    const std::vector<QColor> tints = tintsOf(model);

    REQUIRE(tints.size() >= 3);

    for (const Theme theme : {Theme::Light, Theme::Dark}) {
        const QPalette palette = paletteFor(theme);
        for (const QColor& tint : tints) {
            const QColor seen = washedOver(tint, palette.base().color());
            // Three to one: less than the threshold of bare text, because
            // this is a tinted ground and not a colour of text — but enough for
            // the row to stay read rather than guessed at.
            CHECK(contrastOf(palette.text().color(), seen) > 3.0);
        }
    }
}

TEST_CASE("the tints stay distinct from the background, light and dark alike", "[gui][theme]") {
    // The other half: a tint one reads well but does not see signals nothing.
    // It has to stand out from the ground without wiping the text.
    Session session{damaged()};
    const SubtitleTableModel model{session};

    for (const Theme theme : {Theme::Light, Theme::Dark}) {
        const QColor base = paletteFor(theme).base().color();
        for (const QColor& tint : tintsOf(model))
            CHECK(contrastOf(washedOver(tint, base), base) > 1.05);
    }
}
