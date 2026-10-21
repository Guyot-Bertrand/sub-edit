# Instalace

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

**Tato stránka platí pro oba programy** — `subedit-cli` i `subedit-gui`.
Sestavují se a instalují společně.

Dvě cesty: **nativní balíček**, `.deb` nebo `.rpm`, nebo **sestavení ze
zdrojového kódu**, které se pak nainstaluje, kam chcete.

## Balíčky

Oba vytváří CPack ze **stejných instalačních pravidel**:
umisťují stejné soubory na stejná místa a liší se jen
názvy balíčků, na kterých závisejí — tytéž knihovny se
v Debianu a ve Fedoře jmenují jinak.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| Co balíček instaluje | Kam |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| položku nabídky, ikonu, metadata pro centrum softwaru | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| tuto příručku ve formátu Markdown | `/usr/share/subedit/manual` |
| její překlad do každého instalovaného jazyka — katalánštiny, češtiny, němčiny, španělštiny, finštiny, francouzštiny, galicijštiny, hebrejštiny, maďarštiny, interlingue, islandštiny, italštiny, nizozemštiny, polštiny, portugalštiny, brazilské portugalštiny, ruštiny, srbštiny, turečtiny, zjednodušené čínštiny | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| opravné vzory Gaupolu | `/usr/share/subedit/patterns` |
| manuálové stránky `subedit-cli`, jednu na podpříkaz, a `subedit-gui` — viz [Manuálové stránky](invocation.md#manuálové-stránky) | `/usr/share/man/man1` |
| stránky `subedit-cli` v každém instalovaném jazyce — katalánštině, češtině, němčině, španělštině, finštině, francouzštině, galicijštině, hebrejštině, maďarštině, interlingue, islandštině, italštině, nizozemštině, polštině, portugalštině, brazilské portugalštině, ruštině, srbštině, turečtině, zjednodušené čínštině | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

`ffmpeg` je u obou balíčků **doporučený, nikoli povinný**: bez něj
okno jen přestane nabízet snímkovou frekvenci, kterou film uvádí — viz
[co `ffmpeg` mění](../subedit-gui/video.md#ffmpeg-není-nutný).

### Co balíčky vyžadují od distribuce

**Instalujte pomocí `apt` nebo `dnf`, nikdy pomocí `dpkg -i` nebo `rpm -i`.** Tyto dva
příkazy balíček položí, aniž by vyřešily jeho závislosti: jen
vypíšou ty, které chybějí, a skončí. Takto odpoví `rpm -i` na
Fedoře, která to, co potřebuje, má v dosahu `dnf`:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` tyto
knihovny stáhne, pokud existují.

**Vezměte `.rpm` pro svou Fedoru**: název souboru nese
verzi, `fc42`, `fc43`, `fc44`. Každý se sestavuje na dané Fedoře, proti
její ICU — `cat /etc/fedora-release` řekne, který vzít. `.rpm` z jiné
Fedory žádá ICU, kterou ta vaše nemá.

| Závislost | `.deb` | `.rpm` | Omezení |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, pro okno |
| libmpv | `libmpv2` nebo `libmpv1` | `mpv-libs` | binární soubor se linkuje s `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | kontrola pravopisu a úlohy `join-words` a `split-words` podpříkazu [`correct`](correct.md) |
| **ICU** | **`libicu74`** | `libicu`, tedy `libicuuc.so.N` a `libicui18n.so.N` **té Fedory, která ho sestavila** | **přesně ta verze, kterou má Fedora daného `.rpm`** — viz níže |
| glibc a libstdc++ | — | — | **dostatečně nové**: ty z Ubuntu 24.04 pro `.deb`, ty z Fedory daného `.rpm` pro něj |

**ICU je omezení, na kterém záleží, a nelze ho zmírnit
změnou jednoho řádku balíčku.** Symboly ICU nesou číslo její
hlavní verze (`u_strToUTF8_74`): binární soubor sestavený proti ICU 74 se nemůže
linkovat s ICU 76 ani 77. Šířeji zapsaná závislost by balíček nechala
nainstalovat a pak by selhal při spuštění — deklarovat ji přesně je to, co
zůstává poctivé. **Proto se `.rpm` nesestavuje jednou, ale na
každé vydané Fedoře**, proti její ICU; `.deb` se sestavuje na Ubuntu 24.04.

| Distribuce | Výsledek |
| :----------- | :---------------- |
| Ubuntu 24.04 | `.deb` se nainstaluje: ICU je tam ve verzi 74 |
| Fedora 42, 43, 44 | `.rpm` její verze (`…fc42…`, `…fc43…`, `…fc44…`) se nainstaluje **pomocí `dnf`**, sestavený proti ICU dané Fedory |
| Fedora rawhide (příští verze), ostatní distribuce s `.rpm` | **ne**: žádný `.rpm` se pro ně nesestavuje — sestavte ze zdrojového kódu |
| jakákoli distribuce Debian nebo Ubuntu, jejíž ICU není ve verzi 74 | **ne**: `.deb` vyžaduje `libicu74` — sestavte ze zdrojového kódu |

**Sestavení ze zdrojového kódu je záložní cesta a toto omezení
nemá**: linkuje se s ICU počítače, ať je jakékoli verze.
Je to také to, co je potřeba na distribuci, jejíž glibc je starší
než ta, kterou vyžadují balíčky.

### Co se u každého testuje a co ne

**Oba se neověřují stejně důkladně a je lepší to říct, než nechat
věřit v rovnocennost, která neexistuje.** Vývoj probíhá na Ubuntu;
`.rpm` sestavený tam se tam nedá nainstalovat. Instaluje se jinde: na
Fedoře v kontejneru, jednou týdně.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| seznam souborů | ověřen | ověřen a **porovnán se seznamem `.deb`** |
| deklarované závislosti | ověřena jejich přítomnost | ověřena jejich přítomnost |
| že názvy závislostí v distribuci existují | ano, jsou to názvy Ubuntu | **ano**, vyřeší je `dnf` |
| že se balíček nainstaluje | ne — to vyžaduje práva správce | **ano**, na Fedoře v kontejneru |
| že se nainstalované binární soubory spustí | ano, z dočasného prefixu | **ano**, z nainstalovaného balíčku |

**Kontrola na Fedoře testuje `.rpm` sestavený na Ubuntu, ne ty, které
se vydávají.** Ten obraz nemá vlastní ICU 74: balíček se tam nainstaluje, protože
Fedora udržuje kompatibilní `libicu74`, a kontrola by byla zelená, dokud
ten balíček existuje. Vydávané soubory `.rpm` se sestavují na své vlastní Fedoře a na
této kompatibilitě už nezávisejí. Viz
[co balíčky vyžadují](#co-balíčky-vyžadují-od-distribuce).

**Porovnání obou seznamů je kontrola, na které záleží nejvíc.**
Oba balíčky vycházejí ze stejné instalace: rozdíl mezi nimi by byl
vadou instalačních pravidel, ne formátu.

**Co `.deb` stále neprokazuje, je, že se nainstaluje**, a důvod
se nezměnil: `dpkg -i` vyžaduje práva správce, která kontrola kvality
nemá a nesmí o ně žádat. `.rpm` se naproti tomu instaluje v
kontejneru, který nikomu nepatří. Viz
[ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Flatpak a AppImage se nenabízejí**, a to je volba — ne opomenutí.
Je vysvětlena ve stejném ADR a je **konečná**: distribuce, která
nepatří ani do rodiny Debianu, ani do rodiny Fedory, sestavuje ze
zdrojového kódu, což podrobně popisuje další oddíl.

## Sestavení ze zdrojového kódu

### Předpoklady

| Co je potřeba | Proč | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | sestavení | `cmake` |
| kompilátor C++23 — postačí GCC 13 | — | `g++` |
| `make`, `pkg-config` | sestavení | `make`, `pkg-config` |
| CLI11 | čtení příkazového řádku | `libcli11-dev` |
| Qt 6 ≥ 6.4, modul `Widgets` | okno | `qt6-base-dev` |
| `libmpv` | vestavěný přehrávač | `libmpv-dev` |
| ICU | čtení kódování | `libicu-dev` |
| Enchant 2 | knihovna kontroly pravopisu | `libenchant-2-dev` |

**Poslední čtyři jsou nutné, i když se sestavuje jen
`subedit-cli`**: konfigurace CMake je hledá pro celý projekt a
skončí, pokud chybějí. ICU a Enchant jsou nutné dvojnásob — spustitelný soubor je načítá, jednu
k převodu kódování, druhou pro kontrolu pravopisu — a
jsou to jediné knihovny třetích stran, které načítá za běhu.

**Síťové připojení při prvním `cmake`**: testovací knihovna Catch2
se při konfiguraci stahuje z GitHubu. Později se už znovu
nestahuje. Jak sestavit samotný nástroj bez sítě, viz níže.

#### Co není nutné

| Co je volitelné | Co se bez toho ztratí |
| :-------------------- | :--------------------- |
| `ffmpeg`, kvůli jeho `ffprobe` | okno už nenabízí snímkovou frekvenci, kterou film uvádí — [podrobnosti](../subedit-gui/video.md#ffmpeg-není-nutný) |
| zobrazovací server | nic pro `subedit-cli`; `subedit-gui` ho potřebuje, aby se zobrazil |

### Sestavení

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

Binární soubory vzniknou v `build/dev/bin/` — `subedit-cli` a
`subedit-gui`.

Pro optimalizovanou verzi:

```bash
make release
```

Oba binární soubory jsou pak v `build/release/bin/` — `subedit-cli` a
`subedit-gui`. Cíl nesestavuje nic jiného: ani měření výkonu, ani
sadu end-to-end testů, které mají vlastní cíle.

**Dejte tomuto cíli přednost před dvěma ekvivalentními příkazy `cmake`**, které
nemají jak zjistit, kolik procesů smí optimalizace při linkování
spustit: při každém linkování jich spustí tolik, kolik je jader.
`make release` jí předá `JOBS`, stejně jako zbytku sestavení.

### Nainstalování

```bash
make release
cmake --install build/release --prefix ~/.local
```

Co se nainstaluje, na cesty, které `GNUInstallDirs` určí pro zvolený
prefix:

| Soubor | Kam |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| položka v nabídce pracovního prostředí | `<prefix>/share/applications` |
| ikona | `<prefix>/share/icons/hicolor/scalable/apps` |
| metadata pro centrum softwaru | `<prefix>/share/metainfo` |
| tato příručka ve formátu Markdown | `<prefix>/share/subedit/manual` |
| její překlad do každého instalovaného jazyka — katalánštiny, češtiny, němčiny, španělštiny, finštiny, francouzštiny, galicijštiny, hebrejštiny, maďarštiny, interlingue, islandštiny, italštiny, nizozemštiny, polštiny, portugalštiny, brazilské portugalštiny, ruštiny, srbštiny, turečtiny, zjednodušené čínštiny | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| opravné vzory Gaupolu | `<prefix>/share/subedit/patterns` |
| manuálové stránky obou binárních souborů, pro `subedit-cli` jedna na podpříkaz | `<prefix>/share/man/man1` |
| stránky `subedit-cli` v každém instalovaném jazyce — katalánštině, češtině, němčině, španělštině, finštině, francouzštině, galicijštině, hebrejštině, maďarštině, interlingue, islandštině, italštině, nizozemštině, polštině, portugalštině, brazilské portugalštině, ruštině, srbštině, turečtině, zjednodušené čínštině | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

**Jsou to tytéž soubory, které instalují balíčky**: balíčky z nich vycházejí,
místo aby se popisovaly podruhé.

S `--prefix ~/.local` skončí oba binární soubory v `~/.local/bin`,
který je v `PATH` většiny distribucí. Systémový prefix —
například `/usr/local` — vyžaduje odpovídající práva.

**Položka nabídky umístěná pod `~/.local` se ne vždy objeví
hned**: pracovní prostředí znovu načítají svou mezipaměť vlastním tempem. `subedit-gui` se
mezitím spustí z terminálu jako kterýkoli jiný příkaz.

`DESTDIR` se respektuje, což balíčkovač očekává: instalace do přípravného adresáře
nezasáhne nic mimo tento adresář.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Sestavení samotného nástroje

Testy se sestavují ve výchozím stavu a právě ony vyžadují síť —
`make release` je nesestavuje, ale konfiguruje projekt, a právě
konfigurace stahuje Catch2. Stačí je jednou vypnout:
nastavení zůstane v mezipaměti CMake a další sestavení ho zachovají.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Přispěvatele to od ničeho neosvobozuje: kontrola kvality je
vyžaduje.

### Když sestavení selže při linkování

Lavina hlášení „undefined reference to `std::cout`“ u platného kódu
obvykle znamená, že systémová alternativa `c++` ukazuje na `gcc` místo
na `g++`: C++ se zkompiluje, ale standardní knihovna se nepřilinkuje.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

Konfigurace CMake tento případ rozpozná a skončí tímto hlášením, místo aby
nechala linkování selhat.

### Když si instalace balíčku stěžuje na `libicu`

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Tři příčiny, rozlišené podle zadaného příkazu a podle názvu souboru:

- **`rpm -i`**: nic neřeší. Zkuste to znovu pomocí
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **`.rpm` bez `fcNN` v názvu** nebo z jiné Fedory než té vaší:
  žádá ICU počítače, který ho sestavil. Vezměte ten, jehož `fcNN`
  odpovídá výstupu `cat /etc/fedora-release`;
- **žádný `.rpm` pro vaši verzi**: Fedora rawhide nebo jiná distribuce.
  Žádný balíček z tohoto repozitáře se tam nenainstaluje;
  [sestavte ze zdrojového kódu](#sestavení-ze-zdrojového-kódu).
