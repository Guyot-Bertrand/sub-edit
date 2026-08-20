#include <subedit/core/command/change.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/set_position_command.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/boundary.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QString>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>

namespace subedit::gui {

namespace {

using core::Change;
using core::ChangeKind;
using core::DecimalMark;
using core::SubtitleFormat;

/// The mark the file will be written with, so that the screen shows what the
/// file will hold.
[[nodiscard]] DecimalMark markOf(const core::Project& project) {
    return project.sourceFile().format == SubtitleFormat::WebVtt ? DecimalMark::Period
                                                                 : DecimalMark::Comma;
}

[[nodiscard]] QString written(core::Timestamp position, DecimalMark mark) {
    return QString::fromStdString(position.format(mark));
}

} // namespace

SubtitleTableModel::SubtitleTableModel(core::Session& session, QObject* parent)
    : QAbstractTableModel(parent), m_session(&session) {}

int SubtitleTableModel::rowCount(const QModelIndex& parent) const {
    // A table has no children: a valid parent asks for the rows *under* a cell,
    // and there are none.
    return parent.isValid() ? 0 : static_cast<int>(m_session->project().count());
}

int SubtitleTableModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : kColumnCount;
}

QVariant SubtitleTableModel::data(const QModelIndex& index, int role) const {
    // Les deux rôles rendent la même chose, et c'est ce qui fait marcher les
    // délégués hérités : `setEditorData` lit `Qt::EditRole`, et un éditeur de
    // position doit s'ouvrir sur l'horodatage tel qu'il est écrit à l'écran.
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole))
        return {};

    if (index.row() < 0 || index.row() >= rowCount({}))
        return {};

    // Une colonne hors de la table est une erreur d'appelant, pas un cas à
    // servir : elle rend une valeur vide plutôt que de choisir pour lui.
    if (index.column() < 0 || index.column() >= kColumnCount)
        return {};

    const core::SubtitleIndex position =
        core::SubtitleIndex::fromValue(static_cast<std::size_t>(index.row()));
    const core::Subtitle& subtitle = m_session->project().subtitleAt(position);
    const DecimalMark mark = markOf(m_session->project());

    // Transtypé, et non commuté sur l'entier : la garde ci-dessus a ramené la
    // valeur dans le domaine de l'énumération, et c'est ce qui permet au
    // compilateur de vérifier que les cinq colonnes sont traitées.
    switch (static_cast<Column>(index.column())) {
    case Number:
        // Computed, never stored: an insertion would otherwise renumber every
        // line after it.
        return QString::number(position.number());
    case Start:
        return written(subtitle.start, mark);
    case End:
        return written(subtitle.end, mark);
    case Duration:
        // Derived, and read-only for that reason: the core has no command that
        // sets a duration, and inventing one here would be core work smuggled
        // into an interface.
        return written(core::Timestamp::origin() + subtitle.duration(), mark);
    case Text:
        return QString::fromStdString(subtitle.mainText);
    }

    // Les cinq colonnes sont traitées, la garde ci-dessus écarte le reste, et
    // le compilateur vérifie que l'énumération est épuisée.
    std::unreachable();
}

Qt::ItemFlags SubtitleTableModel::flags(const QModelIndex& index) const {
    const Qt::ItemFlags inherited = QAbstractTableModel::flags(index);
    if (!index.isValid())
        return inherited;

    // Le numéro est un rang et la durée une différence : aucune commande du
    // noyau ne les pose, donc aucun éditeur ne s'ouvre dessus.
    const int column = index.column();
    if (column != Start && column != End && column != Text)
        return inherited;

    return inherited | Qt::ItemIsEditable;
}

bool SubtitleTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (index.row() < 0 || index.row() >= rowCount({}))
        return false;

    if (index.column() < 0 || index.column() >= kColumnCount)
        return false;

    const core::SubtitleIndex position =
        core::SubtitleIndex::fromValue(static_cast<std::size_t>(index.row()));
    const core::Subtitle& subtitle = m_session->project().subtitleAt(position);
    std::string typed = value.toString().toStdString();

    switch (static_cast<Column>(index.column())) {
    case Number:
    case Duration:
        // `flags()` l'a déjà dit à la vue, qui n'ouvrira pas d'éditeur ici. Le
        // refus est pour qui appelle `setData` sans passer par une vue.
        return false;
    case Start:
    case End: {
        const bool start = index.column() == Start;
        const std::optional<core::Timestamp> wanted = core::Timestamp::parse(typed);

        // **Illisible laisse la cellule inchangée** plutôt que d'inventer une
        // position. Se tromper de position est silencieux : rien à l'écran ne
        // distingue un sous-titre mal placé d'un sous-titre bien placé.
        if (!wanted.has_value())
            return false;

        if (*wanted == (start ? subtitle.start : subtitle.end))
            return true;

        applied(m_session->apply(std::make_unique<core::SetPositionCommand>(
            m_session->project(),
            position,
            start ? core::Boundary::Start : core::Boundary::End,
            *wanted)));
        return true;
    }
    case Text:
        // Une validation qui ne change rien ne produit aucune commande :
        // l'historique n'a pas à retenir une frappe suivie d'un `Entrée` sur un
        // texte identique, sans quoi l'annulation se remplirait de
        // non-événements.
        if (typed == subtitle.mainText)
            return true;

        applied(m_session->apply(std::make_unique<core::SetTextCommand>(
            m_session->project(), position, core::Document::Main, std::move(typed))));
        return true;
    }

    std::unreachable();
}

QVariant SubtitleTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    if (section < 0 || section >= kColumnCount)
        return {};

    switch (static_cast<Column>(section)) {
    case Number:
        return QStringLiteral("N°");
    case Start:
        return QStringLiteral("Début");
    case End:
        return QStringLiteral("Fin");
    case Duration:
        return QStringLiteral("Durée");
    case Text:
        return QStringLiteral("Texte");
    }

    std::unreachable();
}

std::pair<int, int> SubtitleTableModel::columnsFor(ChangeKind kind) {
    switch (kind) {
    case ChangeKind::Positions:
        // The duration among them: it is derived from the two positions, so it
        // goes stale with them.
        return {Start, Duration};
    case ChangeKind::MainText:
        return {Text, Text};
    case ChangeKind::TranslationText:
    case ChangeKind::Insertion:
    case ChangeKind::Removal:
        // Either no column shows it — the translation arrives in phase 11 — or
        // the change is structural, and `applied` has already reset the model
        // rather than asking.
        return {-1, -1};
    case ChangeKind::Reordering:
        return {Number, Text};
    }

    std::unreachable();
}

void SubtitleTableModel::applied(std::span<const Change> changes) {
    const bool structural = std::ranges::any_of(changes, [](const Change& change) {
        return change.kind == ChangeKind::Insertion || change.kind == ChangeKind::Removal;
    });

    if (structural) {
        beginResetModel();
        endResetModel();
        return;
    }

    for (const Change& change : changes) {
        const auto [first, last] = columnsFor(change.kind);
        if (first < 0)
            continue;

        for (const core::IndexRange& run : change.subtitles.ranges()) {
            emit dataChanged(index(static_cast<int>(run.first.value()), first),
                             index(static_cast<int>(run.last.value()), last));
        }
    }
}

} // namespace subedit::gui
