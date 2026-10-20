# `split-file`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Sker skrá **í tvennt**: haus og hala. Þetta er `Split Project…` í glugganum — [sama vélin](../subedit-gui/operations.md#split-project),
sömu talningar, sama höfnunin — og **nákvæm andhverfa [`append`](append.md)**: að bæta halanum aftan við
hausinn gefur upphafsskrána aftur.

- **`--at N` er númer fyrsta skjátexta halans**, númerað eins og tafla gluggans
  númerar það: frá 1. `--at 3` setur skjátexta 1 og 2 í hausinn, og þann þriðja og afganginn í
  halann. Það gengur **frá öðrum til þess síðasta**: skurður þarf skjátexta sitt hvorum megin.
- **Halinn er fluttur aftur að upphafspunkti**: hann er færður um mínus endi síðasta skjátextans sem er eftir í hausnum,
  og staðsetningar hans halda röð sinni og bili. Það er það sem gerir `split-file` að andhverfu `append`.
- **Báðir helmingarnir halda sniði, stafakóðun, bætaröðunarmerki og línulokum skrárinnar sem var lesin**, og
  eru endurnúmeraðir frá 1 við ritun, eins og hver skrá sem er skrifuð aftur.
- **Tvö úttök, svo sérstakur áfangastaður**: `--head` og `--tail`, **hvort tveggja nauðsynlegt**. `--output`,
  `--output-dir`, `--in-place` og `--recursive` eru ekki til hjá `split-file` og er hafnað eins og hverjum óþekktum
  valkosti, kóði `1`: hér er hvorki mappa til að deila tveimur nefndum skrám í né inntak til að skrifa yfir.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Röksemdir og valkostir

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :----- | :------ | :----- |
| `<file>` | já | **ein** slóð | — |
| `--at` | já | heiltala, frá 2 upp í fjölda skjátexta skrárinnar | — |
| `--head`, `--tail` | **já, bæði**, nema með `--dry-run` | skráarslóðir; mappa þeirra er búin til ef hana vantar | — |
| `--dry-run` | nei | rofi: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |
| `--sort` | nei | rofi: raðar skjátextunum eftir upphafi þeirra **áður en skorið er**; `--at` telur í þeirri röð | slökkt |
| `--disjoint` | nei | rofi: sker skjátexta sem skarast **áður en skorið er**; `--at` telur í því sem eftir er, sjá [Að losa tímabilin við skörun](#að-losa-tímabilin-við-skörun) | slökkt |

**Með `--dry-run` er enginn áfangastaður nauðsynlegur**: engan má gefa, en ekki bara annan af tveimur —
`--head` og `--tail` fara saman. Þeir sem eru gefnir eru metnir eins og án `--dry-run`.

**Áfangastaðirnir tveir mega hvorki vera inntakið né sama skráin**: að skrifa yfir það sem verið er að
lesa, eða að skrifa halann yfir hausinn, er hafnað áður en nokkuð er lesið, kóði `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

Annar skjátextinn, sem er eftir í hausnum, endar á 4 s: halinn færist aftur um 4 s, og þriðji skjátextinn,
sem byrjaði á 6 s, byrjar á 2 s.

## Skurður í tímaröð

`--at` númerar skjátextana **eins og skráin gefur þá**. Á skrá þar sem röðin er brotin er hausinn því ekki helmingurinn sem er fyrr
í tíma. `--sort` raðar skjátextunum eftir upphöfum þeirra **áður en skurðurinn er metinn** — stöðugt, eins og [`sort`](sort.md) gerir —, og
`--at` telur þá í þeirri röð.

Aldrei sjálfgefið. Með valkostinum segir skráin á fyrsta stigi frásagnar hversu margir skjátextar færðust (`<path>: N subtitles moved`
eða `<path>: already in order`), og færslan fyrir `--format json` ber talninguna `moved`, **fjarverandi án valkostsins**.

## Að losa tímabilin við skörun

Aldrei sjálfgefið. Valkosturinn gerir það sem [`disjoint`](disjoint.md) gerir, **með sjálfgefnum stillingum þess**, áður en skurðurinn er metinn, á eftir röðuninni: `--at` telur í því sem það gefur. Hann þarf skrá í réttri röð: annars er honum hafnað, og `--sort` — sem kemur fyrst — raðar skránni fyrirfram. Skráin segir frá því á fyrsta stigi frásagnar (`<path>: N subtitles split, M merged`), og færslan fyrir `--format json` ber talningarnar `split` og `merged`, **fjarverandi án valkostsins**.

## Þegar skurði er hafnað

**Tveir helmingar sem skarast við skurðinn eru ekki skornir.** Ef skjátexti í halanum byrjaði áður en
síðasti skjátextinn sem verður eftir endar myndi færslan bera hann fyrir upphaf myndskeiðsins — neikvæð staðsetning
sem ekkert snið skrifar. `split-file` hafnar því og **nefnir skjátextann**, með orðum gluggans:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

Kóðinn er `2` og **ekkert er skrifað**. Skerðu annars staðar. Að lenda nákvæmlega á upphafinu er leyfilegt: núll er
staðsetning.

**`--at` utan skrárinnar er hafnað á sama hátt**, þegar skráin hefur verið lesin: `--at 1` — fyrsti skjátextinn getur ekki
hafið hala, ekkert yrði eftir — og `--at` handan síðasta skjátextans, sem skildi eftir tóman
hala. Ekki er hægt að skera skrá með einum skjátexta.

## Úttak

**Staðalúttak** — ekkert: niðurstaðan eru skrárnar tvær sem eru skrifaðar.

| Stig | Hvað bætist við á villuúttakið |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; með `--dry-run`, `(dry run, nothing written)` í stað `-> …` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — snið, stafakóðun, merki og línulok skrárinnar sem var lesin, sett aftur eins og þau voru |
| 3 | `<path>: N bytes read, H and T written`, síðan **sérhvert greiningarboð við lestur** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) kemur **einn hlutur**, þar sem `file` er skráin sem er skorin.
**`destination` er hausinn** — skráin sem inntakið verður — og **`tail` halinn**, slóð, eða `null` með
`--dry-run` eins og `destination`. Talningarnar í `counts`: `subtitles` (skjátextar skrárinnar sem var lesin), `head` og `tail` (skjátextar hvors
helmings).

## Skilakóðar

| Kóði | Hvenær |
| :--- | :---- |
| `0` | báðar skrárnar eru skrifaðar — eða reiknaðar, með `--dry-run` |
| `1` | notkunarvilla: ekkert `--head` og `--tail`, valkostur sem `split-file` hefur ekki, áfangastaður sem er inntakið eða hinn áfangastaðurinn, `--at` sem er ekki jákvæð heiltala |
| `2` | ekki tókst að lesa skrána, **skurðinum er hafnað** (utan skrárinnar, eða skörun), eða ekki tókst að skrifa áfangastað. Kóði `3` er ekki til: ein skrá er unnin |

**Hausinn er skrifaður fyrst**: diskur sem hafnar halanum skilur hausinn eftir á bak við sig, og kóðinn er `2`.

## Villur

| Hvað veldur þeim | Skilaboð |
| :------------------- | :------ |
| hvorki `--head` né `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| aðeins annað af tveimur, með `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| áfangastaðirnir tveir eru sama skráin | `<path>: named as both the head and the tail` |
| áfangastaður er inntakið | `<path>: would be written over the input <input>` |
| `--at` utan skrárinnar | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| skrá með einum skjátexta | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| skörun | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| skrá vantar, er ólæsileg eða á óþekktu sniði | `<path>: does not exist`… — það sama og alls staðar |
