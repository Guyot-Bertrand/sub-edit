# Installazione

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

**Questa pagina vale per entrambi i programmi** — `subedit-cli` e `subedit-gui`.
Vengono compilati e installati insieme.

Due strade: **un pacchetto nativo**, `.deb` o `.rpm`, oppure la **compilazione dal
codice sorgente**, che installa poi dove si vuole.

## I pacchetti

Entrambi sono prodotti da CPack a partire dalle **stesse regole di installazione**:
mettono gli stessi file negli stessi posti, e differiscono soltanto per i
nomi dei pacchetti da cui dipendono — le stesse librerie si chiamano
in modo diverso su Debian e su Fedora.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| Che cosa installa il pacchetto | Dove |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| la voce di menu, l'icona, i metadati per il centro software | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| questo manuale, in Markdown | `/usr/share/subedit/manual` |
| la sua traduzione in ogni lingua installata — catalano, ceco, tedesco, spagnolo, finlandese, francese, galiziano, ebraico, ungherese, interlingue, islandese, italiano, olandese, polacco, portoghese, portoghese brasiliano, russo, serbo, turco, cinese semplificato | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| i modelli di correzione di Gaupol | `/usr/share/subedit/patterns` |
| le pagine di manuale di `subedit-cli`, una per sottocomando, e di `subedit-gui` — vedere [Le pagine di manuale](invocation.md#le-pagine-di-manuale) | `/usr/share/man/man1` |
| le pagine di `subedit-cli` in ogni lingua installata — catalano, ceco, tedesco, spagnolo, finlandese, francese, galiziano, ebraico, ungherese, interlingue, islandese, italiano, olandese, polacco, portoghese, portoghese brasiliano, russo, serbo, turco, cinese semplificato | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

`ffmpeg` è **raccomandato e non obbligatorio** per entrambi i pacchetti: senza di esso, la
finestra smette soltanto di proporre la frequenza dei fotogrammi dichiarata dal film — vedere
[che cosa cambia `ffmpeg`](../subedit-gui/video.md#ffmpeg-non-è-obbligatorio).

### Che cosa i pacchetti richiedono alla distribuzione

**Installare con `apt` o `dnf`, mai con `dpkg -i` o `rpm -i`.** Questi due
comandi depositano il pacchetto senza risolverne le dipendenze: si limitano a
elencare quelle mancanti, e si fermano. Ecco che cosa risponde `rpm -i` su
una Fedora che ha ciò che serve a portata di `dnf`:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` recupera quelle
librerie, dove esistono.

**L'`.rpm` da prendere è quello della propria Fedora**: il nome del file contiene la
versione, `fc42`, `fc43`, `fc44`. Ciascuno è compilato su quella Fedora, con
il suo ICU — `cat /etc/fedora-release` dice quale prendere. Un `.rpm` di un'altra
Fedora richiede un ICU che la propria non ha.

| Dipendenza | `.deb` | `.rpm` | Vincolo |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, per la finestra |
| libmpv | `libmpv2` o `libmpv1` | `mpv-libs` | il binario si collega a `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | il correttore ortografico, e le operazioni `join-words` e `split-words` di [`correct`](correct.md) |
| **ICU** | **`libicu74`** | `libicu`, cioè `libicuuc.so.N` e `libicui18n.so.N` **della Fedora che l'ha compilato** | **la versione che ha la Fedora dell'`.rpm`**, esattamente — vedere più avanti |
| glibc e libstdc++ | — | — | **abbastanza recenti**: quelle di Ubuntu 24.04 per il `.deb`, quelle della Fedora dell'`.rpm` per quest'ultimo |

**ICU è il vincolo che conta, e non si può allentare
cambiando una riga del pacchetto.** I simboli di ICU portano il numero della sua
versione principale (`u_strToUTF8_74`): un binario compilato con ICU 74 non può
collegarsi a ICU 76 o 77. Una dipendenza scritta in modo più ampio farebbe sì che il pacchetto
si installi, per poi fallire all'avvio — dichiararla esatta è ciò che
resta onesto. **Per questo l'`.rpm` non è compilato una volta sola, ma su
ogni Fedora rilasciata**, con il suo ICU; il `.deb` è compilato su Ubuntu 24.04.

| Distribuzione | Che cosa dà |
| :----------- | :---------------- |
| Ubuntu 24.04 | il `.deb` si installa: lì ICU è alla versione 74 |
| Fedora 42, 43, 44 | l'`.rpm` della sua versione (`…fc42…`, `…fc43…`, `…fc44…`) si installa **tramite `dnf`**, compilato con l'ICU di quella Fedora |
| Fedora rawhide (la prossima), le altre distribuzioni `.rpm` | **no**: nessun `.rpm` è compilato per esse — compilare dal codice sorgente |
| qualsiasi distribuzione Debian o Ubuntu il cui ICU non sia alla versione 74 | **no**: il `.deb` richiede `libicu74` — compilare dal codice sorgente |

**La compilazione dal codice sorgente è la soluzione di ripiego, e non ha questo
vincolo**: si collega all'ICU della macchina, qualunque sia la sua versione.
È anche ciò che serve su una distribuzione la cui glibc è più vecchia
di quella dei pacchetti.

### Che cosa è verificato di ciascuno, e che cosa no

**I due non sono verificati fino allo stesso punto, ed è meglio dirlo che lasciar
credere a una parità che non esiste.** Lo sviluppo avviene su Ubuntu; un
`.rpm` compilato lì non vi si può installare. Viene installato altrove: su una
Fedora in un container, una volta alla settimana.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| l'elenco dei file | verificato | verificato, e **confrontato con quello del `.deb`** |
| le dipendenze dichiarate | verificata la presenza | verificata la presenza |
| che i nomi delle dipendenze esistano nella distribuzione | sì, sono quelli di Ubuntu | **sì**, risolti da `dnf` |
| che il pacchetto si installi | no — servono i diritti di amministratore | **sì**, su una Fedora in un container |
| che i binari installati si avviino | sì, da un prefisso temporaneo | **sì**, dal pacchetto installato |

**La verifica su Fedora prova l'`.rpm` compilato su Ubuntu, non quelli che sono
rilasciati.** Quell'immagine non ha un proprio ICU 74: il pacchetto vi si installa perché
Fedora mantiene una `libicu74` di compatibilità, e la verifica resterebbe verde finché
quel pacchetto esiste. Gli `.rpm` rilasciati sono compilati sulla propria Fedora, e non
dipendono più da quella compatibilità. Vedere
[che cosa richiedono i pacchetti](#che-cosa-i-pacchetti-richiedono-alla-distribuzione).

**Il confronto tra i due elenchi è la verifica che conta di più.** I
due pacchetti escono dalla stessa installazione: una differenza tra loro sarebbe un
difetto delle regole di installazione, non del formato.

**Ciò che il `.deb` ancora non prova è che si installi**, e il motivo
non è cambiato: `dpkg -i` richiede i diritti di amministratore, che una porta di
qualità non ha e non deve chiedere. L'`.rpm`, invece, si installa in un
container che non appartiene a nessuno. Vedere
[ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Flatpak e AppImage non sono offerti**, ed è una scelta — non una dimenticanza.
È spiegata nello stesso ADR, ed è **definitiva**: una distribuzione che
non è né della famiglia Debian né della famiglia Fedora compila dal
codice sorgente, che la sezione seguente descrive per intero.

## Compilare dal codice sorgente

### Prerequisiti

| Che cosa serve | Perché | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | la compilazione | `cmake` |
| un compilatore C++23 — GCC 13 va bene | — | `g++` |
| `make`, `pkg-config` | la compilazione | `make`, `pkg-config` |
| CLI11 | la lettura della riga di comando | `libcli11-dev` |
| Qt 6 ≥ 6.4, modulo `Widgets` | la finestra | `qt6-base-dev` |
| `libmpv` | il lettore integrato | `libmpv-dev` |
| ICU | la lettura delle codifiche | `libicu-dev` |
| Enchant 2 | la libreria del correttore ortografico | `libenchant-2-dev` |

**Le ultime quattro sono necessarie anche per compilare il solo
`subedit-cli`**: la configurazione di CMake le cerca per l'intero progetto, e
si ferma se mancano. ICU ed Enchant sono necessarie due volte — l'eseguibile le carica, una
per convertire le codifiche, l'altra per il correttore ortografico — e
sono le uniche librerie di terze parti che carica in fase di esecuzione.

**Una connessione di rete al primo `cmake`**: la libreria di test Catch2
viene scaricata da GitHub durante la configurazione. Non viene scaricata di nuovo
in seguito. Per compilare il solo strumento, senza rete, vedere più avanti.

#### Che cosa non è richiesto

| Che cosa è facoltativo | Che cosa si perde senza di esso |
| :-------------------- | :--------------------- |
| `ffmpeg`, per il suo `ffprobe` | la finestra non propone più la frequenza dei fotogrammi dichiarata dal film — [i dettagli](../subedit-gui/video.md#ffmpeg-non-è-obbligatorio) |
| un server di visualizzazione | nulla per `subedit-cli`; `subedit-gui` ne ha bisogno per mostrarsi |

### Compilazione

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

I binari vengono prodotti in `build/dev/bin/` — `subedit-cli` e
`subedit-gui`.

Per una versione ottimizzata:

```bash
make release
```

Entrambi i binari si trovano allora in `build/release/bin/` — `subedit-cli` e
`subedit-gui`. Il target non compila nient'altro: né il benchmark, né il
banco di prova end-to-end, che hanno i propri target.

**Preferire questo target ai due comandi `cmake` equivalenti**, che non
hanno modo di sapere quanti processi l'ottimizzazione al momento del collegamento può
avviare: a ogni collegamento ne avvia tanti quanti sono i core.
`make release` gli passa `JOBS`, come al resto della compilazione.

### Installazione dei file

```bash
make release
cmake --install build/release --prefix ~/.local
```

Ciò che viene installato, nei percorsi che `GNUInstallDirs` fissa per il prefisso
scelto:

| File | Dove |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| la voce nel menu del desktop | `<prefix>/share/applications` |
| l'icona | `<prefix>/share/icons/hicolor/scalable/apps` |
| i metadati per il centro software | `<prefix>/share/metainfo` |
| questo manuale, in Markdown | `<prefix>/share/subedit/manual` |
| la sua traduzione in ogni lingua installata — catalano, ceco, tedesco, spagnolo, finlandese, francese, galiziano, ebraico, ungherese, interlingue, islandese, italiano, olandese, polacco, portoghese, portoghese brasiliano, russo, serbo, turco, cinese semplificato | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| i modelli di correzione di Gaupol | `<prefix>/share/subedit/patterns` |
| le pagine di manuale di entrambi i binari, una per sottocomando per `subedit-cli` | `<prefix>/share/man/man1` |
| le pagine di `subedit-cli` in ogni lingua installata — catalano, ceco, tedesco, spagnolo, finlandese, francese, galiziano, ebraico, ungherese, interlingue, islandese, italiano, olandese, polacco, portoghese, portoghese brasiliano, russo, serbo, turco, cinese semplificato | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

**Sono gli stessi file che installano i pacchetti**: i pacchetti ne derivano,
anziché essere descritti una seconda volta.

Con `--prefix ~/.local`, entrambi i binari finiscono in `~/.local/bin`,
che si trova nel `PATH` della maggior parte delle distribuzioni. Un prefisso di sistema —
`/usr/local`, per esempio — richiede i diritti corrispondenti.

**Una voce di menu messa sotto `~/.local` non compare sempre
subito**: i desktop rileggono la propria cache con i propri tempi. `subedit-gui` si avvia
da un terminale nel frattempo, come qualsiasi altro comando.

`DESTDIR` è rispettato, come si aspetta chi crea pacchetti: un'installazione
in un'area di staging non tocca nulla al di fuori della directory di staging.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Compilare lo strumento da solo

I test vengono compilati per impostazione predefinita, e sono loro a chiedere la rete —
`make release` non li compila, ma configura il progetto, ed è la
configurazione a scaricare Catch2. Disattivarli una volta basta: l'impostazione
resta nella cache di CMake, e le compilazioni successive la mantengono.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Ciò non esenta nessun collaboratore da nulla: la porta di qualità li
richiede.

### Se la compilazione fallisce al collegamento

Una valanga di "undefined reference to `std::cout`" su codice valido
significa di solito che l'alternativa `c++` del sistema punta a `gcc` anziché
a `g++`: il C++ compila, ma la libreria standard non viene collegata.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

La configurazione di CMake rileva questo caso e si ferma con questo messaggio anziché
lasciare fallire il collegamento.

### Se l'installazione del pacchetto si lamenta di `libicu`

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Tre cause, distinte dal comando digitato e dal nome del file:

- **`rpm -i`**: non risolve nulla. Riprovare con
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **un `.rpm` senza `fcNN` nel nome**, oppure di una Fedora diversa dalla propria:
  richiede l'ICU della macchina che l'ha compilato. Prendere quello il cui `fcNN`
  è l'output di `cat /etc/fedora-release`;
- **nessun `.rpm` per la propria versione**: Fedora rawhide o un'altra distribuzione.
  Nessun pacchetto di questo repository vi si installerà;
  [compilare dal codice sorgente](#compilare-dal-codice-sorgente).
