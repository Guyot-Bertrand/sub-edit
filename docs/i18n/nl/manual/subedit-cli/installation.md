# Installatie

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

**Deze pagina geldt voor beide programma's** — `subedit-cli` en `subedit-gui`.
Ze worden samen gebouwd en samen geïnstalleerd.

Twee wegen: **een native pakket**, `.deb` of `.rpm`, of **bouwen vanuit de
broncode**, waarna men installeert waar men wil.

## De pakketten

Beide worden door CPack gemaakt uit **dezelfde installatieregels**:
ze plaatsen dezelfde bestanden op dezelfde plaatsen, en verschillen alleen in de
namen van de pakketten waarvan ze afhangen — dezelfde bibliotheken heten
op Debian anders dan op Fedora.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| Wat het pakket installeert | Waar |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| het menu-item, het pictogram, de metadata voor het softwarecentrum | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| deze handleiding, in Markdown | `/usr/share/subedit/manual` |
| de vertaling ervan in elke geïnstalleerde taal — Catalaans, Tsjechisch, Duits, Spaans, Fins, Frans, Galicisch, Hebreeuws, Hongaars, Interlingue, IJslands, Italiaans, Nederlands, Pools, Portugees, Braziliaans Portugees | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |
| de correctiepatronen van Gaupol | `/usr/share/subedit/patterns` |
| de man-pagina's van `subedit-cli`, een per subcommando, en van `subedit-gui` — zie [De man-pagina's](invocation.md#de-man-paginas) | `/usr/share/man/man1` |
| de pagina's van `subedit-cli` in elke geïnstalleerde taal — Catalaans, Tsjechisch, Duits, Spaans, Fins, Frans, Galicisch, Hebreeuws, Hongaars, Interlingue, IJslands, Italiaans, Nederlands, Pools, Portugees, Braziliaans Portugees | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |

`ffmpeg` wordt door beide pakketten **aanbevolen en is niet vereist**: zonder dat
biedt het venster alleen de framerate die de film declareert niet meer aan — zie
[wat `ffmpeg` verandert](../subedit-gui/video.md#ffmpeg-is-niet-vereist).

### Wat de pakketten van de distributie vereisen

**Installeer met `apt` of `dnf`, nooit met `dpkg -i` of `rpm -i`.** Die twee
commando's zetten het pakket neer zonder zijn afhankelijkheden op te lossen: ze
sommen alleen de ontbrekende op en stoppen. Dit is wat `rpm -i` antwoordt op
een Fedora die wel heeft wat nodig is binnen bereik van `dnf`:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` haalt die
bibliotheken op, waar ze bestaan.

**De `.rpm` die u moet nemen is die van uw Fedora**: de bestandsnaam bevat de
versie, `fc42`, `fc43`, `fc44`. Elk is gebouwd op die Fedora, tegen
zijn ICU — `cat /etc/fedora-release` zegt welke u moet nemen. Een `.rpm` van een andere
Fedora vraagt om een ICU die de uwe niet heeft.

| Afhankelijkheid | `.deb` | `.rpm` | Beperking |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, voor het venster |
| libmpv | `libmpv2` of `libmpv1` | `mpv-libs` | het binaire bestand linkt `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | de spellingcontrole, en de taken `join-words` en `split-words` van [`correct`](correct.md) |
| **ICU** | **`libicu74`** | `libicu`, dat wil zeggen de `libicuuc.so.N` en `libicui18n.so.N` **van de Fedora die het heeft gebouwd** | **de versie die de Fedora van de `.rpm` heeft**, exact — zie hieronder |
| glibc en libstdc++ | — | — | **recent genoeg**: die van Ubuntu 24.04 voor de `.deb`, die van de Fedora van de `.rpm` voor die laatste |

**ICU is de beperking die ertoe doet, en ze kan niet worden versoepeld door
een regel van het pakket te wijzigen.** De symbolen van ICU dragen het nummer van zijn
hoofdversie (`u_strToUTF8_74`): een binair bestand dat tegen ICU 74 is gebouwd, kan niet
linken met ICU 76 of 77. Een ruimer geschreven afhankelijkheid zou het pakket laten
installeren en dan bij het starten laten mislukken — haar exact declareren is wat
eerlijk blijft. **Daarom wordt de `.rpm` niet één keer gebouwd, maar op
elke uitgebrachte Fedora**, tegen zijn ICU; de `.deb` wordt op Ubuntu 24.04 gebouwd.

| Distributie | Wat ze oplevert |
| :----------- | :---------------- |
| Ubuntu 24.04 | de `.deb` installeert: ICU is daar versie 74 |
| Fedora 42, 43, 44 | de `.rpm` van zijn versie (`…fc42…`, `…fc43…`, `…fc44…`) installeert **via `dnf`**, gebouwd tegen de ICU van die Fedora |
| Fedora rawhide (de volgende), de andere `.rpm`-distributies | **nee**: er wordt geen `.rpm` voor gebouwd — bouw vanuit de broncode |
| elke Debian- of Ubuntu-distributie waarvan de ICU niet versie 74 is | **nee**: de `.deb` vereist `libicu74` — bouw vanuit de broncode |

**Bouwen vanuit de broncode is de terugvaloptie, en ze heeft deze
beperking niet**: ze linkt met de ICU van de machine, welke versie die ook heeft.
Het is ook wat nodig is op een distributie waarvan de glibc ouder is
dan die van de pakketten.

### Wat van elk wordt getest, en wat niet

**De twee worden niet even ver gecontroleerd, en het is beter dat te zeggen dan
een gelijkwaardigheid te laten geloven die niet bestaat.** De ontwikkeling gebeurt op Ubuntu; een
`.rpm` die daar is gebouwd, kan daar niet worden geïnstalleerd. Hij wordt elders geïnstalleerd: op een
Fedora in een container, een keer per week.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| de lijst met bestanden | gecontroleerd | gecontroleerd, en **vergeleken met die van de `.deb`** |
| de gedeclareerde afhankelijkheden | aanwezig gecontroleerd | aanwezig gecontroleerd |
| dat de namen van de afhankelijkheden in de distributie bestaan | ja, het zijn die van Ubuntu | **ja**, opgelost door `dnf` |
| dat het pakket installeert | nee — daarvoor zijn beheerdersrechten nodig | **ja**, op een Fedora in een container |
| dat de geïnstalleerde binaire bestanden starten | ja, vanuit een tijdelijk prefix | **ja**, vanuit het geïnstalleerde pakket |

**De Fedora-controle test de `.rpm` die op Ubuntu is gebouwd, niet degene die
worden uitgebracht.** Dat image heeft geen eigen ICU 74: het pakket installeert er omdat
Fedora een compatibiliteitspakket `libicu74` bewaart, en de controle zou groen zijn zolang
dat pakket bestaat. De uitgebrachte `.rpm`-bestanden worden op hun eigen Fedora gebouwd en
hangen niet meer af van die compatibiliteit. Zie
[wat de pakketten vereisen](#wat-de-pakketten-van-de-distributie-vereisen).

**De twee lijsten vergelijken is de controle die het meest telt.** De
twee pakketten komen uit dezelfde installatie: een verschil tussen beide zou een
gebrek zijn van de installatieregels, niet van het formaat.

**Wat de `.deb` nog altijd niet bewijst, is dat hij installeert**, en de reden
is niet veranderd: `dpkg -i` vereist beheerdersrechten, die een kwaliteitspoort
niet heeft en niet mag vragen. De `.rpm` daarentegen installeert in een
container die van niemand is. Zie
[ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Flatpak en AppImage worden niet aangeboden**, en dat is een keuze — geen vergetelheid.
Het wordt in dezelfde ADR uitgelegd, en het is **definitief**: een distributie die
noch van de Debian-familie noch van de Fedora-familie is, bouwt vanuit de
broncode, wat de volgende sectie volledig beschrijft.

## Bouwen vanuit de broncode

### Vereisten

| Wat nodig is | Waarom | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | het bouwen | `cmake` |
| een C++23-compiler — GCC 13 volstaat | — | `g++` |
| `make`, `pkg-config` | het bouwen | `make`, `pkg-config` |
| CLI11 | de opdrachtregel lezen | `libcli11-dev` |
| Qt 6 ≥ 6.4, module `Widgets` | het venster | `qt6-base-dev` |
| `libmpv` | de ingebouwde speler | `libmpv-dev` |
| ICU | de coderingen lezen | `libicu-dev` |
| Enchant 2 | de bibliotheek van de spellingcontrole | `libenchant-2-dev` |

**De laatste vier zijn vereist, zelfs om alleen
`subedit-cli` te bouwen**: de CMake-configuratie zoekt ze voor het hele project en
stopt als ze ontbreken. ICU en Enchant zijn dubbel vereist — het uitvoerbare bestand laadt ze, de ene
om de coderingen om te zetten, de andere voor de spellingcontrole — en
ze zijn de enige bibliotheken van derden die het tijdens de uitvoering laadt.

**Een netwerkverbinding bij de eerste `cmake`**: de testbibliotheek Catch2
wordt bij de configuratie van GitHub opgehaald. Ze wordt daarna niet opnieuw
gedownload. Om alleen het hulpprogramma te bouwen, zonder netwerk, zie hieronder.

#### Wat niet vereist is

| Wat optioneel is | Wat zonder het verloren gaat |
| :-------------------- | :--------------------- |
| `ffmpeg`, voor zijn `ffprobe` | het venster biedt de framerate die de film declareert niet meer aan — [de details](../subedit-gui/video.md#ffmpeg-is-niet-vereist) |
| een displayserver | niets voor `subedit-cli`; `subedit-gui` heeft er een nodig om zich te tonen |

### Bouwen

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

De binaire bestanden worden geproduceerd in `build/dev/bin/` — `subedit-cli` en
`subedit-gui`.

Voor een geoptimaliseerde versie:

```bash
make release
```

Beide binaire bestanden staan dan in `build/release/bin/` — `subedit-cli` en
`subedit-gui`. Het doel bouwt niets anders: noch de benchmark, noch de
end-to-end-testomgeving, die hun eigen doelen hebben.

**Geef de voorkeur aan dit doel boven de twee gelijkwaardige `cmake`-commando's**, die
niet kunnen weten hoeveel processen de optimalisatie bij het linken mag
starten: bij elke link starten ze er zoveel als er kernen zijn.
`make release` geeft het `JOBS` mee, zoals het dat ook voor de rest van de build doet.

### Installeren

```bash
make release
cmake --install build/release --prefix ~/.local
```

Wat er wordt geïnstalleerd, op de paden die `GNUInstallDirs` voor het gekozen
prefix vastlegt:

| Bestand | Waar |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| het menu-item van het bureaublad | `<prefix>/share/applications` |
| het pictogram | `<prefix>/share/icons/hicolor/scalable/apps` |
| de metadata voor het softwarecentrum | `<prefix>/share/metainfo` |
| deze handleiding, in Markdown | `<prefix>/share/subedit/manual` |
| de vertaling ervan in elke geïnstalleerde taal — Catalaans, Tsjechisch, Duits, Spaans, Fins, Frans, Galicisch, Hebreeuws, Hongaars, Interlingue, IJslands, Italiaans, Nederlands, Pools, Portugees, Braziliaans Portugees | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |
| de correctiepatronen van Gaupol | `<prefix>/share/subedit/patterns` |
| de man-pagina's van beide binaire bestanden, een per subcommando voor `subedit-cli` | `<prefix>/share/man/man1` |
| de pagina's van `subedit-cli` in elke geïnstalleerde taal — Catalaans, Tsjechisch, Duits, Spaans, Fins, Frans, Galicisch, Hebreeuws, Hongaars, Interlingue, IJslands, Italiaans, Nederlands, Pools, Portugees, Braziliaans Portugees | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |

**Het zijn dezelfde bestanden die de pakketten installeren**: de pakketten komen eruit voort,
in plaats van een tweede keer te worden beschreven.

Met `--prefix ~/.local` komen beide binaire bestanden in `~/.local/bin` terecht,
dat in het `PATH` van de meeste distributies staat. Een systeemprefix —
`/usr/local`, bijvoorbeeld — vereist de bijbehorende rechten.

**Een menu-item dat onder `~/.local` is geplaatst, verschijnt niet altijd meteen**:
bureaubladen lezen hun cache in hun eigen tempo opnieuw. `subedit-gui` start
ondertussen vanaf een terminal, zoals elk ander commando.

`DESTDIR` wordt gerespecteerd, wat een pakketmaker verwacht: een gestagede installatie
raakt niets buiten de stagingmap aan.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Alleen het hulpprogramma bouwen

De tests worden standaard gebouwd, en zij zijn wat het netwerk vraagt —
`make release` bouwt ze niet, maar configureert het project, en het is de
configuratie die Catch2 ophaalt. Ze één keer uitschakelen volstaat: de
instelling blijft in de CMake-cache staan, en de volgende builds behouden haar.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Dat ontslaat een bijdrager van niets: de kwaliteitspoort vereist ze
wel.

### Als de build bij het linken mislukt

Een lawine van "undefined reference to `std::cout`" bij geldige code
betekent meestal dat het alternatief `c++` van het systeem naar `gcc` wijst in plaats
van naar `g++`: de C++ compileert, maar de standaardbibliotheek wordt niet gelinkt.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

De CMake-configuratie detecteert dit geval en stopt met deze melding in plaats van
de link te laten mislukken.

### Als het installeren van het pakket klaagt over `libicu`

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Drie oorzaken, die men onderscheidt aan het getypte commando en aan de bestandsnaam:

- **`rpm -i`**: het lost niets op. Probeer opnieuw met
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **een `.rpm` zonder `fcNN` in zijn naam**, of van een andere Fedora dan de uwe:
  hij vraagt om de ICU van de machine die hem heeft gebouwd. Neem degene waarvan `fcNN`
  de uitvoer is van `cat /etc/fedora-release`;
- **geen `.rpm` voor uw versie**: Fedora rawhide of een andere distributie.
  Geen enkel pakket van deze repository installeert daar;
  [bouw vanuit de broncode](#bouwen-vanuit-de-broncode).
