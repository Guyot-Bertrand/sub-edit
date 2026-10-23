#pragma once

// The contract every transcription engine must pass.
//
// Adding an engine is writing an adapter and passing this suite (ADR 0046). The
// suite asks nothing of how an engine works: it checks what a caller may rely on
// — that the answer is well formed, that the engine tells the truth about
// itself, and that it refuses before it works.
//
// A caller supplies a `ContractCase` saying where an audio file is, which
// language the engine can transcribe and, when it has one, which it cannot.

#include <subedit/core/transcription/transcription_engine.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace subedit::test {

struct ContractCase {
    /// An audio file that exists and that the engine can read.
    std::filesystem::path audio;

    /// A model, when the engine needs one.
    std::optional<std::filesystem::path> model{};

    /// A language the engine can transcribe.
    std::string supportedLanguage;

    /// A language it cannot, when there is one (an engine that takes any
    /// language has none).
    std::optional<std::string> unsupportedLanguage{};
};

inline void checkTranscriptionEngineContract(const core::TranscriptionEngine& engine,
                                             const ContractCase& contract) {
    using core::TranscriptionErrorKind;

    const auto capabilities = engine.capabilitiesFor(contract.model);
    REQUIRE(capabilities.has_value());
    CHECK(capabilities->supportsLanguage(contract.supportedLanguage));
    if (contract.unsupportedLanguage)
        CHECK_FALSE(capabilities->supportsLanguage(*contract.unsupportedLanguage));

    // A run in a language it supports: well formed, honest, watched.
    {
        std::vector<double> progress;
        const core::TranscriptionObserver observer{
            .progress = [&progress](double fraction) { progress.push_back(fraction); }};

        const auto done = engine.transcribe({.audio = contract.audio,
                                             .language = contract.supportedLanguage,
                                             .model = contract.model},
                                            observer);

        REQUIRE(done.has_value());
        CHECK(done->language == contract.supportedLanguage);

        core::Timestamp previousStart = core::Timestamp::fromMilliseconds(0);
        for (const core::TranscribedSegment& segment : done->segments) {
            CHECK(segment.start <= segment.end);
            CHECK(previousStart <= segment.start);
            previousStart = segment.start;

            if (!capabilities->timedWords)
                CHECK(segment.words.empty());
            core::Timestamp previousWord = segment.start;
            for (const core::TimedWord& word : segment.words) {
                CHECK(word.start <= word.end);
                CHECK(segment.start <= word.start);
                CHECK(word.end <= segment.end);
                CHECK(previousWord <= word.start);
                previousWord = word.start;
            }
            if (!capabilities->speakers)
                CHECK_FALSE(segment.speaker.has_value());
        }

        // Progress stays in [0, 1], never goes back, and ends at 1.
        double last = 0.0;
        for (const double fraction : progress) {
            CHECK(fraction >= 0.0);
            CHECK(fraction <= 1.0);
            CHECK(fraction >= last);
            last = fraction;
        }
        if (!progress.empty())
            CHECK(progress.back() == 1.0);
    }

    // No language asked for: detection answers, or the engine asks for one.
    {
        const auto done = engine.transcribe({.audio = contract.audio, .model = contract.model});
        if (capabilities->languageDetection) {
            REQUIRE(done.has_value());
            CHECK(done->language.has_value());
        } else if (capabilities->languages.size() != 1) {
            REQUIRE_FALSE(done.has_value());
            CHECK(done.error().kind == TranscriptionErrorKind::LanguageRequired);
        }
    }

    // A language it cannot do is refused **before any work**: no progress.
    if (contract.unsupportedLanguage) {
        int calls = 0;
        const core::TranscriptionObserver observer{.progress = [&calls](double) { ++calls; }};

        const auto done = engine.transcribe({.audio = contract.audio,
                                             .language = contract.unsupportedLanguage,
                                             .model = contract.model},
                                            observer);

        REQUIRE_FALSE(done.has_value());
        CHECK(done.error().kind == TranscriptionErrorKind::UnsupportedLanguage);
        CHECK(calls == 0);
    }

    // A model is required when the engine says so.
    if (capabilities->needsModel) {
        const auto done =
            engine.transcribe({.audio = contract.audio, .language = contract.supportedLanguage});
        REQUIRE_FALSE(done.has_value());
        CHECK(done.error().kind == TranscriptionErrorKind::ModelMissing);
    }

    // Asked to stop, it stops and says so.
    {
        const core::TranscriptionObserver observer{.cancelled = [] { return true; }};
        const auto done = engine.transcribe({.audio = contract.audio,
                                             .language = contract.supportedLanguage,
                                             .model = contract.model},
                                            observer);
        REQUIRE_FALSE(done.has_value());
        CHECK(done.error().kind == TranscriptionErrorKind::Cancelled);
    }

    // An audio file that is not there is reported as such.
    {
        const auto done = engine.transcribe({.audio = contract.audio.parent_path() / "absent.wav",
                                             .language = contract.supportedLanguage,
                                             .model = contract.model});
        REQUIRE_FALSE(done.has_value());
        CHECK(done.error().kind == TranscriptionErrorKind::AudioUnreadable);
    }
}

} // namespace subedit::test
