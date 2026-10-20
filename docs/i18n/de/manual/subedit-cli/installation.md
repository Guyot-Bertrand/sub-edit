# Installation

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

**Diese Seite gilt für beide Programme** — `subedit-cli` und `subedit-gui`.
Sie werden zusammen gebaut und installiert.

Zwei Wege: **ein natives Paket**, `.deb` oder `.rpm`, oder **das Bauen aus dem
Quellcode**, das dann installiert, wohin man will.

## Die Pakete

Beide werden von CPack aus **denselben Installationsregeln** erzeugt:
Sie legen dieselben Dateien an dieselben Orte und unterscheiden sich nur in den
Namen der Pakete, von denen sie abhängen — dieselben Bibliotheken heißen
unter Debian und unter Fedora anders.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| Was das Paket installiert | Wohin |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| den Menüeintrag, das Symbol, die Metadaten für die Softwarezentrale | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| dieses Handbuch, in Markdown | `/usr/share/subedit/manual` |
| seine Übersetzung in jede installierte Sprache — Katalanisch, Tschechisch, Deutsch, Spanisch, Finnisch, Französisch, Galicisch, Hebräisch, Ungarisch, Interlingue, Isländisch, Italienisch, Niederländisch, Polnisch, Portugiesisch, Brasilianisches Portugiesisch | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |
| die Korrekturmuster von Gaupol | `/usr/share/subedit/patterns` |
| die Handbuchseiten von `subedit-cli`, eine pro Unterbefehl, und von `subedit-gui` — siehe [Die Handbuchseiten](invocation.md#die-handbuchseiten) | `/usr/share/man/man1` |
| die Seiten von `subedit-cli` in jeder installierten Sprache — Katalanisch, Tschechisch, Deutsch, Spanisch, Finnisch, Französisch, Galicisch, Hebräisch, Ungarisch, Interlingue, Isländisch, Italienisch, Niederländisch, Polnisch, Portugiesisch, Brasilianisches Portugiesisch | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |

`ffmpeg` wird von beiden Paketen **empfohlen und nicht verlangt**: Ohne es
bietet das Fenster lediglich die Bildrate nicht mehr an, die der Film angibt — siehe
[was `ffmpeg` ändert](../subedit-gui/video.md#ffmpeg-ist-nicht-erforderlich).

### Was die Pakete von der Distribution verlangen

**Installieren Sie mit `apt` oder `dnf`, niemals mit `dpkg -i` oder `rpm -i`.** Diese beiden
Befehle legen das Paket ab, ohne seine Abhängigkeiten aufzulösen: Sie
listen nur die fehlenden auf und hören auf. Das antwortet `rpm -i` auf
einem Fedora, das alles Nötige in Reichweite von `dnf` hat:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` holt diese
Bibliotheken, wo es sie gibt.

**Das zu nehmende `.rpm` ist das Ihres Fedora**: Der Dateiname trägt die
Version, `fc42`, `fc43`, `fc44`. Jedes wird auf diesem Fedora gebaut, gegen
dessen ICU — `cat /etc/fedora-release` sagt, welches Sie nehmen müssen. Ein `.rpm` eines anderen
Fedora verlangt ein ICU, das Ihres nicht hat.

| Abhängigkeit | `.deb` | `.rpm` | Einschränkung |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, für das Fenster |
| libmpv | `libmpv2` oder `libmpv1` | `mpv-libs` | das Binary linkt `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | die Rechtschreibprüfung sowie die Aufgaben `join-words` und `split-words` von [`correct`](correct.md) |
| **ICU** | **`libicu74`** | `libicu`, das heißt `libicuuc.so.N` und `libicui18n.so.N` **des Fedora, das es gebaut hat** | **die Version, die das Fedora des `.rpm` hat**, genau — siehe unten |
| glibc und libstdc++ | — | — | **aktuell genug**: die von Ubuntu 24.04 für das `.deb`, die des Fedora des `.rpm` für dieses |

**ICU ist die Einschränkung, auf die es ankommt, und sie lässt sich nicht lockern, indem
man eine Zeile des Pakets ändert.** Die Symbole von ICU tragen die Nummer seiner
Hauptversion (`u_strToUTF8_74`): Ein gegen ICU 74 gebautes Binary kann sich nicht
gegen ICU 76 oder 77 linken. Eine weiter gefasste Abhängigkeit ließe das Paket
installieren und dann beim Start scheitern — sie exakt anzugeben ist das, was
ehrlich bleibt. **Deshalb wird das `.rpm` nicht einmal gebaut, sondern auf
jedem veröffentlichten Fedora**, gegen dessen ICU; das `.deb` wird auf Ubuntu 24.04 gebaut.

| Distribution | Was es ergibt |
| :----------- | :---------------- |
| Ubuntu 24.04 | das `.deb` installiert sich: ICU liegt dort in Version 74 vor |
| Fedora 42, 43, 44 | das `.rpm` seiner Version (`…fc42…`, `…fc43…`, `…fc44…`) installiert sich **über `dnf`**, gebaut gegen das ICU dieses Fedora |
| Fedora rawhide (das nächste), die anderen `.rpm`-Distributionen | **nein**: Für sie wird kein `.rpm` gebaut — bauen Sie aus dem Quellcode |
| jede Debian- oder Ubuntu-Distribution, deren ICU nicht in Version 74 vorliegt | **nein**: Das `.deb` verlangt `libicu74` — bauen Sie aus dem Quellcode |

**Das Bauen aus dem Quellcode ist der Ausweg, und es hat diese
Einschränkung nicht**: Es linkt gegen das ICU des Rechners, welche Version es auch hat.
Es ist auch das, was auf einer Distribution nötig ist, deren glibc älter
ist als die der Pakete.

### Was an jedem getestet wird und was nicht

**Die beiden werden nicht gleich weit geprüft, und es ist besser, das zu sagen, als
an eine Gleichwertigkeit glauben zu lassen, die es nicht gibt.** Die Entwicklung findet auf Ubuntu statt; ein
dort gebautes `.rpm` lässt sich dort nicht installieren. Es wird anderswo installiert: auf einem
Fedora in einem Container, einmal pro Woche.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| die Liste der Dateien | geprüft | geprüft und **mit der des `.deb` verglichen** |
| die angegebenen Abhängigkeiten | auf Vorhandensein geprüft | auf Vorhandensein geprüft |
| dass die Namen der Abhängigkeiten in der Distribution existieren | ja, es sind die von Ubuntu | **ja**, von `dnf` aufgelöst |
| dass sich das Paket installieren lässt | nein — dafür sind Administratorrechte nötig | **ja**, auf einem Fedora in einem Container |
| dass die installierten Binaries starten | ja, aus einem temporären Präfix | **ja**, aus dem installierten Paket |

**Die Fedora-Prüfung testet das auf Ubuntu gebaute `.rpm`, nicht die
veröffentlichten.** Dieses Image hat kein eigenes ICU 74: Das Paket installiert sich dort, weil
Fedora ein Kompatibilitätspaket `libicu74` behält, und die Prüfung bliebe grün, solange
dieses Paket existiert. Die veröffentlichten `.rpm`-Dateien werden auf ihrem eigenen Fedora gebaut und hängen
nicht mehr von dieser Kompatibilität ab. Siehe
[was die Pakete verlangen](#was-die-pakete-von-der-distribution-verlangen).

**Der Vergleich der beiden Listen ist die Prüfung, auf die es am meisten ankommt.** Die
beiden Pakete gehen aus derselben Installation hervor: Ein Unterschied zwischen ihnen wäre ein
Fehler der Installationsregeln, nicht des Formats.

**Was das `.deb` noch immer nicht beweist, ist, dass es sich installieren lässt**, und der Grund
hat sich nicht geändert: `dpkg -i` braucht Administratorrechte, die eine Qualitätsschranke
nicht hat und nicht verlangen darf. Das `.rpm` seinerseits installiert sich in einem
Container, der niemandem gehört. Siehe
[ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Flatpak und AppImage werden nicht angeboten**, und das ist eine Entscheidung — kein Versehen.
Sie wird in derselben ADR begründet, und sie ist **endgültig**: Eine Distribution, die
weder zur Debian- noch zur Fedora-Familie gehört, baut aus dem
Quellcode, was der nächste Abschnitt vollständig beschreibt.

## Aus dem Quellcode bauen

### Voraussetzungen

| Was nötig ist | Warum | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | der Build | `cmake` |
| ein C++23-Compiler — GCC 13 genügt | — | `g++` |
| `make`, `pkg-config` | der Build | `make`, `pkg-config` |
| CLI11 | das Lesen der Befehlszeile | `libcli11-dev` |
| Qt 6 ≥ 6.4, Modul `Widgets` | das Fenster | `qt6-base-dev` |
| `libmpv` | der eingebaute Player | `libmpv-dev` |
| ICU | das Lesen der Kodierungen | `libicu-dev` |
| Enchant 2 | die Bibliothek der Rechtschreibprüfung | `libenchant-2-dev` |

**Die letzten vier sind selbst dann erforderlich, wenn nur
`subedit-cli` gebaut wird**: Die CMake-Konfiguration sucht sie für das ganze Projekt und
bricht ab, wenn sie fehlen. ICU und Enchant sind doppelt erforderlich — das Programm lädt sie, das eine,
um die Kodierungen umzuwandeln, das andere für die Rechtschreibprüfung —, und
sie sind die einzigen Bibliotheken von Drittanbietern, die es zur Laufzeit lädt.

**Eine Netzwerkverbindung beim ersten `cmake`**: Die Testbibliothek Catch2
wird bei der Konfiguration von GitHub geholt. Sie wird danach nicht erneut
heruntergeladen. Um das Werkzeug allein ohne Netzwerk zu bauen, siehe unten.

#### Was nicht erforderlich ist

| Was optional ist | Was ohne es verloren geht |
| :-------------------- | :--------------------- |
| `ffmpeg`, wegen seines `ffprobe` | das Fenster bietet die Bildrate, die der Film angibt, nicht mehr an — [die Einzelheiten](../subedit-gui/video.md#ffmpeg-ist-nicht-erforderlich) |
| ein Displayserver | nichts für `subedit-cli`; `subedit-gui` braucht einen, um sich anzuzeigen |

### Bauen

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

Die Binaries entstehen in `build/dev/bin/` — `subedit-cli` und
`subedit-gui`.

Für eine optimierte Version:

```bash
make release
```

Beide Binaries liegen dann in `build/release/bin/` — `subedit-cli` und
`subedit-gui`. Das Ziel baut nichts anderes: weder den Benchmark noch die
End-to-End-Testumgebung, die ihre eigenen Ziele haben.

**Ziehen Sie dieses Ziel den beiden gleichwertigen `cmake`-Befehlen vor**, die
nicht wissen können, wie viele Prozesse die Optimierung beim Linken
starten darf: Bei jedem Linken startet sie so viele, wie es Kerne gibt.
`make release` übergibt ihr `JOBS`, wie dem Rest des Builds.

### Installieren

```bash
make release
cmake --install build/release --prefix ~/.local
```

Was installiert wird, unter den Pfaden, die `GNUInstallDirs` für das gewählte
Präfix festlegt:

| Datei | Wohin |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| der Menüeintrag der Arbeitsumgebung | `<prefix>/share/applications` |
| das Symbol | `<prefix>/share/icons/hicolor/scalable/apps` |
| die Metadaten für die Softwarezentrale | `<prefix>/share/metainfo` |
| dieses Handbuch, in Markdown | `<prefix>/share/subedit/manual` |
| seine Übersetzung in jede installierte Sprache — Katalanisch, Tschechisch, Deutsch, Spanisch, Finnisch, Französisch, Galicisch, Hebräisch, Ungarisch, Interlingue, Isländisch, Italienisch, Niederländisch, Polnisch, Portugiesisch, Brasilianisches Portugiesisch | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |
| die Korrekturmuster von Gaupol | `<prefix>/share/subedit/patterns` |
| die Handbuchseiten beider Binaries, eine pro Unterbefehl für `subedit-cli` | `<prefix>/share/man/man1` |
| die Seiten von `subedit-cli` in jeder installierten Sprache — Katalanisch, Tschechisch, Deutsch, Spanisch, Finnisch, Französisch, Galicisch, Hebräisch, Ungarisch, Interlingue, Isländisch, Italienisch, Niederländisch, Polnisch, Portugiesisch, Brasilianisches Portugiesisch | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR` |

**Es sind dieselben Dateien, die die Pakete installieren**: Die Pakete gehen aus ihnen hervor,
statt ein zweites Mal beschrieben zu werden.

Mit `--prefix ~/.local` landen beide Binaries in `~/.local/bin`,
das bei den meisten Distributionen im `PATH` liegt. Ein Systempräfix —
zum Beispiel `/usr/local` — braucht die entsprechenden Rechte.

**Ein unter `~/.local` abgelegter Menüeintrag erscheint nicht immer
sofort**: Die Arbeitsumgebungen lesen ihren Zwischenspeicher in ihrem eigenen Tempo neu ein. `subedit-gui` startet
in der Zwischenzeit aus einem Terminal, wie jeder andere Befehl.

`DESTDIR` wird beachtet, was ein Paketbetreuer erwartet: Eine Installation in ein Staging-Verzeichnis
berührt nichts außerhalb dieses Verzeichnisses.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Das Werkzeug allein bauen

Die Tests werden standardmäßig gebaut, und sie sind es, die das Netzwerk verlangen —
`make release` baut sie nicht, aber es konfiguriert das Projekt, und es ist die
Konfiguration, die Catch2 holt. Es genügt, sie einmal abzuschalten: Die
Einstellung bleibt im CMake-Cache, und die folgenden Builds behalten sie bei.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Das befreit einen Mitwirkenden von nichts: Die Qualitätsschranke
verlangt sie sehr wohl.

### Wenn der Build beim Linken scheitert

Eine Lawine von „undefined reference to `std::cout`“ bei gültigem Code
bedeutet meist, dass die Alternative `c++` des Systems auf `gcc` statt
auf `g++` zeigt: Das C++ kompiliert, aber die Standardbibliothek wird nicht gelinkt.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

Die CMake-Konfiguration erkennt diesen Fall und bricht mit dieser Meldung ab, statt
das Linken scheitern zu lassen.

### Wenn die Installation des Pakets sich über `libicu` beschwert

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Drei Ursachen, zu unterscheiden am eingegebenen Befehl und am Dateinamen:

- **`rpm -i`**: Es löst nichts auf. Versuchen Sie es erneut mit
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **ein `.rpm` ohne `fcNN` im Namen** oder von einem anderen Fedora als Ihrem:
  Es verlangt das ICU des Rechners, der es gebaut hat. Nehmen Sie das, dessen `fcNN`
  die Ausgabe von `cat /etc/fedora-release` ist;
- **kein `.rpm` für Ihre Version**: Fedora rawhide oder eine andere Distribution.
  Kein Paket dieses Repositorys lässt sich dort installieren;
  [bauen Sie aus dem Quellcode](#aus-dem-quellcode-bauen).
