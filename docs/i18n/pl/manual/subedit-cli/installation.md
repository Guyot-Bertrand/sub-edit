# Instalacja

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

**Ta strona dotyczy obu programów** — `subedit-cli` i `subedit-gui`.
Są budowane i instalowane razem.

Dwie drogi: **pakiet natywny**, `.deb` lub `.rpm`, albo **budowanie ze
źródeł**, które instaluje program, gdzie tylko się chce.

## Pakiety

Oba są tworzone przez CPack z **tych samych reguł instalacji**:
umieszczają te same pliki w tych samych miejscach i różnią się tylko
nazwami pakietów, od których zależą — te same biblioteki nazywają się
inaczej w Debianie i w Fedorze.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| Co instaluje pakiet | Gdzie |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| pozycja menu, ikona, metadane centrum oprogramowania | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| ten podręcznik, w Markdown | `/usr/share/subedit/manual` |
| jego tłumaczenie na każdy zainstalowany język — kataloński, czeski, niemiecki, hiszpański, fiński, francuski, galicyjski, hebrajski, węgierski, interlingue, islandzki, włoski, niderlandzki, polski, portugalski, brazylijski portugalski, rosyjski, serbski, turecki, chiński uproszczony | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| wzorce poprawek z Gaupola | `/usr/share/subedit/patterns` |
| strony podręcznika man dla `subedit-cli`, po jednej na podpolecenie, i dla `subedit-gui` — zobacz [Strony podręcznika man](invocation.md#strony-podręcznika-man) | `/usr/share/man/man1` |
| strony `subedit-cli` w każdym zainstalowanym języku — kataloński, czeski, niemiecki, hiszpański, fiński, francuski, galicyjski, hebrajski, węgierski, interlingue, islandzki, włoski, niderlandzki, polski, portugalski, brazylijski portugalski, rosyjski, serbski, turecki, chiński uproszczony | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

`ffmpeg` jest w obu pakietach **zalecany, a nie wymagany**: bez niego okno
tylko przestaje proponować liczbę klatek na sekundę deklarowaną przez film — zobacz
[co zmienia `ffmpeg`](../subedit-gui/video.md#ffmpeg-nie-jest-wymagany).

### Czego pakiety wymagają od dystrybucji

**Instaluj za pomocą `apt` lub `dnf`, nigdy za pomocą `dpkg -i` ani `rpm -i`.** Te dwa
polecenia kładą pakiet na miejscu bez rozwiązywania jego zależności: ograniczają się do
wypisania brakujących i na tym kończą. Oto, co odpowiada `rpm -i` w
Fedorze, w której to, czego trzeba, jest w zasięgu `dnf`:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

Polecenie `sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` pobiera te
biblioteki, tam gdzie istnieją.

**Należy wziąć `.rpm` odpowiedni dla swojej Fedory**: nazwa pliku zawiera
wersję, `fc42`, `fc43`, `fc44`. Każdy jest budowany w tej właśnie Fedorze, względem
jej ICU — `cat /etc/fedora-release` mówi, który wziąć. Plik `.rpm` z innej
Fedory wymaga ICU, którego nie ma Twoja.

| Zależność | `.deb` | `.rpm` | Ograniczenie |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, dla okna |
| libmpv | `libmpv2` lub `libmpv1` | `mpv-libs` | plik binarny łączy się z `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | sprawdzanie pisowni oraz zadania `join-words` i `split-words` polecenia [`correct`](correct.md) |
| **ICU** | **`libicu74`** | `libicu`, czyli `libicuuc.so.N` i `libicui18n.so.N` **z Fedory, która go zbudowała** | **wersja, którą ma Fedora danego `.rpm`**, dokładnie — zobacz niżej |
| glibc i libstdc++ | — | — | **dostatecznie nowe**: z Ubuntu 24.04 dla `.deb`, z Fedory danego `.rpm` dla niego |

**ICU to ograniczenie, które ma znaczenie, i nie da się go złagodzić
zmianą jednego wiersza w pakiecie.** Symbole ICU niosą numer jego
wersji głównej (`u_strToUTF8_74`): plik binarny zbudowany względem ICU 74 nie może się
połączyć z ICU 76 ani 77. Zależność zapisana szerzej pozwoliłaby pakietowi
się zainstalować, a potem zawieść przy uruchomieniu — zadeklarowanie jej jako dokładnej to to, co
pozostaje uczciwe. **Dlatego `.rpm` nie jest budowany raz, lecz w
każdej wydanej Fedorze**, względem jej ICU; `.deb` jest budowany w Ubuntu 24.04.

| Dystrybucja | Co daje |
| :----------- | :---------------- |
| Ubuntu 24.04 | `.deb` się instaluje: ICU jest tam w wersji 74 |
| Fedora 42, 43, 44 | `.rpm` jej wersji (`…fc42…`, `…fc43…`, `…fc44…`) instaluje się **przez `dnf`**, zbudowany względem ICU tej Fedory |
| Fedora rawhide (następna), inne dystrybucje z `.rpm` | **nie**: dla nich żaden `.rpm` nie jest budowany — zbuduj ze źródeł |
| każda dystrybucja Debiana lub Ubuntu, której ICU nie jest w wersji 74 | **nie**: `.deb` wymaga `libicu74` — zbuduj ze źródeł |

**Budowanie ze źródeł to rozwiązanie awaryjne i nie ma tego
ograniczenia**: łączy się z ICU maszyny, jakakolwiek jest jego wersja.
To także to, czego trzeba w dystrybucji, której glibc jest starszy
niż w pakietach.

### Co jest testowane w każdym, a co nie

**Oba nie są sprawdzane równie daleko i lepiej to powiedzieć, niż pozwolić
wierzyć w równość, która nie istnieje.** Rozwój odbywa się w Ubuntu; zbudowany tam
`.rpm` nie może być tam zainstalowany. Instaluje się go gdzie indziej: w
Fedorze w kontenerze, raz w tygodniu.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| lista plików | sprawdzana | sprawdzana i **porównywana z listą `.deb`** |
| zadeklarowane zależności | sprawdzana obecność | sprawdzana obecność |
| czy nazwy zależności istnieją w dystrybucji | tak, to nazwy z Ubuntu | **tak**, rozwiązywane przez `dnf` |
| czy pakiet się instaluje | nie — to wymaga uprawnień administratora | **tak**, w Fedorze w kontenerze |
| czy zainstalowane pliki binarne się uruchamiają | tak, z tymczasowego prefiksu | **tak**, ze zainstalowanego pakietu |

**Sprawdzenie w Fedorze testuje `.rpm` zbudowany w Ubuntu, a nie te,
które są wydawane.** Ten obraz nie ma własnego ICU 74: pakiet instaluje się tam, ponieważ
Fedora utrzymuje pakiet zgodności `libicu74`, a sprawdzenie byłoby zielone, dopóki
ten pakiet istnieje. Wydawane pliki `.rpm` są budowane we własnej Fedorze i nie
zależą już od tej zgodności. Zobacz
[czego pakiety wymagają](#czego-pakiety-wymagają-od-dystrybucji).

**Porównanie obu list to sprawdzenie, które liczy się najbardziej.** Oba
pakiety powstają z tej samej instalacji: różnica między nimi byłaby
wadą reguł instalacji, a nie formatu.

**Czego `.deb` nadal nie dowodzi, to tego, że się instaluje**, a powód
się nie zmienił: `dpkg -i` wymaga uprawnień administratora, których bramka
jakości nie ma i nie powinna o nie prosić. `.rpm` natomiast instaluje się w
kontenerze, który do nikogo nie należy. Zobacz
[ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Flatpak i AppImage nie są oferowane**, i jest to wybór — a nie przeoczenie.
Wyjaśnia go ta sama decyzja ADR i jest on **ostateczny**: dystrybucja, która
nie należy ani do rodziny Debiana, ani do rodziny Fedory, buduje ze
źródeł, co opisuje w całości następna sekcja.

## Budowanie ze źródeł

### Wymagania wstępne

| Co jest potrzebne | Po co | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | budowanie | `cmake` |
| kompilator C++23 — wystarczy GCC 13 | — | `g++` |
| `make`, `pkg-config` | budowanie | `make`, `pkg-config` |
| CLI11 | odczyt wiersza poleceń | `libcli11-dev` |
| Qt 6 ≥ 6.4, moduł `Widgets` | okno | `qt6-base-dev` |
| `libmpv` | wbudowany odtwarzacz | `libmpv-dev` |
| ICU | odczyt kodowań | `libicu-dev` |
| Enchant 2 | biblioteka sprawdzania pisowni | `libenchant-2-dev` |

**Cztery ostatnie są wymagane nawet do zbudowania samego
`subedit-cli`**: konfiguracja CMake szuka ich dla całego projektu i
zatrzymuje się, gdy ich brakuje. ICU i Enchant są wymagane podwójnie — plik wykonywalny je ładuje, jedno
do konwersji kodowań, drugie dla sprawdzania pisowni — i
to jedyne biblioteki zewnętrzne, które ładuje w czasie działania.

**Połączenie z siecią przy pierwszym `cmake`**: biblioteka testowa Catch2
jest pobierana z GitHuba podczas konfiguracji. Później nie jest
pobierana ponownie. Aby zbudować samo narzędzie, bez sieci, zobacz niżej.

#### Czego nie wymaga

| Co jest opcjonalne | Co tracimy bez tego |
| :-------------------- | :--------------------- |
| `ffmpeg`, ze względu na `ffprobe` | okno przestaje proponować liczbę klatek na sekundę deklarowaną przez film — [szczegóły](../subedit-gui/video.md#ffmpeg-nie-jest-wymagany) |
| serwer wyświetlania | nic dla `subedit-cli`; `subedit-gui` potrzebuje go, aby się wyświetlić |

### Budowanie

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

Pliki binarne powstają w `build/dev/bin/` — `subedit-cli` i
`subedit-gui`.

Wersja zoptymalizowana:

```bash
make release
```

Oba pliki binarne są wtedy w `build/release/bin/` — `subedit-cli` i
`subedit-gui`. Ten cel nie buduje niczego innego: ani testu wydajności, ani
zestawu testów end-to-end, które mają własne cele.

**Lepiej używać tego celu niż dwóch równoważnych poleceń `cmake`**, które
nie mają jak wiedzieć, ile procesów wolno uruchomić optymalizacji na etapie łączenia:
przy każdym łączeniu uruchamia ona tyle, ile jest rdzeni.
`make release` przekazuje jej `JOBS`, tak jak reszcie budowania.

### Instalowanie

```bash
make release
cmake --install build/release --prefix ~/.local
```

Co jest instalowane, w ścieżkach, które `GNUInstallDirs` ustala dla wybranego
prefiksu:

| Plik | Gdzie |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| pozycja menu pulpitu | `<prefix>/share/applications` |
| ikona | `<prefix>/share/icons/hicolor/scalable/apps` |
| metadane centrum oprogramowania | `<prefix>/share/metainfo` |
| ten podręcznik, w Markdown | `<prefix>/share/subedit/manual` |
| jego tłumaczenie na każdy zainstalowany język — kataloński, czeski, niemiecki, hiszpański, fiński, francuski, galicyjski, hebrajski, węgierski, interlingue, islandzki, włoski, niderlandzki, polski, portugalski, brazylijski portugalski, rosyjski, serbski, turecki, chiński uproszczony | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| wzorce poprawek z Gaupola | `<prefix>/share/subedit/patterns` |
| strony podręcznika man obu plików binarnych, po jednej na podpolecenie dla `subedit-cli` | `<prefix>/share/man/man1` |
| strony `subedit-cli` w każdym zainstalowanym języku — kataloński, czeski, niemiecki, hiszpański, fiński, francuski, galicyjski, hebrajski, węgierski, interlingue, islandzki, włoski, niderlandzki, polski, portugalski, brazylijski portugalski, rosyjski, serbski, turecki, chiński uproszczony | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

**To te same pliki, które instalują pakiety**: pakiety powstają z nich,
zamiast być opisywane po raz drugi.

Z opcją `--prefix ~/.local` oba pliki binarne trafiają do `~/.local/bin`,
który jest w `PATH` większości dystrybucji. Prefiks systemowy —
na przykład `/usr/local` — wymaga odpowiednich uprawnień.

**Pozycja menu umieszczona w `~/.local` nie zawsze pojawia się od
razu**: pulpity odczytują swoją pamięć podręczną we własnym tempie. W międzyczasie `subedit-gui` uruchamia się
z terminala, jak każde inne polecenie.

`DESTDIR` jest respektowany, czego oczekuje twórca pakietów: instalacja w katalogu tymczasowym
nie dotyka niczego poza tym katalogiem.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Budowanie samego narzędzia

Testy są budowane domyślnie i to one wymagają sieci —
`make release` ich nie buduje, ale konfiguruje projekt, a to
konfiguracja pobiera Catch2. Wystarczy raz je wyłączyć: to
ustawienie zostaje w pamięci podręcznej CMake i kolejne budowania je zachowują.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

To nie zwalnia współtwórcy z niczego: bramka jakości
ich wymaga.

### Gdy budowanie kończy się błędem na etapie łączenia

Lawina komunikatów „undefined reference to `std::cout`” w poprawnym kodzie
zwykle oznacza, że alternatywa `c++` systemu wskazuje na `gcc` zamiast na
`g++`: C++ się kompiluje, ale biblioteka standardowa nie jest dołączana.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

Konfiguracja CMake wykrywa ten przypadek i zatrzymuje się z tym komunikatem, zamiast
pozwolić zawieść łączeniu.

### Gdy instalacja pakietu zgłasza problem z `libicu`

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Trzy przyczyny, rozróżniane po wpisanym poleceniu i po nazwie pliku:

- **`rpm -i`**: niczego nie rozwiązuje. Spróbuj ponownie za pomocą
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **plik `.rpm` bez `fcNN` w nazwie** lub z innej Fedory niż Twoja:
  wymaga ICU maszyny, która go zbudowała. Weź ten, którego `fcNN`
  jest wynikiem `cat /etc/fedora-release`;
- **brak `.rpm` dla Twojej wersji**: Fedora rawhide lub inna dystrybucja.
  Żaden pakiet z tego repozytorium się tam nie zainstaluje;
  [zbuduj ze źródeł](#budowanie-ze-źródeł).
