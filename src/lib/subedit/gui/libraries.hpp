#pragma once

// The libraries the window runs against — issue #716.

#include <subedit/core/libraries.hpp>

#include <vector>

namespace subedit::gui {

/// The core's libraries, then the two only the window links: Qt (`qVersion()` against
/// `QT_VERSION_STR`) and libmpv. **libmpv is told by the version of its client API**
/// (`mpv_client_api_version()` against `MPV_CLIENT_API_VERSION`, `2.2`): it is what the library
/// says of itself, and what `mpv.pc` announces.
///
/// The one list `--version` and `About subedit` both show, so that they cannot disagree.
[[nodiscard]] std::vector<core::Library> windowLibraries();

} // namespace subedit::gui
