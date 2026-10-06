#include <subedit/cli/opening.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/format/open_error.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/wording/formats.hpp>

#include <expected>
#include <utility>

namespace subedit::cli {

namespace {

[[nodiscard]] std::optional<core::OpenedFile>
reported(std::expected<core::OpenedFile, core::OpenError> opened,
         const std::string& path,
         const Reporter& reporter) {
    if (!opened) {
        reportFailure(
            reporter, path, Failure{idOf(opened.error()), std::string{reasonOf(opened.error())}});
        return std::nullopt;
    }
    return std::move(*opened);
}

} // namespace

std::optional<core::OpenedFile> openReporting(const core::FileSystem& files,
                                              const std::string& path,
                                              const std::optional<core::Encoding>& reading,
                                              const Reporter& reporter) {
    return reported(reading ? core::openProject(files, path, *reading)
                            : core::openProject(files, path),
                    path,
                    reporter);
}

std::optional<core::OpenedFile> openReporting(const core::FileSystem& files,
                                              const std::string& path,
                                              const core::ReadingChoices& reading,
                                              const Reporter& reporter) {
    return reported(core::openProject(files, path, reading), path, reporter);
}

} // namespace subedit::cli
