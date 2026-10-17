#include <subedit/cli/json.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/cli/version_report.hpp>
#include <subedit/core/libraries.hpp>
#include <subedit/core/version.hpp>
#include <subedit/core/wording/version.hpp>

#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace subedit::cli {

void writeVersion(std::ostream& out,
                  const std::vector<core::Library>& libraries,
                  const std::vector<std::string>& languages,
                  bool json) {
    if (!json) {
        out << core::versionReport(libraries, languages);
        return;
    }

    Json listed = Json::array();
    for (const core::Library& library : libraries) {
        listed.push(Json::object()
                        .set("name", library.name)
                        .set("loaded", library.loaded)
                        .set("built", library.built));
    }
    Json codes = Json::array();
    for (const std::string& language : languages)
        codes.push(language);

    out << Json::object()
               .set("schema", kSchema)
               .set("command", "version")
               .set("version", core::versionString())
               .set("libraries", std::move(listed))
               .set("languages", std::move(codes))
               .dump()
        << '\n';
}

} // namespace subedit::cli
