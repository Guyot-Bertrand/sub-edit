# `dialogue-dashes`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```
**Párbeszéd-kötőjelet** tesz a sorok elejére, vagy leveszi őket. Az [ablak](../subedit-gui/operations.md)
`Dialogue Dashes` művelete.

**Két kapcsoló, mint az [`italics`](italics.md) esetében**: az `--add` és a `--remove` kizárja egymást, és az egyik kötelező.

- **Az `--add`** kötőjelet tesz minden olyan sor elé, amelyben nincs: a `Bonjour` eredménye `- Bonjour`, a
  `Bonjour\nMarie` eredménye `- Bonjour\n- Marie`. **A már meglévő kötőjel nem kettőződik meg.**
- **Három kötőjelet ismer fel a program** — a billentyűzet kötőjelét és a tipográfus két hosszú gondolatjelét —,
  **és a kötőjel íródik ki**: a `— Bonjour` eredménye `- Bonjour`. A kötőjel utáni
  felesleges szóközök eltűnnek.
- A kötőjel **a szöveg elé kerül, nem a nyitó címke elé**: a `<i>Bonjour</i>` eredménye
  `<i>- Bonjour</i>`. Egyetlen címke sem mozdul.
- **A `--remove`** leveszi a kezdő kötőjeleket, mindhárom alakban.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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
| `--add` / `--remove` | **a kettő közül egy** | jelzők | — |
| `--range` | nem | `N-M`, vagy `N-` a végéig — lásd: [`adjust`](adjust.md#--range) | az egész fájl |
| `--document` | nem | `main` vagy `translation` — lásd: [Fordítás](invocation.md#fordítás) | `main` |
| `-t`, `--translation-file` | a `--document translation` esetén | fordításfájl, **egyetlen** bemenethez | — |
| `--align-method` | nem, és csak a `-t` mellett | `position` vagy `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **a háromból egy**, kivéve a `--dry-run` esetén | lásd: [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | jelző: kiszámít és közöl, nem ír — lásd: [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

**Alapértelmezés szerint a fő szöveg; a fordítás a `--document translation -t FILE` megadással.** Ilyenkor
a fordítás módosul és íródik ki, és csak az — lásd: [Fordítás](invocation.md#fordítás).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```
## Kimenet

**Szabványos kimenet** — semmi: az eredmény a kiírt fájl. A `--dry-run` esetén a módosítások
listája, amelyet a [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) ír le.

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, vagy `N subtitles undashed`, vagy `nothing to change` |
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
| sem az `--add`, sem a `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| mindkettő | `--add and --remove say opposite things; give one of them` |
| rosszul írt vagy a fájlon kívüli tartomány | lásd: [`adjust`](adjust.md#--range) |

A cél hibái közösek az író alparancsokban:
lásd: [Indítás](invocation.md#a-cél).
