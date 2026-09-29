#pragma once

// The pattern list widget of a task page — issue #505, task 8.

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/text/correction_pattern.hpp>

#include <QWidget>

#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {
class PatternCatalogue;
} // namespace subedit::core

class QCheckBox;
class QVBoxLayout;

namespace subedit::gui {

/// One tick box per pattern *name* a cascade holds — several records may
/// share a name and share a box (`CorrectionPattern::name`).
class PatternList final : public QWidget {
    Q_OBJECT

public:
    PatternList(const core::PatternCatalogue& catalogue,
                core::PatternKind kind,
                QWidget* parent = nullptr);

    /// Rebuilds the boxes for `code`'s cascade, each opened from `settings`.
    void setCode(std::string_view code, const core::CorrectionSettings& settings);

    /// One `PatternActivation` per underlying record of a *touched* box — one
    /// whose state now differs from the one `setCode` opened it on — that
    /// disagrees with that record's shipped default; once per (kind, code,
    /// name), even when two records of the same code share a name and so the
    /// same key. An untouched box reports nothing: the overrides it opened on
    /// are left where they are rather than written again.
    [[nodiscard]] std::vector<core::PatternActivation> activations() const;

    /// Every record a *touched* box stands for, agreeing with its default or
    /// not — what `core::foldActivations` erases before it adds
    /// `activations()` back. An untouched box stands for none: a shared box
    /// merely shown must not erase an override written for one of its records
    /// under another code.
    [[nodiscard]] std::vector<const core::CorrectionPattern*> shownRecords() const;

    /// Folds what the boxes now say into `activations` — `core::foldActivations`
    /// over `shownRecords()` and `activations()`. Meant for the activations
    /// `setCode` opened on, or a copy of them: after folding into those very
    /// settings, reopen with `setCode` before folding again, or a box turned
    /// back to its opened state would look untouched.
    void foldInto(std::vector<core::PatternActivation>& activations) const;

signals:
    void changed();

private:
    struct Entry {
        std::string name;
        QCheckBox* box;
        std::vector<const core::CorrectionPattern*> records;
        /// The state `setCode` opened the box on, before the user had a say.
        bool openedChecked = false;
    };

    [[nodiscard]] static bool touched(const Entry& entry);

    const core::PatternCatalogue* m_catalogue;
    core::PatternKind m_kind;
    QVBoxLayout* m_layout;
    std::vector<Entry> m_entries;
};

} // namespace subedit::gui
