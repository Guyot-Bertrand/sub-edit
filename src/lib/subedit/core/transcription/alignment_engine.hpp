#pragma once

#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/engine_capabilities.hpp>
#include <subedit/core/transcription/transcription_engine.hpp>
#include <subedit/core/transcription/transcription_error.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace subedit::core {

/// What an alignment is asked to do: find where a known text is spoken.
struct AlignmentRequest {
    std::filesystem::path audio;

    /// The lines of the text, in order. The engine finds where each is said.
    std::vector<std::string> lines;

    std::optional<std::string> language{};
    std::optional<std::filesystem::path> model{};
};

/// Where one line is said. **Nothing for a line the engine did not find**:
/// phase 23 reports those instead of inventing a position.
struct AlignedLine {
    std::optional<Timestamp> start{};
    std::optional<Timestamp> end{};

    friend bool operator==(const AlignedLine&, const AlignedLine&) = default;
};

/// The second port: audio and a text it already has in, timings out.
///
/// **Only its contract is written in phase 22**; the first adapter and the
/// command come with phase 23. It is a port of its own because aligning a text
/// one has is not the problem of finding what was said, and an engine can offer
/// one without the other — a single adapter can still offer both.
///
/// The answer has exactly one `AlignedLine` per line of the request, in order.
class AlignmentEngine {

public:
    AlignmentEngine(const AlignmentEngine&) = delete;
    AlignmentEngine(AlignmentEngine&&) = delete;
    AlignmentEngine& operator=(const AlignmentEngine&) = delete;
    AlignmentEngine& operator=(AlignmentEngine&&) = delete;
    virtual ~AlignmentEngine() = default;

    [[nodiscard]] virtual std::string_view name() const = 0;

    [[nodiscard]] virtual std::expected<EngineCapabilities, TranscriptionError>
    capabilitiesFor(const std::optional<std::filesystem::path>& model) const = 0;

    [[nodiscard]] virtual std::expected<std::vector<AlignedLine>, TranscriptionError>
    align(const AlignmentRequest& request, const TranscriptionObserver& observer) const = 0;

protected:
    AlignmentEngine() = default;
};

} // namespace subedit::core
