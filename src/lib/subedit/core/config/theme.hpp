#pragma once

namespace subedit::core {

/// Which palette the window wears.
///
/// **Three values, and the first does nothing** — decision D3 of the scoping of
/// phase 7. Qt is at 6.4.2 on the target and has no colour scheme API at all:
/// `QStyleHints::colorScheme` arrived in 6.5. The application can therefore
/// neither ask the system what it prefers nor be told when it changes its mind.
///
/// Gaupol makes the same thing of the same constraint, one version ahead: its
/// "system" theme is resolved on GTK 4.20 and above alone, and below that its
/// update does nothing whatever. That is what turns the constraint into
/// designed behaviour rather than a dead end — we ship the two themes a user
/// can ask for, and we do not invent a reading of the system we cannot do.
///
/// **And it is that shape which makes the theme testable**: because light and
/// dark are palettes we lay down, a test can lay them down too and read what it
/// gets.
enum class Theme {
    System, ///< rien n'est posé : la palette reste celle de la plate-forme
    Light,
    Dark,
};

} // namespace subedit::core
