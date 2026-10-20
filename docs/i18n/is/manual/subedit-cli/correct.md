# `correct`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```
Leiðréttir textana með **mynstrum Gaupol**, eins og aðstoðarforritið
[`Correct Texts…`](../subedit-gui/correct-texts.md) í glugganum — sömu mynstur,
sama vélin, sömu talningar. Sex verk eru til hér: **tilvísanir fyrir
heyrnarskerta**, **sameining** og **skipting orða**, **algengar villur**,
**hástafir** og **línuskipting**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
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

## Hvað er valið og hvað ekki

**Ekkert er hakað fyrirfram.** Glugginn opnar reiti sína eins og notandinn skildi við þá; skipanalína
tekur verk sín, kóða, línulengd og tungumál af sinni eigin línu,
aldrei úr stillingaskránni: hvorki verkin sem einhvern tíma voru hökuð í glugganum, né mynstrareitirnir
sem þar voru stilltir (`correction.activations`), né skiptilisti stafsetningarathugunarinnar eru
lesin, og ekkert er skrifað þeim megin.

**Fjórar stillingar koma sjálfgefið úr stillingaskránni** — gildin sem `Correct Texts…`
man, eða sem [`config set`](config.md) skrifaði. **Valkostur á línunni vinnur alltaf**, síðan
skráin, síðan innbyggða sjálfgefna gildið; sjá [Stillingaskráin](invocation.md#stillingaskráin)
um hvar hún er, `--config` og `--no-config`.

| Lykill | Hvað hann leggur til | Valkostur sem yfirskrifar hann |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | flokkar algengra villna sem er beitt | `--classes` |
| `correction.remove-blank` | hvort skjátextar sem leiðréttingin tæmir eru fjarlægðir | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | mesti línufjöldi `line-break` | `--max-lines` |

- **Hver þessara fjögurra hefur valkost sem afturkallar hann**, hvað sem skráin segir: `--classes human,ocr`
  beitir báðum flokkum, `--remove-blank-subtitles` fjarlægir tæmda skjátexta, `--max-lines 3`
  gefur aftur línur Gaupol. `--keep-blank-subtitles` og `--remove-blank-subtitles` saman eru
  notkunarvilla.
- **Báðum flokkum slökkt í skránni er hunsað, og sagt frá því** á stigi 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` gæti ekki skrifað það, og það myndi
  skilja common-errors eftir án verks. Báðum flokkum er beitt í staðinn.
- **Línufjöldi utan 1 til 1000 er alls ekki lesinn úr skránni**: lesari skrárinnar
  hafnar honum — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, á
  stigi 1 — og innbyggða gildið 3 er notað.

Dæmin á þessari síðu eru keyrð með `--no-config`, til að þau sýni hvað innbyggðu
sjálfgefnu gildin gera, hverjar sem stillingar vélarinnar sem keyrir þau eru.

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :----- | :------ | :----- |
| `<file>...` | já | ein eða fleiri slóðir | — |
| `--tasks` | **já** | listi aðskilinn með kommum: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | ekkert verk |
| `--code` | **já** með mynstraverki, og **á ekki við** annars | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **já** með `join-words` eða `split-words`, og **á ekki við** annars | tungumálskóði: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | nei | `human`, `ocr` eða `human,ocr` | báðir — eða það sem stillingaskráin segir |
| `--enable`, `--disable` | nei, má endurtaka | enskt heiti mynsturs, eða `type:name` | skráðu `.conf`-skrárnar sem fylgja |
| `--max-length` | **já** með `line-break` | fjöldi **stafa** yfir núlli | **engin** — sjá hér að neðan |
| `--max-lines` | nei | heil tala lína, yfir núlli | 3 — eða það sem stillingaskráin segir |
| `--skip-length`, `--skip-lines` | nei | tala, eða `off` | sömu mörk og `--max-length` og `--max-lines` |
| `--keep-blank-subtitles` | nei | fáni: tæmdir skjátextar haldast, tómir | tæmdir skjátextar eru fjarlægðir — nema stillingaskráin segi annað |
| `--remove-blank-subtitles` | nei | fáni: tæmdir skjátextar eru fjarlægðir, hvað sem stillingaskráin segir; hafnað með `--keep-blank-subtitles` | slökkt |
| `--range` | nei | `N-M`, eða `N-` til loka — sjá [`adjust`](adjust.md#--range) | öll skráin |
| `--document` | nei | `main` eða `translation` — sjá [Þýðing](invocation.md#þýðing) | `main` |
| `-t`, `--translation-file` | með `--document translation` | þýðingarskrá, fyrir **eitt** inntak | — |
| `--align-method` | nei, og aðeins með `-t` | `position` eða `number` | `position` |
| `--recursive`, `-r` | nei | fáni | slökkt — sjá [Vinnsla á möpputré](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | fáni: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

### Verkin

**Röð listans skiptir engu máli**: verkin eru framkvæmd í röð
Gaupol — tilvísanir, síðan sameining og skipting orða, síðan algengar villur, síðan hástafir, síðan línuskipting —, hvert á
textanum sem það fyrra skildi eftir. Skjátexti sem tilvísanirnar tæmdu tekur engan
þátt í því sem á eftir kemur.

| Verk | Hvað það gerir |
| :---- | :-------------- |
| `mentions` | fjarlægir hljóðin milli hornklofa, og milli sviga þegar kveikt er á `Sound in parentheses` (sópun [`hearing-impaired`](hearing-impaired.md)), síðan söngtexta- og mælandamynstrin |
| `join-words` | límir saman orð sem var skorið í tvennt, úr orðabók `--language` |
| `split-words` | skiptir tveimur orðum sem eru föst saman, úr sömu orðabók |
| `common-errors` | leiðréttir algengar villur: bil, greinarmerki, samstafsmerki, OCR-villur |
| `capitalization` | setur hástafinn þar sem tungumálið krefst hans |
| `line-break` | skiptir línum skjátexta upp á nýtt þannig að engin fari yfir `--max-length`, með því að vega skiptimynstur kóðans |

### Mynsturkóðinn

**`--code` nefnir skriðuna**: `Zyyy` fyrir hvaða letur sem er, síðan letrið, tungumálið
og landið — `Latn-en-US` les mynstur `Zyyy`, `Latn`, `Latn-en` og
`Latn-en-US`. **Hann er nauðsynlegur**, því skipanalína hefur ekkert umhverfistungumál:
altækt sjálfgefið gildi myndi beita *minna* en maður heldur, án þess að segja frá því. Hann gildir um
hvert verk sömu keyrslu; verk sem vill sinn eigin kóða er önnur
keyrsla. Illa skrifaður kóði er notkunarvilla:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Mynstrin sem notandinn setur í `$XDG_DATA_HOME/subedit/patterns` (eða
`~/.local/share/subedit/patterns`) **bætast við mynstrin sem fylgja**, eins og í
glugganum, og mynstur sem fylgir og ekki er hægt að lesa er nefnt á stigi 1: `patterns: <file>, line N (<reason>)`.

### Sameining og skipting orða, og orðabókin

**Þessi tvö verk lesa engin mynstur**: þau leita í **orðabók kerfisins, í gegnum
Enchant** — háð pakki í pökkunum (`libenchant-2-2` á Debian og Ubuntu, `enchant2` á
Fedora), sem `subedit-cli` deilir með glugganum. Þau hafa því hvorki kóða né heiti til að stilla:
`--code` á ekki við án mynstraverks, og `--language` ekki án annars þessara tveggja
— að gefa annað án hins er notkunarvilla, kóði `1`.

- **`join-words`** límir orð sem orðabókin þekkir ekki við það sem fer á undan eða
  fylgir því, **þegar ein átt gefur þekkt orð**: `hel lo` verður `hello`; tvær mögulegar
  áttir, eða engin, ráða engu.
- **`split-words`** skiptir orði sem orðabókin þekkir ekki **þegar nákvæmlega ein tillagna þess
  er það orð með bili**: `hellothere` verður `hello there`. Orð með upphafshástaf
  og síðan lágstöfum er látið í friði: oftast er það nafn.

**Án orðabókar fyrir tungumálið er keyrslunni hafnað**, kóði `1`, **áður en skrá er
lesin** og með orðum gluggans, sem grámar eiginleikann og segir frá því — hér hafnar skipanalínan
og segir frá því, frekar en að þegja:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**Skiptilisti gluggans er hvorki lesinn né skrifaður.** Hann *fyllist* með
vali notandans í stafsetningarathuguninni; skipanalína hefur ekkert slíkt
val, og að lesa listann myndi gera skriftu háða því sem höfundur hennar setti einhvern tíma í hann.

### Línuskipting

**Hún er mæld í stöfum, og það er munurinn frá glugganum.** Glugginn mælir í *ems* —
breidd sem er stillt eftir leturgerðinni, við 0,55 em á staf —, og sjálfgefna gildið 24 þar er gildi
í ems. Skipanalína hefur enga leturgerð: hún telur stafi, þá sem eru í
textanum. **Að afrita 24 yfir sem stafi myndi skera nánast allt, án þess að
segja frá því**; þess vegna er **engin sjálfgefin lengd**, og `--max-length` er nauðsynlegt með
`line-break`:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` er 3**, gildi Gaupol, sem einingin breytir ekki; línuskipting tekur
eina til viðbótar þegar þrjár nægja ekki. **Undanþágan** — skjátexti sem er þegar innan markanna er
látinn í friði — tekur sjálfgefið **sömu mörk** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), eins og Gaupol gerir; `--skip-length off` og `--skip-lines off`
slökkva á henni, og textinn er þá lesinn aftur sem einn og skorinn aftur:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

Skiptimynstrin eru kóðans: `Latn` skiptir ekki milli tölu og einingar hennar né
á eftir þankastriki, `Latn-en` skiptir ekki á eftir greini, forsetningu, titli
eða eignarfornafni. **Undir `Zyyy` er ekkert skiptimynstur til**: línurnar eru
aðeins jafnaðar. **Þessu verki er aldrei hafnað vegna skorts á virku mynstri**: mynstur þess
vega hvar á að skipta, skiptingin á sér stað án þeirra. Valkostirnir fjórir eiga aðeins við með
`line-break`; séu þeir gefnir án þess eru þeir notkunarvilla.

### Flokkarnir, og að kveikja og slökkva eftir heiti

**`--classes` síar algengar villur til frambúðar**: að afhaka `ocr` tekur mynstur þess
úr beitingu — samstafsmerkið `ﬁ` helst þá `ﬁ`. Hinar þrjár gerðir mynstra
bera engan flokk.

**`--enable` og `--disable` stilla mynstur eftir enska heitinu þess**, heitið er
lykillinn (ADR 0037); eitt heiti getur átt við nokkrar færslur, sem þá deila einum
reit. Það er nauðsynlegt, og það er það sem ýtir sjálfgefnum gildum gluggans til hliðar: **`.conf`-skrárnar sem
fylgja slökkva á tilvísanamynstrunum**, svo `--tasks mentions` eitt og sér
beitti engu. Mynstrin tvö „Sound in brackets“ og „Sound in parentheses“
stjórna hvort sínu afmörkunarmerki — hornklofunum, svigunum — og eru stillt með sömu
valkostum: að kveikja á öðru einu og sér fjarlægir ekki hitt.

Þrennt eru **notkunarvillur**, kóði `1`, sagðar áður en skrá er lesin:

| Hvað veldur henni | Skilaboð |
| :------------------ | :------ |
| heiti sem á við ekkert mynstur verkanna sem eru gefin, undir kóðanum sem er gefinn | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| heiti sem á við mynstur af tveimur gerðum — það er þá skrifað `type:name`, þar sem gerðin er `common-error`, `capitalization`, `hearing-impaired` eða `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| heiti sem er gefið bæði `--enable` og `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**Innsláttarvilla er hávær**: glugginn hunsar úrelta stillingu því
stilling getur lifað lengur en mynstur sem hvarf; kall á skipun á engan eftirlifanda.

**Verki án nokkurs virks mynsturs er hafnað**, kóði `1` — „ekkert að gera“ er
svar sem er sagt, ekki þögul velgengni:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

Sama skrá, með sópun hornklofanna kveikta:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Hvað er skrifað og hvað er sagt

Snið, stafakóðun og línulok skrárinnar sem var lesin **haldast**. **Skjátextar
sem leiðréttingin tæmir eru fjarlægðir**, eins og sjálfgefinn reitur gluggans;
`--keep-blank-subtitles` lætur þá vera tóma, og `--remove-blank-subtitles` fjarlægir þá
jafnvel þegar stillingaskráin heldur þeim. **Tæmd þýðing helst tæmd**
og heldur skjátexta sínum.

**Talningin er gluggans** — „Edited N and removed M subtitles“: textar
sem breyttust og skjátextar sem voru fjarlægðir, **aldrei samsvaranir**. Skrá þar sem ekkert
breytist er skrifuð samt: áfangastaður sem er gefinn er áfangastaður sem er skrifaður.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Mynstur sem getur ekki átt við

**Mynstur sem ekki er hægt að lesa, ekki er hægt að þýða eða lýkur ekki er nefnt, með
skjátextanum þar sem það gafst upp**, og þar segir skipanalínan meira en
glugginn: staðfestingarsíðan nefnir mynstrið aðeins einu sinni, án textans.

| Ástæða | Hvað hún þýðir |
| :----- | :------------------- |
| `cannot be translated` | segðin notar það sem þýðingin yfir í ICU hafnar |
| `will not compile` | vélin hafnaði segðinni |
| `has an invalid replacement` | skiptin lesast ekki eins og Python les þau |
| `timed out` | vélin gafst upp á þeim texta |
| `never settled` | `Repeat` var enn að breyta textanum eftir hundrað umferðir |
| `grew the text too long` | textinn fór fram úr því sem skjátexti getur geymt |

Á stigi 1, á villuúttakinu: 
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — án endisins þegar mynstrið er lagt til hliðar fyrir alla texta. **Númerið er
skrárinnar**, hver verk sem fjarlægðu skjátexta á undan. Á JSON-sniði er það
`pattern-failed`-viðvörun skrárinnar, þar sem `detail` ber setninguna. **Mynstur sem er
gefið upp á bátinn er ekki skrá sem brást**: skráin er skrifuð og skilakóðinn er `0`.

## Úttak

**Staðalúttak** — ekkert, þar sem niðurstaðan er skráin sem er skrifuð. **Með `--dry-run` eru það
fyrirhugaðar breytingar**, ein blokk fyrir hvern skjátexta: númer hans, textinn fyrir, textinn eftir —
og `(removed)` án `+`-línu fyrir skjátexta sem leiðréttingin myndi fjarlægja; með
`--keep-blank-subtitles` helst hann, tómur. Sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er).

| Stig | Hverju er bætt við villuúttakið |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, mynstrin sem gátu ekki átt við, mynstraskrárnar sem ekki er hægt að lesa — og fyrst stillingaskráin sem var lesin — `settings: read <path>` — og það af henni sem var hunsað |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, síðan hvert lesgreiningarboð |

Á JSON-sniði ber `counts` `corrected` (textarnir sem breyttust) og `removed` (skjátextarnir sem voru
fjarlægðir), og `changes` skjátextana sem var snert. Með `-t` segir lykillinn `alignment` hvernig
þýðingin var lögð.
`settings` nefnir stillingaskrána sem var lesin, eða er `null` þegar engin var lesin.

## Skilakóðar

Þeir sömu og í verkfærinu: `0` ef hver skrá var skrifuð, `2` ef engin, `3` ef aðeins
sumar, `1` við notkunarvillu.

## Villur

| Hvað veldur henni | Hvað er skrifað, á villuúttakið |
| :------------------ | :--------------------------------------- |
| skrá sem `--config` nefnir og er ekki til — skrá notandans sjálfs, sem vantar, gefur sjálfgefnu gildin | `settings file not found: <FILE>` |
| `--tasks` vantar | `--tasks is required`, og síðan ábending um `--help` |
| óþekkt verk | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| `--code` vantar | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` án mynstraverks | `--code is for the tasks that read patterns, and none was asked for` |
| `--language` vantar með `join-words` eða `split-words` | `--language is required by the tasks that check words: <tasks>` |
| `--language` sem er ekki tungumálskóði | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` án annars þessara tveggja verka | `--language is for the tasks join-words and split-words, which were not asked for` |
| tungumál án orðabókar | `no dictionary for <language>` |
| illa skrifaður kóði | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| `--max-length` vantar með `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length` sem er ekki tala yfir núlli | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines` sem eru ekki heil tala lína | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` eða `--skip-lines` ólæsilegt | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| einn af valkostunum fjórum án `line-break` | `--max-length is for the task line-break, which was not asked for` |
| óþekktur flokkur | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` með `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| rangt mynstursheiti | sjá töfluna hér að ofan |
| verk án virks mynsturs | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, áfangastaðurinn | þær sem eru í [Ræsing](invocation.md#villur) og í [Þýðing](invocation.md#þýðing) |
