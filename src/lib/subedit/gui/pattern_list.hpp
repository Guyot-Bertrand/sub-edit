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

    /// One `PatternActivation` per underlying record whose box now disagrees
    /// with that record's shipped default.
    [[nodiscard]] std::vector<core::PatternActivation> activations() const;

signals:
    void changed();

private:
    struct Entry {
        std::string name;
        QCheckBox* box;
        std::vector<const core::CorrectionPattern*> records;
    };

    const core::PatternCatalogue* m_catalogue;
    core::PatternKind m_kind;
    QVBoxLayout* m_layout;
    std::vector<Entry> m_entries;
};

} // namespace subedit::gui
