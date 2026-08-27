#pragma once

#include <subedit/core/time/frame_rate.hpp>

#include <QComboBox>

class QWidget;

namespace subedit::gui {

/// The eight standard frame rates, in a box, named as a report names them.
///
/// **A closed list rather than free entry.** A rate that is not one of these is
/// a rate no video carries, and an operation that rewrites a whole file is not
/// the place to accept a typo.
///
/// **One class rather than a function copied.** The conversion and the
/// alignment both ask for a rate, and each carried its own copy of the list and
/// of the search through it — down to the comment justifying the second. Phase
/// 9 brings an editing mode in frames and phase 14 a fine alignment; both will
/// want the same list, and the third copy would have written itself.
class FrameRateBox final : public QComboBox {
    Q_OBJECT

public:
    explicit FrameRateBox(QWidget* parent = nullptr);

    /// The rate the box is showing.
    [[nodiscard]] core::FrameRate rate() const;

    /// Shows `wanted`, and does nothing at all when the list does not carry it.
    ///
    /// **Finding nothing is part of the contract**, and a caller has to know
    /// it: a film may declare a rate this closed list does not have, so opening
    /// the box on what the film says takes two steps — a rate that is certainly
    /// in the list first, the film's second. Without the first step the box
    /// would stay where it was built, on the first of the eight, showing a rate
    /// nobody named.
    void pick(core::FrameRate wanted);
};

} // namespace subedit::gui
