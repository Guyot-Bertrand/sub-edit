#pragma once

#include <subedit/core/config/settings.hpp>

#include <array>
#include <cstddef>

class QAction;
class QObject;
class QTableView;

namespace subedit::core {
enum class Document;
} // namespace subedit::core

namespace subedit::gui {

struct ProjectPage;

/// The columns of the table: which are shown, in what order, at what width,
/// and the text the current cell aims at — ADR 0034, issue #460.
///
/// **It knows the table and nothing else of the window.** The five entries of
/// `View ▸ Columns` are its own; the window puts them in the menu and, when one
/// is toggled, asks for a `refresh` and does what follows on screen.
class TableColumns final {

public:
    /// Builds the five entries, owned by `owner` for their lifetime, for
    /// `table`, which must outlive this.
    TableColumns(QTableView& table, QObject* owner);

    /// The entry that shows `column` or takes it away, or nothing for the text,
    /// which has none.
    [[nodiscard]] QAction* action(core::TableColumn column) const;

    /// How many entries `View ▸ Columns` lists: every column but the text.
    static constexpr std::size_t kEntryCount = 5;

    /// The entries in the order `View ▸ Columns` lists them.
    [[nodiscard]] std::array<QAction*, kEntryCount> entries() const;

    /// Shows or hides every column as the entries say and as `page` allows — the
    /// translation only while the project has one — then takes the current cell
    /// out of a column that has just gone.
    void refresh(const ProjectPage& page);

    /// The text the current cell aims at: the translation when the cell is in
    /// its column and the column is shown, the main text anywhere else.
    [[nodiscard]] core::Document targetDocument() const;

    /// Whether the translation column is on screen — which is what « two texts »
    /// means everywhere the window says which one it aims at.
    [[nodiscard]] bool translationShown() const;

    /// Lays down the widths, the order and the hidden columns a session left.
    void apply(const core::Settings& settings);

    /// Writes into `settings` what `apply` reads back.
    void write(core::Settings& settings) const;

private:
    /// Shows or hides one of the four position columns, keeping the width of
    /// one that goes: a hidden section measures zero, and the settings would
    /// write a width their reader refuses.
    void setPositionColumnShown(int column, bool shown);

    QTableView* m_table;

    /// The number and the three positions, in the order of the model.
    std::array<QAction*, 4> m_positions{};
    QAction* m_translation = nullptr;

    /// The width each of those four had when it was hidden.
    std::array<int, 4> m_hiddenWidths{};
};

} // namespace subedit::gui
