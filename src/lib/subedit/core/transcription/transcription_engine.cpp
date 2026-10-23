#include <subedit/core/transcription/transcription_engine.hpp>

namespace subedit::core {

std::expected<void, TranscriptionError> checkRequest(const EngineCapabilities& capabilities,
                                                     const TranscriptionRequest& request) {
    if (capabilities.needsModel && !request.model)
        return std::unexpected(TranscriptionError{.kind = TranscriptionErrorKind::ModelMissing});

    if (request.language) {
        if (!capabilities.supportsLanguage(*request.language))
            return std::unexpected(TranscriptionError{
                .kind = TranscriptionErrorKind::UnsupportedLanguage, .detail = *request.language});
        return {};
    }

    // No language asked for: detection answers, or the only language there is.
    if (capabilities.languageDetection || capabilities.languages.size() == 1)
        return {};
    return std::unexpected(TranscriptionError{.kind = TranscriptionErrorKind::LanguageRequired});
}

std::expected<Transcription, TranscriptionError>
TranscriptionEngine::transcribe(const TranscriptionRequest& request,
                                const TranscriptionObserver& observer) const {
    const std::expected<EngineCapabilities, TranscriptionError> capabilities =
        capabilitiesFor(request.model);
    if (!capabilities)
        return std::unexpected(capabilities.error());

    if (const std::expected<void, TranscriptionError> checked =
            checkRequest(*capabilities, request);
        !checked)
        return std::unexpected(checked.error());

    return run(request, *capabilities, observer);
}

} // namespace subedit::core
