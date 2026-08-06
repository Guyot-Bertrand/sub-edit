#pragma once

namespace subedit::core {

/// One of the two texts a subtitle carries.
///
/// A subtitle holds both texts and a single pair of positions: a translation
/// has no timing of its own. Operations that apply to one text or the other
/// name it with this enumeration rather than with a flag.
enum class Document {
    Main,
    Translation,
};

} // namespace subedit::core
