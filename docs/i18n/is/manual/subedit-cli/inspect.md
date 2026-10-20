# `inspect`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Tilkynnir úr hverju hver skrá er gerð. **Breytir engu og skrifar enga
skrá.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Röksemdir

| Röksemd | Nauðsynlegt | Gildi | Sjálfgefið |
| :------- | :----- | :----- | :----- |
| `<file>...` | já | ein eða fleiri slóðir skjátextaskráa | — |
| `--recursive`, `-r` | nei | fáni | slökkt — sjá [Vinnsla á möpputré](lots.md) |
| `--detail` | nei | fáni: nefnir hvert frávik í stað þess að telja þau — sjá [Hvað `anomalies` tilkynnir](#hvað-anomalies-tilkynnir) | slökkt |

Engin slóð er notkunarvilla, og því kóði `1`.

## Valkostir

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | nei | tíðni í römmum á sekúndu: `25`, `23.976`, `29.97` | 23,976 |

**Hann nýtist aðeins skrám sem eru taldar í römmum**, það er MicroDVD, eina
sniðið af níu. MicroDVD-skrá ber ekkert nema rammanúmer; tíðnin
sem þau eru lesin með ræður **hverri staðsetningu í skýrslunni**,
fyrst `span`. Ef hann er gefinn með öðrum sniðum breytir hann engu og er ekki
villa.

Gildi sem nefnir ekkert er notkunarvilla, og því kóði `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

Þetta er sami valkosturinn og hjá [`convert`](convert.md), orð fyrir orð, því að
þetta er sama tíðnin. `convert` notar hann líka til að **skrifa**.

## Þýðing: `-t`

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | nei | þýðingarskrá, fyrir **eina** inntaksskrá | — |
| `--align-method` | nei, og aðeins með `-t` | `position` eða `number` | `position` |

Með `-t` er þýðingin lögð á skrána **eins og glugginn gerir þegar hann opnar hana** — sjá
[Þýðing](invocation.md#þýðing) — og skýrslan segir **hvernig línur hennar fundu
skjátexta sinn**. Skripta veit þannig hvort þýðing er samstillt áður en hún er leiðrétt.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

Fyrsta línan sem bætist við nefnir skrána og aðferðina; sú seinni er **setningin sem glugginn
segir** þegar hann opnar þýðingu, hver hluti aðeins þegar hann er ekki núll:

| Hluti | Hvað hann telur |
| :----- | :---------------- |
| `N lines attached` | línur þýðingarinnar sem fundu skjátexta í aðalskránni |
| `N subtitles born of a line` | línurnar sem fundu engan og fæddu af sér eigin skjátexta, á eigin staðsetningum |
| `N subtitles left without a translation` | skjátextar aðalskrárinnar sem engin lína náði til |
| `N lines out of order` | línurnar sem komu á eftir síðari línu í skránni |

Þýðing sem ber enga línu segir `translation: the file holds no line`. **Skýrsla
skrárinnar sjálfrar er áfram skýrsla skrárinnar einnar**: skjátexti sem fæðist af línu er ekki talinn í
`subtitles`.

Villur valkostarins `-t` eru villur lesturs, sagðar með slóð þýðingarinnar, og þessi:
`<translation>: the file is already open as the main document`, þegar skráin sem er gefin er aðalskráin.
Valkosturinn `--align-method` án `-t` er notkunarvilla, og það er `-t` með nokkrum inntaksskrám líka — sjá
[Þýðing](invocation.md#þýðing).

## Úttak

Skýrslan fer á **staðalúttak** — hún er niðurstaða skipunarinnar og
er skrifuð á hverju stigi, `--quiet` meðtöldu. Ein blokk fyrir hverja skrá, á undan fer
slóðin:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Svið | Hvað það segir |
| :---- | :----------- |
| `format` | eitt af sniðunum sem `--to` tekur við, ritað í heild: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | stafakóðunin sem var lesin, og á eftir **hvaðan svarið kemur**: `, from its byte order mark` ef skráin lýsir henni yfir, `, as asked for` ef `--encoding` setti hana, `, detected` ef hún var vegin |
| `byte order mark` | `present` eða `absent` |
| `line endings` | `LF`, `CRLF` eða `CR`, og á eftir `, mixed from line N` ef skráin blandar þeim |
| `subtitles` | fjöldi skjátexta sem voru lesnir |
| `span` | frá fyrsta upphafi til síðasta enda, `HH:MM:SS.mmm` |
| `frame rate grid` | rammatíðnin sem staðsetningarnar voru reiknaðar út frá, **dregin ályktun um** — sjá hér á eftir |
| `frame rate` | fyrir skrá sem er talin í römmum, tíðnin sem hún var lesin með og **hvaðan hún kemur** — kemur í stað fyrri línu, sjá hér á eftir |
| `anomalies` | `none` á sömu línu, eða það sem er að: **ein tegund á línu með fjölda sínum**, inndregin undir `anomalies:`; með `--detail`, **eitt frávik á línu**, skjátexti fyrir skjátexta |

**`span` er ekki „frá fyrsta til síðasta“** heldur frá því fyrsta í tíma til þess síðasta:
á skrá þar sem röðin er brotin eru þessi tvö ólík og aðeins seinna segir
satt um það sem skráin nær yfir.

**`anomalies`, með `--detail`, telur skjátextana frá 1**, eins og skráin
númerar þá — en ekki línur skrárinnar. Númer skjátexta lifir af
breytingu; línunúmer gerir það ekki. Sjá hér á eftir.

## Hvað `frame rate grid` tilkynnir

**Skjátextaskrá lýsir ekki rammatíðni sinni yfir.** SubRip hefur engan
haus, haus WebVTT er frjáls texti. Þessi lína les því ekkert:
hún **dregur ályktun** með því að mæla hvort staðsetningarnar falli á net ramma.

Það sem er mælt eru **upphöfin**. Hvar sem net er til eru þau á því
án undantekningar, en endarnir víkja oft frá því — endir er
oft reiknaður með reglu um lestrarhraða frekar en settur á
ramma.

Fyrsta línan segir alltaf það sama: tíðnina sem var valin, dóminn
og þéttnina sem styður hann.

| Dómur | Hvað hann þýðir |
| :------ | :----------------- |
| `clean` | staðsetningarnar eru á neti |
| `partial` | hluti þeirra er á neti, afgangurinn ekki |
| `none` | enginn frambjóðandi skýrir neitt |

`none` hefur **tvær orsakir og þær eru ekki sama svarið**: annaðhvort passar ekkert
net — línan gefur þá besta frambjóðandann og einkunn hans,
svo að sjá megi hve mikið vantar — eða skráin telur of fáa
skjátexta til að nokkuð verði sagt um hana. Tvö upphöf virðast alltaf
fullkomlega samstillt; það sannar ekkert.

**Regluleg skrá á tíðni sem er ekki staðalbundin gefur `none`, og það er
ætlunin.** Mengi frambjóðenda er lokað — tíðnirnar átta sem
umbreytingin býður — og að tilkynna þá sem minnst skeikar myndi gefa
rangt svar þar sem „ég veit það ekki“ er rétta svarið.

### Línurnar sem fylgja, þegar þær hafa eitthvað að segja

| Lína | Hvenær hún birtist |
| :---- | :------------------ |
| `grid offset` | staðsetningarnar eru á netinu **að föstu frávikið undanskildu**: skránni var hliðrað og hér er hve mikið |
| `also fits` | annar frambjóðandi passar jafn vel, því að hann er heilt margfeldi þess sem var valinn |
| `too short a span to separate` | tímabil skrárinnar leyfir ekki að greina valda tíðni frá þeim sem eru nefndar |
| `off the grid` | hve mörg upphöf víkja frá netinu, og í hve mörgum **runum** |

**`also fits` er ekki hik.** Net við 25 ramma á sekúndu er
*innifalið* í neti við 50: skrá sem er tímasett á 25 passar því við bæði.
Hið gagnstæða er rangt — raunveruleg 50 hrynur á 25 — svo að sú lægsta
er valin og hin er nefnd í stað þess að þegja um hana. Einu
pörin sem koma við sögu eru 25 og 50, 29,97 og 59,94, 30 og 60.

**`too short a span to separate` ver gegn of öruggu svari.** 23,976
og 24 reka í sundur um eina millisekúndu á hverja sekúndu af kvikmynd: á kvikmynd í fullri lengd
greinast þær hafið yfir vafa, á tíu sekúndum er ekki hægt að greina þær að. Línan segir
hvor möguleikinn er enn eftir.

**`off the grid` greinir tvær sögur sem hundraðshlutinn einn ruglar saman.**
Margar runur með einu upphafi eru staðsetningar leiðréttar í höndunum,
ein í einu. Fáar langar runur eru kafli sem var tímasettur upp á nýtt eða skrá
sem var sett saman úr tveimur öðrum. Fjöldi rana er það sem skilur þær að.

### Dæmi

Tólf línur settar á net við 25 ramma á sekúndu — svo á
staðsetningar sem eru margfeldi af 40 millisekúndum — dreifðar á tæpar níu mínútur:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

Dæmið býr til sína eigin skrá í stað þess að lesa eina úr hugbúnaðarsafninu, og það er
viljandi: hægt er að endurspila það óbreytt og lykkjan sýnir hvað net er
betur en setning — staðsetningar sem eru margfeldi af tímalengd ramma.

## Hvað `frame rate` tilkynnir, og hvers vegna það er ekki net

**Aðeins eitt sniðanna níu telur í römmum**: MicroDVD, þar sem hver lína
ber tvö rammanúmer og ekkert annað. Á slíkri skrá skrifar `inspect`
ekki `frame rate grid` heldur `frame rate`, og línurnar tvær eru
aldrei sýndar saman.

**Ástæðan rúmast í einni setningu.** Staðsetningar MicroDVD-skrár voru
reiknaðar *út frá* römmum hennar, á ákveðinni tíðni; að draga þær ályktun af
neti myndi gefa sömu tíðni til baka, eins og hún var nýbúin að vera gefin. Það yrði
gefin tala klædd upp sem mæling.

Það sem er skrifað í staðinn er **lestrartíðnin og hvaðan hún kemur** —
sama form og í línunni `encoding`:

| Það sem línan segir | Hvenær |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | `--frame-rate` setti hana |
| `frame rate: 24000/1001 fps, assumed` | enginn sagði hana; hún er sjálfgefin |

**Tvö svör en ekki þrjú**, þar sem `encoding` hefur þrjú: MicroDVD-skrá
lýsir engu yfir. Það er allur vandinn — tíðnin hlýtur að koma
utan frá og skýrslan segir hvor utanaðkomandi uppsprettan það er.

**Tíðnin ræður öllum afgangi skýrslunnar.** `span` fer í gegnum hana og tvær
lesningar sömu skráar á tveimur tíðnum gefa tvö tímabil:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## Hvað `anomalies` tilkynnir

Þrennt getur verið að í skjali og hvað er lagað á sinn hátt.
**Sjálfgefið telur `inspect` þau**, ein lína fyrir hverja tegund, í röð töflunnar:
illa gerð skrá ber hundruð þeirra og að vita hve mörg eru af hverri tegund
segir hvað á að keyra. **`--detail` nefnir þau eitt og eitt.**

| Það sem er skrifað | Með `--detail` | Hvað það þýðir |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | endirinn kemur á undan upphafinu |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | hann skarast við þann á undan |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | hann brýtur röð skrárinnar — [`sort`](sort.md) kemur honum aftur á sinn stað |

Tegund sem vantar er ekki skrifuð. Í vinstri dálki er `N` fjöldi
skjátexta; í þeim næsta, númer skjátexta.

**Sami skjátexti getur birst tvisvar**, og það er ætlunin: sá sem
byrjar áður en sá á undan hefur byrjað byrjar líka áður en hann hefur
endað. Fyrra atriðið er leyst með því að færa skjátextann, það seinna með því að
breyta tímalengd.

**Skjátextinn sem er nefndur er sá sem er á röngum stað**, ekki sá sem hann er
á röngum stað gagnvart. Á upphöfum `0`, `4 s`, `2 s`, `3 s`: sá þriðji brýtur röðina, sá
fjórði fylgir þeim þriðja og er því ekki nefndur — ekkert er við því
að gera.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

Og með `--detail`, skjátextinn sem er nefndur:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**Ein lína fyrir hvert frávik með `--detail`**, því að hundrað niðurstöður
þjappað á eina línu er hvorki hægt að lesa né leita í. Skjal án galla heldur
einni línu sinni, `anomalies: none`, sem skripta leitar að. **Færslan fyrir
`--format json` listar alltaf hvert frávik**: skripta les smáatriðin og
`--detail` breytir aðeins textanum.

**Þetta er ekki greiningarboð lesturs.** Greiningarboðin segja hvað
lesturinn rakst á og vísa á **línu í skránni**; fráviksskýrslan segir
hvað skjalið *er* og vísar á **skjátexta**. Skrá þar sem röðin
er brotin er ekki gölluð fyrir það: lesturinn hefur ekkert um það að segja.

## Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) skrifar `inspect` **einn
hlut fyrir hverja skrá** í stað blokkanna hér að ofan; frásögnin helst á villuúttakinu.
Lyklarnir lýsa því sem textinn segir og **engan þeirra á að lesa út úr setningu**:

| Lykill | Innihald |
| :-- | :------ |
| `format` | auðkenni sniðsins, það sem `convert --to` tekur við: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` er `byte-order-mark`, `asked` eða `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` eða `cr`; línan þar sem önnur tegund birtist, eða `null` |
| `subtitles` | fjöldi skjátexta |
| `span_ms` | `{"start", "end"}` — fyrsta upphaf og síðasti endir, í millisekúndum |
| `frame_rate` | fyrir skrá sem er talin í römmum: `{"rate", "origin"}` (`asked` eða `assumed`); annars `null` |
| `grid` | fyrir hinar: netið sem var dregin ályktun um, eða `null` fyrir skrá sem er talin í römmum |
| `anomalies` | fylki af `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **aðeins með `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` er `position` eða `number` og tölurnar fjórar eru þær úr setningunni hér að ofan |
| `warnings` | það sem lesturinn rakst á, á hverju stigi |

Lykillinn `grid` ber `verdict` (`clean`, `partial` eða `silent`), `enough_starts`, `rate` (strengur,
eða `null` þegar ekkert er nefnt), `concentration_permille` (þúsundustu hlutar, `null` án nógu margra
upphafa), `offset_ms`, `also_fits` (tíðnin sem þessi er heill deilir í, eða
`null`), `not_separated` (tíðnirnar sem of stutt tímabil greinir ekki að), `strays`
(upphöfin utan netsins) og `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Frásögn

Á villuúttakinu, eftir því stigi sem beðið er um:

| Stig | Hvað bætist við |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, síðan **hvert þeirra** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef hver skrá var lesin, `2` ef engin, `3` ef
aðeins sumar, `1` við notkunarvillu.

## Villur

Þær sömu og við lestur — skrárinnar, og þýðingarinnar þegar `-t` er gefinn — og
þær sem tilheyra `-t`: sjá hér að ofan. `inspect` skrifar ekkert sem hægt væri að hafna.

| Hvað kallar þær fram | Skilaboð |
| :------------------ | :------ |
| engin slóð gefin | `files is required`, og á eftir vísun í `--help` |
| skrá vantar | `<path>: does not exist` |
| skrá ekki læsileg | `<path>: cannot be opened: permission denied` |
| bæti sem ekki afkóðast | `<path>: cannot be decoded in the chosen encoding` |
| snið ekki þekkt | `<path>: is in no format this tool knows` |
| ekkert sem líkist skjátexta | `<path>: holds nothing recognisable as a subtitle` |

Þetta eru sömu skilaboð fyrir undirskipanirnar sautján: uppskriftin að því að opna
er skrifuð einu sinni, í kjarnanum.

## Dæmi sem bilar

Tóm skrá passar við ekkert snið. Hún er ekki tilkynnt sem tóm:
henni er hafnað og hún nefnd.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
