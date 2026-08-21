#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/transforming.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/transform_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/wording.hpp>

#include <cstddef>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>

namespace subedit::cli {

namespace {

/// A reference as the option that carried it was written: `3=00:00:10.000`.
///
/// The period, and not the comma of SubRip: this is the grammar of the command
/// line, and echoing back something the user could not have typed would be an
/// odd way of telling them what was done.
std::string wordedOf(const Reference& reference) {
    return std::to_string(reference.number) + "=" +
           reference.target.format(core::DecimalMark::Period);
}

/// Says why `reference` names nothing in a file of `count` subtitles.
std::optional<std::string> beyondOf(const Reference& reference, std::size_t count) {
    if (reference.number <= count) {
        return std::nullopt;
    }
    return "subtitle " + std::to_string(reference.number) + " is past the end: the file holds " +
           core::countOf(count, "subtitle");
}

/// Says which subtitle the transform would take before the origin, if any.
///
/// Read from the project **after** the transform rather than predicted from the
/// ratio: the arithmetic that decides belongs to the core, and a second copy of
/// it here would be one to keep in step.
std::optional<std::string> beforeOriginOf(const core::Project& project) {
    const std::span<const core::Subtitle> subtitles = project.subtitles();
    for (std::size_t i = 0; i < subtitles.size(); ++i) {
        if (subtitles[i].start.milliseconds() < 0 || subtitles[i].end.milliseconds() < 0) {
            return "subtitle " + std::to_string(i + 1) +
                   " would land before the origin, which no subtitle file can hold";
        }
    }
    return std::nullopt;
}

} // namespace

std::expected<Transform, std::string> Transform::between(Reference first, Reference last) {
    if (first.number == last.number) {
        return std::unexpected{"--first and --last both name subtitle " +
                               std::to_string(first.number) +
                               ", and two references on one subtitle define no transform"};
    }
    return Transform{first, last};
}

ExitCode transformAll(core::FileSystem& files,
                      const std::vector<std::string>& paths,
                      const Transform& transform,
                      const Destination& destination,
                      const Reporter& reporter) {
    const Operation retime =
        [&transform](core::Session& session) -> std::expected<std::string, std::string> {
        const std::size_t count = session.project().subtitles().size();
        if (const std::optional<std::string> beyond = beyondOf(transform.first(), count)) {
            return std::unexpected{*beyond};
        }
        if (const std::optional<std::string> beyond = beyondOf(transform.last(), count)) {
            return std::unexpected{*beyond};
        }

        std::optional<core::TransformCommand> command = core::TransformCommand::create(
            session.project(),
            core::Selection::all(session.project()),
            core::TransformReference{.index =
                                         core::SubtitleIndex::fromNumber(transform.first().number),
                                     .target = transform.first().target},
            core::TransformReference{.index =
                                         core::SubtitleIndex::fromNumber(transform.last().number),
                                     .target = transform.last().target});
        if (!command.has_value()) {
            return std::unexpected{"subtitles " + std::to_string(transform.first().number) +
                                   " and " + std::to_string(transform.last().number) +
                                   " start at the same moment, so they define no transform"};
        }

        session.apply(std::make_unique<core::TransformCommand>(std::move(*command)));

        // Undone rather than left applied: an operation that says it failed
        // leaves the session as it found it, whatever the caller does next.
        if (const std::optional<std::string> refused = beforeOriginOf(session.project())) {
            session.undo();
            return std::unexpected{*refused};
        }
        return core::countOf(session.project().count(), "subtitle") + " transformed onto " +
               wordedOf(transform.first()) + " and " + wordedOf(transform.last());
    };

    return rewriteAll(files, paths, destination, reporter, "transformed", retime);
}

} // namespace subedit::cli
