# `italics`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

A szövegeket **dőlt betűssé teszi**, vagy **kiveszi belőlük a dőlt betűt**, a fájl formátumának címkéivel. Ez az [ablak](../subedit-gui/operations.md) `Italic` művelete.

**Két szó, és nem átkapcsoló**: az ablak gombja megkérdezi a magot, melyik irányba játszik; itt ezt ki kell mondani. Az `--on` és az `--off` kizárja egymást, és az egyik kell — ha egyik sincs megadva, vagy mindkettő, az használati hiba (`1` kód).

- **Az `--on`** minden szöveget egyetlen címkepárba foglal, **a benne lévők eltávolítása után**: a már dőlt szöveg nem kerül kétszer címkébe, és a kettévágott dőlt részek egyben készülnek újra. Több soron át egy pár fedi az egészet.
- **Az `--off`** eltávolítja a dőlt címkéket; **a címke többi része megmarad** (a `{\b1\i1}` a `{\b1}` címkét adja).
- A szöveg **úgy íródik, ahogy a fájl formátuma a dőlt betűt írja**: `<i>…</i>` a SubRip, a WebVTT és a SubViewer 2 esetén, `{\i1}…{\i0}` a két Sub Station Alpha esetén, `{Y:i}` a MicroDVD esetén.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
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
| `<file>...` | igen | egy vagy több elérési út | — |
| `--recursive`, `-r` | nem | kapcsoló érték nélkül | ki — lásd [Könyvtárfa feldolgozása](lots.md) |
| `--on` / `--off` | **a kettő közül egy** | kapcsolók érték nélkül | — |
| `--range` | nem | `N-M`, vagy `N-` a végéig — lásd [`adjust`](adjust.md#--range) | az egész fájl |
| `--document` | nem | `main` vagy `translation` — lásd [Fordítás](invocation.md#fordítás) | `main` |
| `-t`, `--translation-file` | a `--document translation` kapcsolóval | fordításfájl, **egyetlen** bemenethez | — |
| `--align-method` | nem, és csak a `-t` kapcsolóval | `position` or `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **a három közül egy**, a `--dry-run` kapcsoló kivételével | lásd [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | kapcsoló érték nélkül: kiszámolja és közli, nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

**Alapértelmezés szerint a fő szöveg; a fordítás a `--document translation -t FILE` kapcsolókkal.** Ilyenkor a fordítás módosul és íródik meg, és csakis az — lásd [Fordítás](invocation.md#fordítás).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## A stílus nélküli formátum elutasít

**A TMPlayer és az LRC nem ír stílust**: nincs mit dőltté tenni, és nincs mit kivenni. Az ablak szürkévé teszi a menüpontot, ahelyett hogy elrejtené; a parancssor **fájlonként elutasít, az ok megjelölésével**, és a köteg többi fájlja folytatódik. Az számít, amit a formátum hordozni tud, nem az, ahogyan írja: a SubRip és a WebVTT ugyanúgy írja a dőlt betűt, és a színről nem ugyanazt mondják.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

JSON-ban a hiba azonosítója **`no-style`**.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a megírt fájl. A `--dry-run` kapcsolóval a módosítások listája, amelyet a [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) ír le.

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, vagy `N subtitles taken out of italics`, vagy `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — az olvasott fájl formátuma, kódolása, jelölője és sorvége, úgy visszaállítva, ahogy volt |
| 3 | `<path>: N bytes read, M written`, majd **minden olvasási diagnosztika** — lásd [Indítás](invocation.md#olvasási-diagnosztika) |

Az `N` azoknak a feliratsoroknak a száma, **amelyeknek a szövege megváltozott**, magából a műveletből kiolvasva.

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) kapcsolóval az egyes fájlok objektuma tartalmazza a `counts.changed` mezőt — a közlemény `N` értékét — és a `changes` mezőt, a módosított feliratsorok listáját (`{"subtitle", "document", "before", "after"}`, **üres**, ha semmi sem változott).

## Kilépési kódok

Az eszköz kódjai: `0`, ha minden fájl feldolgozása megtörtént — az a fájl is feldolgozottnak számít, amelyben semmi sem változik —, `2`, ha egyé sem, `3`, ha csak egyeseké, `1` használati hiba esetén.

## Hibák

| Mi váltja ki | Üzenet |
| :------------------ | :------ |
| sem az `--on`, sem az `--off` | `italics needs --on to put the texts in italics, or --off` |
| mindkettő | `--on and --off say opposite things; give one of them` |
| stílus nélküli formátum | `<path>: TMPlayer writes no style: there are no italics to put on` (vagy `take out`) |
| rosszul megírt vagy a fájlon kívüli tartomány | lásd [`adjust`](adjust.md#--range) |

A cél hibái közösek az író alparancsokban: lásd [Indítás](invocation.md#a-cél).
