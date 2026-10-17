# Installation

**This page holds for both programs** — `subedit-cli` and `subedit-gui`.
They are built and installed together.

Two paths: **a native package**, `.deb` or `.rpm`, or **building from
source**, which then installs wherever one wants.

## The packages

Both are produced by CPack from **the same installation rules**:
they put the same files in the same places, and differ only in the
names of the packages they depend on — the same libraries are called
differently on Debian and on Fedora.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| What the package installs | Where |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| the menu entry, the icon, the software-centre metadata | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| this manual, in Markdown | `/usr/share/subedit/manual` |
| its translation into each installed language — French | `/usr/share/subedit/manual/<language>` — `fr` |
| Gaupol's correction patterns | `/usr/share/subedit/patterns` |
| the manual pages of `subedit-cli` and `subedit-gui` | `/usr/share/man/man1` |

`ffmpeg` is **recommended and not required** by both packages: without it, the
window only stops offering the frame rate the film declares — see
[what `ffmpeg` changes](../subedit-gui/video.md#ffmpeg-is-not-required).

### What the packages require of the distribution

**Install with `apt` or `dnf`, never with `dpkg -i` or `rpm -i`.** Those two
commands put the package down without resolving its dependencies: they merely
list the missing ones, and stop. This is what `rpm -i` answers on
a Fedora that does have what it needs within reach of `dnf`:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` fetches those
libraries, where they exist.

**The `.rpm` to take is the one of your Fedora**: the file name carries the
version, `fc42`, `fc43`, `fc44`. Each one is built on that Fedora, against
its ICU — `cat /etc/fedora-release` says which one to take. An `.rpm` from another
Fedora asks for an ICU yours does not have.

| Dependency | `.deb` | `.rpm` | Constraint |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, for the window |
| libmpv | `libmpv2` or `libmpv1` | `mpv-libs` | the binary links `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | the spell checker, and the `join-words` and `split-words` tasks of [`correct`](correct.md) |
| **ICU** | **`libicu74`** | `libicu`, that is the `libicuuc.so.N` and `libicui18n.so.N` **of the Fedora that built it** | **the version the Fedora of the `.rpm` has**, exactly — see below |
| glibc and libstdc++ | — | — | **recent enough**: those of Ubuntu 24.04 for the `.deb`, those of the Fedora of the `.rpm` for that one |

**ICU is the constraint that matters, and it cannot be relaxed by
changing a line of the package.** ICU's symbols carry the number of its
major version (`u_strToUTF8_74`): a binary built against ICU 74 cannot
link to ICU 76 or 77. A dependency written wider would let the package
install, then fail at launch — declaring it exact is what
stays honest. **That is why the `.rpm` is not built once, but on
every released Fedora**, against its ICU; the `.deb` is built on Ubuntu 24.04.

| Distribution | What it gives |
| :----------- | :---------------- |
| Ubuntu 24.04 | the `.deb` installs: ICU is at version 74 there |
| Fedora 42, 43, 44 | the `.rpm` of its version (`…fc42…`, `…fc43…`, `…fc44…`) installs **through `dnf`**, built against the ICU of that Fedora |
| Fedora rawhide (the next one), the other `.rpm` distributions | **no**: no `.rpm` is built for them — build from source |
| any Debian or Ubuntu distribution whose ICU is not at version 74 | **no**: the `.deb` requires `libicu74` — build from source |

**Building from source is the fallback, and it does not have this
constraint**: it links to the machine's ICU, whatever its version.
It is also what is needed on a distribution whose glibc is older
than the packages'.

### What is tested of each, and what is not

**The two are not checked as far, and it is better to say so than to let
one believe in a parity that does not exist.** Development happens on Ubuntu; an
`.rpm` built there cannot be installed there. It is installed elsewhere: on a
Fedora in a container, once a week.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| the list of files | checked | checked, and **compared with the `.deb`'s** |
| the declared dependencies | checked present | checked present |
| that the dependency names exist in the distribution | yes, they are Ubuntu's | **yes**, resolved by `dnf` |
| that the package installs | no — that needs administrator rights | **yes**, on a Fedora in a container |
| that the installed binaries start | yes, from a temporary prefix | **yes**, from the installed package |

**The Fedora check tests the `.rpm` built on Ubuntu, not the ones that are
released.** That image has no ICU 74 of its own: the package installs there because
Fedora keeps a compatibility `libicu74`, and the check would be green as long as
that package exists. The released `.rpm` files are built on their own Fedora, and no longer
depend on that compatibility. See
[what the packages require](#what-the-packages-require-of-the-distribution).

**Comparing the two lists is the check that matters most.** The
two packages come out of the same installation: a gap between them would be a
flaw of the installation rules, not of the format.

**What the `.deb` still does not prove is that it installs**, and the reason
has not changed: `dpkg -i` needs administrator rights, which a quality gate
does not have and must not ask for. The `.rpm`, for its part, installs in a
container that belongs to nobody. See
[ADR 0023](../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Flatpak and AppImage are not offered**, and that is a choice — not an oversight.
It is explained in the same ADR, and it is **final**: a distribution that
is neither of the Debian family nor of the Fedora family builds from
source, which the next section describes in full.

## Building from source

### Prerequisites

| What is needed | Why | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | the build | `cmake` |
| a C++23 compiler — GCC 13 will do | — | `g++` |
| `make`, `pkg-config` | the build | `make`, `pkg-config` |
| CLI11 | reading the command line | `libcli11-dev` |
| Qt 6 ≥ 6.4, `Widgets` module | the window | `qt6-base-dev` |
| `libmpv` | the built-in player | `libmpv-dev` |
| ICU | reading the encodings | `libicu-dev` |
| Enchant 2 | the spell checker's library | `libenchant-2-dev` |

**The last four are required even to build only
`subedit-cli`**: the CMake configuration looks for them for the whole project, and
stops if they are missing. ICU and Enchant are required twice over — the executable loads them, one
to convert the encodings, the other for the spell checker — and
they are the only third-party libraries it loads at run time.

**A network connection at the first `cmake`**: the Catch2 test library
is fetched from GitHub at configuration. It is not downloaded again
afterwards. To build the tool alone, without a network, see below.

#### What is not required

| What is optional | What is lost without it |
| :-------------------- | :--------------------- |
| `ffmpeg`, for its `ffprobe` | the window no longer offers the frame rate the film declares — [the details](../subedit-gui/video.md#ffmpeg-is-not-required) |
| a display server | nothing for `subedit-cli`; `subedit-gui` needs one to show itself |

### Building

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

The binaries are produced in `build/dev/bin/` — `subedit-cli` and
`subedit-gui`.

For an optimised version:

```bash
make release
```

Both binaries are then in `build/release/bin/` — `subedit-cli` and
`subedit-gui`. The target builds nothing else: neither the benchmark, nor the
end-to-end harness, which have their own targets.

**Prefer this target to the two equivalent `cmake` commands**, which have
no way of knowing how many processes link-time optimisation is allowed to
start: at every link, it starts as many as there are cores.
`make release` passes it `JOBS`, as it does to the rest of the build.

### Installing

```bash
make release
cmake --install build/release --prefix ~/.local
```

What is installed, at the paths `GNUInstallDirs` sets for the chosen
prefix:

| File | Where |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| the desktop menu entry | `<prefix>/share/applications` |
| the icon | `<prefix>/share/icons/hicolor/scalable/apps` |
| the software-centre metadata | `<prefix>/share/metainfo` |
| this manual, in Markdown | `<prefix>/share/subedit/manual` |
| its translation into each installed language — French | `<prefix>/share/subedit/manual/<language>` — `fr` |
| Gaupol's correction patterns | `<prefix>/share/subedit/patterns` |
| the manual pages of both binaries | `<prefix>/share/man/man1` |

**They are the same files the packages install**: the packages come out of them,
rather than being described a second time.

With `--prefix ~/.local`, both binaries land in `~/.local/bin`,
which is in the `PATH` of most distributions. A system prefix —
`/usr/local`, for example — needs the matching rights.

**A menu entry put under `~/.local` does not always appear straight
away**: desktops reread their cache at their own pace. `subedit-gui` starts
from a terminal in the meantime, like any other command.

`DESTDIR` is honoured, which a packager expects: a staged installation
touches nothing outside the staging directory.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Building the tool alone

The tests are built by default, and they are what asks for the network —
`make release` does not build them, but it configures the project, and it is the
configuration that fetches Catch2. Disabling them once is enough: the
setting stays in the CMake cache, and the following builds keep it.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

That exempts a contributor from nothing: the quality gate does
require them.

### If the build fails at link time

An avalanche of "undefined reference to `std::cout`" on valid code
usually means that the system's `c++` alternative points to `gcc` instead
of `g++`: the C++ compiles, but the standard library is not linked.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

The CMake configuration detects this case and stops with this message rather than
letting the link fail.

### If installing the package complains about `libicu`

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Three causes, told apart by the command typed and by the file name:

- **`rpm -i`**: it resolves nothing. Try again with
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **an `.rpm` without `fcNN` in its name**, or from a Fedora other than yours:
  it asks for the ICU of the machine that built it. Take the one whose `fcNN`
  is the output of `cat /etc/fedora-release`;
- **no `.rpm` for your version**: Fedora rawhide or another distribution.
  No package of this repository will install there;
  [build from source](#building-from-source).

