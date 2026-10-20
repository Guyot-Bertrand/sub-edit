# `disjoint`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Skerðir skjátexta sem skarast þannig að **ekkert `[start, end]` tímabil skarist við annað**. Þetta er `Make Ranges Disjoint…` í glugganum — [sama vélin](../subedit-gui/lines.md#að-losa-tímabilin-við-skörun).

**Ekkert er stytt og ekkert lengt**: á hverju augnabliki sýnir skráin sömu texta og áður. Það sem breytist er
hvernig tímalínan er skorin:

- hún er skorin við **hvert upphaf og hvern endi** skjátextanna í hópi sem skarast;
- bil sem **einn** skjátexti þekur er áfram sá skjátexti;
- bil sem **nokkrir** þekja verður að **einum** skjátexta þar sem línurnar eru þeirra, **í
  röð upphafs þeirra** — sá fyrsti í línu 1, sá næsti í línu 2;
- skjátexti sem skarast ekki við neinn er látinn í friði og röð hinna haldst.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Röksemdir og valkostir

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :----- | :------ | :----- |
| `<file>...` | já | ein eða fleiri slóðir | — |
| `--minimum` | nei | tímalengd: `0.08`, `1.5`, `00:00:00,080`. Núll heldur öllum bilum | `0.08` (um tveir rammar) |
| `--max-lines` | nei | heil tala: hámarksfjöldi lína í sameinuðum skjátexta; `0` fellir mörkin niður | `2` |
| `--dashes` | nei | fáni: þankastrik framan við textann sem hver skjátexti leggur til sameinaðs skjátexta | slökkt |
| `--sort` | nei | fáni: raðar skjátextunum **fyrst** í röð upphafs þeirra — sjá [Skrá í réttri röð er nauðsynleg](#skrá-í-réttri-röð-er-nauðsynleg) | slökkt |
| `--recursive`, `-r` | nei | fáni | slökkt — sjá [Vinnsla á möpputré](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | fáni: reiknar og tilkynnir, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

Tímabilin `A [0, 5]` og `B [3, 8]` gefa þrjá skjátexta án skörunar: `A` einan, `A` og `B` saman, `B` einan. Ekkert
augnablik tapast og engu er bætt við.

## Hvað aðgerðin ákveður

**Lína er aldrei tvöfölduð.** Tveir skjátextar með sama texta gefa eina línu þar sem þeir
skarast, og tveir nálægir hlutar sem sýna það sama verða aðeins einn skjátexti. Texti
skjátexta sem er skorinn í marga hluta er að öðru leyti **endurtekinn í hverjum þeirra**: það er það sem er á skjánum á
því augnabliki.

**Mjög stutt bil er tekið yfir af nágranna.** Undir `--minimum` hverfur bil: nágranninn
**lengir tímabil sitt þannig að það nái yfir það**, svo að ekkert augnablik verði autt og ekkert færist um meira en þau
mörk. Nágranninn sem er valinn er sá sem línur bilsins **eru þegar í**, annars
sá lengsti. Texti bilsins tapast aðeins ef enginn nágranni ber hann, og þá í skemmri tíma en mörkin segja.
**Heill skjátexti sem er styttri en mörkin er aldrei tekinn yfir**: hann er látinn vera og talinn.

**Fleiri línur en mörkin leyfa: gluggar.** Sameinaður skjátexti sem fengi fleiri en `--max-lines` línur
er sýndur **í röð glugga** af þeirri stærð, í jöfnum hlutum tímalengdarinnar. Þrjár línur á þremur
sekúndum:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Fjórar línur gefa þrjá glugga og svo framvegis: glugginn rennur eina línu í einu. Engu er
sleppt og ekkert skorið aftur.

**Þýðingin fylgir sömu skurðum**, því að skjátexti ber báða textana. **Það sem sniðið ber að auki**
— stíll, staðsetning — er frá fyrsta skjátextanum sem þekur bilið; tapið er talið þegar
sameinuðu skjátextarnir eru ólíkir.

**Skjátexti sem endar áður en hann byrjar** er ekki giskað á: hann er látinn vera þar sem hann er og talinn.

## Skrá í réttri röð er nauðsynleg

Hóparnir eru runur nágranna: ef röðin er röng eru nágrannarnir ekki þeir sem skarast. Skrá
þar sem upphöfin eru ekki í réttri röð er því **hafnað** og skilaboðin segja það:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` raðar fyrst — eins og [`sort`](sort.md) — og segir frá því. Kjarninn raðar aldrei að eigin frumkvæði.

Önnur umferð breytir engu: þegar tímabilin eru laus við skörun er ekkert eftir að skera og
setningin er `already disjoint`.

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem var skrifuð.

| Stig | Það sem bætist við á villuúttakinu |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, og á eftir `;` það sem ekki tókst að gera að fullu: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — eða `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, síðan **öll greiningarboð lesturs** |

`N subtitles split` telur upprunalegu skjátextana sem voru skornir í marga hluta, `M merged` hlutana sem
sýna marga skjátexta saman.

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) eru `counts` hverrar skrár:

| Talning | Hvað hún segir |
| :----- | :----------- |
| `subtitles` | skjátextar skrárinnar sem var skrifuð |
| `split`, `merged` | eins og í setningunni |
| `windowed` | hlutarnir sem eru sýndir í gluggum |
| `absorbed` | bilin sem nágranni tók yfir |
| `too_short` | skjátextarnir sem eru styttri en mörkin, látnir heilir |
| `ending_before_start` | skjátextarnir sem enda áður en þeir byrja, látnir í friði |
| `extras_dropped` | sameinuðu hlutarnir þar sem stíll eða staðsetning er ólík |
| `moved` | aðeins með `--sort`: staðirnir sem breyttust |

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef hver skrá var unnin — skrá án skörunar telst unnin —, `2` ef
engin, `3` ef aðeins sumar, `1` við notkunarvillu.

## Villur

| Hvað kallar þær fram | Skilaboð |
| :------------------- | :------ |
| skrá í rangri röð án `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` er ekki tímalengd, eða er neikvætt | `--minimum: …` |
| `--max-lines` er ekki heil tala | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Skrá sem ekki er hægt að lesa bilar eins og hjá hverri undirskipun; villur áfangastaðarins eru sameiginlegar
undirskipunum sem skrifa: sjá [Ræsing](invocation.md#áfangastaðurinn).

## Í hinum skipununum

[`convert`](convert.md#að-losa-tímabilin-við-skörun), [`append`](append.md#að-losa-tímabilin-við-skörun) og
[`split-file`](split-file.md#að-losa-tímabilin-við-skörun) taka við `--disjoint`, sem gerir það sama með
sjálfgefnum stillingum, áður en skrifað er.

Á illa skiptri skrá — texta sem er skorinn í hluta sem fylgja hver öðrum, gefnir í rangri röð —, gefur röðun og skurður
ein og sér línur sem flökta frá einum hluta til þess næsta: fyrst [`join-repeats`](join-repeats.md), eða
[`repair`](repair.md), sem keðjar öll þrjú.
