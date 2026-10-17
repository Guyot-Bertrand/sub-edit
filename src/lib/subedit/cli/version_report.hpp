#pragma once

// What `--version` writes on the command line — issue #716.

#include <subedit/core/libraries.hpp>

#include <iosfwd>
#include <string>
#include <vector>

namespace subedit::cli {

/// Writes the program's version, the libraries it runs against and the installed languages.
///
/// **In text, the block of `core::versionReport`**, the window's as well: `subedit <version>` on
/// the first line, alone, then the rest.
///
/// **In `json`, one object**: `{"schema":1,"command":"version","version":"…","libraries":
/// [{"name":"ICU","loaded":"74.2","built":"74.2"},…],"languages":["en","fr"]}` — the shape of
/// `--list-encodings`, an answer about the tool and not a file's account (ADR 0038), on one line.
/// Nothing in it is translated.
void writeVersion(std::ostream& out,
                  const std::vector<core::Library>& libraries,
                  const std::vector<std::string>& languages,
                  bool json);

} // namespace subedit::cli
