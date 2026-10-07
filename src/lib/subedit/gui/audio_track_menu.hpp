#pragma once

#include <subedit/core/video/video_player.hpp>

#include <QObject>
#include <QString>

#include <span>

class QActionGroup;
class QMenu;

namespace subedit::gui {

/// What an entry of the audio menu says of a track: its number, and then its language and its
/// title, whichever the file gave — issue #616.
///
/// **The number is always there**, for a reason that is not decoration: two tracks may both say
/// `eng`, and a film whose container wrote nothing at all still has a first track and a second.
/// `number` counts from one, in the file's order, which is not the identifier the player uses. The
/// ampersands of a title are doubled, so that a title with one is not read as a mnemonic.
[[nodiscard]] QString audioTrackLabel(const core::AudioTrack& track, int number);

/// The entries of `Video ▸ Audio ▸ Language`: one per audio track of the film that is open, the one
/// that plays marked.
///
/// **It is rebuilt, never edited**: a track number belongs to its file and nothing here is kept
/// from one film to the next — the entries of the film just left are gone before those of the new
/// one are made. The menu is **out with no track**, and **in with one**: the choice is a single
/// entry, but it says the language of the film. The entry that opens it is the one put out, so that
/// the volume beside it stays within reach.
class AudioTrackMenu final : public QObject {
    Q_OBJECT

public:
    /// `menu` is the one the entries go into, and must outlive this.
    AudioTrackMenu(QMenu& menu, QObject* parent);

    /// Lays the entries for `tracks` — none for an empty list — and marks the selected one.
    void refresh(std::span<const core::AudioTrack> tracks);

signals:
    /// The person chose the track with this identifier.
    void chosen(int id);

private:
    QMenu* m_menu;
    QActionGroup* m_group;
};

} // namespace subedit::gui
