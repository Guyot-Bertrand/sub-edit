#pragma once

#include <QDialog>

#include <memory>

class QCheckBox;
class QDialogButtonBox;
class QLabel;
class QTableWidget;

namespace subedit::gui {

struct SynchronizationPreview;

/// The result of `Synchronize with Audio…`, shown before it moves anything: for each subtitle that
/// was looked for, the start it had, the start it takes, how far that is, and what there is to
/// check about it — and two choices, apply it or leave the project as it is.
///
/// **The lines to check are marked and can be shown alone**: a subtitle that was not found keeps
/// its times, one only partly heard or moved differently from its neighbours takes the place found
/// but is flagged. The table is read, not edited.
class SynchronizationPreviewDialog final : public QDialog {
    Q_OBJECT

public:
    SynchronizationPreviewDialog(const SynchronizationPreviewDialog&) = delete;
    SynchronizationPreviewDialog& operator=(const SynchronizationPreviewDialog&) = delete;
    SynchronizationPreviewDialog(SynchronizationPreviewDialog&&) = delete;
    SynchronizationPreviewDialog& operator=(SynchronizationPreviewDialog&&) = delete;
    ~SynchronizationPreviewDialog() override;

    explicit SynchronizationPreviewDialog(SynchronizationPreview preview,
                                          QWidget* parent = nullptr);

    /// Six columns: the number of the subtitle, its text, the start it had, the start it takes, the
    /// shift between the two, and what to check.
    [[nodiscard]] QTableWidget* table() const { return m_table; }

    /// What it came to, in a sentence: how many were placed, how many are to check, and the median
    /// shift and the drift.
    [[nodiscard]] QLabel* summary() const { return m_summary; }

    /// Shows only the subtitles to check.
    [[nodiscard]] QCheckBox* onlyToCheckBox() const { return m_onlyToCheck; }

    [[nodiscard]] QDialogButtonBox* buttons() const { return m_buttons; }

private:
    void showOnlyToCheck(bool only);

    std::unique_ptr<SynchronizationPreview> m_preview;
    QLabel* m_summary;
    QCheckBox* m_onlyToCheck;
    QTableWidget* m_table;
    QDialogButtonBox* m_buttons;
};

} // namespace subedit::gui
