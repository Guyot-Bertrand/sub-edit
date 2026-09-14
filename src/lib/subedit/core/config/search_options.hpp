#pragma once

namespace subedit::core {

/// The two options of a search, as the window keeps them from one session to
/// the next.
///
/// **Gaupol's two, and Gaupol's defaults**: plain text rather than a regular
/// expression, and the case ignored. They live beside the other settings rather
/// than beside the search, because a setting is what the file of preferences
/// carries — the same reason `InsertPlacement` lives here.
struct SearchOptions {
    /// Reads the pattern as a regular expression rather than as plain text.
    bool regex = false;

    /// Finds « bonjour » in « Bonjour ».
    bool ignoreCase = true;

    friend bool operator==(const SearchOptions&, const SearchOptions&) = default;
};

} // namespace subedit::core
