#pragma once

#include <filesystem>
#include <span>
#include <string_view>

namespace subedit::core {

/// Tells whether `path` carries the extension of a video file.
///
/// **A closed list, and a judgement on the name only.** Nothing here opens the
/// file: this answers « would this name be offered as a film », which is what
/// the naming convention and the file chooser both ask.
///
/// The list is Gaupol's, in `aeidon.util.is_video_file`. What is not taken
/// from it is its first branch, `mimetypes.guess_type`, which reads a table
/// that differs from one machine to the next — the same directory would then
/// propose a video here and nothing there, with no way for the user to tell
/// why. A closed list is poorer and says the same thing everywhere.
///
/// The extension is compared without regard to case, the rest of the name is
/// not: files that came from elsewhere carry the case of elsewhere.
[[nodiscard]] bool isVideoFile(const std::filesystem::path& path);

/// The extensions themselves, dot included, in the order Gaupol lists them.
///
/// Exposed because a file chooser has to name what it filters on, and reading
/// that list from the same place the recognition uses is what keeps a chooser
/// from offering a file the rest of the program would not call a video.
[[nodiscard]] std::span<const std::string_view> videoExtensions();

} // namespace subedit::core
