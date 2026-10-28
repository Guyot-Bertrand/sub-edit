#pragma once

#include <subedit/core/transcription/alignment_engine.hpp>
#include <subedit/core/transcription/word_matching.hpp>

#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace subedit::core {

/// The name of the alignment engine used when the setting names none.
inline constexpr std::string_view kDefaultAlignmentEngine = "transcript-matching";

/// The first adapter of the alignment port (ADR 0047): it has a transcription engine listen to the
/// audio, then matches the lines of the text to the words that engine heard.
///
/// It borrows the engine and does not own it: the registry does. Everything the transcription
/// port already refuses — a missing model, a language the pair cannot transcribe, an unreadable
/// audio file — it refuses with the same errors and before any work, since it goes through
/// `transcribe`.
class TranscriptMatchingAligner final : public AlignmentEngine {

public:
    explicit TranscriptMatchingAligner(const TranscriptionEngine& transcriber,
                                       double minimumMatch = kDefaultMinimumMatch);

    [[nodiscard]] std::string_view name() const override;

    [[nodiscard]] std::expected<EngineCapabilities, TranscriptionError>
    capabilitiesFor(const std::optional<std::filesystem::path>& model) const override;

    [[nodiscard]] std::expected<std::vector<AlignedLine>, TranscriptionError>
    align(const AlignmentRequest& request, const TranscriptionObserver& observer) const override;

    /// What `align` is made of, with the counts the doubt is judged on: how many words each line
    /// has and how many were heard.
    [[nodiscard]] std::expected<std::vector<LineMatch>, TranscriptionError>
    match(const AlignmentRequest& request, const TranscriptionObserver& observer) const;

private:
    const TranscriptionEngine* m_transcriber;
    double m_minimumMatch;
};

/// The alignment engine called `name`, listening through `transcriber`; nothing when there is
/// none of that name.
[[nodiscard]] std::unique_ptr<AlignmentEngine>
makeAlignmentEngine(std::string_view name,
                    const TranscriptionEngine& transcriber,
                    double minimumMatch = kDefaultMinimumMatch);

} // namespace subedit::core
