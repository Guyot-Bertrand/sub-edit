#include <subedit/cli/encoding_listing.hpp>
#include <subedit/cli/json.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/core/model/encoding.hpp>

#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

namespace subedit::cli {

std::size_t listEncodings(std::ostream& out, bool json) {
    const std::vector<std::string> names = core::availableEncodings();
    for (const std::string& name : names) {
        if (json) {
            out << Json::object()
                       .set("schema", kSchema)
                       .set("command", "list-encodings")
                       .set("name", name)
                       .dump()
                << '\n';
        } else {
            out << name << '\n';
        }
    }
    return names.size();
}

} // namespace subedit::cli
