# `sort`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Setur skjátextana aftur **í röð upphafa þeirra**. Ekkert annað breytist: hvorki textarnir, né
staðsetningarnar, né snið skrárinnar. Skjátextarnir eru **endurnúmeraðir** frá 1 við ritun,
eins og í hverri skrá sem er skrifuð aftur.

**Kjarninn raðar aldrei af sjálfsdáðum**: lestur skráar skilur röð hennar eftir eins og hún var, og
[`inspect`](inspect.md#hvað-anomalies-tilkynnir) tilkynnir brot á röð með
`subtitle N starts before the previous one starts`. `sort` er **handtakið sem er gert af ásettu ráði**, fyrir skrá
eða runu þar sem röðin er brotin — og það má afturkalla í glugganum, eins og hverja aðgerð.

- **Röðunin er stöðug**: tveir skjátextar sem byrja á sama augnabliki **halda þeirri röð sem
  skráin gaf þeim**. Hvorugur kemur á undan hinum; að færa þá væri ákvörðun sem enginn
  bað um.
- **`sort` tekur hvorki við bili né skjali**: það er ekkert að velja, röðin er röð upphafanna.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

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
| `--recursive`, `-r` | nei | rofi | slökkt — sjá [Vinnsla á möpputré](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | rofi: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## Hvað er talið

**Talin eru sætin þar sem skjátextinn breyttist**, eins og kjarninn tilkynnir það — það sem
saga gluggans myndi kalla fært. Einn skjátexti á röngum stað færir nokkra: í
dæminu hér að ofan heldur ekkert af sætunum þremur skjátexta sínum, og setningin segir `3 subtitles moved`.

**Skrá sem er þegar í röð er skrifuð engu að síður** og segir það: tilgreindur áfangastaður er
skrifaður áfangastaður, og að gera `sort` að undantekningu myndi neyða skriftu til að vita hvaða
undirskipanir búa stundum til skrá og stundum ekkert.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem er skrifuð. Með `--dry-run` heldur ekki neitt: það er
engin skrá, og frásagnarlínan segir það — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er).

| Stig | Hvað bætist við á villuúttakið |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, eða `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — snið, stafakóðun, merki og línulok skrárinnar sem var lesin, sett aftur eins og þau voru |
| 3 | `<path>: N bytes read, M written`, síðan **sérhvert greiningarboð við lestur** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) ber hlutur hverrar skrár
`counts.subtitles` (hvern skjátexta skrárinnar) og `counts.moved` (sætin sem breyttust;
`0` fyrir skrá sem er þegar í röð).

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef sérhver skrá var unnin — skrá sem er þegar í röð er það —, `2` ef
engin, `3` ef aðeins sumar, `1` við notkunarvillu.

## Villur

Engar sem eru sérstakar fyrir `sort`: ólæsileg skrá bregst eins og hjá hverri undirskipun, og
villur áfangastaðarins eru sameiginlegar undirskipununum sem skrifa:
sjá [Ræsing](invocation.md#áfangastaðurinn).
