# `append`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```
Setur skrár **á eftir þeirri fyrstu**, í **eina** skrá: kvikmyndin í tveimur hlutum, gerð að
einu safni skjátexta. Þetta er `Append File…` í glugganum — [sama vélin](../subedit-gui/operations.md#append-file),
sömu talningar — beitt jafn oft og skrár eru til að bæta við.

- **Fyrsta skráin er grunnurinn.** Hún gefur niðurstöðunni **snið sitt, stafakóðun, bætaröðunarmerki
  og línulok**.
- **Engu er skotið inn milli tveggja skráa**: hver skrá er færð um **endi síðasta skjátexta þess sem
  fer á undan henni**, og staðsetningar hennar halda röð sinni og bili. Sú þriðja fylgir þeirri annarri, ekki
  grunninum: skrárnar fylgja hver annarri í þeirri röð sem þær eru gefnar.
- **Hver skrá sem er bætt við fer í gegnum umbreytinguna** í snið grunnsins: merki hennar eru þýdd,
  og það sem sniðið getur ekki skrifað er sagt, skrá fyrir skrá, með orðum gluggans.
- **Skrárnar sem er bætt við hafa enga þýðingu**: skrá sem er opnuð ein og sér hefur enga að gefa.
- **Gefa má sömu skrána oftar en einu sinni**, eða bæta henni við sjálfa sig: ekkert bannar það.

**`append` er ekki runuvinnsla.** N inntök, **eitt** úttak: lykkja hinna undirskipananna — ein niðurstaða
fyrir hverja skrá og bilun sem lætur hinar halda áfram — á ekki við hér. Þrennt leiðir af því:

- **áfangastaðurinn er `--output`, og ekkert annað.** `--output-dir` og `--in-place` eru ekki til fyrir
  `append` og er hafnað eins og hverjum öðrum óþekktum valkosti, kóði `1`; `--recursive` sömuleiðis: röð
  skránna er sú sem er gefin, og gengið tré hefur enga;
- **allt eða ekkert**: fyrsta skráin sem ekki er hægt að lesa stöðvar keyrsluna og **ekkert er skrifað**. Kvikmynd
  í þremur hlutum þar sem einn vantar er ekki kvikmynd í tveimur;
- **ein lína á hverja keyrslu** fyrir niðurstöðuna, og á JSON-sniði **einn hlutur**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Röksemdir og valkostir

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | já, **að minnsta kosti tvær** | slóðir: grunnurinn, síðan það sem er sett á eftir honum, í röð | — |
| `--output` | **já**, nema með `--dry-run` | skráarslóð; mappan hennar er búin til ef hún vantar | — |
| `--dry-run` | nei | fáni: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |
| `--sort` | nei | fáni: raðar heildinni eftir upphafi **þegar öllu hefur verið bætt við**, sjá [Röðun niðurstöðunnar](#röðun-niðurstöðunnar) | slökkt |
| `--disjoint` | nei | fáni: sker skjátexta sem skarast **þegar öllu hefur verið bætt við og það raðað**, sjá [Að losa tímabilin við skörun](#að-losa-tímabilin-við-skörun) | slökkt |

**Áfangastaðurinn má ekki vera neitt af inntökunum**, grunnurinn meðtalinn: að skrifa yfir skrá sem verið er að
lesa er hafnað áður en nokkuð er lesið, kóði `1`. Fyrirliggjandi `--output` sem er ólík inntökunum er
skrifað yfir, eins og alls staðar.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

Síðasti skjátexti `partie1.srt` endar á 4 s; sá í `partie2.srt`, sem byrjaði á 0,5 s, byrjar á
4,5 s.

## Röðun niðurstöðunnar

Hver skrá er færð **frá enda þess sem fer á undan henni**: röðin sem getur brotnað er því röð skránna sjálfra. `--sort`
raðar niðurstöðunni eftir upphafi, **í einu lagi, eftir síðustu viðbót**, eins og [`sort`](sort.md) myndi gera — stöðugt, og án þess að breyta neinu
í innihaldinu.

Aldrei sjálfgefið. Með valkostinum segir grunnurinn frá því á fyrsta frásagnarstigi, `<base>: N subtitles moved` eða `<base>: already in order`, og
`--format json`-færslan ber talninguna `moved`, **sem vantar án valkostsins**.

## Að losa tímabilin við skörun

Aldrei sjálfgefið. Valkosturinn gerir það sem [`disjoint`](disjoint.md) gerir, **með sjálfgefnum stillingum þess**, þegar öllu hefur verið bætt við, á heildina. Hann krefst skrár í réttri röð: annars er honum hafnað, og `--sort` — sem kemur á undan — kemur henni í röð áður. Skráin segir frá því á fyrsta frásagnarstigi (`<path>: N subtitles split, M merged`), og `--format json`-færslan ber talningarnar `split` og `merged`, **sem vantar án valkostsins**.

## Þegar sniðin eru ólík

Niðurstaðan er á **sniði grunnsins**, hvað sem snið hinna er. Það sem krossunin kostar er
sagt skrá fyrir skrá, með orðum gluggans — sama setning og í `Append File…` og í
[að líma úr öðru sniði](../subedit-gui/clipboard.md#merki-úr-öðru-sniði-eru-þýdd):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

Það sem tapast ræðst aðeins af sniði **hverrar** skrár sem er bætt við: grunnur í Advanced SSA tekur við
SubRip-skrá án þess að tapa neinu, en ekki öfugt. Til að velja snið niðurstöðunnar skaltu setja fyrst skrá
sem er þegar á því sniði, eða umbreyta á eftir með [`convert`](convert.md).

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem er skrifuð.

| Stig | Hverju er bætt við villuúttakið |
| :----- | :------------------------------------- |
| 1 | ein lína fyrir hverja skrá sem er bætt við — `<path>: appended N subtitles`, og þegar sniðið kostaði eitthvað fylgir `; <source format> into <base format>: <what is lost>` — síðan `<base>: N subtitles from M files -> <destination>` (með `--dry-run` kemur `(dry run, nothing written)` í stað `-> <destination>`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — snið, stafakóðun, merki og línulok grunnsins, sett aftur eins og þau voru |
| 3 | `<base>: N bytes read, M written` (allar skrárnar saman), síðan **hvert lesgreiningarboð** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) kemur **einn hlutur** fyrir alla keyrsluna,
þar sem `file` er **grunnurinn**. `counts` hans:

| Talning | Hvað hún segir |
| :----- | :----------- |
| `files` | skrár keyrslunnar, grunnurinn meðtalinn |
| `subtitles` | skjátextar niðurstöðunnar |
| `appended` | þeir sem koma úr skránum sem er bætt við |
| `lost_ends`, `lost_header` | `0` eða `1`: ein skrárinnar sem var bætt við tapaði því |
| `joined_lines`, `lost_tags`, `lost_fields` | summa þess sem skrárnar sem var bætt við töpuðu |
| `furthest_ms` | mesta tilfærsla staðsetningar, allar skrár saman |

**Fylkið `inputs`** fylgir með `counts`, utan þeirra: eitt stak fyrir hverja skrá sem er bætt við, í röð,
`{"file", "counts"}` — þar sem `counts` ber `appended`, síðan sama tap, **fyrir þá skrá eina**. Það er það sem
segir, í keyrslu með tíu skrám, hver þeirra kostaði hvað. Lesviðvörunum allra skránna
er safnað í `warnings`.

Bilun er líka hlutur, **einn einasti**: hlutur fyrstu skrárinnar sem ekki var hægt að lesa.

## Skilakóðar

| Kóði | Hvenær |
| :--- | :---- |
| `0` | niðurstaðan er skrifuð — eða reiknuð, með `--dry-run` |
| `1` | notkunarvilla: færri en tvær skrár, enginn `--output`, valkostur sem `append` hefur ekki, áfangastaður sem er eitt inntakanna |
| `2` | **ekki tókst að lesa skrá**, eða ekki tókst að skrifa áfangastaðinn: ekkert er skrifað. Enginn kóði `3` er til: keyrsla sem skrifar aðeins eina skrá heppnast eða ekki |

## Villur

| Hvað veldur þeim | Skilaboð |
| :------------------- | :------ |
| aðeins ein skrá gefin | `append needs a base file and at least one file to put after it` |
| hvorki `--output` né `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` nefnir inntak | `<destination>: would be written over the input <input>` |
| skrá vantar, er ólæsileg eða á óþekktu sniði | `<path>: does not exist`, `<path>: is in no format this tool knows`… — eins og alls staðar |

**Tómri skrá er ekki bætt við**: ekkert snið les án að minnsta kosti eins skjátexta, og skránni er
hafnað eins og hverri skrá sem ekki er hægt að lesa — `<path>: holds nothing recognisable as a subtitle` —, sem
stöðvar keyrsluna og skrifar ekkert.
