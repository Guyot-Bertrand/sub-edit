# `case`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```
A szövegeket **címszerű írásra, mondatszerű írásra, nagybetűsre vagy kisbetűsre** állítja, egyetlen
címkét sem érintve. Az [ablak](../subedit-gui/operations.md) `Eszközök ▸ Kis- és nagybetű` menüje, a saját szabályaival.

| `--to` | Hatás | A `bonjour marie` ebből lesz |
| :----- | :---- | :---------------------- |
| `title` | minden szó nagybetűt kap, a többi kisbetűs lesz | `Bonjour Marie` |
| `sentence` | a szöveg első betűje, és csak az; a többi kisbetűs lesz | `Bonjour marie` |
| `upper` | minden nagybetűs | `BONJOUR MARIE` |
| `lower` | minden kisbetűs | `bonjour marie` |

- **Ami az első betű vagy az első számjegy előtt áll, ahhoz nem nyúl a program**: a párbeszéd-kötőjel,
  a nyitó idézőjel vagy a zárójel megmarad (a `- bonjour` eredménye `- Bonjour`).
- **A címkék nem szöveg**: nem változtatják a kis- és nagybetűt — a `<I>` marad `<I>` —, és nem
  változtatnak helyet: az a címke, amely kettévág egy szót, utána is kettévágja.
- **Az ICU dönt arról, hol kezdődik a szó**: a `l'été sera chaud` eredménye `L'été Sera Chaud`, az ékezetek
  megmaradnak (az `éléonore` eredménye `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```
## Argumentumok és kapcsolók

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :----- | :------ | :----- |
| `<file>...` | igen | egy vagy több útvonal | — |
| `--recursive`, `-r` | nem | jelző | ki — lásd: [Könyvtárfa feldolgozása](lots.md) |
| `--to` | **igen** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | nem | `N-M`, vagy `N-` a végéig — lásd: [`adjust`](adjust.md#--range) | az egész fájl |
| `--document` | nem | `main` vagy `translation` — lásd: [Fordítás](invocation.md#fordítás) | `main` |
| `-t`, `--translation-file` | a `--document translation` esetén | fordításfájl, **egyetlen** bemenethez | — |
| `--align-method` | nem, és csak a `-t` mellett | `position` vagy `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **a háromból egy**, kivéve a `--dry-run` esetén | lásd: [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | jelző: kiszámít és közöl, nem ír — lásd: [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

**Alapértelmezés szerint a fő szöveg; a fordítás a `--document translation -t FILE` megadással.** Ilyenkor
a fordítás módosul és íródik ki, és csak az — lásd: [Fordítás](invocation.md#fordítás).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```
Az a fájl, amelyben **egyetlen szöveg sem változik** — minden már a kért kis- és nagybetűs alakban van —, változatlanul íródik ki, és
ezt jelzi is:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```
## Kimenet

**Szabványos kimenet** — semmi: az eredmény a kiírt fájl. A `--dry-run` esetén a módosítások
listája, amelyet a [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) ír le.

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, vagy `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — a beolvasott fájl formátuma, kódolása, jelölője és sorvégei, az eredetiek szerint visszaírva |
| 3 | `<path>: N bytes read, M written`, majd **minden olvasási diagnosztika** — lásd: [Indítás](invocation.md#olvasási-diagnosztika) |

Az `N` azoknak a feliratsoroknak a száma, **amelyeknek a szövege megváltozott**, magából a műveletből kiolvasva.

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) esetén minden fájl objektuma tartalmazza a
`counts.changed` mezőt — a mondatbeli `N` értékét — és a `changes` mezőt, a módosított feliratsorok listáját
(`{"subtitle", "document", "before", "after"}`, **üres**, ha semmi sem változott).

## Kilépési kódok

Az eszköz általános kódjai: `0`, ha minden fájl feldolgozása megtörtént — az a fájl is annak számít, amelyben semmi sem változik —,
`2`, ha egyiké sem, `3`, ha csak néhányé, `1` használati hiba esetén.

## Hibák

| Mi váltja ki | Üzenet |
| :------------------ | :------ |
| hiányzó `--to` | `--to is required` |
| a négy közül egyik sem érték | `--to: … not in {title,sentence,upper,lower}` |
| rosszul írt vagy a fájlon kívüli tartomány | lásd: [`adjust`](adjust.md#--range) |

A cél hibái közösek az író alparancsokban:
lásd: [Indítás](invocation.md#a-cél).
