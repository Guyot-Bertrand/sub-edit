#include <subedit/core/libraries.hpp>
#include <subedit/gui/libraries.hpp>

#include <QtGlobal>
#include <mpv/client.h>

#include <string>
#include <vector>

namespace subedit::gui {

namespace {

/// A version of libmpv's client API, `major.minor`, from the number the library packs it in.
[[nodiscard]] std::string clientApiVersion(unsigned long packed) {
    constexpr unsigned kMinorBits = 16;
    constexpr unsigned long kMinorMask = 0xffffUL;
    return std::to_string(packed >> kMinorBits) + '.' + std::to_string(packed & kMinorMask);
}

} // namespace

std::vector<core::Library> windowLibraries() {
    std::vector<core::Library> linked = core::coreLibraries();
    linked.push_back({.name = "Qt", .loaded = qVersion(), .built = QT_VERSION_STR});
    linked.push_back({.name = "libmpv",
                      .loaded = clientApiVersion(mpv_client_api_version()),
                      .built = clientApiVersion(MPV_CLIENT_API_VERSION)});
    return linked;
}

} // namespace subedit::gui
