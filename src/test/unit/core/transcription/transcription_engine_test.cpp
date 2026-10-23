// The port: what an engine declares, what a request may ask of it, and the
// registry that finds engines by name.

#include <subedit/core/transcription/alignment_engine.hpp>
#include <subedit/core/transcription/engine_registry.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "fake_transcription_engine.hpp"
#include "transcription_engine_contract.hpp"

namespace {

using subedit::core::EngineCapabilities;
using subedit::core::EngineRegistry;
using subedit::core::Timestamp;
using subedit::core::TranscribedSegment;
using subedit::core::TranscriptionErrorKind;
using subedit::core::TranscriptionRequest;
using subedit::test::ContractCase;
using subedit::test::FakeEngineScript;
using subedit::test::FakeTranscriptionEngine;

/// A directory that removes itself, holding an audio file that the fake engine
/// will find.
class ScratchAudio {

public:
    ScratchAudio() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        m_directory = std::filesystem::temp_directory_path() /
                      ("subedit-transcription-" + std::to_string(stamp));
        std::filesystem::create_directories(m_directory);
        m_audio = m_directory / "audio.wav";
        std::ofstream{m_audio} << "not really audio";
    }

    ScratchAudio(const ScratchAudio&) = delete;
    ScratchAudio(ScratchAudio&&) = delete;
    ScratchAudio& operator=(const ScratchAudio&) = delete;
    ScratchAudio& operator=(ScratchAudio&&) = delete;

    ~ScratchAudio() {
        std::error_code ignored;
        std::filesystem::remove_all(m_directory, ignored);
    }

    [[nodiscard]] const std::filesystem::path& audio() const { return m_audio; }

private:
    std::filesystem::path m_directory;
    std::filesystem::path m_audio;
};

TranscribedSegment segment(std::int64_t start, std::int64_t end, std::string text) {
    return {.text = std::move(text),
            .start = Timestamp::fromMilliseconds(start),
            .end = Timestamp::fromMilliseconds(end)};
}

/// A multilingual model with detection and timed words — whisper with a model of that kind.
FakeEngineScript multilingual() {
    FakeEngineScript script;
    script.name = "multilingual";
    script.capabilities = {.timedWords = true, .languageDetection = true};
    TranscribedSegment first = segment(0, 2000, "Hello there.");
    first.words = {{.text = "Hello",
                    .start = Timestamp::fromMilliseconds(0),
                    .end = Timestamp::fromMilliseconds(800)},
                   {.text = "there.",
                    .start = Timestamp::fromMilliseconds(900),
                    .end = Timestamp::fromMilliseconds(1900)}};
    script.segments = {first, segment(2500, 4000, "How are you?")};
    return script;
}

/// An English-only model: no detection, one language, no words.
FakeEngineScript englishOnly() {
    FakeEngineScript script;
    script.name = "english-only";
    script.capabilities = {.languages = {"en"}};
    script.segments = {segment(0, 1500, "Good morning.")};
    return script;
}

} // namespace

TEST_CASE("an engine that lists no language takes any", "[transcription]") {
    const EngineCapabilities any{};

    CHECK(any.supportsLanguage("fr"));
    CHECK(any.supportsLanguage("zh"));
}

TEST_CASE("an engine that lists its languages takes those and no others", "[transcription]") {
    const EngineCapabilities only{.languages = {"en"}};

    CHECK(only.supportsLanguage("en"));
    CHECK_FALSE(only.supportsLanguage("fr"));
}

TEST_CASE("a request without a model is refused when the engine needs one", "[transcription]") {
    const EngineCapabilities needs{.languageDetection = true, .needsModel = true};

    const auto checked = subedit::core::checkRequest(needs, TranscriptionRequest{.audio = "a.wav"});

    REQUIRE_FALSE(checked.has_value());
    CHECK(checked.error().kind == TranscriptionErrorKind::ModelMissing);
}

TEST_CASE("a request without a language is refused when the engine cannot detect one",
          "[transcription]") {
    const EngineCapabilities blind{.languages = {"en", "fr"}, .needsModel = false};

    const auto checked = subedit::core::checkRequest(blind, TranscriptionRequest{.audio = "a.wav"});

    REQUIRE_FALSE(checked.has_value());
    CHECK(checked.error().kind == TranscriptionErrorKind::LanguageRequired);
}

TEST_CASE("a request without a language is fine when there is only one to take",
          "[transcription]") {
    const EngineCapabilities single{.languages = {"en"}, .needsModel = false};

    CHECK(subedit::core::checkRequest(single, TranscriptionRequest{.audio = "a.wav"}).has_value());
}

TEST_CASE("the multilingual fake passes the contract", "[transcription][contract]") {
    const ScratchAudio scratch;
    const FakeTranscriptionEngine engine{multilingual()};

    subedit::test::checkTranscriptionEngineContract(
        engine,
        ContractCase{.audio = scratch.audio(),
                     .model = std::filesystem::path{"model.bin"},
                     .supportedLanguage = "fr"});
}

TEST_CASE("the English-only fake passes the contract", "[transcription][contract]") {
    const ScratchAudio scratch;
    const FakeTranscriptionEngine engine{englishOnly()};

    subedit::test::checkTranscriptionEngineContract(
        engine,
        ContractCase{.audio = scratch.audio(),
                     .model = std::filesystem::path{"model.bin"},
                     .supportedLanguage = "en",
                     .unsupportedLanguage = "fr"});
}

TEST_CASE("the refusal comes from the port, not from the adapter", "[transcription]") {
    const ScratchAudio scratch;
    const FakeTranscriptionEngine engine{englishOnly()};
    int calls = 0;

    const auto done = engine.transcribe(
        {.audio = scratch.audio(), .language = "de", .model = std::filesystem::path{"m.bin"}},
        {.progress = [&calls](double) { ++calls; }});

    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == TranscriptionErrorKind::UnsupportedLanguage);
    CHECK(calls == 0);
}

TEST_CASE("a run reports the language it was asked for, or the one it detected",
          "[transcription]") {
    const ScratchAudio scratch;
    FakeEngineScript script = multilingual();
    script.detectedLanguage = "es";
    const FakeTranscriptionEngine engine{script};

    const auto detected =
        engine.transcribe({.audio = scratch.audio(), .model = std::filesystem::path{"m.bin"}});
    const auto forced = engine.transcribe(
        {.audio = scratch.audio(), .language = "fr", .model = std::filesystem::path{"m.bin"}});

    REQUIRE(detected.has_value());
    REQUIRE(forced.has_value());
    CHECK(detected->language == "es");
    CHECK(forced->language == "fr");
}

TEST_CASE("the registry finds an engine by its name", "[transcription][registry]") {
    EngineRegistry registry;
    REQUIRE(registry.add(std::make_unique<FakeTranscriptionEngine>(multilingual())));
    REQUIRE(registry.add(std::make_unique<FakeTranscriptionEngine>(englishOnly())));

    REQUIRE(registry.find("english-only") != nullptr);
    CHECK(registry.find("english-only")->name() == "english-only");
    CHECK(registry.find("absent") == nullptr);
    CHECK(registry.names() == std::vector<std::string>{"multilingual", "english-only"});
}

TEST_CASE("the registry refuses a second engine of the same name", "[transcription][registry]") {
    EngineRegistry registry;
    REQUIRE(registry.add(std::make_unique<FakeTranscriptionEngine>(englishOnly())));

    CHECK_FALSE(registry.add(std::make_unique<FakeTranscriptionEngine>(englishOnly())));
    CHECK(registry.names().size() == 1);
}

TEST_CASE("the alignment port answers one line per line of the text",
          "[transcription][alignment]") {
    class Stub final : public subedit::core::AlignmentEngine {
    public:
        [[nodiscard]] std::string_view name() const override { return "stub"; }

        [[nodiscard]] std::expected<EngineCapabilities, subedit::core::TranscriptionError>
        capabilitiesFor(const std::optional<std::filesystem::path>& /*model*/) const override {
            return EngineCapabilities{};
        }

        [[nodiscard]] std::expected<std::vector<subedit::core::AlignedLine>,
                                    subedit::core::TranscriptionError>
        align(const subedit::core::AlignmentRequest& request,
              const subedit::core::TranscriptionObserver& /*observer*/) const override {
            return std::vector<subedit::core::AlignedLine>(request.lines.size());
        }
    };

    const Stub stub;

    const auto aligned = stub.align({.audio = "a.wav", .lines = {"one", "two"}}, {});

    REQUIRE(aligned.has_value());
    CHECK(aligned->size() == 2);
    CHECK_FALSE((*aligned)[0].start.has_value());
}

TEST_CASE("an engine that cannot say what it can do is reported, and does no work",
          "[transcription]") {
    class Absent final : public subedit::core::TranscriptionEngine {
    public:
        [[nodiscard]] std::string_view name() const override { return "absent"; }

        [[nodiscard]] std::expected<EngineCapabilities, subedit::core::TranscriptionError>
        capabilitiesFor(const std::optional<std::filesystem::path>& /*model*/) const override {
            return std::unexpected(subedit::core::TranscriptionError{
                .kind = TranscriptionErrorKind::EngineMissing, .detail = "no such program"});
        }

        mutable bool ran = false;

    protected:
        [[nodiscard]] std::expected<subedit::core::Transcription, subedit::core::TranscriptionError>
        run(const TranscriptionRequest& /*request*/,
            const EngineCapabilities& /*capabilities*/,
            const subedit::core::TranscriptionObserver& /*observer*/) const override {
            ran = true;
            return subedit::core::Transcription{};
        }
    };

    const Absent engine;

    const auto done = engine.transcribe({.audio = "a.wav"});

    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == TranscriptionErrorKind::EngineMissing);
    CHECK_FALSE(engine.ran);
}

TEST_CASE("the registry refuses an engine that is not there", "[transcription][registry]") {
    EngineRegistry registry;

    CHECK_FALSE(registry.add(nullptr));
    CHECK(registry.names().empty());
}
