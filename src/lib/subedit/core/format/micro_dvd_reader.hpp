#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_reader.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <expected>
#include <string_view>

namespace subedit::core {

/// Reads MicroDVD — the other of the two formats behind a `.sub`.
///
/// **The only one of the nine that does not count in time.** Its positions
/// *are* frame numbers, `{1194}{1285}`, and no MicroDVD file states the rate
/// they were counted at. The model counts in whole milliseconds since ADR 0006,
/// so reading one takes a rate from outside — and writing it back takes the
/// same one, which is what makes the round trip exact whatever the rate was.
///
/// The rate is carried by the reader rather than guessed from the content:
/// there is nothing to guess from. Frame numbers fall on every grid at once,
/// which is why the deduction of phase 16 can say nothing here. ADR 0030 holds
/// the three places it comes from instead.
class MicroDvdReader final : public SubtitleReader {

public:
    explicit MicroDvdReader(FrameRate rate, bool rateWasChosen)
        : m_rate(rate), m_rateWasChosen(rateWasChosen) {}

    [[nodiscard]] std::expected<ReadResult, ReadError>
    read(std::string_view content) const override;

private:
    FrameRate m_rate;

    /// Whether someone said that rate, or it is the declared default.
    ///
    /// The reading says which — a file opened under a rate nobody chose is the
    /// one case where the positions on screen are a guess, and ADR 0008 asks
    /// for that to be said rather than kept.
    bool m_rateWasChosen;
};

} // namespace subedit::core
