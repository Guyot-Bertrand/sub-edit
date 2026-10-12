#include <subedit/cli/verbosity.hpp>
#include <subedit/core/i18n/messages.hpp>

namespace subedit::cli {

std::expected<int, std::string> levelFrom(bool quiet, int verboseCount) {
    if (quiet && verboseCount > 0) {
        return std::unexpected{std::string{
            core::translate("--quiet and -v ask for opposite things; give one or the other")}};
    }
    if (quiet) {
        return 0;
    }
    return verboseCount > 0 ? verboseCount : 1;
}

} // namespace subedit::cli
