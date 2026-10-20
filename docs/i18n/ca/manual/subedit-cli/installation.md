# Instal·lació

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

**Aquesta pàgina val per a tots dos programes** — `subedit-cli` i `subedit-gui`.
Es compilen i s'instal·len junts.

Dos camins: **un paquet natiu**, `.deb` o `.rpm`, o **compilar des del
codi font**, que després s'instal·la on es vulgui.

## Els paquets

Tots dos els produeix CPack a partir de **les mateixes regles d'instal·lació**:
posen els mateixos fitxers als mateixos llocs, i només difereixen en els
noms dels paquets dels quals depenen — les mateixes biblioteques s'anomenen
de manera diferent a Debian i a Fedora.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| Què instal·la el paquet | On |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| l'entrada de menú, la icona, les metadades del centre de programari | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| aquest manual, en Markdown | `/usr/share/subedit/manual` |
| la seva traducció a cada llengua instal·lada — català, txec, alemany, espanyol, finès, francès, gallec, hebreu, hongarès, interlingua, islandès, italià, neerlandès, polonès, portuguès, portuguès del Brasil | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |
| els patrons de correcció de Gaupol | `/usr/share/subedit/patterns` |
| les pàgines de manual de `subedit-cli`, una per subordre, i de `subedit-gui` — vegeu [Les pàgines de manual](invocation.md#les-pàgines-de-manual) | `/usr/share/man/man1` |
| les pàgines de `subedit-cli` a cada llengua instal·lada — català, txec, alemany, espanyol, finès, francès, gallec, hebreu, hongarès, interlingua, islandès, italià, neerlandès, polonès, portuguès, portuguès del Brasil | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |

Tots dos paquets **recomanen `ffmpeg` però no l'exigeixen**: sense ell, la
finestra només deixa d'oferir la freqüència de fotogrames que declara la pel·lícula — vegeu
[què canvia `ffmpeg`](../subedit-gui/video.md#ffmpeg-no-és-obligatori).

### Què requereixen els paquets de la distribució

**Instal·leu amb `apt` o `dnf`, mai amb `dpkg -i` o `rpm -i`.** Aquestes dues
ordres col·loquen el paquet sense resoldre'n les dependències: es limiten a
llistar les que falten, i s'aturen. Això és el que respon `rpm -i` en
una Fedora que sí que té el que necessita a l'abast de `dnf`:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` obté aquestes
biblioteques, allà on existeixen.

**El `.rpm` que heu d'agafar és el de la vostra Fedora**: el nom del fitxer porta la
versió, `fc42`, `fc43`, `fc44`. Cadascun es compila en aquesta Fedora, contra
la seva ICU — `cat /etc/fedora-release` diu quin cal agafar. Un `.rpm` d'una altra
Fedora demana una ICU que la vostra no té.

| Dependència | `.deb` | `.rpm` | Restricció |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, per a la finestra |
| libmpv | `libmpv2` o `libmpv1` | `mpv-libs` | el binari enllaça `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | el corrector ortogràfic, i les tasques `join-words` i `split-words` de [`correct`](correct.md) |
| **ICU** | **`libicu74`** | `libicu`, és a dir, les `libicuuc.so.N` i `libicui18n.so.N` **de la Fedora que l'ha compilat** | **la versió que té la Fedora del `.rpm`**, exactament — vegeu més avall |
| glibc i libstdc++ | — | — | **prou recents**: les d'Ubuntu 24.04 per al `.deb`, les de la Fedora del `.rpm` per a aquest |

**ICU és la restricció que compta, i no es pot relaxar
canviant una línia del paquet.** Els símbols d'ICU porten el número de la seva
versió principal (`u_strToUTF8_74`): un binari compilat contra ICU 74 no pot
enllaçar amb ICU 76 o 77. Una dependència escrita de manera més àmplia deixaria que el paquet
s'instal·lés, i després fallaria en arrencar — declarar-la exacta és el que
és honest. **Per això el `.rpm` no es compila una sola vegada, sinó a
cada Fedora publicada**, contra la seva ICU; el `.deb` es compila a Ubuntu 24.04.

| Distribució | Què dona |
| :----------- | :---------------- |
| Ubuntu 24.04 | el `.deb` s'instal·la: ICU hi és a la versió 74 |
| Fedora 42, 43, 44 | el `.rpm` de la seva versió (`…fc42…`, `…fc43…`, `…fc44…`) s'instal·la **amb `dnf`**, compilat contra la ICU d'aquesta Fedora |
| Fedora rawhide (la següent), les altres distribucions `.rpm` | **no**: no s'hi compila cap `.rpm` — compileu des del codi font |
| qualsevol distribució Debian o Ubuntu la ICU de la qual no sigui a la versió 74 | **no**: el `.deb` requereix `libicu74` — compileu des del codi font |

**Compilar des del codi font és la solució de recurs, i no té aquesta
restricció**: enllaça amb la ICU de la màquina, sigui quina sigui la seva versió.
També és el que cal en una distribució la glibc de la qual és més antiga
que la dels paquets.

### Què es prova de cadascun, i què no

**Tots dos no es comproven fins al mateix punt, i val més dir-ho que deixar
creure en una paritat que no existeix.** El desenvolupament es fa a Ubuntu; un
`.rpm` compilat allà no s'hi pot instal·lar. S'instal·la en un altre lloc: en una
Fedora dins d'un contenidor, una vegada per setmana.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| la llista de fitxers | comprovada | comprovada, i **comparada amb la del `.deb`** |
| les dependències declarades | se'n comprova la presència | se'n comprova la presència |
| que els noms de les dependències existeixen a la distribució | sí, són els d'Ubuntu | **sí**, resolts per `dnf` |
| que el paquet s'instal·la | no — això necessita drets d'administrador | **sí**, en una Fedora dins d'un contenidor |
| que els binaris instal·lats arrenquen | sí, des d'un prefix temporal | **sí**, des del paquet instal·lat |

**La comprovació de Fedora prova el `.rpm` compilat a Ubuntu, no els que es
publiquen.** Aquesta imatge no té cap ICU 74 pròpia: el paquet s'hi instal·la perquè
Fedora manté una `libicu74` de compatibilitat, i la comprovació seria verda mentre
existeixi aquest paquet. Els fitxers `.rpm` publicats es compilen a la seva pròpia Fedora, i ja no
depenen d'aquesta compatibilitat. Vegeu
[què requereixen els paquets](#què-requereixen-els-paquets-de-la-distribució).

**Comparar les dues llistes és la comprovació que més compta.** Els
dos paquets surten de la mateixa instal·lació: una diferència entre ells seria un
defecte de les regles d'instal·lació, no del format.

**El que el `.deb` encara no demostra és que s'instal·la**, i el motiu
no ha canviat: `dpkg -i` necessita drets d'administrador, que una porta de qualitat
no té i no ha de demanar. El `.rpm`, per la seva banda, s'instal·la en un
contenidor que no és de ningú. Vegeu
[l'ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**No s'ofereixen Flatpak ni AppImage**, i és una elecció — no un oblit.
S'explica al mateix ADR, i és **definitiva**: una distribució que
no és ni de la família Debian ni de la família Fedora compila des del
codi font, cosa que la secció següent descriu sencera.

## Compilar des del codi font

### Requisits previs

| Què cal | Per què | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | la compilació | `cmake` |
| un compilador de C++23 — GCC 13 n'hi ha prou | — | `g++` |
| `make`, `pkg-config` | la compilació | `make`, `pkg-config` |
| CLI11 | la lectura de la línia d'ordres | `libcli11-dev` |
| Qt 6 ≥ 6.4, mòdul `Widgets` | la finestra | `qt6-base-dev` |
| `libmpv` | el reproductor integrat | `libmpv-dev` |
| ICU | la lectura de les codificacions | `libicu-dev` |
| Enchant 2 | la biblioteca del corrector ortogràfic | `libenchant-2-dev` |

**Els quatre últims són necessaris fins i tot per compilar només
`subedit-cli`**: la configuració de CMake els cerca per a tot el projecte, i
s'atura si falten. ICU i Enchant són doblement necessaris — l'executable els carrega, l'un
per convertir les codificacions, l'altre per al corrector ortogràfic — i
són les úniques biblioteques de tercers que carrega en temps d'execució.

**Una connexió a la xarxa en el primer `cmake`**: la biblioteca de proves Catch2
es descarrega de GitHub durant la configuració. No es torna a descarregar
després. Per compilar l'eina sola, sense xarxa, vegeu més avall.

#### Què no és necessari

| Què és opcional | Què es perd sense ell |
| :-------------------- | :--------------------- |
| `ffmpeg`, pel seu `ffprobe` | la finestra ja no ofereix la freqüència de fotogrames que declara la pel·lícula — [els detalls](../subedit-gui/video.md#ffmpeg-no-és-obligatori) |
| un servidor de visualització | res per a `subedit-cli`; `subedit-gui` en necessita un per mostrar-se |

### Compilar

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

Els binaris es produeixen a `build/dev/bin/` — `subedit-cli` i
`subedit-gui`.

Per a una versió optimitzada:

```bash
make release
```

Aleshores tots dos binaris són a `build/release/bin/` — `subedit-cli` i
`subedit-gui`. L'objectiu no compila res més: ni el banc de proves de rendiment, ni
l'arnès de punta a punta, que tenen els seus propis objectius.

**Preferiu aquest objectiu a les dues ordres `cmake` equivalents**, que no
tenen cap manera de saber quants processos pot
iniciar l'optimització en temps d'enllaç: a cada enllaç, n'inicia tants com nuclis hi ha.
`make release` li passa `JOBS`, com fa amb la resta de la compilació.

### Instal·lar

```bash
make release
cmake --install build/release --prefix ~/.local
```

Què s'instal·la, als camins que `GNUInstallDirs` fixa per al
prefix triat:

| Fitxer | On |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| l'entrada del menú de l'escriptori | `<prefix>/share/applications` |
| la icona | `<prefix>/share/icons/hicolor/scalable/apps` |
| les metadades del centre de programari | `<prefix>/share/metainfo` |
| aquest manual, en Markdown | `<prefix>/share/subedit/manual` |
| la seva traducció a cada llengua instal·lada — català, txec, alemany, espanyol, finès, francès, gallec, hebreu, hongarès, interlingua, islandès, italià, neerlandès, polonès, portuguès, portuguès del Brasil | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |
| els patrons de correcció de Gaupol | `<prefix>/share/subedit/patterns` |
| les pàgines de manual de tots dos binaris, una per subordre per a `subedit-cli` | `<prefix>/share/man/man1` |
| les pàgines de `subedit-cli` a cada llengua instal·lada — català, txec, alemany, espanyol, finès, francès, gallec, hebreu, hongarès, interlingua, islandès, italià, neerlandès, polonès, portuguès, portuguès del Brasil | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |

**Són els mateixos fitxers que instal·len els paquets**: els paquets en surten,
en lloc de descriure'ls una segona vegada.

Amb `--prefix ~/.local`, tots dos binaris van a parar a `~/.local/bin`,
que és al `PATH` de la majoria de distribucions. Un prefix del sistema —
`/usr/local`, per exemple — necessita els drets corresponents.

**Una entrada de menú posada sota `~/.local` no sempre apareix
immediatament**: els escriptoris tornen a llegir la seva memòria cau al seu ritme. Mentrestant,
`subedit-gui` s'inicia des d'un terminal, com qualsevol altra ordre.

`DESTDIR` es respecta, cosa que un empaquetador espera: una instal·lació en un directori intermedi
no toca res fora d'aquest directori.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Compilar l'eina sola

Les proves es compilen per defecte, i són elles les que demanen la xarxa —
`make release` no les compila, però configura el projecte, i és la
configuració la que descarrega Catch2. N'hi ha prou de desactivar-les una vegada: el
paràmetre es queda a la memòria cau de CMake, i les compilacions següents el conserven.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Això no eximeix de res un col·laborador: la porta de qualitat sí que
les requereix.

### Si la compilació falla en l'enllaç

Una allau de «undefined reference to `std::cout`» en un codi vàlid
normalment vol dir que l'alternativa `c++` del sistema apunta a `gcc` en lloc
de `g++`: el C++ es compila, però la biblioteca estàndard no s'enllaça.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

La configuració de CMake detecta aquest cas i s'atura amb aquest missatge en lloc
de deixar que falli l'enllaç.

### Si la instal·lació del paquet es queixa de `libicu`

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Tres causes, que es distingeixen per l'ordre escrita i pel nom del fitxer:

- **`rpm -i`**: no resol res. Torneu-ho a provar amb
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **un `.rpm` sense `fcNN` al nom**, o d'una Fedora diferent de la vostra:
  demana la ICU de la màquina que l'ha compilat. Agafeu aquell el `fcNN` del qual
  és la sortida de `cat /etc/fedora-release`;
- **cap `.rpm` per a la vostra versió**: Fedora rawhide o una altra distribució.
  Cap paquet d'aquest repositori no s'hi instal·larà;
  [compileu des del codi font](#compilar-des-del-codi-font).
