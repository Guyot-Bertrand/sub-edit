# Ræsing

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Án undirskipunar skrifar tólið hjálp sína og stoppar.

<!-- example: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors
  --encoding NAME             Encoding to read the files in; detected by default
  --format text|json          Form of the result on standard output: text or json
  --config FILE               Read the operation settings from FILE instead of the user's settings file
  --no-config Excludes: --config
                              Ignore any settings file: use the built-in defaults
  --list-encodings            Write the encodings that can be read and written, one a line, and stop

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  snap                        Move every position onto the nearest frame of a frame rate (see framerate)
  hearing-impaired            Remove the sounds described between square brackets
  adjust                      Bring the duration of every subtitle within a reading speed and limits
  replace                     Replace a text in the subtitles, without breaking a tag
  case                        Put the texts in title, sentence, upper or lower case, tags intact
  italics                     Put the texts in italics, or take their italics out
  dialogue-dashes             Put dialogue dashes on the lines, or take them off
  sort                        Put the subtitles in the order of their start, keeping ties as they are
  join-repeats                Join the subtitles that repeat each other, one after the other, into one
  disjoint                    Cut overlapping subtitles so that no range overlaps another, merging what overlaps
  repair                      Join the repeated subtitles, put them in order, then make the ranges disjoint
  correct                     Correct the texts with the patterns of the correction assistant
  append                      Put subtitle files one after another into a single file
  split-file                  Cut a subtitle file in two, a head and a tail
  pair                        Write a translation at the positions of its main file, and say how it lined up
  config                      List, show and change the operation settings
```

## Tungumál

Skipanalínan talar tungumál umhverfis síns: **hjálp hennar, frásögn og
villur** fylgja tungumálinu sem beðið er um. Dæmin í þessari handbók eru keyrð á ensku —
blokkirnar eru búnar til með `make manual`, aldrei afritaðar í höndunum.

**Hvernig tungumálið er valið.** Fyrsta breytan af `LC_ALL`, `LC_MESSAGES` og `LANG` sem er stillt og
ekki tóm er staðfærslan; `LANGUAGE`, ef hún er stillt, telur þá upp tungumálin sem óskað er eftir, aðgreind með
tvípunktum (`LANGUAGE=pt_BR:fr`), og er lesin fyrst. Þetta er röðin sem gettext fylgir, lesin af
forritinu sjálfu: staðfærslan þarf ekki að vera mynduð á vélinni.

| Regla | Áhrif |
| :--- | :----- |
| stafakóðun og breytir | hunsað: `fr_FR.UTF-8@euro` leitar að `fr_FR`, síðan `fr` |
| staðfærslan `LC_ALL=C`, `C` eða `POSIX`, eða engin staðfærsla | enska, og `LANGUAGE` er ekki lesin |
| færsla sem nefnir ensku (`en`, `en_GB`) | enska: það sem á eftir henni kemur í `LANGUAGE` er ekki lesið |
| tungumál sem þýðingaskráin er ekki uppsett fyrir | það næsta sem beðið er um, og enska á eftir því síðasta, **án þess að orð sé sagt** |
| skilaboð sem þýðingaskráin þýðir ekki | sýnd á ensku |

**Heiti leiðréttingarmynsturs** sem `correct` segist ekki hafa getað beitt fylgir tungumálinu
líka, þegar þýðingaskráin þekkir það — mynstrin sem fylgja forritinu; eigið mynstur er
nefnt eins og skráin þess ritar það. Mynstraskrárnar sjálfar eru aldrei endurskrifaðar.

Þýðingaskrárnar eru settar upp undir `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`
og fundnar út frá eigin möppu keyrsluskrárinnar. Enska er innbyggð. Þessi útgáfa setur upp `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` og `zh_CN` — katalónsku,
tékknesku, þýsku, spænsku, finnsku, frönsku, galisísku, hebresku, ungversku, interlingue, íslensku, ítölsku, hollensku, pólsku, portúgölsku, brasilísku portúgölsku, rússnesku, serbnesku, tyrknesku og einfaldaða kínversku — þýddar að fullu. Franskan hefur verið yfirfarin
af lesanda tungumálsins; katalónska, tékkneska, þýska, spænska, finnska, galisíska, hebreska, ungverska, interlingue, íslenska, ítalska, hollenska, pólska, portúgalska, brasilíska portúgalska, rússneska, serbneska, tyrkneska og einfölduð kínverska eru
vélþýddar og enn ekki yfirfarnar af lesanda tungumálsins.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
Undir `LANG=fr_FR.UTF-8` byrjar `subedit-cli --help` á „Lire,
inspecter et recaler des fichiers de sous-titres.“ Skripta sem les orðin í textaúttakinu
stillir `LANGUAGE=en` (eða `LC_ALL=C`) — eða les `--format json`, sem breytist aldrei.

**Það sem fylgir aldrei tungumálinu** — það sem skripta les:

| Aldrei þýtt | Hvers vegna |
| :--------------- | :-- |
| úttak `--format json`: lyklar, gildi, og `message` og `detail` í bilunum og aðvörunum | sömu bæti á hverju tungumáli; orðin eru ensk |
| skilakóðar | kóðarnir fjórir í [Skilakóðar](#skilakóðar), á hverju tungumáli |
| nöfnin sem eru slegin inn: undirskipanir, valkostir, gildi þeirra (`off`, `srt`), breytuheiti (`FILE`, `TEXT`, `OPTIONS`) | skipanalína sem er skrifuð einu sinni virkar undir hverju tungumáli |
| heiti leiðréttingarmynstra sem `correct --enable` og `--disable` taka við | mynstur er kallað ensku heiti sem skrá þess ber |
| heiti sniða og stafakóðana | snið er kallað sínu heiti |
| eigin greiningarboð CLI11 — nauðsynleg röksemd vantar, óþekktur valkostur, gildi sem valkostur tekur ekki við — og `Run with --help for more information.` | orðuð inni í þáttaranum sem skipanalínan er smíðuð á; þau haldast ensk |

Að biðja um ensku, hvert sem umhverfið er:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Altækir valkostir

**Þeir eru ritaðir á undan heiti undirskipunarinnar, og aðeins þar.**
`subedit-cli --encoding cp1252 inspect film.srt` er rétta formið;
`subedit-cli inspect --encoding cp1252 film.srt` er hafnað, með
`The following argument was not expected: --encoding`. Hið sama gildir um
`--version`, `-v` og `--quiet`.

**`--help` er undantekningin og er í raun ekki undantekning**:
`subedit-cli inspect --help` er tekið gilt því að hver undirskipun ber sína eigin, og það er
hjálp undirskipunarinnar sem hún skrifar, ekki tólsins.

| Valkostur | Áhrif |
| :----- | :---- |
| `-h`, `--help` | skrifar hjálpina og stoppar; hjálp undirskipunarinnar ef ein er nefnd |
| `--version` | skrifar `subedit <version>`, söfnin sem tólið keyrir gegn og uppsett tungumál, og stoppar — sjá [`--version`](#--version) |
| `-q`, `--quiet` | stig 0 — engin frásögn |
| `-v`, `-vv`, `-vvv` | stig 1 til 3; stig 1 er sjálfgefið |
| `--encoding NAME` | les skrárnar í þessari stafakóðun, í stað þess að giska á hana |
| `--list-encodings` | skrifar stafakóðanirnar sem tólið getur lesið og skrifað, eina á línu, og **stoppar** (kóði `0`) — sjá [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | form niðurstöðunnar á staðalúttaki: textinn, eða einn JSON-hlutur fyrir hverja skrá — sjá [Úttak sem skriftur geta lesið](#úttak-sem-skriftur-geta-lesið) |
| `--config FILE` | les aðgerðastillingarnar úr `FILE` í stað stillingaskrár notandans — sjá [Stillingaskráin](#stillingaskráin) |
| `--no-config` | les enga stillingaskrá: sérhver aðgerðastilling sem er ekki gefin á línunni tekur innbyggt sjálfgefið gildi sitt — sjá [Stillingaskráin](#stillingaskráin) |

Valkostunum `--quiet` og `-v` í sama kalli er hafnað: tvær andstæðar fyrirætlanir
eru ekki útkljáðar þeirri síðari í vil.

Valkostunum `--config` og `--no-config` í sama kalli er einnig hafnað, með
`--config excludes --no-config`, og eins `--config` með tómu heiti, með
`--config needs a file name`: tómt heiti nefnir enga skrá og skrá notandans sjálfs er ekki lesin
í staðinn.

### Stillingaskráin

**[`adjust`](adjust.md) og [`correct`](correct.md) lesa aðgerðastillingar
stillingaskrárinnar sjálfgefið** — gildin sem `Adjust Durations…` og `Correct Texts…` muna í
glugganum, eða sem [`config set`](config.md) skrifaði. Engin önnur undirskipun les hana. Skráin er:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

eða `~/.config/subedit/settings.conf` þegar `XDG_CONFIG_HOME` er óstillt, tóm eða ekki alger
slóð — skrá gluggans, lýst í [handbók hans](../subedit-gui/preferences.md#skráin).

**Forgangur, sá sterkasti fyrst: valkosturinn á skipanalínunni, síðan skráin, síðan innbyggða
sjálfgefna gildið.** Valkostur vinnur alltaf: `--speed off` slekkur á hraða sem skráin setur. Aðeins
**aðgerðablokk** skrárinnar er lesin; birtingarkjörstillingarnar (stærð og staða, dálkar, þema…)
eru það aldrei. [`config list`](config.md#config-list) nefnir lyklana og hverja þeirra skipanalínan
les: lyklana níu `duration-adjust.*`, `correction.human`, `correction.ocr`,
`correction.remove-blank` og `correction.line-break.max-lines`. **Hver þeirra hefur valkost
sem yfirskrifar hana í einu kalli**, hvernig sem skráin stillir hana — töflur
[`adjust`](adjust.md#hvað-stillingaskráin-leggur-til) og
[`correct`](correct.md#hvað-er-valið-og-hvað-ekki) nefna þá.

**Lestur skrár notandans lætur skipun aldrei bregðast.** Skrá sem vantar gefur sjálfgefnu gildin; skrá
sem ekki er hægt að lesa gefur sjálfgefnu gildin og segir frá því; gildi sem ekki er hægt að lesa heldur sjálfgefnu gildi sínu og er nefnt;
óþekktur lykill er hunsaður. Skráin er aldrei skrifuð af `adjust` eða `correct`.

**Það sem var lesið er sagt**, á sjálfgefnu stigi 1, á villuúttakinu, á undan fyrstu skránni:

| Tilvik | Lína |
| :--- | :--- |
| skráin var lesin | `settings: read <path>` |
| ekki var hægt að lesa gildi í henni | `settings: <key>: cannot read '<value>', default kept` |
| skrá notandans er ekki til | `settings: <path> not found, defaults used` |
| ekki er hægt að lesa skrána | `settings: <path> unreadable, defaults used` |
| `--no-config` | ekkert |

Með [`--format json`](#úttak-sem-skriftur-geta-lesið) ber hlutur hverrar skrár hjá `adjust` og
`correct` lykilinn `settings`: slóðina sem var lesin, eða `null` þegar engin var lesin.


**`--no-config` gerir keyrslu endurtakanlega**: sama línan gefur sömu niðurstöðu á hverri
vél, hvað sem notandi hennar stillti í glugganum. Notaðu það í skriftum, prófunum og villuskýrslum — eða
`--config FILE` með skrá sem fylgir þeim.

**`--config FILE` nefnir aðra skrá**, lesna á sama hátt — **nema að hún verður að vera til**. Heiti
sem er gefið á skipanalínunni er ekki beiðni um sjálfgefnu gildin: þegar `FILE` er ekki til
stoppa `adjust` og `correct` áður en nokkurt inntak er lesið, skrifa `settings file not found: <FILE>` á
villuúttakið og skila notkunarkóðanum `1`. `FILE` sem er til en er ekki læsileg
gefur sjálfgefnu gildin og segir frá því, eins og skrá notandans. [`config show`](config.md#config-show)
sýnir enn `FILE` sem vantar sem `(not found)` og [`config set`](config.md#config-set-og-config-unset) býr hana enn
til.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**Útgáfa forritsins, ein á fyrstu línu** — `subedit <version>`, það sem úttakið var þar til
0.15.38 og það sem skripta les. Síðan **ein lína fyrir hvert safn sem tólið keyrir gegn**, síðan uppsett
viðmótstungumál, á staðalúttak; tólið stoppar, með kóða `0`. Undirskipun sem er gefin að auki er
ekki keyrð, og `-q` og `-v` breyta engu.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 1.0.3
```

Allt úttakið, á vél með ICU 74.2 og Enchant 2.3.3 — tölurnar eru vélarinnar:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Safn | Til hvers tólið notar það |
| :------ | :------------------------ |
| `ICU` | að lesa og skrifa stafakóðanir, greina þær og breyta stafstöðu bókstafa |
| `Enchant` | stafsetningarathugun `correct` |
| `CLI11` | að lesa þessa skipanalínu |

- **Hver lína gefur tvær útgáfur**: þá sem fannst við keyrslu, fyrst, og þá sem forritið var
  þýtt gegn, innan sviga. Þær eru ólíkar þegar safn vélarinnar var uppfært eftir að
  forritið var smíðað — fyrsta atriðið sem á að gefa í villuskýrslu.
- **Hausar Enchant bera enga útgáfu**: sú innan sviga er sú sem `pkg-config` tilkynnti
  þegar smíðin var stillt.
- **CLI11 er þýtt inn í forritið**, hausar þess eru allt sem til er af því: útgáfurnar tvær eru
  alltaf þær sömu.
- **Hvorki Qt né libmpv**: skipanalínan tengist hvorugu og telur aðeins upp það sem hún keyrir gegn.
  Glugginn telur þau upp — sjá [handbók hans](../subedit-gui/invocation.md#--version).
- **`Languages`** telur upp `en`, frumtungumálið, sem þarf enga þýðingaskrá, síðan hverja þýðingaskrá
  sem er uppsett undir `<prefix>/share/subedit/locale` og er læsileg, eftir kóða — tungumálin sem
  `Edit ▸ Preferences…` býður.
- **Það sem fylgir tungumálinu**: `built with` og `Languages`. Fyrsta línan, nöfnin,
  tölurnar og tungumálakóðarnir gera það aldrei.
- **Með `--format json`**, á undan eða eftir `--version`, **einn hlutur á einni línu**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — fylki af
  `{"name":"ICU","loaded":"74.2","built":"74.2"}`, í röð textans — og `languages`, fylki
  af kóðum. Ekkert í því er þýtt.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**Stafakóðanirnar sem tólið getur lesið og skrifað, ein á línu, raðað eftir heiti**, á staðalúttak —
síðan stoppar tólið, með kóða `0`, eins og `--version`: þetta er svarið, ekki skref. Undirskipun sem er
gefin að auki er ekki keyrð.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **Þetta eru heitin sem `--encoding` tekur við**, þau viðurkenndu; samheiti þeirra (`cp1252`) eru líka tekin gild,
  án þess að vera talin upp.
- **Fjöldinn fer eftir uppsettu ICU**, og ekkert heiti er lofað umfram þau sem hvert ICU hefur
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). Handbókin gefur því engan fjölda.
- **`UTF-16` og `UTF-32` eru ekki í honum**: ICU þekkir þau en umbreytir þeirra skrifar eigið merki
  og líkanið hafnar þeim — sjá hér á eftir. Listi til að velja úr býður ekki það sem næsti
  reitur myndi hafna.
- **Með `--format json`**, **einn hlutur fyrir hverja stafakóðun**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. Þetta er ekki skýrsla skráar: þessir
  hlutir **bera hvorki `file` né `ok`**, eina undantekningin frá umgjörð
  [uppbyggingarinnar](#uppbyggingin). Þeir eru þó enn einn hlutur á línu, einn fyrir hverja færslu.
- **Stig frásagnar breytir engu**: listi er niðurstaða og `-q` tekur hann ekki burt.

### `--encoding`, og það sem hann getur ekki þvingað fram

**Hann gildir um hverja undirskipun**, því að þær lesa allar: skrá þar sem
stafakóðun var giskað rangt helst röng hvað sem er gert við hana, og valkostur
sem aðeins `inspect` bæri myndi skilja eftir misræmi án úrræðis.

**Heitið er frá ICU og öll samheiti þess eru tekin gild**: `cp1252`,
`windows-1252` og `WINDOWS 1252` nefna sömu stafakóðun. Mengið er hvergi
skrifað: það er það sem ICU á vélinni getur umbreytt, og
[`--list-encodings`](#--list-encodings) telur það upp. Heiti sem það getur ekki umbreytt er
hafnað áður en ein einasta skrá er lesin:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Ein undantekning, og aðeins ein: stafakóðunin verður að nefna bætaröð sína.**
`UTF-16` og `UTF-32` eru heiti sem ICU þekkir og umbreytir þeirra skrifar **eigið
merki**. Slík stafakóðun getur ekki lifað í þessu líkani — merkið hættir að
vera val, `--no-bom` yrði tekinn gildur og óhlýðnast — svo að báðum er
hafnað, bæði við lestur og ritun:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` og `UTF-16BE` lesa og skrifa sömu bæti, merki meðtalið; það sem
nákvæma heitið bætir við er að maður veit hvort.

**Bætaraðarmerki sigrar valkostinn.** Það er það eina sem skjátextaskrá
lýsir yfir um sjálfa sig, og að lesa hana öðruvísi en hún lýsir sér yfir væri
að hlýða þeim sem kallar gegn skránni. Misræmið er **sagt**, sem
greiningarboð:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

Þetta er það sem Gaupol gerir — það hefur lesturinn upp á nýtt með stafakóðun
merkisins — en án þess að segja frá því; hér er bilið milli þess sem var beðið um og
þess sem var gert ekki þögn.

## Undirskipanir

| Undirskipun | Hvað hún gerir | Skrifar hún? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | tilkynnir úr hverju skrá er gerð, án þess að breyta neinu | nei |
| [`convert`](convert.md) | skrifar skrá á öðru sniði, eða í annarri mynd | já |
| [`shift`](shift.md) | færir hverja staðsetningu um sömu stærð | já |
| [`transform`](transform.md) | leiðréttir hverja staðsetningu út frá tveimur punktum sem vitað er að eru réttir | já |
| [`framerate`](framerate.md) | tímasetur skrá upp á nýtt frá einni rammatíðni til annarrar | já |
| [`snap`](snap.md) | færir hvert tímamerki á næsta ramma | já |
| [`hearing-impaired`](hearing-impaired.md) | fjarlægir tilvísanir fyrir heyrnarskerta | já |
| [`adjust`](adjust.md) | lagar tímalengdir að lestrarhraða og mörkum | já |
| [`replace`](replace.md) | skiptir út texta í skjátextunum, án þess að brjóta merki | já |
| [`case`](case.md) | setur textana í upphafsstafi, setningastafi, hástafi eða lágstafi | já |
| [`italics`](italics.md) | setur textana í skáletur, eða tekur skáletrið úr þeim | já |
| [`dialogue-dashes`](dialogue-dashes.md) | setur þankastrik samtala á, eða tekur þau af | já |
| [`sort`](sort.md) | kemur skjátextunum aftur í röð upphafs þeirra | já |
| [`join-repeats`](join-repeats.md) | gerir einn skjátexta úr hlutum eins texta sem fylgja hver öðrum | já |
| [`disjoint`](disjoint.md) | skerðir skjátexta sem skarast þannig að ekkert tímabil skarist við annað lengur | já |
| [`repair`](repair.md) | keðjar `join-repeats`, `sort` og `disjoint` | já |
| [`correct`](correct.md) | leiðréttir textana með mynstrum Gaupol: tilvísanir, sameiningu og skiptingu orða, algengar villur, hástafi, línuskiptingu | já |
| [`append`](append.md) | setur skrár aftan við þá fyrstu, í **eina** skrá | já |
| [`split-file`](split-file.md) | skiptir skrá í **tvær**: haus og hala | já |
| [`pair`](pair.md) | skrifar þýðingu á staðsetningar aðalskrár hennar og segir hvernig línur hennar röðuðust | já |
| [`config`](config.md) | telur upp, sýnir og breytir aðgerðastillingum stillingaskrárinnar | aðeins stillingaskrána |

Þær tuttugu og ein eru allar hér; hjálp tólsins telur þær upp í sömu röð.

**Aðeins ein snertir ekkert**, og það er hægri dálkurinn: `inspect` les og
tilkynnir. Hinar nítján skrifa, og það sem á eftir fer gildir um sautján þeirra:
[`append`](append.md) og [`split-file`](split-file.md) hafa hvor sína eigin fjölda — N skrár fyrir
**eina** niðurstöðu, ein skrá fyrir **tvær** — og þar með sinn eigin áfangastað, sem þær lýsa á sinni síðu.
[`config`](config.md) skrifar enga skjátextaskrá: hún les og breytir stillingaskránni og
ekkert af því sem á eftir fer á við um hana.

## Áfangastaðurinn

Undirskipanirnar sautján sem skrifa — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md) — taka við áfangastað sínum á sama hátt. ([`append`](append.md) og
[`split-file`](split-file.md) hafa sinn eigin, á sinni síðu.) [`inspect`](inspect.md) skrifar enga skrá og
tekur við engum þessara valkosta.

**Áfangastaður sem er þegar til er yfirskrifaður**, án spurningar og án valkosts til að
komast hjá því: ritunin er lotubundin — skráin er aldrei hálfskrifuð og
rofin ritun skilur gömlu skrána eftir óskerta —, en fyrra innihald
tapast. Þetta er ekki galli sem er látinn standa: þetta er reglan, og að keyra skipun
aftur á sömu úttaksmöppu keyrir hana aftur yfir sjálfa sig.

**Aldrei er neitt skrifað án skýrs áfangastaðar.** Leiðirnar þrjár til að
gefa hann útiloka hver aðra og eina þarf: án hennar er ekkert skrifað og skilakóðinn
er `1` — nema með [`--dry-run`](#að-sjá-áður-en-skrifað-er), sem skrifar ekkert
og krefst því einskis.

| Valkostur | Hvert niðurstaðan fer |
| :----- | :---------------- |
| `--output FILE` | í þessa skrá, undir þessu nákvæma heiti; ein inntaksskrá |
| `--output-dir DIR` | í þessa möppu, undir grunnheiti inntaksins |
| `--in-place` | yfir inntakið, með lotubundinni ritun |

Tól sem yfirskrifar inntak sitt vegna þess að því var ekkert sagt er tól sem maður
hættir að nota í annað sinn. Höfnunin kostar eina línu og bjargar
skránni.

Valkostinum `--output` með mörgum inntaksskrám er hafnað: síðasta skráin yrði skrifuð yfir
fyrri skrárnar. Með runu er `--output-dir` sá eini af þremur sem
á við, ásamt `--in-place`.

**Úttaksmappan er búin til** ef hana vantar, ásamt yfirmöppum — fyrir
`--output-dir`, og fyrir möppuna sem `--output` skrifar í. Það er gert þegar
skipanalínan hefur verið staðfest og **áður en fyrsta skráin er lesin**. Ef það tekst
ekki er það sagt einu sinni, engin skrá er lesin og skilakóðinn er
`2`.

**Runan er dæmd í heild áður en skrifað er.** Áfangastaður hverrar inntaksskrár
er reiknaður fyrst og tvær höfnunir leiða af því — notkunarvillur, kóði
`1`, **engin skrá skrifuð** og engin mappa búin til:

- **tvær inntaksskrár sem enda í sömu skrá** — `a/film.srt` og
  `b/film.srt` með `--output-dir`, eða `a/film.srt` og `b/film.vtt` sem báðar eru
  umbreyttar í WebVTT, þar sem lokaáfangastaðurinn skipti um endingu. Skilaboðin
  nefna áfangastaðinn og báðar inntaksskrárnar; að endurnefna þá seinni eða
  láta þá síðustu vinna myndi tapa skrá án þess að segja frá því, og að hafna
  kostar eina línu;
- **áfangastaður sem er ein inntaksskránna**, án `--in-place` — líka
  í gegnum `--output-dir .`, í gegnum `--output` eða í gegnum táknrænan tengil. Að skrifa yfir
  inntak sitt er athöfn sem maður nefnir.

Tvær slóðir nefna sömu skrána þegar kerfið segir það, ekki þegar þær eru
ritaðar eins: `in/../in/film.srt` og `in/film.srt` eru ein skrá.

**Endingin fylgir sniðinu sem er skrifað.** Hún breytist aðeins hjá
[`convert`](convert.md), einu undirskipuninni sem breytir sniði; hinar
halda sniði skrárinnar sem var lesin og þar með endingu hennar.

| Hvað kallar þær fram | Skilaboð |
| :------------------ | :------ |
| enginn áfangastaður, án `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| tveir áfangastaðir | `--output, --output-dir and --in-place exclude one another` |
| `--output` á runu | `--output names one file but several were given: use --output-dir instead` |
| tvær inntaksskrár, einn áfangastaður | `<destination>: would be written by both <input> and <input>` |
| inntak yrði yfirskrifað | `<destination>: would be written over the input <input>` — sama setning fyrir [`append`](append.md) og [`split-file`](split-file.md) |
| úttaksmappa sem ekki er hægt að búa til | `<directory>: cannot be created: permission denied`, eða `: cannot be created` |
| áfangastaður sem ekki er hægt að skrifa | `<path>: <destination>: cannot be written: permission denied`, eða `: cannot be written` |
| stafur sem vantar í stafakóðunina sem er skrifuð | `<path>: holds a character the chosen encoding cannot write` |

**Sú síðasta á skilið setningu.** Skrá er skrifuð til baka í **þeirri stafakóðun sem hún
var lesin í**, eins og með línulok hennar og merki. Ef breyting
kom inn í hana staf sem þessi stafakóðun þekkir ekki — `ł` í Latin-1 —,
bilar ritunin og **ekkert er skrifað**. Að skipta honum út
fyrir `?` væri að tapa texta fyrir augum þess sem skrifaði hann nýlega.

## Að sjá áður en skrifað er

**`--dry-run`** er tekið gilt af hverri undirskipananna sextán sem skrifa og þýðir það sama
alls staðar: **lesa, reikna, tilkynna, skrifa enga skrá**. Maður bætir því við
línuna sem maður ætlaði að keyra; að beita er að keyra sömu línu aftur án þess. Útreikningurinn er
ákvarðandi: það sem `--dry-run` sýndi er það sem næsta keyrsla mun skrifa, svo lengi sem
skráin hefur ekki breyst.

- **Það skrifar enga skrá og býr til enga möppu**, `--in-place` meðtalið: inntaksskráin
  helst eins og hún var.
- **Það krefst engis áfangastaðar.** Einn má gefa, og **hann er þá athugaður eins og án
  `--dry-run`**: tvær inntaksskrár fyrir sama áfangastað, inntak sem yrði yfirskrifað,
  `--output` á runu, tvær leiðir til að segja hvert — skipanalínu sem er hafnað án `--dry-run`
  er hafnað með því, með sömu setningu og sama kóða `1`.
- **Skilakóðinn er sá sami og í raunverulegri keyrslu**: `0` þegar allt var reiknað, `2` eða `3`
  þegar ekki var hægt að lesa eða vinna skrár, `1` fyrir notkun. Stafur sem
  stafakóðun skrárinnar gat ekki skrifað lætur þurrkeyrslu bila eins og raunverulega keyrslu.
  **Að breytingar séu til breytir ekki kóðanum**: kóðarnir fjórir segja hvort tólið
  tókst, ekki hvort skráin er í samræmi.
- **Frásögnin segir frá því.** Þar sem raunveruleg keyrsla skrifar `-> <destination>` skrifar þurrkeyrsla
  `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**Á undirskipun sem breytir textum** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) og [`correct`](correct.md) —, **ber staðalúttakið fyrirhugaðar
breytingar**. Ein blokk fyrir hvern skjátexta sem breytist, í röð skráarinnar:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- **númerið** er það sem skjátextinn hefur í skránni sem var lesin, talið frá einum;
- hver lína textans fyrir ber `- ` fyrir framan, hver lína textans eftir `+ `: texti
  með mörgum línum lesast eins og hann er, og lína sem sjálf byrjar á þankastriki
  samtals gefur `- - `;
- **skjátexti sem aðgerðin myndi fjarlægja** segir það á fyrstu línu sinni,
  `<path>: subtitle <number> (removed)`, og hefur engar `+` línur;
- skjátexti í þýðingunni, með `--document translation`, tilkynnir sig `(translation)` á sama
  stað — sjá [Þýðing](#þýðing);
- fyrsta lína hverrar blokkar byrjar á slóðinni, svo að `grep '^film.srt: '`
  finnur blokkir einnar skrár í runu;
- **engin blokk** þegar ekkert myndi breytast.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt
mentions.srt: 2 subtitles cleaned, 1 removed (dry run, nothing written)
mentions.srt: subtitle 1 (removed)
- [Bruit de pas]
mentions.srt: subtitle 2
- Attends [il tousse] Marie.
+ Attends Marie.
mentions.srt: subtitle 3
- - [Grincement]
- - Qui est là ?
+ Qui est là ?
```

**Listinn er sá sami á JSON-sniði**: hlutur skráarinnar ber fylkið `changes`, sjá
[uppbygginguna](#uppbyggingin), með `after` sett á `null` fyrir skjátexta sem er fjarlægður. **Hann er aðeins
reiknaður þegar `--dry-run` eða `--format json` biður um hann**: runa með þúsundum
skjátexta býr hann ekki til að óþörfu.

Á **hinum undirskipununum** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) —, ber
staðalúttakið það sem úttak raunverulegrar keyrslu myndi bera, það er ekkert á textasniði; á
JSON-sniði, hlut hverrar skrár með `"dry_run":true`, `"destination":null` og sömu
`counts`.

## Tvö úttök, tvö hlutverk

| Úttak | Hvað það ber |
| :----- | :--------------- |
| staðal | **niðurstöðuna, og aðeins niðurstöðuna** — skýrslu `inspect`, fyrirhugaðar breytingar textaundirskipunar með [`--dry-run`](#að-sjá-áður-en-skrifað-er); með `--format json`, einn hlut fyrir hverja skrá — einn fyrir hverja keyrslu hjá [`append`](append.md), [`split-file`](split-file.md) og [`pair`](pair.md) |
| villa | **allt annað** — frásögnina, aðvaranirnar, villurnar |

Það er þessi skipting sem leyfir manni að beina niðurstöðunni annað án þess að fá söguna með:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

Skýrslan fer í skrána, frásögnin helst á skjánum.

## Úttak sem skriftur geta lesið

**`--format json`** kemur í stað texta staðalúttaksins fyrir **einn JSON-hlut fyrir hverja
inntaksskrá**, á sniðinu JSON Lines: einn þéttur hlutur á línu, á UTF-8 án
merkis, línulokin eru `\n`. `text` er sjálfgefið og breytist ekki. Þetta er
altækur valkostur og er því ritaður á undan undirskipuninni; gildi annað en
`text` og `json` er notkunarvilla, kóði `1`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?"
a.srt: 1 subtitle shifted by 1.000 s -> sortie/a.srt
{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":false,"destination":"sortie/a.srt","counts":{"subtitles":1,"shifted_by_ms":1000},"warnings":[]}
absent.srt: does not exist
{"schema":1,"command":"shift","file":"absent.srt","ok":false,"error":{"kind":"not-found","message":"absent.srt: does not exist"}}
1 of 2 files shifted, 1 failed
code=3
```

**Einn hlutur fyrir hverja inntaksskrá, aldrei núll, aldrei tveir**, hvort sem inntakið tókst eða ekki, í
þeirri röð sem inntaksskrárnar voru gefnar — eða fundnar, með [`--recursive`](lots.md). Maður
telur línurnar, maður finnur skrár sínar. **Þrjár undirskipanir eru undantekning, því að fjöldi þeirra
er ekki fjöldi runu: [`append`](append.md) (N inntaksskrár, eitt úttak), [`split-file`](split-file.md)
(ein inntaksskrá, tvö úttök) og [`pair`](pair.md) (tvær skrár, eitt úttak) skrifa **einn hlut fyrir alla
keyrsluna**, sem nefnir skrána sem síða þeirra segir.** **Undirskipanirnar sem endurskrifa skrifa því líka
á staðalúttak**, sem samningur þeirra gerði ekki á textasniði: skráin
er áfram niðurstaða þeirra og hluturinn er skýrslan um hana.

**Frásögnin breytist ekki.** `-q`, `-v`, `-vv` og `-vvv` verka á villuúttakið,
á textasniði, eins og án `--format`; **staðalúttakið breytist ekki með þeim**.
Greiningarboð lesturs, sem textinn heldur á stigi 3, eru gögn og eru
**alltaf** í `warnings`. Notkunarvilla skrifar engan hlut: hún greinist
áður en nokkur skrá er snert, svo að ekkert er til að tengja hana við; hún fer á
villuúttakið með kóða `1` og staðalúttakið helst tómt.

### Uppbyggingin

| Lykill | Innihald | Til staðar |
| :-- | :------ | :------- |
| `schema` | heil tala, útgáfa þessarar uppbyggingar — `1` | alltaf |
| `command` | heiti undirskipunarinnar | alltaf |
| `file` | slóðin **eins og hún var gefin**, eða eins og yfirferð möppu setti hana saman | alltaf |
| `ok` | rökgildi | alltaf |
| `error` | `{"kind", "message"}` | ef `ok` er false |
| `dry_run` | rökgildi: true fyrir [`--dry-run`](#að-sjá-áður-en-skrifað-er) | ef `ok` er true, á undirskipun sem skrifar |
| `destination` | slóðin sem var skrifuð, eða `null` þegar `dry_run` er true: ekkert var skrifað | sömuleiðis |
| `counts` | hlutur með **heilum tölum**, sérstakur fyrir undirskipunina | sömuleiðis |
| `changes` | fylki af `{"subtitle", "document", "before", "after"}`: textarnir sem breyttust, sjá hér á eftir | á undirskipun sem breytir textum |
| `warnings` | fylki af `{"kind", "line"?, "detail"?, "settled"?}` | ef `ok` er true |
| `alignment` | hvernig þýðing var lögð, sjá [Þýðing](#þýðing) | með `-t`, og fyrir `pair` |
| `constraints` | stillingarnar sem voru notaðar | `adjust` |
| `settings` | stillingaskráin sem var lesin, eða `null` þegar engin var lesin — sjá [Stillingaskráin](#stillingaskráin) | `adjust`, `correct`, ef `ok` er true |
| `inputs`, `tail` | einn þáttur fyrir hverja skrá sem er bætt aftan við; slóð halans | `append`; `split-file` |

`line` vantar í greiningarboð um alla skrána; `detail` vantar þegar
engu er við að bæta; `settled` er `true` þegar lesturinn **útkljáði** eitthvað — endurmyndaða
númerun —, og vantar þegar hann lét hlutina standa eins og þeir eru.

**`changes`** telur upp, í röð skráarinnar, hvern skjátexta þar sem textinn breytist:
`subtitle` er númer hans í skránni sem var lesin, talið frá einum; `document` er `"main"`
eða `"translation"`; `before` er textinn fyrir; `after` textinn eftir, eða **`null` þegar
skjátextinn er fjarlægður**. Línuskil texta eru `\n` í honum. Hann er
skrifaður fyrir raunverulega keyrslu eins og fyrir [`--dry-run`](#að-sjá-áður-en-skrifað-er), og **tómur
(`[]`) þegar ekkert breytist**: nærvera hans segir að undirskipunin telji upp breytingar sínar,
fjarvera hans að hún hafi engar.

**Engin fleytitala, hvergi.** Staðsetning eða tímalengd er heil tala af
**millisekúndum** (`…_ms`); fjöldi er heil tala; rammatíðni er **strengur**
(`"25"`, `"24000/1001"`) því að hún er nákvæm, nokkuð sem rökræn fleytitala getur ekki
geymt; þéttni nets er heil tala í **þúsundustu hlutum** — `1000` er
fullkomið net. Tvö úttök bera sig þá saman bæti fyrir bæti, án námundunar.

**Slóð sem er ekki UTF-8** — Linux leyfir það — verður ekki sýnd í JSON: hún
er skrifuð með U+FFFD í stað ógildu bætanna og hluturinn ber aðvörunina
`path-not-utf8`. Trúfesti slíkrar slóðar er ekki lofað og það er sagt.

### Hvað hver undirskipun setur í `counts`

| Undirskipun | Lyklar í `counts` |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (með formerki) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (staðsetningar sem voru færðar), `furthest_ms` (stærsta færslan) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (textar endurskrifaðir), `removed` (skjátextar fjarlægðir) |
| [`replace`](replace.md) | `replaced` (samsvaranir sem var skipt út í skjátextunum sem breyttust), `matched` (samsvaranir sem fundust) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (skjátextar þar sem textinn breyttist) |
| [`correct`](correct.md) | `corrected` (textar sem breyttust), `removed` (skjátextar fjarlægðir) — aldrei samsvaranir |
| [`sort`](sort.md) | `subtitles`, `moved` (staðirnir þar sem skjátexti breyttist) |
| [`adjust`](adjust.md) | `subtitles` (skjátextarnir sem miðað var á), `adjusted` (þeir þar sem endirinn færðist), síðan hópurinn `sacrificed`: `speed`, `minimum`, `gap`; og, utan `counts`, `constraints` — stillingarnar sem voru notaðar |
| [`convert`](convert.md) | `subtitles`, síðan það sem umbreytingin tapaði: `lost_ends` og `lost_header` (0 eða 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, síðan `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` lagt saman; og, utan `counts`, `inputs` — einn þáttur fyrir hverja skrá sem er bætt aftan við |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; og, utan `counts`, `tail` — slóð halans (`destination` er hausinn) |
| [`pair`](pair.md) | `subtitles` (þeir í skránni sem var skrifuð); og, utan `counts`, `alignment` |
| [`inspect`](inspect.md) | engin `counts`: lýsing skrárinnar, sjá síðu hennar |

### Auðkenni villna

`error.kind` er eitt af þessum; **lesandi meðhöndlar óþekkt auðkenni sem
„other“**.

| `kind` | Hvað það er |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | kerfið neitaði að lesa — eða skrifa — skrána |
| `unknown-format`, `no-subtitle-found`, `undecodable` | skráin er lesin og lætur ekki lesa sig |
| `unencodable` | stafur sem valin stafakóðun getur ekki skrifað |
| `no-byte-order-mark`, `no-frame-rate` | það sem `convert` getur ekki skrifað án þess að vera sagt það |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | aðgerð sem getur ekki átt við þessa skrá |
| `range-out-of-bounds` | `--range` sem þessi skrá nær ekki yfir |
| `no-style` | snið sem ritar engan stíl (TMPlayer, LRC), fyrir `italics` |
| `refused` | hver önnur höfnun aðgerðar |

### Hvað er lofað og hvað ekki

**Lofað svo lengi sem `schema` er `1`**: heiti lyklanna, tegund þeirra og eining; merking
`ok`, `file` og `destination`; **einn hlutur fyrir hverja inntaksskrá, í röð inntaksskránna** — einn fyrir hverja keyrslu hjá undirskipununum þremur með eigin fjölda;
auðkennin `kind` sem eru til; ákvarðanleiki — sömu inntök, sömu röksemdir,
sömu bæti.

**Ekki lofað**: **texti skilaboðanna** (`message`, `detail`), enskur fyrir mann
og leiðréttur þegar þörf krefur — skripta sem ber hann saman er biluð fyrirfram; **röð
lyklanna** í hlut, föst en ekki tryggð af JSON; tilvist **lykla sem er ekki
búið að bæta við**.

**Lesandi hunsar því óþekktan lykil** og meðhöndlar óþekkt `kind` sem „other“:
að bæta við lykli eða auðkenni breytir ekki `schema`. Að fjarlægja eða endurnefna lykil,
breyta tegund eða einingu, fjarlægja auðkenni hækkar það — það er rof,
tilkynnt í CHANGELOG og hér.

## Stig frásagnar

Hvert stig **inniheldur það fyrra**: að fara upp um eitt bætir við, skiptir aldrei
út.

| Stig | Hvernig | Hvað villuúttakið ber |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | ekkert, **nema villurnar** |
| 1 | sjálfgefið, eða `-v` | ein lína fyrir hverja skrá sem er unnin, og samantekt um leið og þær eru fleiri en ein |
| 2 | `-vv` | og það sem var þekkt: snið, stafakóðun, BOM, línulok |
| 3 | `-vvv` | og rakningarslóðin: bæti lesin og skrifuð, og **öll greiningarboð lesturs** |

**Villur eru aldrei þaggaðar, `--quiet` meðtalið.** Skipun sem bilar
þegjandi myndi ekki skilja neitt eftir nema skilakóða sinn.

Samantektin birtist aðeins frá tveimur skrám: á einni inntaksskrá myndi hún
endurtaka línuna á undan.

## Að lesa greiningarboð

Skjátextasnið eru lesin **eins vel og unnt er**: andspænis frávikinu stoppar
lesarinn ekki — hann ákveður, eða lætur hlutina standa eins og þeir eru, og segir frá því. Það sem
hann rakst á er skrifað á **stigi 3**, ein lína fyrir hvert greiningarboð, á
villuúttakið:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Allar undirskipanirnar sautján tilkynna þau, ekki aðeins [`inspect`](inspect.md):
skrá sem er lesin eins vel og unnt er og síðan skrifuð til baka fór í gegnum sömu ákvarðanir, og að halda þeim þögulum
myndi láta menn trúa að ekkert hafi gerst.

**Greiningarboð er aldrei bilun.** Skráin var lesin, skipunin
tókst, skilakóðinn er `0`. Þess vegna eru þau á nákvæmasta
stiginu: neðar myndu þau drekkja línunni sem segir hvað var í raun
gert.

### Hvað hver lína ber

| Hluti | Hvað hann segir |
| :----- | :------------- |
| `line N` | hvar, talið frá 1, eins og ritill sýnir; **vantar** fyrir það sem varðar alla skrána |
| setningin | það sem var rekist á, meðal flokkanna tólf hér á eftir |
| `("…")` | gallaði textinn úr skránni, þegar flokkurinn nægir ekki; styttur í 80 bæti |
| endirinn | hvað var gert í málinu: `settled by the reader`, eða `left as it stands` |

**Tveir flokkar hafa enga línu**, og það eru þeir tveir sem varða
stafakóðunina: bætin voru vegin, eða merki þeirra lesið, áður en ein einasta
lína skrárinnar var til. Lína þeirra byrjar því á setningunni, án `line N`
fyrir framan — enginn staður er til að nefna.

**Endir línunnar skiptir mestu máli.** `settled by the reader` þýðir
að lesarinn ákvað og að skráin sem er skrifuð ber ákvörðun hans — vantandi
númerun er endurmynduð. `left as it stands` þýðir að hann snerti
ekkert því að **aðeins þú getur ákveðið**: skjátexti sem endar áður en hann
byrjar helst eins og hann er.

### Línunúmer blokkar

Frávik um **heila blokk** er fest við tímalínu hennar,
en ekki við nákvæmu línuna sem veldur því. Ósamkvæm númerun
á línu 5 er því tilkynnt á línu 6, línu tímalínunnar sem
á eftir kemur — gallaða númerið er í `("…")`. Aðeins frávik um
**eina línu** — tímalína sem ekki er hægt að lesa, texti á undan þeirri fyrstu — eru
tilkynnt á sjálfum sér.

### Flokkarnir tólf

| Setning | Hvað kallar hana fram |
| :----- | :------------------ |
| `a line that fits nowhere` | lína sem fer í enga blokk |
| `a timing line that could not be read` | tímalína sem ekki er hægt að lesa |
| `a SubRip block without its number` | númerun vantar, endurmynduð við ritun |
| `SubRip numbers that do not follow` | númerun sem hoppar |
| `text before the first timing line` | texti á undan fyrstu tímalínu |
| `a WebVTT block of an unknown kind` | WebVTT-blokk sem ekki er þekkt |
| `declares an event column this tool cannot fill` | atburðadálkur í Sub Station Alpha sem ekkert hér fyllir |
| `counts in frames and states no rate; it was read at` | skrá í römmum sem gefur enga rammatíðni — sú sem er notuð er smáatriðið |
| `carries no end times; each one was taken from the next start` | snið án enda, TMPlayer eða LRC: **enginn endir í töflunni kemur úr skránni** |
| `more than one kind of line ending` | blönduð línulok |
| `an encoding nothing declared` | stafakóðunin var lögð til með því að vega bætin og hún er ekki UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | skráin ber merki og `--encoding` nefndi aðra — merkið vinnur |

**Þeir þrír í miðjunni eru frá áfanga 9**, og síðustu tveir þessara
þriggja tala ekki um línu heldur um alla skrána: þeir segja að
gildi sem er sýnt hafi ekki verið lesið. `End` dálkur fylltur á LRC, staðsetning
á MicroDVD — skráin bar ekkert af því.

**Aðgerð getur bætt við sínum eigin**, sem eru ekki greiningarboð lesturs:
[`correct`](correct.md#mynstur-sem-getur-ekki-átt-við) segir, með skjátextanum, mynstrið
sem gafst upp. Það er sagt á stigi 1 og ber, í hlut skráarinnar, auðkennið
`pattern-failed` — aldrei bilun skráarinnar.

## Þýðing

Þýðingarskrá er unnin **eins og hver önnur skjátextaskrá**:
`subedit-cli case --to sentence film.fr.srt` breytir henni án nokkurs nýs. Það sem pörun bætir við, og það sem
þrír valkostir segja, er þröngt: **að vita hvernig línur þýðingar falla á
skjátexta aðalskrárinnar** — það er `inspect -t` —, og **að breyta þýðingunni og halda henni
lagðri á aðalskrána**, með sömu reglum og glugginn.

| Valkostur | Hvar | Gildi | Sjálfgefið |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | þýðingarskráin | — |
| `--document` | textaundirskipanirnar sex, ekki `inspect` né `pair` | `main` eða `translation` | `main` |
| `--align-method` | eins og `-t` | `position` eða `number` | `position` |

- **`position`** ber miðju hverrar línu saman við mörk skjátextanna: lína sem vantar færir ekki
  þær sem á eftir koma. **`number`** sendir línu *n* á skjátexta *n*, án þess að líta á
  staðsetningarnar. Staðsetning er sjálfgefin, af þeirri ástæðu sem glugginn hefur: lína sem vantar í
  miðjuna færir allan afganginn af þýðingu sem er pöruð eftir númeri.
- **`--document translation` krefst `-t`, og `-t` krefst `--document translation`** á
  textaundirskipun — annað án hins er notkunarvilla, kóði `1`, því að skrá sem er lesin og engin athöfn
  notar er yfirsjón en ekki val. `-t` á `inspect` hefur ekkert `--document`: það
  tilkynnir. **`--align-method` án `-t` er líka notkunarvilla.**
- **`-t` nefnir skrá og á því aðeins við um eina inntaksskrá**: með mörgum inntaksskrám, eða
  með möppu sem `--recursive` tekur, er því hafnað eins og `--output`, **áður en skrá er
  lesin**. Fyrir runu, keyrðu eitt kall fyrir hvert par.
- **Þýðingarskrá sem er eins og aðalskráin er hafnað** — hvernig sem slóð hennar er
  rituð: `<translation>: the file is already open as the main document`, kóði `2`.
- **Aðeins skjalið sem er miðað á er skrifað**, á **sinni eigin slóð og sínu eigin sniði**: sniði,
  stafakóðun og línulokum þýðingarskrárinnar. `--output` nefnir skrá
  þýðingarinnar, `--output-dir` setur grunnheiti hennar þangað, `--in-place` endurskrifar hana; **aðalskráin
  er aldrei snert**. Slóðirnar í skýrslunni eru slóðir þýðingarinnar.
- **Skjátextarnir sem pörun vekur til lífs** — lína sem ekkert náði til — eru skrifaðir í
  þýðinguna með **sínum eigin staðsetningum**, hver sem aðferðin er; **skjátexti sem enginn
  þýddi er skrifaður sem blokk án texta**, sem les án nokkurs fráviks.
- **`--encoding` og `--frame-rate` gilda um báðar skrárnar.**
- **Staðsetningar**-undirskipanirnar — `shift`, `transform`, `framerate`, `snap`, `adjust` — hafa ekki
  þessa valkosti: gefðu þeim báðar skrárnar, eins og fyrir runu,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **Að skrifa þýðinguna tímasetta upp á nýtt á aðalskrána** — það eina sem pörun gerir og skrá ein og sér
  gerir ekki — er verk [`pair`](pair.md), sem hefur hvorki `--document` né `--recursive`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt
film.fr.srt: 1 subtitle cleaned, 0 removed -> propre.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Með `--format json` ber hlutur hverrar skrár líka lykilinn `alignment`, sem segir hvernig
þýðingin var lögð — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, eins og
lykillinn `translation` hjá [`inspect`](inspect.md#á-json-sniði) — og, **með `--dry-run`**, telur `changes` upp
skjátexta þýðingarinnar, hvern með `"document":"translation"`.

## Margar skrár

Hver undirskipun tekur við mörgum slóðum, nema [`append`](append.md), [`split-file`](split-file.md) og
[`pair`](pair.md), þar sem fjöldinn er sá sem er á síðu þeirra. Hver skrá er unnin
sjálfstætt: bilun einnar truflar ekki hinar, og bilanirnar
eru tilkynntar með nafni skrárinnar og ástæðunni.

**Mappa stendur fyrir skrárnar sem hún inniheldur, með `--recursive` (`-r`)**:
án þessa valkosts er notkunarvilla að gefa hana, kóði `1`, og ekkert er
unnið — að hafna henni er betra en að vinna ekkert og skila `0`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

Yfirferðin, tréð sem er varðveitt undir `--output-dir` og það sem er látið
liggja er lýst í [Vinnsla á möpputré](lots.md).

## Man-síðurnar

Uppsetning setur **eina man-síðu fyrir hverja undirskipun** við hlið `subedit-cli(1)`:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` og hinar, eina fyrir hverja línu í
[töflu undirskipananna](#undirskipanir). `subedit-cli(1)` geymir þennan kafla — altæku valkostina,
úttökin, skilakóðana — og hver önnur síða geymir kafla undirskipunar sinnar, með dæmum
hennar. Síða gluggans, `subedit-gui(1)`, er við hliðina á þeim.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**Síðurnar eru ekki skrifaðar óháð þessari handbók**: þær eru búnar til úr henni þegar
forritið er smíðað, svo að þær segja það sem hún segir og geta ekki vikið frá
henni. Hlutinn SKRÁR á hverri síðu nefnir möppuna sem handbókin er sett upp í.

| Síður | Hvar þær eru settar upp |
| :---- | :----------------------- |
| enska | `share/man/man1` |
| hvert uppsett tungumál — katalónska, tékkneska, þýska, spænska, finnska, franska, galisíska, hebreska, ungverska, interlingue, íslenska, ítalska, hollenska | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` velur tungumálið út frá staðfærslunni**, eins og forritið: undir franskri staðfærslu
sýnir `man subedit-cli` frönsku síðuna, og tungumál þar sem síðurnar eru ekki uppsettar fær
þær ensku. `-L` biður um tungumál hvað sem staðfærslunni líður:

```console
$ man -L fr subedit-cli-shift
```

## Skilakóðar

| Kóði | Merking |
| :--- | :------------ |
| `0` | allt tókst |
| `1` | notkunarvilla — óþekktur valkostur, ógilt gildi, bönnuð samsetning |
| `2` | ekki var hægt að vinna neina skrá |
| `3` | sumar skrár voru unnar, aðrar ekki |

`2` og `3` eru aðgreind svo að skripta viti hvernig á að bregðast við án þess að lesa úttakið aftur:
„ekkert virkaði“ og „ein vantar“ kalla ekki á sömu viðbrögð.

Notkunarvilla greinist **áður en nokkur vinnsla hefst**: hún skilur aldrei eftir
runu hálfunna.

## Villur

| Hvað kallar þær fram | Hvað er skrifað, á villuúttakið |
| :------------------ | :--------------------------------------- |
| óþekktur valkostur eða undirskipun | gallaða heitið og vísun í `--help` |
| `--quiet` með `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` með `--no-config` | `--config excludes --no-config` |
| `--config` með tómu heiti | `--config needs a file name` |
| skrá vantar | `<path>: does not exist` |
| skrá ekki læsileg | `<path>: cannot be opened: permission denied` |
| stafakóðun sem skrifar eigið merki | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| bæti sem ekki afkóðast | `<path>: cannot be decoded in the chosen encoding` |
| snið ekki þekkt | `<path>: is in no format this tool knows` |
| ekkert sem líkist skjátexta | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` án `-t` | `--document translation needs the translation file: use -t` |
| `-t` án `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` án `-t` | `--align-method needs a translation file: use -t` |
| `-t` með mörgum inntaksskrám, eða möppu | `-t names one file but several inputs were given: use one invocation per pair` |
| þýðing eins og aðalskráin | `<translation>: the file is already open as the main document` |
