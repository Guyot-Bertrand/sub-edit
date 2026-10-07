// The entries of `Video ▸ Audio ▸ Language` — issue #616, GUI-AUDIO-01.
//
// What the menu says of a track, which one is marked, when it is out, and what choosing an entry
// announces. The film is a list of tracks here: nothing is decoded.

#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/audio_track_menu.hpp>

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QString>
#include <catch2/catch_test_macros.hpp>

#include <vector>

namespace {

using subedit::core::AudioTrack;
using subedit::gui::audioTrackLabel;
using subedit::gui::AudioTrackMenu;

[[nodiscard]] AudioTrack track(int id, const char* language, const char* title, bool selected) {
    return AudioTrack{.id = id, .language = language, .title = title, .selected = selected};
}

/// The labels of the entries of `menu`, in order.
[[nodiscard]] QStringList labelsOf(const QMenu& menu) {
    QStringList labels;
    for (const QAction* entry : menu.actions())
        labels << entry->text();
    return labels;
}

} // namespace

TEST_CASE("an entry says the language and the title the file gave", "[gui][GUI-AUDIO-01]") {
    CHECK(audioTrackLabel(track(1, "fra", "Original", true), 1) ==
          QStringLiteral("1: fra — Original"));
    CHECK(audioTrackLabel(track(2, "eng", "", false), 2) == QStringLiteral("2: eng"));
    CHECK(audioTrackLabel(track(3, "", "Commentary", false), 3) == QStringLiteral("3: Commentary"));
}

// A container that wrote nothing still has a first track and a second.
TEST_CASE("a track the file says nothing of is its number", "[gui][GUI-AUDIO-01]") {
    CHECK(audioTrackLabel(track(7, "", "", false), 1) == QStringLiteral("1"));
}

// The number counts from one in the file's order: it is not the identifier the player uses.
TEST_CASE("the number is the place of the track and not its identifier", "[gui][GUI-AUDIO-01]") {
    CHECK(audioTrackLabel(track(4, "fra", "", false), 1) == QStringLiteral("1: fra"));
}

// A title is somebody's text, and an ampersand in a menu is a mnemonic.
TEST_CASE("an ampersand in a title is doubled so that it is drawn", "[gui][GUI-AUDIO-01]") {
    CHECK(audioTrackLabel(track(1, "", "Tom & Jerry", false), 1) ==
          QStringLiteral("1: Tom && Jerry"));
}

TEST_CASE("the menu lists a track per entry and marks the one that plays", "[gui][GUI-AUDIO-01]") {
    QMenu menu;
    AudioTrackMenu entries{menu, nullptr};
    const std::vector<AudioTrack> tracks{track(1, "fra", "Original", true),
                                         track(2, "eng", "Commentary", false)};

    entries.refresh(tracks);

    CHECK(labelsOf(menu) ==
          QStringList{QStringLiteral("1: fra — Original"), QStringLiteral("2: eng — Commentary")});
    REQUIRE(menu.actions().size() == 2);
    CHECK(menu.actions().at(0)->isChecked());
    CHECK_FALSE(menu.actions().at(1)->isChecked());
}

TEST_CASE("the menu is out with no track, in with one or more", "[gui][GUI-AUDIO-01]") {
    QMenu menu;
    AudioTrackMenu entries{menu, nullptr};

    entries.refresh({});
    CHECK_FALSE(menu.menuAction()->isEnabled());

    entries.refresh(std::vector<AudioTrack>{track(1, "fra", "", true)});
    CHECK(menu.menuAction()->isEnabled());
    // The one track is the whole menu: it says what the film has.
    CHECK(menu.actions().size() == 1);

    entries.refresh(std::vector<AudioTrack>{track(1, "fra", "", true), track(2, "eng", "", false)});
    CHECK(menu.menuAction()->isEnabled());
}

TEST_CASE("choosing an entry announces the identifier of its track", "[gui][GUI-AUDIO-01]") {
    QMenu menu;
    AudioTrackMenu entries{menu, nullptr};
    std::vector<int> chosen;
    QObject::connect(
        &entries, &AudioTrackMenu::chosen, &entries, [&chosen](int id) { chosen.push_back(id); });
    // Identifiers that are not the places: the second track is number 5 of its file.
    entries.refresh(std::vector<AudioTrack>{track(3, "fra", "", true), track(5, "eng", "", false)});

    menu.actions().at(1)->trigger();

    REQUIRE(chosen.size() == 1U);
    CHECK(chosen.back() == 5);
}

// A track number belongs to its file: the entries of the film left behind are gone before the
// new film's are made, whatever they were.
TEST_CASE("a new listing replaces the old one", "[gui][GUI-AUDIO-01]") {
    QMenu menu;
    AudioTrackMenu entries{menu, nullptr};
    entries.refresh(std::vector<AudioTrack>{
        track(1, "fra", "", true), track(2, "eng", "", false), track(3, "deu", "", false)});
    REQUIRE(menu.actions().size() == 3);

    entries.refresh(std::vector<AudioTrack>{track(1, "spa", "", true), track(2, "ita", "", false)});

    CHECK(labelsOf(menu) == QStringList{QStringLiteral("1: spa"), QStringLiteral("2: ita")});

    entries.refresh({});
    CHECK(menu.actions().isEmpty());
}
