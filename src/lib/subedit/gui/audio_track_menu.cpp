#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/audio_track_menu.hpp>

#include <QAction>
#include <QActionGroup>
#include <QList>
#include <QMenu>
#include <QString>

#include <span>

namespace subedit::gui {

namespace {} // namespace

QString audioTrackLabel(const core::AudioTrack& track, int number) {
    const QString language = QString::fromStdString(track.language);
    const QString title = QString::fromStdString(track.title);

    QString description;
    if (!language.isEmpty() && !title.isEmpty())
        description = language + QStringLiteral(" — ") + title;
    else if (!language.isEmpty())
        description = language;
    else
        description = title;

    // A mnemonic is introduced by an ampersand, and a title is somebody's text.
    description.replace(QLatin1Char('&'), QStringLiteral("&&"));

    return description.isEmpty() ? QStringLiteral("%1").arg(number)
                                 : QStringLiteral("%1: %2").arg(number).arg(description);
}

AudioTrackMenu::AudioTrackMenu(QMenu& menu, QObject* parent)
    : QObject{parent}, m_menu(&menu), m_group(new QActionGroup{this}) {
    m_group->setExclusive(true);
}

void AudioTrackMenu::refresh(std::span<const core::AudioTrack> tracks) {
    // The entries of the film just left go first: they name tracks the new one may not have, and
    // an identifier is a number of its own file.
    for (QAction* entry : m_group->actions()) {
        m_menu->removeAction(entry);
        m_group->removeAction(entry);
        delete entry;
    }

    int number = 0;
    for (const core::AudioTrack& track : tracks) {
        auto* entry = new QAction{audioTrackLabel(track, ++number), m_group};
        entry->setCheckable(true);
        entry->setChecked(track.selected);
        const int id = track.id;
        connect(entry, &QAction::triggered, this, [this, id] { emit chosen(id); });
        m_menu->addAction(entry);
    }

    m_menu->menuAction()->setEnabled(!tracks.empty());
}

} // namespace subedit::gui
