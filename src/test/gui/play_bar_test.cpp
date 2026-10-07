// The bar under the picture, on its own — issue #615.
//
// What it is asked here is what a person does to it: a click, a drag, a key. The cases with a
// player are in `video_pane_test.cpp`; these have none, and read the signals the bar sends.

#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/play_bar.hpp>

#include <QColor>
#include <QIcon>
#include <QImage>
#include <QPalette>
#include <QPoint>
#include <QSize>
#include <QSlider>
#include <QTest>
#include <QToolButton>
#include <Qt>
#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace {

using subedit::core::Duration;
using subedit::core::Timestamp;
using subedit::gui::PlayBar;

/// Ten minutes, which is what a slider counted in milliseconds has to cross in one click.
constexpr std::int64_t kTenMinutes = 600000;

/// A bar of ten minutes, shown, and the positions it has asked for.
struct Booth {
    PlayBar bar;
    std::vector<std::int64_t> asked;
    int finished = 0;

    Booth() {
        bar.resize(800, 40);
        bar.show();
        bar.showPosition(Timestamp::origin(), Duration::fromMilliseconds(kTenMinutes));
        QObject::connect(&bar, &PlayBar::seekRequested, &bar, [this](Timestamp position) {
            asked.push_back(position.milliseconds());
        });
        QObject::connect(&bar, &PlayBar::seekFinished, &bar, [this] { ++finished; });
    }

    /// The point on the slider at `share` of its groove — a fraction of its width, which is
    /// where a click lands when somebody aims at « three quarters of the way ».
    [[nodiscard]] QPoint at(double share) const {
        const QSlider* slider = bar.positionSlider();
        return {static_cast<int>(slider->width() * share), slider->height() / 2};
    }
};

} // namespace

// The defect that was seen on a real window: a click in the groove moved the film by a page of ten
// milliseconds, which is to say not at all, and only dragging the handle moved it.
TEST_CASE("a click in the groove goes where it was clicked", "[gui][GUI-SEEK-01]") {
    Booth booth;

    QTest::mouseClick(booth.bar.positionSlider(), Qt::LeftButton, {}, booth.at(0.75));

    REQUIRE_FALSE(booth.asked.empty());
    // Three quarters of ten minutes, within what the width of a handle makes of the aim: a
    // fraction of a per cent of the film.
    const std::int64_t wanted = kTenMinutes * 3 / 4;
    const std::int64_t tolerance = kTenMinutes / 20;
    CHECK(booth.asked.back() > wanted - tolerance);
    CHECK(booth.asked.back() < wanted + tolerance);
}

TEST_CASE("a click near the start and near the end reach the ends", "[gui][GUI-SEEK-01]") {
    Booth booth;

    QTest::mouseClick(booth.bar.positionSlider(), Qt::LeftButton, {}, booth.at(0.99));
    REQUIRE_FALSE(booth.asked.empty());
    CHECK(booth.asked.back() > kTenMinutes * 9 / 10);

    booth.asked.clear();
    QTest::mouseClick(booth.bar.positionSlider(), Qt::LeftButton, {}, booth.at(0.01));
    REQUIRE_FALSE(booth.asked.empty());
    CHECK(booth.asked.back() < kTenMinutes / 10);
}

// A press that does not let go is a drag: the film keeps following the hand.
TEST_CASE("a click that is held goes on as a drag", "[gui][GUI-SEEK-01]") {
    Booth booth;
    QSlider* slider = booth.bar.positionSlider();

    QTest::mousePress(slider, Qt::LeftButton, {}, booth.at(0.25));
    const std::size_t afterPress = booth.asked.size();
    REQUIRE(afterPress >= 1U);
    CHECK(slider->isSliderDown());

    QTest::mouseMove(slider, booth.at(0.60));
    QTest::mouseRelease(slider, Qt::LeftButton, {}, booth.at(0.60));

    CHECK(booth.asked.size() > afterPress);
    CHECK(booth.asked.back() > kTenMinutes / 2);
    CHECK(booth.finished == 1);
}

// The handle is where it is: pressing on it does not move it, which is what lets a drag
// start from where the film is.
TEST_CASE("pressing the handle itself does not move it", "[gui][GUI-SEEK-01]") {
    Booth booth;
    QSlider* slider = booth.bar.positionSlider();
    booth.bar.showPosition(Timestamp::fromMilliseconds(kTenMinutes / 2),
                           Duration::fromMilliseconds(kTenMinutes));
    booth.asked.clear();

    QTest::mousePress(slider, Qt::LeftButton, {}, booth.at(0.5));

    CHECK(booth.asked.empty());
    QTest::mouseRelease(slider, Qt::LeftButton, {}, booth.at(0.5));
}

TEST_CASE("a click on the volume goes where it was clicked", "[gui][GUI-VOLUME-01]") {
    Booth booth;
    std::vector<int> volumes;
    QObject::connect(&booth.bar, &PlayBar::volumeRequested, &booth.bar, [&volumes](int volume) {
        volumes.push_back(volume);
    });
    booth.bar.showVolume(100);
    QSlider* slider = booth.bar.volumeSlider();
    slider->resize(100, 20);

    QTest::mouseClick(
        slider, Qt::LeftButton, {}, QPoint{slider->width() / 4, slider->height() / 2});

    REQUIRE_FALSE(volumes.empty());
    CHECK(volumes.back() < 45);
    CHECK(volumes.back() > 5);
}

// The icons are drawn from the palette: the window goes from the light one to the dark one at the
// press of a menu entry, and an icon that kept the colors of the first would be unreadable on the
// second.
TEST_CASE("the icons are drawn again when the palette changes", "[gui][GUI-FOLLOW-03]") {
    constexpr QSize kLooked{32, 32};
    PlayBar bar;
    const auto drawn = [&] {
        return std::vector<QImage>{
            bar.followButton()->icon().pixmap(kLooked, QIcon::Normal, QIcon::On).toImage(),
            bar.followButton()->icon().pixmap(kLooked, QIcon::Normal, QIcon::Off).toImage(),
            bar.playButton()->icon().pixmap(kLooked).toImage(),
            bar.stepBackButton()->icon().pixmap(kLooked).toImage(),
            bar.stepForwardButton()->icon().pixmap(kLooked).toImage()};
    };
    const std::vector<QImage> before = drawn();

    QPalette other = bar.palette();
    other.setColor(QPalette::ButtonText, QColor{Qt::red});
    other.setColor(QPalette::Highlight, QColor{Qt::green});
    bar.setPalette(other);

    const std::vector<QImage> after = drawn();
    REQUIRE(after.size() == before.size());
    for (std::size_t icon = 0; icon < before.size(); ++icon)
        CHECK(after.at(icon) != before.at(icon));
}
