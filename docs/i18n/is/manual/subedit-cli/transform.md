# `transform`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Leiðréttir **allar** staðsetningar skrárinnar út frá tveimur punktum þar sem rétta gildið
er þekkt. Þetta er aðgerðin sem skjátextun á reki krefst
— rétt í upphafi, röng í lokin — þar sem [`shift`](shift.md), sem færir allt um
sama magn, getur ekkert gert.

Snið skrárinnar sem er lesin er **varðveitt**: að breyta sniði er verk
[`convert`](convert.md).

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **já** | viðmið, `<index>=<time>` | — |
| `--last` | **já** | viðmið, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | rofi: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

Engin stutt mynd er til: þessa tvo valkosti má aðeins rita
í fullri lengd.

## Að rita viðmið

Viðmið nefnir skjátexta og segir hvar **upphaf** hans á nú að lenda:

```
<index>=<time>
```

**Vísirinn er talinn frá 1**, eins og hann er sýndur og eins og SubRip-skrá
númerar. `0` er hafnað, og það er líka vísi handan síðasta
skjátextans.

**Tíminn er ritaður eins og hjá [`shift`](shift.md)** — sekúndur eða
tímamerki, aukastafaskiljan punktur, upplausnin millisekúnda:

| Ritháttur | Dæmi |
| :---- | :------- |
| sekúndur | `10`, `2.999` |
| tímamerki | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` og `--first 3=00:00:10.000` segja það sama.

`--first` og `--last` nefna tvo punkta, ekki bil. Nöfn þeirra segja
til um væntanlega notkun — annað viðmiðið í upphafi skrárinnar, hitt í lokin — en ekki
skilyrði: að víxla þeim gefur sömu niðurstöðu, og **allt utan
viðmiðanna tveggja fylgir eins og það sem er á milli þeirra**.

## Hvað hinar staðsetningarnar verða

Viðmiðin tvö ákvarða kvarðastuðul, og hver staðsetning fylgir honum:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` eru núverandi upphöf skjátextanna tveggja sem eru nefndir, `y₁`, `y₂` tímarnir
sem var beðið um. Stuðullinn er geymdur sem nákvæmt almennt brot og **námundunin
fer aðeins fram einu sinni**, að millisekúndu; það er það sem lætur viðmiðin tvö
lenda **nákvæmlega** á tímunum sem var beðið um en ekki innan þúsundasta hluta
(sjá [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Hver skjátexti helst á skjánum **í réttu hlutfalli** við hversu lengi hann var þar:
endirinn fylgir sama kvarða og upphafið. Umbreyting er ekki
færsla og hefur ekki eiginleika hennar.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

Fyrsta viðmiðið hreyfist ekki, það þriðja fer úr 9 s í 10 s: stuðullinn er
`9/8`, og annar skjátextinn — sem enginn nefndi — fer úr
`5,001` í `5,501`, það er `1000 + 4001 × 9/8` námundað einu sinni.

## Tvö viðmið sem eru það ekki

| Staða | Hvenær hún sést | Kóði |
| :-------- | :------------- | :--- |
| `--first` og `--last` nefna sama vísi | áður en nokkuð er lesið | `1` |
| vísir er handan síðasta skjátextans | þegar skráin er opnuð | `2` eða `3` |
| skjátextarnir tveir sem eru nefndir byrja á sama augnabliki | þegar skráin er opnuð | `2` eða `3` |

Öllum þremur er hafnað af sömu ástæðu: `x₂ − x₁` er núll, og tveir
punktar sem falla saman ákvarða enga línu. Að skila deilingu með núlli klæddri sem
niðurstöðu væri verra en að hafna.

Fyrsta tilvikið er **notkunarvilla** — valkostirnir tveir eru sökudólgurinn hverjar
sem skrárnar eru, svo ein skilaboð eru við hæfi, ekki ein á hverja skrá. Hin
tvö ráðast af skránni sem er opnuð og teljast því vinnslubilun.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Vísi utan marka er hafnað og **markið er nefnt**, sem segir um leið
hvað hefði mátt skrifa:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Staðsetningu fyrir upphafspunkt er hafnað

Eins og hjá [`shift`](shift.md), og af sömu ástæðu: neikvæð staðsetning
er skrifuð `-00:00:01,000`, ritháttur sem ekkert af sniðunum níu skilgreinir og enginn
lesari tekur við. Tilvikið kemur upp hér þegar skjátexti liggur **fyrir
fyrsta viðmiðið** og kvarðinn ýtir honum yfir núll. Ekkert er
skrifað, kóðinn er `2`, og skilaboðin nefna skjátextann sem er sökudólgurinn.

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem er skrifuð. Með `--dry-run` heldur ekki neitt: það er engin skrá, og frásagnarlínan segir það — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er).

| Stig | Hvað bætist við á villuúttakið |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — snið, stafakóðun, merki og línulok skrárinnar sem var lesin, sett aftur eins og þau voru |
| 3 | `<path>: N bytes read, M written`, síðan **sérhvert greiningarboð við lestur** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

Viðmiðin eru skrifuð aftur með aukastafapunkti, hvernig sem þau voru slegin inn.

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) ber hlutur hverrar skrár `counts.subtitles` (hvern skjátexta skrárinnar). Heildartaflan er í [Ræsing](invocation.md#hvað-hver-undirskipun-setur-í-counts).

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef sérhver skrá var skrifuð, `2` ef engin, `3` ef
aðeins sumar, `1` við notkunarvillu.

## Villur

| Hvað veldur henni | Skilaboð |
| :------------------ | :------ |
| viðmið án `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| viðmið sem vantar annan helminginn | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| vísir `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| vísir sem er ekki heiltala | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| ógildur tími | þær sömu og hjá [`shift`](shift.md) |
| `--first` og `--last` á sama vísi | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| vísir utan marka | `<path>: subtitle N is past the end: the file holds M subtitles` |
| tveir skjátextar sem byrja saman | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| staðsetning fyrir upphafspunkt | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**Ekkert er skrifað án eins af valkostunum þremur um áfangastað**, og villur
þeirra eru sameiginlegar undirskipununum sem skrifa:
sjá [Ræsing](invocation.md#áfangastaðurinn).
