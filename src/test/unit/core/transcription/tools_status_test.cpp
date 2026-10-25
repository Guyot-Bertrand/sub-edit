// What the transcription needs, and which of it is there.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/transcription/tools_status.hpp>
#include <subedit/core/wording/transcription.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <string>
#include <tuple>

#include "scratch_directory.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::inspectTranscriptionTools;
using subedit::core::ModelState;
using subedit::core::RealFileSystem;
using subedit::core::TranscriptionSettings;
using subedit::test::ScratchDirectory;

std::string header(unsigned vocabulary) {
    std::string bytes = "lmgg";
    for (int index = 0; index < 4; ++index)
        bytes += static_cast<char>((vocabulary >> (8 * index)) & 0xFFU);
    return bytes + std::string(8, '\0');
}

} // namespace

TEST_CASE("nothing installed is nothing found", "[transcription][tools]") {
    const ScratchDirectory scratch;
    const RealFileSystem files;

    const auto tools =
        inspectTranscriptionTools(files, scratch.path().string(), TranscriptionSettings{});

    CHECK_FALSE(tools.engine.has_value());
    CHECK_FALSE(tools.extractor);
    CHECK(tools.model == ModelState::NotSet);
    CHECK_FALSE(tools.ready());
}

TEST_CASE("the engine and ffmpeg are found on the search path", "[transcription][tools]") {
    const ScratchDirectory scratch;
    const RealFileSystem files;
    const auto engine = scratch.standIn("whisper-cli", "/bin/true", "");
    std::ignore = scratch.standIn("ffmpeg", "/bin/true", "");

    const auto tools =
        inspectTranscriptionTools(files, scratch.path().string(), TranscriptionSettings{});

    CHECK(tools.engine == engine);
    CHECK(tools.extractor);
}

TEST_CASE("the executable of the settings is taken before the search path",
          "[transcription][tools]") {
    const ScratchDirectory scratch;
    const RealFileSystem files;
    const auto mine = scratch.standIn("my-whisper", "/bin/true", "");
    TranscriptionSettings settings;
    settings.executable = mine;

    CHECK(inspectTranscriptionTools(files, "", settings).engine == mine);

    settings.executable = scratch.file("absent");
    CHECK_FALSE(inspectTranscriptionTools(files, "", settings).engine.has_value());
}

TEST_CASE("an engine nobody registered is not found", "[transcription][tools]") {
    const ScratchDirectory scratch;
    const RealFileSystem files;
    std::ignore = scratch.standIn("whisper-cli", "/bin/true", "");
    TranscriptionSettings settings;
    settings.engine = "other";

    CHECK_FALSE(
        inspectTranscriptionTools(files, scratch.path().string(), settings).engine.has_value());
}

TEST_CASE("a model is told apart by its first bytes", "[transcription][tools]") {
    const ScratchDirectory scratch;
    const RealFileSystem files;
    const auto state = [&](const std::filesystem::path& model) {
        TranscriptionSettings settings;
        settings.model = model;
        return inspectTranscriptionTools(files, "", settings).model;
    };

    CHECK(state(scratch.write("multi.bin", header(51865))) == ModelState::Multilingual);
    CHECK(state(scratch.write("english.bin", header(51864))) == ModelState::EnglishOnly);
    CHECK(state(scratch.write("other.bin", "not a model at all")) == ModelState::NotAModel);
    CHECK(state(scratch.file("absent.bin")) == ModelState::NotFound);
    CHECK(state({}) == ModelState::NotSet);
}

TEST_CASE("everything there means a transcription can start", "[transcription][tools]") {
    const ScratchDirectory scratch;
    const RealFileSystem files;
    std::ignore = scratch.standIn("whisper-cli", "/bin/true", "");
    std::ignore = scratch.standIn("ffmpeg", "/bin/true", "");
    TranscriptionSettings settings;
    settings.model = scratch.write("multi.bin", header(51865));

    CHECK(inspectTranscriptionTools(files, scratch.path().string(), settings).ready());
}

TEST_CASE("the report says each tool on its own line", "[transcription][tools][wording]") {
    TranscriptionSettings settings;
    settings.model = "/models/small.bin";

    const std::string missing = subedit::core::transcriptionToolsReport({}, settings);
    CHECK_THAT(missing, ContainsSubstring("whisper.cpp: not found"));
    CHECK_THAT(missing, ContainsSubstring("ffmpeg: not found"));
    CHECK_THAT(missing, ContainsSubstring("Model: none set"));

    const subedit::core::TranscriptionTools there{.engine =
                                                      std::filesystem::path{"/usr/bin/whisper-cli"},
                                                  .extractor = true,
                                                  .model = ModelState::Multilingual};
    const std::string found = subedit::core::transcriptionToolsReport(there, settings);
    CHECK_THAT(found, ContainsSubstring("whisper.cpp: found (/usr/bin/whisper-cli)"));
    CHECK_THAT(found, ContainsSubstring("ffmpeg: found"));
    CHECK_THAT(found, ContainsSubstring("Model: multilingual (/models/small.bin)"));
}

TEST_CASE("the report names each state of the model", "[transcription][tools][wording]") {
    TranscriptionSettings settings;
    settings.model = "/m.bin";
    const auto said = [&](ModelState state) {
        return subedit::core::transcriptionToolsReport({.model = state}, settings);
    };

    CHECK_THAT(said(ModelState::NotFound), ContainsSubstring("Model: not found (/m.bin)"));
    CHECK_THAT(said(ModelState::NotAModel), ContainsSubstring("Model: not a model (/m.bin)"));
    CHECK_THAT(said(ModelState::EnglishOnly), ContainsSubstring("Model: English only (/m.bin)"));
}

TEST_CASE("the engine says what it can do with the model, once both are there",
          "[transcription][tools]") {
    const ScratchDirectory scratch;
    const RealFileSystem files;
    std::ignore = scratch.standIn("whisper-cli", "/bin/true", "");
    TranscriptionSettings settings;

    // No model: nothing to ask the engine about, and the option is not refused for it.
    auto tools = inspectTranscriptionTools(files, scratch.path().string(), settings);
    CHECK_FALSE(tools.capabilities.has_value());
    CHECK(tools.suppressNotesAvailable());

    settings.model = scratch.write("multilingual.bin", header(51865));
    tools = inspectTranscriptionTools(files, scratch.path().string(), settings);
    REQUIRE(tools.capabilities.has_value());
    const subedit::core::EngineCapabilities known =
        tools.capabilities.value_or(subedit::core::EngineCapabilities{});
    CHECK(known.languageDetection);
    CHECK(known.suppressNotes);
    CHECK(tools.suppressNotesAvailable());

    // An engine that says it cannot makes the option unavailable.
    tools.capabilities = subedit::core::EngineCapabilities{.suppressNotes = false};
    CHECK_FALSE(tools.suppressNotesAvailable());
}
