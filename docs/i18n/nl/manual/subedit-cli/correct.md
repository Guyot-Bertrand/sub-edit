# `correct`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

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

Corrigeert de teksten met **de patronen van Gaupol**, zoals de assistent
[`Correct Texts…`](../subedit-gui/correct-texts.md) van het venster — dezelfde patronen,
dezelfde engine, dezelfde tellingen. Er bestaan hier zes taken: de **vermeldingen voor
slechthorenden**, woorden **samenvoegen** en **splitsen**, **veelvoorkomende fouten**,
**hoofdlettergebruik** en **regelafbreking**.

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

## Wat wordt gekozen, en wat niet

**Er is vooraf niets aangevinkt.** Het venster opent zijn vakjes zoals de gebruiker ze heeft achtergelaten; een
opdrachtregel neemt zijn taken, zijn code, zijn regellengte en zijn taal van zijn eigen regel,
nooit uit het instellingenbestand: noch de taken die op een dag in het venster zijn aangevinkt, noch de patroonvakjes
die daar zijn ingeschakeld (`correction.activations`), noch de vervanglijst van de spellingcontrole worden
gelezen, en aan die kant wordt niets geschreven.

**Vier instellingen komen standaard uit het instellingenbestand** — de waarden die `Correct Texts…`
onthoudt, of die [`config set`](config.md) heeft geschreven. **Een optie op de regel wint altijd**, daarna
het bestand, daarna de ingebouwde standaardwaarde; zie [Het instellingenbestand](invocation.md#het-instellingenbestand)
voor de plaats ervan, `--config` en `--no-config`.

| Sleutel | Wat hij aanlevert | Optie die hem overschrijft |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | de toegepaste klassen van veelvoorkomende fouten | `--classes` |
| `correction.remove-blank` | of de ondertitels die de correctie leegmaakt, worden verwijderd | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | het grootste aantal regels van `line-break` | `--max-lines` |

- **Elk van de vier heeft een optie die hem ongedaan maakt**, wat het bestand ook zegt: `--classes human,ocr`
  past beide klassen toe, `--remove-blank-subtitles` verwijdert de leeggemaakte ondertitels, `--max-lines 3`
  geeft de regels van Gaupol terug. `--keep-blank-subtitles` en `--remove-blank-subtitles` samen zijn een
  gebruiksfout.
- **Beide klassen uit in het bestand wordt genegeerd, en gemeld** op niveau 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` kan het niet schrijven, en het zou
  common-errors niets te doen laten. In plaats daarvan worden beide klassen toegepast.
- **Een aantal regels buiten 1 tot 1000 wordt helemaal niet uit het bestand gelezen**: de lezer van het bestand
  weigert het — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, op
  niveau 1 — en de ingebouwde 3 wordt gebruikt.

De voorbeelden op deze pagina worden uitgevoerd met `--no-config`, zodat ze laten zien wat de ingebouwde
standaardwaarden doen, wat de instellingen van de machine die ze uitvoert ook zijn.

| Optie | Verplicht | Waarden | Standaard |
| :----- | :----- | :------ | :----- |
| `<file>...` | ja | een of meer paden | — |
| `--tasks` | **ja** | een door komma's gescheiden lijst: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | geen taak |
| `--code` | **ja** met een patroontaak, en **niet van toepassing** anders | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **ja** met `join-words` of `split-words`, en **niet van toepassing** anders | een taalcode: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | nee | `human`, `ocr` of `human,ocr` | beide — of wat het instellingenbestand zegt |
| `--enable`, `--disable` | nee, herhaalbaar | de Engelse naam van een patroon, of `type:name` | de meegeleverde `.conf`-bestanden |
| `--max-length` | **ja** met `line-break` | een aantal **tekens** boven nul | **geen** — zie hieronder |
| `--max-lines` | nee | een geheel aantal regels, boven nul | 3 — of wat het instellingenbestand zegt |
| `--skip-length`, `--skip-lines` | nee | een getal, of `off` | dezelfde grenzen als `--max-length` en `--max-lines` |
| `--keep-blank-subtitles` | nee | een vlag: de leeggemaakte ondertitels blijven, leeg | de leeggemaakte ondertitels worden verwijderd — tenzij het instellingenbestand anders zegt |
| `--remove-blank-subtitles` | nee | een vlag: de leeggemaakte ondertitels worden verwijderd, wat het instellingenbestand ook zegt; geweigerd met `--keep-blank-subtitles` | uit |
| `--range` | nee | `N-M`, of `N-` tot het einde — zie [`adjust`](adjust.md#--range) | het hele bestand |
| `--document` | nee | `main` of `translation` — zie [Een vertaling](invocation.md#een-vertaling) | `main` |
| `-t`, `--translation-file` | met `--document translation` | een vertaalbestand, voor **één enkel** invoerbestand | — |
| `--align-method` | nee, en alleen met `-t` | `position` of `number` | `position` |
| `--recursive`, `-r` | nee | een vlag | uit — zie [Een mappenstructuur verwerken](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

### De taken

**De volgorde van de lijst maakt niets uit**: de taken worden toegepast in de
volgorde van Gaupol — de vermeldingen, daarna woorden samenvoegen en splitsen, daarna de veelvoorkomende fouten, daarna hoofdlettergebruik, daarna regelafbreking —, elk op de
tekst die de vorige heeft achtergelaten. Een ondertitel die de vermeldingen hebben leeggemaakt, speelt geen
rol in wat volgt.

| Taak | Wat ze doet |
| :---- | :-------------- |
| `mentions` | verwijdert de geluiden tussen rechte haken, en tussen haakjes wanneer `Sound in parentheses` is ingeschakeld (de veegactie van [`hearing-impaired`](hearing-impaired.md)), daarna de patronen voor liedteksten en sprekers |
| `join-words` | plakt een in tweeën geknipt woord weer aan elkaar, uit het woordenboek van `--language` |
| `split-words` | splitst twee aan elkaar geplakte woorden, uit hetzelfde woordenboek |
| `common-errors` | corrigeert de veelvoorkomende fouten: spaties, interpunctie, ligaturen, OCR-fouten |
| `capitalization` | zet de hoofdletter waar de taal die wil |
| `line-break` | knipt de regels van een ondertitel opnieuw zodat geen enkele voorbij `--max-length` gaat, waarbij de afbrekingspatronen van de code worden gewogen |

### De patroncode

**`--code` noemt de cascade**: `Zyyy` voor elk schrift, daarna het schrift, de taal
en het land — `Latn-en-US` leest de patronen van `Zyyy`, van `Latn`, van `Latn-en` en van
`Latn-en-US`. **Hij is verplicht**, omdat een opdrachtregel geen omgevingstaal heeft:
een universele standaardwaarde zou *minder* toepassen dan men denkt, zonder het te zeggen. Hij geldt voor
elke taak van dezelfde run; een taak die een eigen code wil, is een tweede
run. Een slecht geschreven code is een gebruiksfout:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

De patronen die de gebruiker in `$XDG_DATA_HOME/subedit/patterns` (of
`~/.local/share/subedit/patterns`) zet, **worden toegevoegd aan de meegeleverde patronen**, zoals in het
venster, en een meegeleverd patroon dat niet kan worden gelezen, wordt op niveau 1 gemeld: `patterns: <file>, line N (<reason>)`.

### Woorden samenvoegen en splitsen, en het woordenboek

**Deze twee taken lezen geen patroon**: ze raadplegen het **woordenboek van het systeem, via
Enchant** — een afhankelijkheid van de pakketten (`libenchant-2-2` op Debian en Ubuntu, `enchant2` op
Fedora), die `subedit-cli` met het venster deelt. Ze hebben dus noch een code noch een naam om in te stellen:
`--code` heeft geen zin zonder een patroontaak, en `--language` geen zonder een van deze twee
— de een zonder de ander opgeven is een gebruiksfout, code `1`.

- **`join-words`** plakt een woord dat het woordenboek niet kent aan het woord dat eraan voorafgaat of
  erop volgt, **wanneer één enkele richting een bekend woord geeft**: `hel lo` wordt `hello`; twee mogelijke
  richtingen, of geen, beslissen niets.
- **`split-words`** splitst een woord dat het woordenboek niet kent **wanneer precies een van zijn
  suggesties dat woord met een spatie is**: `hellothere` wordt `hello there`. Een woord met een beginhoofdletter
  gevolgd door kleine letters blijft ongemoeid: het is meestal een naam.

**Zonder woordenboek voor de taal wordt de run geweigerd**, code `1`, **voordat een bestand wordt
gelezen** en in de woorden van het venster, dat de functie grijs maakt en dat zegt — hier
weigert de opdrachtregel en zegt het, in plaats van te zwijgen:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**De vervanglijst van het venster wordt noch gelezen noch geschreven.** Ze *vult zich* door de
keuzes van de gebruiker in de spellingcontrole; een opdrachtregel heeft zo'n
keuze niet, en het lezen van de lijst zou een script laten afhangen van wat zijn auteur er ooit in heeft gezet.

### Regelafbreking

**Ze wordt in tekens gemeten, en dat is het verschil met het venster.** Het venster meet in *em* —
een breedte die op het lettertype is afgestemd, op 0.55 em per letter —, en zijn standaard 24 is een waarde
in em. Een opdrachtregel heeft geen lettertype: ze telt tekens, die van de
tekst. **De 24 als tekens overnemen zou bijna alles afknippen, zonder het
te zeggen**; er is dus **geen standaardlengte**, en `--max-length` is verplicht bij
`line-break`:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` is 3**, de waarde van Gaupol, die de eenheid niet wijzigt; de regelafbreking neemt er
een extra als drie niet genoeg zijn. **De uitzondering** — een ondertitel die al binnen de limieten valt, wordt
gelaten zoals hij is — neemt standaard **dezelfde grenzen** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), zoals Gaupol; `--skip-length off` en `--skip-lines off`
schakelen haar uit, en de tekst wordt dan weer als één geheel gelezen en opnieuw geknipt:

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

De afbrekingspatronen zijn die van de code: `Latn` breekt niet tussen een getal en zijn eenheid en niet
na een dialoogstreepje, `Latn-en` breekt niet na een lidwoord, een voorzetsel, een titel
of een bezittelijk voornaamwoord. **Onder `Zyyy` bestaat geen afbrekingspatroon**: de regels worden
alleen in evenwicht gebracht. **Deze taak wordt nooit geweigerd bij gebrek aan een actief patroon**: haar patronen
wegen waar te breken, de afbreking vindt ook zonder plaats. De vier opties hebben alleen zin met
`line-break`; zonder haar opgegeven zijn ze een gebruiksfout.

### De klassen, en schakelen op naam

**`--classes` filtert de veelvoorkomende fouten definitief**: `ocr` uitvinken haalt zijn
patronen uit de toepassing — de ligatuur `ﬁ` blijft dan `ﬁ`. De andere drie typen patronen
hebben geen klasse.

**`--enable` en `--disable` stellen een patroon in op zijn Engelse naam**, waarbij de naam de sleutel is
(ADR 0037); een naam kan meerdere records aanduiden, die dan één enkel
vakje delen. Het is nodig, en het is wat de standaardwaarden van het venster opzijzet: **de
meegeleverde `.conf`-bestanden schakelen de vermeldingspatronen uit**, zodat `--tasks mentions` alleen
niets zou toepassen. De twee patronen "Sound in brackets" en "Sound in parentheses"
sturen elk hun eigen scheidingsteken aan — de rechte haken, de haakjes — en worden door dezelfde
opties ingesteld: er een alleen inschakelen verwijdert het andere niet.

Drie dingen zijn **gebruiksfouten**, code `1`, gemeld voordat een bestand wordt gelezen:

| Wat het veroorzaakt | Melding |
| :------------------ | :------ |
| een naam die geen patroon van de opgegeven taken aanduidt, onder de opgegeven code | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| een naam die patronen van twee typen aanduidt — ze wordt dan geschreven als `type:name`, waarbij het type `common-error`, `capitalization`, `hearing-impaired` of `line-break` is | `--enable: "<name>" names patterns of several types: write type:name, …` |
| een naam die aan `--enable` en aan `--disable` wordt gegeven | `"<name>" is given to --enable and to --disable: choose one` |

**Een typefout valt op**: het venster negeert een verouderde instelling omdat een
instelling een patroon dat is verdwenen, kan overleven; een aanroep heeft geen overlevende.

**Een taak zonder enig actief patroon wordt geweigerd**, code `1` — "niets te doen" is een
antwoord dat wordt gezegd, geen stil succes:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

Hetzelfde bestand, met de veegactie voor rechte haken ingeschakeld:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Wat er wordt geschreven, en wat er wordt gezegd

Het formaat, de codering en de regeleinden van het gelezen bestand blijven **behouden**. **De
ondertitels die de correctie leegmaakt, worden verwijderd**, zoals het standaardvakje
van het venster; `--keep-blank-subtitles` laat ze leeg, en `--remove-blank-subtitles` verwijdert ze
zelfs wanneer het instellingenbestand ze behoudt. **Een leeggemaakte vertaling blijft leeg**,
en behoudt haar ondertitel.

**De telling is die van het venster** — "Edited N and removed M subtitles": gewijzigde
teksten en verwijderde ondertitels, **nooit overeenkomsten**. Een bestand waarin niets
verandert, wordt toch geschreven: een opgegeven bestemming is een geschreven bestemming.

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

### Een patroon dat niet kan worden toegepast

**Een patroon dat niet kan worden gelezen, niet kan worden omgezet of niet eindigt, wordt genoemd, met de
ondertitel waarop het het opgaf**, en hier zegt de opdrachtregel meer dan het
venster: de bevestigingspagina noemt het patroon maar één keer, zonder de tekst.

| Reden | Wat het betekent |
| :----- | :------------------- |
| `cannot be translated` | zijn expressie gebruikt wat de omzetting naar ICU weigert |
| `will not compile` | de engine weigerde zijn expressie |
| `has an invalid replacement` | zijn vervanging is niet te lezen zoals Python haar leest |
| `timed out` | de engine gaf het op bij die tekst |
| `never settled` | `Repeat` veranderde de tekst nog na honderd doorgangen |
| `grew the text too long` | de tekst ging voorbij wat een ondertitel kan bevatten |

Op niveau 1, op de foutuitvoer: 
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — zonder het einde wanneer het patroon voor elke tekst terzijde wordt gelegd. **Het nummer is dat van het
bestand**, welke taken er ook ondertitels hebben verwijderd. In JSON is het
een waarschuwing `pattern-failed` van het bestand, waarvan `detail` de zin bevat. **Een opgegeven
patroon is geen mislukt bestand**: het bestand wordt geschreven, en de afsluitcode is `0`.

## Uitvoer

**Standaarduitvoer** — niets, het resultaat is het geschreven bestand. **Met `--dry-run` zijn het de
voorgestelde wijzigingen**, één blok per ondertitel: zijn nummer, de tekst ervoor, de tekst erna —
en `(removed)` zonder regel `+` voor een ondertitel die de correctie zou verwijderen; met
`--keep-blank-subtitles` blijft hij, leeg. Zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven).

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, de patronen die niet konden worden toegepast, de patroonbestanden die niet kunnen worden gelezen — en, eerst, het gelezen instellingenbestand — `settings: read <path>` — en wat ervan is genegeerd |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, daarna elke leesmelding |

In JSON bevat `counts` `corrected` (de gewijzigde teksten) en `removed` (de verwijderde
ondertitels), en `changes` de aangeraakte ondertitels. Met `-t` zegt de sleutel `alignment` hoe de
vertaling is gelegd.
`settings` noemt het instellingenbestand dat is gelezen, of is `null` als er geen was.

## Afsluitcodes

Die van het programma: `0` als elk bestand is geschreven, `2` als geen enkel, `3` als slechts
sommige, `1` bij een gebruiksfout.

## Fouten

| Wat het veroorzaakt | Wat er op de foutuitvoer wordt geschreven |
| :------------------ | :--------------------------------------- |
| een bestand dat door `--config` wordt genoemd en niet bestaat — het eigen bestand van de gebruiker, ontbrekend, geeft de standaardwaarden | `settings file not found: <FILE>` |
| `--tasks` ontbreekt | `--tasks is required`, gevolgd door een verwijzing naar `--help` |
| een onbekende taak | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| `--code` ontbreekt | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` zonder een patroontaak | `--code is for the tasks that read patterns, and none was asked for` |
| `--language` ontbreekt bij `join-words` of `split-words` | `--language is required by the tasks that check words: <tasks>` |
| `--language` dat geen taalcode is | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` zonder een van die twee taken | `--language is for the tasks join-words and split-words, which were not asked for` |
| een taal zonder woordenboek | `no dictionary for <language>` |
| een slecht geschreven code | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| `--max-length` ontbreekt bij `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length` dat geen getal boven nul is | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines` die geen geheel aantal regels zijn | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` of `--skip-lines` onleesbaar | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| een van de vier opties zonder `line-break` | `--max-length is for the task line-break, which was not asked for` |
| een onbekende klasse | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` met `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| een foute patroonnaam | zie de tabel hierboven |
| een taak zonder actief patroon | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, de bestemming | die van [Aanroep](invocation.md#fouten) en van [Een vertaling](invocation.md#een-vertaling) |
