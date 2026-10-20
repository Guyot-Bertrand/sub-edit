# Installation

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

**Ca págine vale por ambi programmas** — `subedit-cli` e `subedit-gui`.
Les es construet e installat together.

Du vias: **un paccage nativ**, `.deb` o `.rpm`, o **compilar ex li
code-fonte**, quel tande installa u on vole.

## Li paccages

Ambi es produit per CPack ex **li sam regules de installation**:
les mette li sam files in li sam locs, e diffre solmen per li
nómines del paccages de queles les depende — li sam bibliotecas es nominat
diferentmen sur Debian e sur Fedora.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| Quo li paccage installa | U |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| li entrata de menu, li icone, li metadata del centre de software | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| ca manual, in Markdown | `/usr/share/subedit/manual` |
| su traduction in chascun lingue installat — catalan, tchec, german, hispan, finnic, francesi, galician, hebraic, hungaric, interlingue | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie` |
| li modelles de correction de Gaupol | `/usr/share/subedit/patterns` |
| li págines de manual de `subedit-cli`, un per subcomande, e de `subedit-gui` — vide [Li págines de manual](invocation.md#li-págines-de-manual) | `/usr/share/man/man1` |
| li págines de `subedit-cli` in chascun lingue installat — catalan, tchec, german, hispan, finnic, francesi, galician, hebraic, hungaric, interlingue | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie` |

`ffmpeg` es **recomandat e ne obligatori** per ambi paccages: sin it, li
fenestre solmen cessa oferter li frequentie de fotogrammas quel li film declara — vide
[quo `ffmpeg` changea](../subedit-gui/video.md#ffmpeg-ne-es-obligatori).

### Quo li paccages besona del distribution

**Installa con `apt` o `dnf`, nequande con `dpkg -i` o `rpm -i`.** Ti du
comandes posa li paccage sin resolver su dependenties: les solmen
lista ti queles manca, e se ferma. To es quo `rpm -i` responde sur
un Fedora quel have bon quo it besona disponibil per `dnf`:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` fetche ti
bibliotecas, u les existe.

**Li `.rpm` a prender es ti de vor Fedora**: li nómine del file porta li
version, `fc42`, `fc43`, `fc44`. Chascun es construet sur ti Fedora, contra
su ICU — `cat /etc/fedora-release` di qual prender. Un `.rpm` de un altri
Fedora demanda un ICU quel vor machine ne have.

| Dependentie | `.deb` | `.rpm` | Restriction |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, por li fenestre |
| libmpv | `libmpv2` o `libmpv1` | `mpv-libs` | li binarie linka `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | li controlator de ortografie, e li taches `join-words` e `split-words` de [`correct`](correct.md) |
| **ICU** | **`libicu74`** | `libicu`, to es li `libicuuc.so.N` e `libicui18n.so.N` **del Fedora quel l'ha construet** | **li version quel li Fedora del `.rpm` have**, exactmen — vide infra |
| glibc e libstdc++ | — | — | **suficentmen recent**: ti de Ubuntu 24.04 por li `.deb`, ti del Fedora del `.rpm` por ti-ci |

**ICU es li restriction quel importa, e it ne posse esser relaxat per
changear un linea del paccage.** Li simboles de ICU porta li númere de su
version major (`u_strToUTF8_74`): un binarie construet contra ICU 74 ne posse
linkar a ICU 76 o 77. Un dependentie scrit plu large lassarea li paccage
installar se, pois falir al lancement — declarar it exact es quo
resta honest. **To es pro quo li `.rpm` ne es construet un vez, ma sur
chascun Fedora publicat**, contra su ICU; li `.deb` es construet sur Ubuntu 24.04.

| Distribution | Quo it dona |
| :----------- | :---------------- |
| Ubuntu 24.04 | li `.deb` se installa: ICU es ibi al version 74 |
| Fedora 42, 43, 44 | li `.rpm` de su version (`…fc42…`, `…fc43…`, `…fc44…`) se installa **per `dnf`**, construet contra li ICU de ti Fedora |
| Fedora rawhide (li sequent), li altri distributiones `.rpm` | **no**: null `.rpm` es construet por les — compilar ex li code-fonte |
| quelcunc distribution Debian o Ubuntu de quel ICU ne es al version 74 | **no**: li `.deb` besona `libicu74` — compilar ex li code-fonte |

**Compilar ex li code-fonte es li recurse, e it ne have ca
restriction**: it linka al ICU del machine, quel que su version.
It es anc quo on besona sur un distribution de quel glibc es plu old
quam ti del paccages.

### Quo es testat de chascun, e quo ne es

**Li du ne es controlat tam lontan, e it es mey bon dir it quam lassar
creder a un paritá quel ne existe.** Li developation evenit sur Ubuntu; un
`.rpm` construet ibi ne posse esser installat ibi. It es installat altrubi: sur un
Fedora in un container, un vez per semane.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| li liste de files | controlat | controlat, e **comparat con ti del `.deb`** |
| li dependenties declarat | controlat presente | controlat presente |
| que li nómines de dependenties existe in li distribution | yes, les es ti de Ubuntu | **yes**, resolvet per `dnf` |
| que li paccage se installa | no — to besona drets de administrator | **yes**, sur un Fedora in un container |
| que li binaries installat comensa | yes, ex un prefix temporari | **yes**, ex li paccage installat |

**Li controle Fedora testa li `.rpm` construet sur Ubuntu, ne ti queles es
publicat.** Ti image have null ICU 74 propri: li paccage se installa ibi pro que
Fedora conserva un `libicu74` de compatibilitá, e li controle esserea verd til que
ti paccage existe. Li files `.rpm` publicat es construet sur lor propri Fedora, e ne plu
depende de ti compatibilitá. Vide
[quo li paccages besona](#quo-li-paccages-besona-del-distribution).

**Comparar li du listes es li controle quel importa max.** Li
du paccages veni del sam installation: un diferentie inter les esserea un
defecte de-in li regules de installation, ne del formate.

**Quo li `.deb` ancor ne proba es que it se installa**, e li rason
ne ha changeat: `dpkg -i` besona drets de administrator, quel un porta de
qualitá ne have e ne deve demandar. Li `.rpm`, in su parte, se installa in un
container quel apartene a nequi. Vide
[ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Flatpak e AppImage ne es oferit**, e to es un electe — ne un omission.
It es explicat in li sam ADR, e it es **definitiv**: un distribution quel
ne es del familie Debian ni del familie Fedora compila ex
li code-fonte, quel li section sequent descrie in plen.

## Compilar ex li code-fonte

### Prerequisites

| Quo es besonat | Pro quo | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | li construction | `cmake` |
| un compilator C++23 — GCC 13 va bastar | — | `g++` |
| `make`, `pkg-config` | li construction | `make`, `pkg-config` |
| CLI11 | leer li linea de comande | `libcli11-dev` |
| Qt 6 ≥ 6.4, modul `Widgets` | li fenestre | `qt6-base-dev` |
| `libmpv` | li reproductor integrat | `libmpv-dev` |
| ICU | leer li codifications | `libicu-dev` |
| Enchant 2 | li biblioteca del controlator de ortografie | `libenchant-2-dev` |

**Li ultim quar es besonat mem por construer solmen
`subedit-cli`**: li configuration CMake les cerca por li tot projecte, e
se ferma si les manca. ICU e Enchant es besonat du vezes — li executabile les carga, un
por converter li codifications, li altri por li controlator de ortografie — e
les es li sol bibliotecas de tertie que it carga durant li execution.

**Un connexion de rete al prim `cmake`**: li biblioteca de tests Catch2
es telecargat de GitHub durant li configuration. It ne es telecargat denov
pos. Por construer li instrument sol, sin rete, vide infra.

#### Quo ne es besonat

| Quo es facultativ | Quo es perdit sin it |
| :-------------------- | :--------------------- |
| `ffmpeg`, por su `ffprobe` | li fenestre cessa oferter li frequentie de fotogrammas quel li film declara — [li detail](../subedit-gui/video.md#ffmpeg-ne-es-obligatori) |
| un servitor de monstration | nequo por `subedit-cli`; `subedit-gui` besona un por monstrar se |

### Construer

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

Li binaries es produit in `build/dev/bin/` — `subedit-cli` e
`subedit-gui`.

Por un version optimisat:

```bash
make release
```

Ambi binaries es tande in `build/release/bin/` — `subedit-cli` e
`subedit-gui`. Li target construe nequo altri: ni li benchmark, ni li
harnas end-to-end, queles have lor propri targets.

**Prefere ca target a li du comandes `cmake` equivalent**, queles
ne save quant processus li optimisation al tempore de linkage have li permisse de
comensar: a chascun linkage, it comensa tant quant hay coros.
`make release` passa a it `JOBS`, quam it fa al resto del construction.

### Installar

```bash
make release
cmake --install build/release --prefix ~/.local
```

Quo es installat, ad li camines que `GNUInstallDirs` fixa por li prefix
electet:

| File | U |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| li entrata de menu del table de labor | `<prefix>/share/applications` |
| li icone | `<prefix>/share/icons/hicolor/scalable/apps` |
| li metadata del centre de software | `<prefix>/share/metainfo` |
| ca manual, in Markdown | `<prefix>/share/subedit/manual` |
| su traduction in chascun lingue installat — catalan, tchec, german, hispan, finnic, francesi, galician, hebraic, hungaric, interlingue | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie` |
| li modelles de correction de Gaupol | `<prefix>/share/subedit/patterns` |
| li págines de manual de ambi binaries, un per subcomande por `subedit-cli` | `<prefix>/share/man/man1` |
| li págines de `subedit-cli` in chascun lingue installat — catalan, tchec, german, hispan, finnic, francesi, galician, hebraic, hungaric, interlingue | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie` |

**Es li sam files que li paccages installa**: li paccages veni de les,
vice esser descrit un duesim vez.

Con `--prefix ~/.local`, ambi binaries cade in `~/.local/bin`,
quel es in li `PATH` del plu distributiones. Un prefix de sistema —
`/usr/local`, p.ex. — besona li drets correspondent.

**Un entrata de menu mettet sub `~/.local` ne apare sempre tam
tost**: li tables de labor relee lor cache a lor propri rhitmo. `subedit-gui` comensa
ex un terminal in li intervalle, quam quelcunc altri comande.

`DESTDIR` es respectat, quo un empaccator expecte: un installation
stagiat tange nequo foris li directoria de stagiation.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Construer li instrument sol

Li tests es construet predefinit, e es les quel demanda li rete —
`make release` ne les construe, ma it configura li projecte, e es li
configuration quel telecarga Catch2. Desactivar les un vez basta: li
parametre resta in li cache CMake, e li constructiones sequent lo conserva.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

To exempte un contributor de nequo: li porta de qualitá
les besona.

### Si li construction falie al linkage

Un avalanche de «undefined reference to `std::cout`» sur code valid
significa usualmen que li alternativa `c++` del sistema indica `gcc` vice
`g++`: li C++ compila, ma li biblioteca standard ne es linkat.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

Li configuration CMake detecte ca caso e se ferma con ca missage vice
lassar li linkage falir.

### Si installar li paccage se plange pri `libicu`

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Tri causas, distinet per li comande tipat e per li nómine del file:

- **`rpm -i`**: it resolve nequo. Prova denov con
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **un `.rpm` sin `fcNN` in su nómine**, o de un Fedora altri quam vor:
  it demanda li ICU del machine quel l'ha construet. Prende ti de quel `fcNN`
  es li output de `cat /etc/fedora-release`;
- **null `.rpm` por vor version**: Fedora rawhide o un altri distribution.
  Null paccage de ca repositoria va installar se ibi;
  [compilar ex li code-fonte](#compilar-ex-li-code-fonte).
