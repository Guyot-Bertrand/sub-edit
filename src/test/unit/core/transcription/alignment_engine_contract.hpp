#pragma once

// The contract every alignment engine must pass.
//
// Adding an engine is writing an adapter and passing this suite (ADR 0046), as for transcription.
// It asks nothing of how an engine finds where a line is said: it checks what a caller may rely on
// — that the answer has one entry per line of the text, in order; that **a line not found has no
// position at all, never an invented one**; that the engine refuses before it works; and that it
// stops when asked.

#include <subedit/core/transcription/alignment_engine.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace subedit::test {

struct AlignmentContractCase {
    /// An audio file that exists and that the engine can read.
    std::filesystem::path audio;

    /// A model, when the engine needs one.
    std::optional<std::filesystem::path> model{};

    /// At least three lines, in the order they are said.
    std::vector<std::string> lines;
};

inline void checkAlignmentEngineContract(const core::AlignmentEngine& engine,
                                         const AlignmentContractCase& contract) {
    using core::TranscriptionErrorKind;

    const auto capabilities = engine.capabilitiesFor(contract.model);
    REQUIRE(capabilities.has_value());

    // One entry per line, in order, well formed.
    {
        std::vector<double> progress;
        const core::TranscriptionObserver observer{
            .progress = [&progress](double fraction) { progress.push_back(fraction); }};

        const auto done = engine.align(
            {.audio = contract.audio, .lines = contract.lines, .model = contract.model}, observer);

        REQUIRE(done.has_value());
        REQUIRE(done->size() == contract.lines.size());
        std::optional<core::Timestamp> previous;
        for (const core::AlignedLine& line : *done) {
            // A line has a position or has none: never half of one.
            CHECK(line.start.has_value() == line.end.has_value());
            if (!line.start)
                continue;
            CHECK(*line.start <= *line.end);
            // The text is in the order it is said, so the starts do not go back.
            if (previous)
                CHECK(*previous <= *line.start);
            previous = line.start;
        }

        double last = 0.0;
        for (const double fraction : progress) {
            CHECK(fraction >= 0.0);
            CHECK(fraction <= 1.0);
            CHECK(fraction >= last);
            last = fraction;
        }
    }

    // No text, no entry.
    {
        const auto done =
            engine.align({.audio = contract.audio, .lines = {}, .model = contract.model}, {});
        REQUIRE(done.has_value());
        CHECK(done->empty());
    }

    // A model is required when the engine says so, and refused **before any work**.
    if (capabilities->needsModel) {
        int calls = 0;
        const core::TranscriptionObserver observer{.progress = [&calls](double) { ++calls; }};
        const auto done =
            engine.align({.audio = contract.audio, .lines = contract.lines}, observer);
        REQUIRE_FALSE(done.has_value());
        CHECK(done.error().kind == TranscriptionErrorKind::ModelMissing);
        CHECK(calls == 0);
    }

    // An audio file that is not there.
    {
        const auto done = engine.align({.audio = contract.audio.parent_path() / "absent.wav",
                                        .lines = contract.lines,
                                        .model = contract.model},
                                       {});
        REQUIRE_FALSE(done.has_value());
        CHECK(done.error().kind == TranscriptionErrorKind::AudioUnreadable);
    }

    // Asked to stop, it stops and says so.
    {
        const core::TranscriptionObserver observer{.cancelled = [] { return true; }};
        const auto done = engine.align(
            {.audio = contract.audio, .lines = contract.lines, .model = contract.model}, observer);
        REQUIRE_FALSE(done.has_value());
        CHECK(done.error().kind == TranscriptionErrorKind::Cancelled);
    }
}

} // namespace subedit::test
