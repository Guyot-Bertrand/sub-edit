# `adjust`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```
Lagar **tímalengd** hvers skjátexta að lestrarhraða og mörkum: skjátexti sem er of stuttur til að
hægt sé að lesa hann er lengdur, skjátexti sem fer of seint af skjánum
má stytta, og tveir skjátextar skarast ekki. **Aðeins endirinn
færist**: að færa upphafið myndi færa skjátextann, og það er verk
[`shift`](shift.md). Þetta er aðgerðin `Adjust Durations…` í
[glugganum](../subedit-gui/operations.md#adjust-durations), með sömu sjálfgefnu gildum.

## Skilyrðin fjögur og röð þeirra

Þau gilda **í þessari röð**, hvert þeirra setur endinn og það síðasta vinnur:

1. **lestrarhraðinn** — tímalengdin sem textinn þarf, miðað við tiltekinn fjölda stafa á
   sekúndu (merki teljast ekki, línuskil gera það);
2. **lágmarkið** — tímalengd undir því er hækkuð upp í lágmarkið;
3. **hámarkið** — tímalengd yfir því er lækkuð niður í hámarkið;
4. **bilið** — tíminn sem er eftir fram að næsta skjátexta. Það kemur síðast og
   **það vinnur**: tveir skjátextar sýndir saman eru galli sem áhorfandinn sér,
   skjátexti sem er ögn of stuttur er það ekki.

Síðasti skjátexti skrárinnar á engan næsta og þar með ekkert bil til að halda. Næsti skjátexti
er sá sem er í skránni, **hvort sem hann er valinn** með `--range` **eða ekki**.

| Skilyrði | Valkostur | Sjálfgefið | Að slökkva á því |
| :--------- | :----- | :----- | :------- |
| hraði | `--speed CPS` | **15** stafir á sekúndu | `--speed off` |
| stefna hraðans | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | hann **lengir** aðeins | — |
| lágmark | `--minimum TIME` | **1,5 s** | `--minimum off` |
| hámark | `--maximum TIME` | **ekkert** | `--maximum off` |
| bil | `--gap TIME` | **0** | `--gap off` |

Þetta eru stillingar Gaupol og fyrstu opnunar gluggans. **`off` slekkur á
skilyrði: það er ekki lengur athugað.** Það er ekki það sama og að setja það á núll — lágmark `0`
er lágmark upp á núll, bil `0` bannar alla skörun.

## Hvað stillingaskráin leggur til

**Sjálfgefnu gildin hér að ofan eru þau innbyggðu.** Sjálfgefið les `adjust` stillingar aðgerða úr
stillingaskránni — gildin sem `Adjust Durations…` man í glugganum, eða sem
[`config set`](config.md) skrifaði — og þau koma í stað innbyggðu sjálfgefnu gildanna. **Valkostur
sem er gefinn á línunni vinnur alltaf**, síðan skráin, síðan innbyggða sjálfgefna gildið. Sjá
[Stillingaskráin](invocation.md#stillingaskráin) um hvar skráin er, `--config` og
`--no-config`.

| Lykill | Hvað hann leggur til | Valkostur sem yfirskrifar hann |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | lestrarhraðinn, í stöfum á sekúndu | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | í hvora átt hraðinn má færa endi; bæði `false` slökkva á hraðanum | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; að gefa `--speed CPS` eða stefnu kveikir aftur á hraða sem skráin slökkti á, með lengingu |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | stysta tímalengdin, eða engin | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | lengsta tímalengdin, eða engin | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | minnsta bilið, eða ekkert | `--gap TIME`, `--gap off` |

**Sérhver lykill hefur valkost sem afturkallar hann.** Hvor stefna hefur par fána sem stilla hana
hvað sem skráin segir: `--shorten` og `--no-shorten` fyrir styttingu, `--lengthen` og
`--no-lengthen` fyrir lengingu; fáni sem ekki er gefinn lætur stefnu skrárinnar halda sér.
`--maximum off` fellir niður hámark sem skráin kveikir á. Með `--format json` segir `constraints`
frá stillingunum sem voru raunverulega notaðar, hvaðan sem þær komu, og `settings` skrána sem var lesin:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Hin dæmin á þessari síðu eru keyrð með `--no-config`, til að þau sýni hvað innbyggðu
sjálfgefnu gildin gera, hverjar sem stillingar vélarinnar sem keyrir þau eru.

<!-- example: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-shorten                Do not let the reading speed bring an end earlier
  --lengthen                  Let the reading speed move an end later
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME|off          Longest duration, or off (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `--speed` | nei | fjöldi stafa á sekúndu, **yfir núlli** (`15`, `12.5`), eða `off` | `15` |
| `--shorten` | nei | fáni: hraðinn má líka **færa** endi **fyrr** | slökkt |
| `--no-shorten` | nei | fáni: hraðinn **færir** ekki lengur endi **fyrr**, hvað sem stillingaskráin segir | slökkt |
| `--lengthen` | nei | fáni: hraðinn **færir** endi **síðar**, hvað sem stillingaskráin segir | slökkt |
| `--no-lengthen` | nei | fáni: hraðinn **færir** ekki lengur endi **síðar** | slökkt |
| `--minimum` | nei | tímalengd upp á núll eða meira, eða `off` | `1.5` |
| `--maximum` | nei | tímalengd **yfir núlli**, eða `off` | engin |
| `--gap` | nei | tímalengd upp á núll eða meira, eða `off` | `0` |
| `--range` | nei | `N-M`, eða `N-` til loka — sjá hér að neðan | öll skráin |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | fáni: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

Tímalengd er skrifuð eins og fyrir [`shift`](shift.md#að-rita-tímalengd): `2.5`, `00:00:02.500`.

**Án nokkurs virks skilyrðis er `adjust` hafnað** (kóði `1`, ekkert skrifað): það væri
ekkert að laga að, og skipun sem gerir ekkert en segist hafa heppnast er versta
svarið. Það á við um `--speed off --minimum off --gap off` án hámarks, og um
`--no-lengthen` eitt og sér þegar slökkt er á lágmarkinu og bilinu: hraði sem færir engan
enda takmarkar ekkert.

`--shorten`, `--no-shorten`, `--lengthen` og `--no-lengthen` segja í hvora átt hraðinn færir
endi; þeir eiga aðeins við með hraða, og **`--speed off` hafnar þeim**. Fána sem er gefinn með
andstæðu sinni — `--shorten --no-shorten`, `--lengthen --no-lengthen` — er líka hafnað.

### `--range`

`--range 3-7` takmarkar lagfæringuna við skjátexta 3 til 7, **taldir frá 1, mörkin
meðtalin**; `--range 3-` nær til þess síðasta. Stakur skjátexti er skrifaður `4-4`.

- Tímabili **sem endar áður en það byrjar**, `0`, formerki eða stakri tölu er
  hafnað sem notkunarvillu (kóði `1`).
- **Það gildir fyrir hverja skrá rununnar**, sem er aðeins skynsamlegt ef skrárnar
  líkjast. Það er metið **gagnvart hverri skrá**: tímabili sem fer út fyrir þá skrá er
  hafnað **áður en hún er snert**, með nafni markanna —
  `range 3-9 ends after the last subtitle: the file has 4` —, sú skrá bregst (kóði `2` eða `3`, auðkenni
  `range-out-of-bounds`) og hinar halda áfram.
- Talningar skýrslunnar varða aðeins valda skjátexta.

## Hvað er fórnað

Tvö skilyrði geta mótmælt hvort öðru á sama skjátexta: lágmark upp á 1,5 s passar ekki
í þær 0,8 s sem næsti skjátexti skilur eftir. **Röðin ræður og skýrslan segir hvað
varð að víkja** — sem Gaupol gerir ekki: það brýtur þau þegjandi.

Hvert skilyrði er aðeins nefnt ef **að minnsta kosti einn skjátexti fékk það ekki**, eftir að
hin hafa verið framkvæmd. Hámarkið hefur enga talningu: það er framkvæmt á undan bilinu, sem getur
aðeins stytt, svo það heldur alltaf. Lágmark yfir hámarkinu víkur fyrir hámarkinu og er
talið sem lágmark.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

Þriðji skjátextinn vildi enda á 15,7 s til að vara í 1,5 s, en sá fjórði byrjar á
15,0 s: **bilið vinnur**, hann endar á 15,0 s og skýrslan nefnir lágmarkið.

Ef slökkt er á bilinu vinnur lágmarkið — og skjátextarnir tveir skarast:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

Og engu er fórnað né lagað þegar skráin passar þegar innan marka sinna:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

Skráin er skrifuð eins og hún er: **áfangastaður sem er gefinn er áfangastaður sem er skrifaður**.

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem er skrifuð.

| Stig | Hverju er bætt við villuúttakið |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, eða `<path>: no duration to adjust -> <destination>` — og fyrst stillingaskráin sem var lesin — `settings: read <path>` — sjá [Stillingaskráin](invocation.md#stillingaskráin) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — snið, stafakóðun, merki og línulok skrárinnar sem var lesin, sett aftur eins og þau voru |
| 3 | `<path>: N bytes read, M written`, síðan **hvert lesgreiningarboð** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

Setningin er gluggans: `N` er fjöldi skjátexta þar sem endirinn færðist, og
afgangurinn, sem vantar þegar ekkert varð að víkja, telur upp það sem hélst ekki — `the reading speed`,
`the minimum duration`, `the gap` — með fjölda skjátexta sem það varðar.

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) ber hlutur hverrar skrár,
auk umslagsins:

| Lykill | Innihald |
| :-- | :------ |
| `counts.subtitles` | skjátextarnir **sem miðað er á**: öll skráin, eða tímabilið |
| `counts.adjusted` | þeir sem endirinn færðist á |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | skjátextarnir sem halda enn ekki þessu skilyrði, eftir að hin hafa verið framkvæmd, **þar með taldir þeir sem endirinn færðist ekki á** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — stafirnir á sekúndu **sem strengur** (`"15"`, `"12.5"`) — eða `null` ef slökkt er á |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | tímalengdirnar sem voru notaðar, í **heilum millisekúndum**, eða `null` ef slökkt er á |
| `settings` | stillingaskráin sem var lesin, eða `null` þegar engin var lesin |

`constraints` segir frá **stillingunum sem voru raunverulega notaðar**, sjálfgefnum gildum
meðtöldum: tvo útreikninga á sömu skjátextum má þá bera saman án þess að lesa skipanalínuna.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Skilakóðar

Þeir sömu og í verkfærinu: `0` ef hver skrá var unnin, `2` ef engin, `3` ef aðeins
sumar, `1` við notkunarvillu. Skrá sem passar þegar innan marka sinna er unnin.

## Villur

| Hvað veldur henni | Skilaboð |
| :------------------ | :------ |
| skrá sem `--config` nefnir og er ekki til — skrá notandans sjálfs, sem vantar, gefur sjálfgefnu gildin | `settings file not found: <FILE>` |
| ekkert virkt skilyrði | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| hraði sem er ekki hraði | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| stefnufáni með `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| stefnufáni með andstæðu sinni | `--shorten and --no-shorten say opposite things; give one of them` (sömuleiðis `--lengthen` og `--no-lengthen`) |
| tímalengd sem er ekki tímalengd | `--minimum: "…" is not a time: …` (sömuleiðis `--maximum`, `--gap`) |
| neikvæð tímalengd | `--minimum: "-1" is not a duration of zero or more` (sömuleiðis `--gap`) |
| núll eða neikvætt hámark | `--maximum: "0" is not a duration above zero` |
| tímabil illa skrifað | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| tímabil sem endar áður en það byrjar | `--range: "5-3" is not a range: it ends before it starts` |
| tímabil utan skrárinnar | `<path>: range N-M starts after the last subtitle: the file has K`, eða `… ends after …` |

Villur áfangastaðarins eru sameiginlegar undirskipununum sem skrifa:
sjá [Ræsing](invocation.md#áfangastaðurinn).
