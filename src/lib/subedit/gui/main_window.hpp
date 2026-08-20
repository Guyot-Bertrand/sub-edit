#pragma once

#include <QMainWindow>

#include <memory>

namespace subedit::core {
class Project;
class Session;
} // namespace subedit::core

class QTableView;

namespace subedit::gui {

class SubtitleTableModel;

/// The window, and everything a project needs to be looked at.
///
/// It owns the session — the project, its history and its order policy — and
/// the table model that reads and writes through it. **The three editable
/// cells are wired**: a start, an end and a text each open the editor their
/// nature calls for. The actions — undo, redo, open, save — arrive next.
///
/// Kept in `subedit::gui` and not in `main.cpp` because that is what makes it
/// testable: `check-architecture.sh` refuses a `main` that defines a class, and
/// the reason is exactly this one.
class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    /// Opens a window on `project`, which it takes over.
    explicit MainWindow(core::Project project, QWidget* parent = nullptr);

    ~MainWindow() override;

    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    MainWindow& operator=(MainWindow&&) = delete;

    /// Returns the table, for a test to look at what the window shows.
    [[nodiscard]] QTableView* table() const { return m_table; }

private:
    QTableView* m_table;

    /// Held by pointer so that this header stays parsable by `moc`, which
    /// chokes on the C++20 library headers the core drags in.
    std::unique_ptr<core::Session> m_session;
    std::unique_ptr<SubtitleTableModel> m_model;
};

} // namespace subedit::gui
