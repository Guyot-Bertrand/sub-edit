# Uppsetning

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

**Þessi síða á við um bæði forritin** — `subedit-cli` og `subedit-gui`.
Þau eru smíðuð og sett upp saman.

Tvær leiðir: **innfæddur pakki**, `.deb` eða `.rpm`, eða **smíði úr
frumkóða**, sem setur upp hvar sem maður vill.

## Pakkarnir

Báðir eru framleiddir af CPack úr **sömu uppsetningarreglum**:
þeir setja sömu skrár á sömu staði og eru aðeins ólíkir í
heitum pakkanna sem þeir byggja á — sömu söfnin heita
ólíkum nöfnum á Debian og á Fedora.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| Það sem pakkinn setur upp | Hvar |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| valmyndarfærslan, táknmyndin, lýsigögn hugbúnaðarmiðstöðvarinnar | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| þessi handbók, á Markdown-sniði | `/usr/share/subedit/manual` |
| þýðing hennar á hvert uppsett tungumál — katalónsku, tékknesku, þýsku, spænsku, finnsku, frönsku, galisísku, hebresku, ungversku, interlingue, íslensku, ítölsku, hollensku, pólsku, portúgölsku, brasilísku portúgölsku, rússnesku, serbnesku, tyrknesku, einfaldaða kínversku | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| leiðréttingarmynstur Gaupol | `/usr/share/subedit/patterns` |
| man-síður `subedit-cli`, ein fyrir hverja undirskipun, og `subedit-gui` — sjá [Man-síðurnar](invocation.md#man-síðurnar) | `/usr/share/man/man1` |
| síður `subedit-cli` á hverju uppsettu tungumáli — katalónsku, tékknesku, þýsku, spænsku, finnsku, frönsku, galisísku, hebresku, ungversku, interlingue, íslensku, ítölsku, hollensku, pólsku, portúgölsku, brasilísku portúgölsku, rússnesku, serbnesku, tyrknesku, einfaldaða kínversku | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

`ffmpeg` er **mælt með en ekki nauðsynlegt** fyrir báða pakkana: án þess hættir
glugginn aðeins að bjóða rammatíðnina sem kvikmyndin lýsir yfir — sjá
[hvað `ffmpeg` breytir](../subedit-gui/video.md#ffmpeg-er-ekki-nauðsynlegt).

### Hvað pakkarnir krefjast af dreifingunni

**Settu upp með `apt` eða `dnf`, aldrei með `dpkg -i` eða `rpm -i`.** Þessar tvær
skipanir leggja pakkann niður án þess að leysa úr háðum hans: þær
telja aðeins upp það sem vantar og stoppa. Þetta er það sem `rpm -i` svarar á
Fedora sem hefur það sem þarf innan seilingar `dnf`:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` sækir þessi
söfn, þar sem þau eru til.

**`.rpm`-pakkinn sem á að taka er sá fyrir þína Fedora**: skráarheitið ber
útgáfuna, `fc42`, `fc43`, `fc44`. Hver þeirra er smíðuð á þeirri Fedora, gegn
ICU hennar — `cat /etc/fedora-release` segir hver á að verða fyrir valinu. `.rpm` frá annarri
Fedora biður um ICU sem þín hefur ekki.

| Háð | `.deb` | `.rpm` | Skilyrði |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, fyrir gluggann |
| libmpv | `libmpv2` eða `libmpv1` | `mpv-libs` | tvíundarskráin tengist `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | stafsetningarathugunin, og verkin `join-words` og `split-words` í [`correct`](correct.md) |
| **ICU** | **`libicu74`** | `libicu`, það er `libicuuc.so.N` og `libicui18n.so.N` **frá Fedora sem smíðaði hann** | **útgáfan sem Fedora á `.rpm`-pakkanum hefur**, nákvæmlega — sjá hér á eftir |
| glibc og libstdc++ | — | — | **nógu nýleg**: þau frá Ubuntu 24.04 fyrir `.deb`, þau frá Fedora á `.rpm`-pakkanum fyrir hann |

**ICU er skilyrðið sem skiptir máli og það verður ekki rýmkað með
því að breyta línu í pakkanum.** Tákn ICU bera númer
aðalútgáfunnar (`u_strToUTF8_74`): tvíundarskrá sem er smíðuð gegn ICU 74 getur ekki
tengst ICU 76 eða 77. Rýmra skilyrði myndi leyfa pakkanum að
setjast upp og bregðast svo við ræsingu — að lýsa því nákvæmu er það sem
heldur heiðarleikanum. **Þess vegna er `.rpm` ekki smíðaður einu sinni, heldur á
hverri útgefinni Fedora**, gegn ICU hennar; `.deb` er smíðaður á Ubuntu 24.04.

| Dreifing | Hvað hún fær |
| :----------- | :---------------- |
| Ubuntu 24.04 | `.deb` setst upp: ICU er þar í útgáfu 74 |
| Fedora 42, 43, 44 | `.rpm` fyrir útgáfu hennar (`…fc42…`, `…fc43…`, `…fc44…`) setst upp **í gegnum `dnf`**, smíðaður gegn ICU þeirrar Fedora |
| Fedora rawhide (sú næsta), aðrar `.rpm`-dreifingar | **nei**: enginn `.rpm` er smíðaður fyrir þær — smíðaðu úr frumkóða |
| hver Debian- eða Ubuntu-dreifing þar sem ICU er ekki í útgáfu 74 | **nei**: `.deb` krefst `libicu74` — smíðaðu úr frumkóða |

**Smíði úr frumkóða er varaleiðin og hún hefur ekki þetta
skilyrði**: hún tengist ICU vélarinnar, hver sem útgáfan er.
Hún er líka það sem þarf á dreifingu þar sem glibc er eldra
en í pökkunum.

### Hvað er prófað af hvoru um sig, og hvað ekki

**Þeir eru ekki athugaðir jafn langt og betra er að segja það en að láta
menn trúa á jafngildi sem er ekki til.** Þróun fer fram á Ubuntu; `.rpm`
sem er smíðaður þar verður ekki settur upp þar. Hann er settur upp annars staðar: á
Fedora í gámi, einu sinni í viku.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| skráalistinn | athugaður | athugaður, og **borinn saman við lista `.deb`** |
| háðu sem eru lýst yfir | athugað að þau séu til staðar | athugað að þau séu til staðar |
| að heiti háðu séu til í dreifingunni | já, þau eru frá Ubuntu | **já**, leyst af `dnf` |
| að pakkinn setjist upp | nei — til þess þarf stjórnandaréttindi | **já**, á Fedora í gámi |
| að uppsettu tvíundarskrárnar ræsist | já, úr tímabundnu forskeyti | **já**, úr uppsetta pakkanum |

**Fedora-athugunin prófar `.rpm` sem er smíðaður á Ubuntu, ekki þá sem eru
gefnir út.** Sú mynd hefur ekkert eigið ICU 74: pakkinn setst þar upp vegna þess að
Fedora geymir samhæfnipakkann `libicu74`, og athugunin yrði græn svo lengi sem
sá pakki er til. Útgefnu `.rpm`-skrárnar eru smíðaðar á sinni eigin Fedora og
byggja ekki lengur á þeim samhæfnipakka. Sjá
[hvað pakkarnir krefjast](#hvað-pakkarnir-krefjast-af-dreifingunni).

**Samanburður listanna tveggja er athugunin sem skiptir mestu máli.** Pakkarnir
tveir koma úr sömu uppsetningu: munur á þeim væri galli í
uppsetningarreglunum, ekki í sniðinu.

**Það sem `.deb` sannar enn ekki er að hann setjist upp**, og ástæðan
er óbreytt: `dpkg -i` þarf stjórnandaréttindi, sem gæðahlið hefur
ekki og má ekki biðja um. `.rpm` setst hins vegar upp í
gámi sem enginn á. Sjá
[ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Flatpak og AppImage eru ekki í boði**, og það er val — ekki yfirsjón.
Það er útskýrt í sömu ADR og er **endanlegt**: dreifing sem er
hvorki af Debian-ætt né Fedora-ætt smíðar úr
frumkóða, sem næsti kafli lýsir í heild.

## Smíði úr frumkóða

### Forsendur

| Það sem þarf | Hvers vegna | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | smíðin | `cmake` |
| C++23-þýðandi — GCC 13 dugar | — | `g++` |
| `make`, `pkg-config` | smíðin | `make`, `pkg-config` |
| CLI11 | lestur skipanalínunnar | `libcli11-dev` |
| Qt 6 ≥ 6.4, eining `Widgets` | glugginn | `qt6-base-dev` |
| `libmpv` | innbyggði spilarinn | `libmpv-dev` |
| ICU | lestur stafakóðana | `libicu-dev` |
| Enchant 2 | safn stafsetningarathugunarinnar | `libenchant-2-dev` |

**Síðustu fjögur eru nauðsynleg jafnvel til að smíða aðeins
`subedit-cli`**: CMake-stillingin leitar að þeim fyrir allt verkefnið og
stoppar ef þau vantar. ICU og Enchant eru nauðsynleg á tvo vegu — keyrsluskráin hleður þeim, annað
til að umbreyta stafakóðunum, hitt fyrir stafsetningarathugunina — og
þau eru einu safnir þriðja aðila sem hún hleður við keyrslu.

**Nettenging við fyrsta `cmake`**: prófunarsafnið Catch2
er sótt frá GitHub við stillingu. Það er ekki sótt aftur
eftir það. Til að smíða tólið eitt og sér, án nets, sjá hér á eftir.

#### Hvað er ekki nauðsynlegt

| Það sem er valfrjálst | Það sem tapast án þess |
| :-------------------- | :--------------------- |
| `ffmpeg`, fyrir `ffprobe` | glugginn býður ekki lengur rammatíðnina sem kvikmyndin lýsir yfir — [nánari upplýsingar](../subedit-gui/video.md#ffmpeg-er-ekki-nauðsynlegt) |
| skjáþjónn | ekkert fyrir `subedit-cli`; `subedit-gui` þarf einn til að sýna sig |

### Að smíða

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

Tvíundarskrárnar eru framleiddar í `build/dev/bin/` — `subedit-cli` og
`subedit-gui`.

Fyrir hagræðta útgáfu:

```bash
make release
```

Báðar tvíundarskrárnar eru þá í `build/release/bin/` — `subedit-cli` og
`subedit-gui`. Markmiðið smíðar ekkert annað: hvorki afkastaprófið né
heildarprófunarbúnaðinn, sem hafa sín eigin markmið.

**Veldu frekar þetta markmið en skipanirnar tvær í `cmake` sem eru jafngildar**, því að þær
hafa enga leið til að vita hve mörg ferli tengingarbestun má
ræsa: við hverja tengingu ræsir hún jafn mörg og kjarnarnir eru.
`make release` gefur henni `JOBS`, eins og það gerir fyrir afganginn af smíðinni.

### Að setja upp

```bash
make release
cmake --install build/release --prefix ~/.local
```

Það sem er sett upp, á slóðunum sem `GNUInstallDirs` ákveður fyrir valið
forskeyti:

| Skrá | Hvar |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| valmyndarfærsla skjáborðsins | `<prefix>/share/applications` |
| táknmyndin | `<prefix>/share/icons/hicolor/scalable/apps` |
| lýsigögn hugbúnaðarmiðstöðvarinnar | `<prefix>/share/metainfo` |
| þessi handbók, á Markdown-sniði | `<prefix>/share/subedit/manual` |
| þýðing hennar á hvert uppsett tungumál — katalónsku, tékknesku, þýsku, spænsku, finnsku, frönsku, galisísku, hebresku, ungversku, interlingue, íslensku, ítölsku, hollensku, pólsku, portúgölsku, brasilísku portúgölsku, rússnesku, serbnesku, tyrknesku, einfaldaða kínversku | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| leiðréttingarmynstur Gaupol | `<prefix>/share/subedit/patterns` |
| man-síður beggja tvíundarskránna, ein fyrir hverja undirskipun hjá `subedit-cli` | `<prefix>/share/man/man1` |
| síður `subedit-cli` á hverju uppsettu tungumáli — katalónsku, tékknesku, þýsku, spænsku, finnsku, frönsku, galisísku, hebresku, ungversku, interlingue, íslensku, ítölsku, hollensku, pólsku, portúgölsku, brasilísku portúgölsku, rússnesku, serbnesku, tyrknesku, einfaldaða kínversku | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

**Þetta eru sömu skrár og pakkarnir setja upp**: pakkarnir koma úr þeim,
í stað þess að vera lýst í annað sinn.

Með `--prefix ~/.local` lenda báðar tvíundarskrárnar í `~/.local/bin`,
sem er í `PATH` á flestum dreifingum. Kerfisforskeyti —
`/usr/local`, til dæmis — krefst viðeigandi réttinda.

**Valmyndarfærsla sem er sett undir `~/.local` birtist ekki alltaf
strax**: skjáborð lesa skyndiminni sitt upp á nýtt á eigin hraða. `subedit-gui` ræsist
úr skel á meðan, eins og hver önnur skipun.

`DESTDIR` er virt, sem pökkunaraðili býst við: uppsetning í millisvæði
snertir ekkert utan millisvæðisins.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Að smíða aðeins tólið

Prófanirnar eru smíðaðar sjálfgefið og þær eru það sem biður um netið —
`make release` smíðar þær ekki, en það stillir verkefnið, og það er
stillingin sem sækir Catch2. Nóg er að slökkva á þeim einu sinni: stillingin
helst í CMake-skyndiminninu og næstu smíðar halda henni.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Þetta leysir engan þátttakanda undan neinu: gæðahliðið
krefst þeirra.

### Ef smíðin bilar við tengingu

Skriða af „undefined reference to `std::cout`“ á gildum kóða
þýðir yfirleitt að `c++`-valkostur kerfisins bendir á `gcc` í stað
`g++`: C++ þýðist en staðalsafnið er ekki tengt.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

CMake-stillingin greinir þetta tilvik og stoppar með þessum skilaboðum í stað þess að
láta tenginguna bila.

### Ef uppsetning pakkans kvartar yfir `libicu`

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Þrjár orsakir, sem eru greindar að með skipuninni sem var slegin inn og skráarheitinu:

- **`rpm -i`**: hún leysir ekkert. Reyndu aftur með
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **`.rpm` án `fcNN` í heitinu**, eða frá annarri Fedora en þinni:
  hann biður um ICU vélarinnar sem smíðaði hann. Taktu þann þar sem `fcNN`
  er úttak `cat /etc/fedora-release`;
- **enginn `.rpm` fyrir þína útgáfu**: Fedora rawhide eða önnur dreifing.
  Enginn pakki úr þessu safni mun setjast þar upp;
  [smíðaðu úr frumkóða](#smíði-úr-frumkóða).
