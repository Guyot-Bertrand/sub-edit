# 0023 — Two native packages, `.deb` and `.rpm`, for the first release

**Date:** 2026-08-28
**Status:** accepted

Decided when phase 7 was scoped (#237).

> **Status note — 2026-08-30, after the end-of-phase-7 review.**
> What follows says that Flatpak and AppImage are **deferred**; they have since
> been **dismissed**. The question was examined in
> [#265](https://github.com/Guyot-Bertrand/sub-edit/issues/265) and closed:
> `.deb` and `.rpm` are enough for the intended scope, and a distribution that
> belongs neither to the Debian family nor to the Fedora family builds from
> source — a path that `docs/manual/subedit-cli/installation.md` describes in
> full.
>
> **The body of the ADR is not rewritten**, because a decision record keeps
> what it decided and when. The two "Why not" sections therefore remain right
> in substance: they already said the cost was not justified. Only their "for
> now" has stopped being true.

## Context

Phase 7 is the one that makes the tool installable. The project has today **no
`install()` rule** — checked, no occurrence in the tree — and
`docs/manual/subedit-cli/installation.md` still says the tool is run from the
build tree.

The roadmap puts the question as a choice between three formats: Flatpak,
`.deb`, AppImage. It has been left open since phase 3 was scoped, deliberately
— "a target written before the packaging format is settled would prejudge the
answer".

### What Gaupol does

A `Makefile` with `DESTDIR` and `PREFIX`, which installs the launcher into
`BINDIR`, the icons under `$DATADIR/icons/hicolor/{scalable,symbolic}/apps`,
the `.desktop` file into `$DATADIR/applications`, the AppStream file into
`$DATADIR/metainfo`, the manual page into `$MANDIR/man1`, and the translations
under `LOCALEDIR`. It calls `update-desktop-database` only when `DESTDIR` is
empty, that is, when the installation is not being staged for a package.

And, **alongside**, a Flatpak manifest. `PACKAGING.md` is addressed to
distribution packagers and describes two possible splits.

In other words: Gaupol does not choose a format. It produces a **correct
installation**, and the formats build on it.

## Decision

**`install()` rules in CMake, honouring `DESTDIR` and `CMAKE_INSTALL_PREFIX`,
then two native packages produced by CPack: a `.deb` and an `.rpm`.**

Flatpak and AppImage are not refused: they are **deferred**, and the reason is
written below.

The order has a meaning and it is half of the decision: **the installation
first, the formats after.** That is what Gaupol shows, and it is what makes the
choice of format reversible — one more format builds on the same installation.

## Why two, and why these two

**Two families cover most of the Linux desktop**, and these are the ones:
Debian and its derivatives on one side, Fedora and its relatives on the other.
Shipping for only one would mean choosing between two halves of the users on a
criterion that has nothing to do with the software.

**The second package is what puts the decision to the test.** "The
installation first, the formats after" is an assertion as long as a single
format exercises it. A second format built from the same `install()` rules
verifies it: **if it required touching up the installation, then the
installation was wrong.** That is the best reason to make two right away rather
than one now and one later.

**Ubuntu remains the declared development target** — `setup-toolchain.sh` says
so in its first line, ADR 0003 targets Linux first, and the toolchain is
described as APT packages. It is the platform on which the package will be
verified most completely, and that is an asymmetry worth writing down rather
than letting a parity that does not exist be believed — see "what we will not
be able to prove".

**CPack produces both from the same `install()` rules**, hence without a
separate description to keep in step. A package derived from the installation
cannot diverge from it.

## What really differs between the two, and it is the only real work

Not the list of files: it comes from the installation, once for both. **The
declared dependencies**, on the other hand, do not translate:
`CPACK_DEBIAN_PACKAGE_DEPENDS` and `CPACK_RPM_PACKAGE_REQUIRES` name the same
libraries by different package names, and those names are specific to each
family — Qt 6 and libmpv first and foremost.

That is where the package can be wrong without anything showing it at build
time: a wrong dependency name produces a perfectly valid `.rpm` that refuses to
install, or worse, that installs and does not start.

## What we will not be able to prove, and which must be written down

**An `.rpm` built on Ubuntu cannot be installed there.** `rpmbuild` can produce
it — the `rpm` package provides it —, but testing it requires a Fedora machine,
which neither the gate nor CI has.

What remains verifiable without Fedora, and it is not nothing:

| Verifiable here | Not verifiable here |
| :-------------- | :------------------ |
| the list of files, through `rpm -qlp` | that the package installs |
| the declared dependencies, through `rpm -qp --requires` | that the dependency names exist on Fedora |
| that the paths are those of `GNUInstallDirs` | that the binary starts once installed |

**The consequence is an acknowledged asymmetry**: the `.deb` is proven end to
end by #239, the `.rpm` is proven up to its contents. Saying so is better than
shipping both while letting people believe they were tested alike.

What would close the gap: a Fedora container. It is a heavy runtime dependency
for the gate, and this month's Actions quota is already exhausted (#232). To be
reopened when someone actually installs the `.rpm`.

## The gap is closed — August 2026, issue #266

**The container exists.** `src/scripts/check-rpm.sh` installs the `.rpm` on a
`fedora:42`, and `.github/workflows/fedora.yml` replays it on Tuesdays on
`main`. The table above no longer has a right-hand column: `dnf` carries out
the transaction, the two installed binaries start, `man subedit-cli` renders
the page, and the manual is readable where the page says it is.

**It found a defect on the first try, and not the one being watched for.** The
dependency names were right — `qt6-qtbase-gui` and `mpv-libs` resolve
thirty-nine packages. It was the directories: the `.rpm` claimed to own
`/usr/share/applications`, `/usr/share/icons` and six others, which
`filesystem` and `hicolor-icon-theme` already own, and `dnf` refused the whole
transaction over six conflicts. The exclusion had existed since #244 and
excluded nothing — it was written in relative paths, CPack expects absolute
ones.

**What this says about this ADR's asymmetry is more interesting than the
defect.** "Proven up to its contents" was true and sounded like "almost
proven". It was not: the package was flawless on inspection and unusable on
installation, and no degree of inspection led there — an ownership conflict is
not in the package, it is between the package and the distribution. A package
is only proven by installing it.

**The lesson also has a cheap half**, and it is the one we pay every day:
`check-installation.sh` now refuses an `.rpm` that owns a directory outside
`share/subedit`. That is the cause of the defect, it can be read without
Fedora, and it holds the pull requests while the container holds the week. The
two do not see the same thing — the local check would see neither a wrong
dependency name nor a library missing at run time — and that is why there are
two of them.

## Why not Flatpak, for now

It is the best format for wide distribution, and that is precisely why it does
not fit here: **it requires decisions that have nothing to do with this
phase.** A choice of runtime and of its version, a manifest that redeclares
every dependency, a build sandbox, and above all the question of access to the
user's files and to video — a subtitle editor opens arbitrary files and plays a
film alongside.

**libmpv makes that question heavier, not lighter.** ADR 0020 brought it in
through `pkg-config`; in a sandbox, it is the runtime that decides what the
player can decode.

And two native packages reduce its urgency: what Flatpak would add is coverage
of the distributions we do not name. It is a real need, it is not that of a
first release.

## Why not AppImage

It bundles everything, Qt included, which makes it the largest artefact and the
least reviewable: what is shipped is no longer what the repository describes
but a packaging of one machine's build tree. For a first release whose purpose
is precisely to verify that the installation is right, it is the format that
verifies it least.

## What the installation must lay down, and which does not exist yet

The choice of formats exempts from none of these files, and that is why they
belong to the installation rather than to the packages:

| File | Where |
| :--- | :---- |
| `subedit-gui`, `subedit-cli` | `${CMAKE_INSTALL_BINDIR}` |
| an icon | `${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps` |
| a `.desktop` | `${CMAKE_INSTALL_DATADIR}/applications` |
| an AppStream file | `${CMAKE_INSTALL_DATADIR}/metainfo` |
| the manual | `${CMAKE_INSTALL_DATADIR}/subedit/manual` |
| a manual page `subedit-cli.1` | `${CMAKE_INSTALL_MANDIR}/man1` |

These paths come from `GNUInstallDirs`, not from hand-written constants: that
is what lets a packager move them.

## What the second format revealed — 2026-08-30, issue #244

The ADR wagered that a second format would put the installation to the test:
"if it required touching up the installation, then the installation was
wrong". **It required a touch-up, and the installation was indeed wrong** — on
one point, small and real.

`rpmbuild` compresses manual pages itself. The `.deb`, for its part, carried
`subedit-cli.1` uncompressed, which the Debian policy does not want. The two
file lists therefore did not match, and the gap pointed to a shortcoming on the
Debian side rather than to a divergence between the formats.

The touch-up was made to the installation and not to either package: it now
compresses the page itself, both packages carry the same `subedit-cli.1.gz`,
and an installation by hand benefits from it too. **That is exactly the way of
working that the order "the installation first" promised**: what one format
learns, the installation corrects, and the other format benefits.

Two other gaps appeared and were not gaps: `rpmbuild` lays down
`/usr/lib/.build-id/…` links for its own debugging tools, and an `.rpm` owns
the directories its distribution does not already own. Those are properties of
the formats, and `check-installation.sh` excludes them by name rather than
wholesale — excluding in bulk would have hidden the first gap along with them.

## Consequences

**Three tooling dependencies appear**, and issue #239 carries them:
`desktop-file-validate` and `appstreamcli`, which validate two of the files
above, and **`rpm`, which provides `rpmbuild`** and without which CPack's RPM
generator produces nothing. The first two are present on the development
machine through Ubuntu's desktop base; `rpm` is not. None of the three is
declared in `setup-toolchain.sh`, and ADR 0004 requires a dependency to be
justified.

**The installed manual decides what `Help ▸ Manual` opens**, and therefore when
that entry can be switched back on — after packaging, which puts it last in the
phase.

**The manual's examples become true.** They show `$ subedit-cli` as if the tool
were on the `PATH`, which it is only from here on. Generation, however, does
not change: `generate-manual.sh` puts `build/dev/bin` at the head of the
`PATH`, and it must stay so — otherwise the manual would describe the installed
version rather than the repository's.

**The manual must say what is tested and what is not.** A Fedora user has the
right to know that their package is produced by the same rules as the other one
and verified less far.

**The day Flatpak is taken up again, none of this is lost.** A manifest builds
on the `install()` rules, as Gaupol's builds on its `Makefile`.

## Revision: the `.rpm` is built on each released Fedora

Observed in v0.13.11: an `.rpm` built on Ubuntu requires `libicuuc.so.74`, and
a Fedora that has neither ICU 74 nor its compatibility package refuses to
install it. The original decision — **a single `.rpm`, built where development
happens** — assumed that dependencies resolve everywhere; ICU, whose symbols
carry the major version, shows that they do not.

**The published `.rpm` is now built on Fedora, against its ICU, once per
released version** (42, 43 and 44): `src/scripts/build-rpm.sh`, run by the
`rpm` job of `release.yml` in the `fedora:N` image. The file name carries the
distribution (`…-1.fc43.x86_64.rpm`). The `.deb` does not change. **The cost**:
one `.rpm` per Fedora and one more build per version; a Fedora missing from the
matrix builds from source. `make rpm-check` still tests the `.rpm` built on
Ubuntu, and therefore no longer says anything about the published packages.
