#include <subedit/cli/records.hpp>
#include <subedit/cli/reporter.hpp>

#include <utility>
#include <variant>

namespace subedit::cli {

std::string_view idOf(core::DiagnosticKind kind) {
    using core::DiagnosticKind;
    switch (kind) {
    case DiagnosticKind::IgnoredLine:
        return "ignored-line";
    case DiagnosticKind::MalformedTimestamp:
        return "malformed-timestamp";
    case DiagnosticKind::MissingNumbering:
        return "missing-numbering";
    case DiagnosticKind::InconsistentNumbering:
        return "inconsistent-numbering";
    case DiagnosticKind::TextBeforeAnyTimestamp:
        return "text-before-any-timestamp";
    case DiagnosticKind::UnknownBlock:
        return "unknown-block";
    case DiagnosticKind::UnknownEventField:
        return "unknown-event-field";
    case DiagnosticKind::AssumedFrameRate:
        return "assumed-frame-rate";
    case DiagnosticKind::DeducedEnds:
        return "deduced-ends";
    case DiagnosticKind::MixedNewlines:
        return "mixed-newlines";
    case DiagnosticKind::GuessedEncoding:
        return "guessed-encoding";
    case DiagnosticKind::MarkOverridesEncoding:
        return "mark-overrides-encoding";
    }
    std::unreachable();
}

std::string_view idOf(core::FileErrorKind kind) {
    switch (kind) {
    case core::FileErrorKind::NotFound:
        return "not-found";
    case core::FileErrorKind::PermissionDenied:
        return "permission-denied";
    case core::FileErrorKind::Io:
        return "io";
    }
    std::unreachable();
}

std::string_view idOf(core::ReadErrorKind kind) {
    switch (kind) {
    case core::ReadErrorKind::Undecodable:
        return "undecodable";
    case core::ReadErrorKind::NoSubtitleFound:
        return "no-subtitle-found";
    case core::ReadErrorKind::UnknownFormat:
        return "unknown-format";
    }
    std::unreachable();
}

std::string_view idOf(core::WriteErrorKind kind) {
    switch (kind) {
    case core::WriteErrorKind::Unencodable:
        return "unencodable";
    }
    std::unreachable();
}

std::string_view idOf(const core::OpenError& error) {
    return std::visit([](const auto& one) { return idOf(one.kind); }, error);
}

Json countsOf(const std::vector<Count>& counts) {
    Json object = Json::object();
    for (const Count& count : counts) {
        object.set(count.key, count.value);
    }
    return object;
}

Json warningsOf(std::span<const core::Diagnostic> diagnostics) {
    Json warnings = Json::array();
    for (const core::Diagnostic& diagnostic : diagnostics) {
        Json one = Json::object();
        one.set("kind", idOf(diagnostic.kind));
        // A diagnostic about the whole file has no line, and none is invented.
        if (diagnostic.line != core::kWholeFile) {
            one.set("line", diagnostic.line);
        }
        if (!diagnostic.detail.empty()) {
            one.set("detail", diagnostic.detail);
        }
        // The reader decided something, as against leaving it as it stood.
        if (diagnostic.severity == core::Severity::Recovered) {
            one.set("settled", true);
        }
        warnings.push(std::move(one));
    }
    return warnings;
}

Json recordOf(std::string_view command, std::string_view file, bool ok, Json& warnings) {
    const CleanedText cleaned = cleanedUtf8(file);
    if (cleaned.replaced) {
        warnings.push(Json::object().set("kind", "path-not-utf8"));
    }

    Json record = Json::object();
    record.set("schema", kSchema);
    record.set("command", command);
    record.set("file", cleaned.text);
    record.set("ok", ok);
    return record;
}

Json failureRecord(std::string_view command,
                   std::string_view file,
                   const std::string& message,
                   std::string_view kind) {
    Json warnings = Json::array();
    Json record = recordOf(command, file, false, warnings);
    record.set("error", Json::object().set("kind", kind).set("message", message));
    // Not part of the shape of a failure, and written only when the path needs it.
    if (cleanedUtf8(file).replaced) {
        record.set("warnings", std::move(warnings));
    }
    return record;
}

Json writtenRecord(std::string_view command,
                   std::string_view file,
                   const std::filesystem::path& destination,
                   const std::vector<Count>& counts,
                   Json warnings) {
    Json record = recordOf(command, file, true, warnings);
    record.set("dry_run", false);
    record.set("destination", destination.string());
    record.set("counts", countsOf(counts));
    record.set("warnings", std::move(warnings));
    return record;
}

void reportFailure(const Reporter& reporter, std::string_view file, const Failure& failure) {
    const std::string line = std::string{file} + ": " + failure.message;
    reporter.failed(line);
    if (reporter.recording()) {
        reporter.record(failureRecord(reporter.command(), file, line, failure.kind));
    }
}

} // namespace subedit::cli
