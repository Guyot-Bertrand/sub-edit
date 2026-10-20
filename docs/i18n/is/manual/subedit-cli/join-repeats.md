# `join-repeats`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Gerir **einn skjátexta** úr skjátextum sem sýna það sama hvern á eftir öðrum: texta sem er skorinn í hluta
sem fylgja hver öðrum, eins og sum skiptingartól skilja hann eftir. Þetta er `Join Repeated Subtitles` í glugganum —
[sama vélin](../subedit-gui/lines.md#að-sameina-endurtekna-skjátexta).

**Endurtekning** er skjátexti sem hefur **sama texta, sömu þýðingu og sömu sniðstillingar** (stíl,
staðsetningu, stillingar skjátexta) og annar, **og byrjar í síðasta lagi þegar sá endar**: þeir
snertast eða skarast. Röð endurtekninga verður að **þeim sem byrjar fyrst**, og endir hans
færist á síðasta endi raðarinnar; hinir hverfa.

- **Röð skrárinnar skiptir engu og er ekki breytt**: skjátextinn sem er haldið er sá sem byrjar fyrst,
  hvar sem hinir eru. Skrá í réttri röð helst í réttri röð.
- **Ekkert er sameinað fyrir mistök**: annar texti, önnur þýðing, aðrar stillingar, bil milli tveggja
  hluta, skjátexti án texta eða sem endar áður en hann byrjar — í öllum þessum tilvikum haldast þeir aðskildir.
- **Á að gera á undan öðru**: röðun og [að skera skörunina af](disjoint.md) lesa hreina röð betur.
  [`repair`](repair.md) keðjar öll þrjú og [`convert`](convert.md#að-losa-tímabilin-við-skörun) tekur við því sem valkosti (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Röksemdir og valkostir

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :----- | :------ | :----- |
| `<file>...` | já | ein eða fleiri slóðir | — |
| `--recursive`, `-r` | nei | fáni | slökkt — sjá [Vinnsla á möpputré](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | fáni: reiknar og tilkynnir, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

Hlutarnir þrír í `Alpha`, sem fylgja hver öðrum frá 1 s til 6 s, mynda nú aðeins einn. `Beta`, aðskilinn með bili,
helst. Önnur umferð breytir engu og segir það: `no repeated subtitles`.

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem var skrifuð.

| Stig | Það sem bætist við á villuúttakinu |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, eða `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, síðan **öll greiningarboð lesturs** |

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) eru `counts` hverrar skrár: `subtitles` (skjátextar skrárinnar
sem var skrifuð), `joined` (skjátextarnir sem voru fjarlægðir vegna þess að annar þekur þá) og `chains` (raðirnar sem voru sameinaðar).

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef hver skrá var unnin — skrá án endurtekninga telst unnin —, `2` ef engin, `3` ef
aðeins sumar, `1` við notkunarvillu.

## Villur

Engin sérstök fyrir `join-repeats`: skrá sem ekki er hægt að lesa bilar eins og hjá hverri undirskipun og villur
áfangastaðarins eru sameiginlegar undirskipunum sem skrifa: sjá [Ræsing](invocation.md#áfangastaðurinn).
