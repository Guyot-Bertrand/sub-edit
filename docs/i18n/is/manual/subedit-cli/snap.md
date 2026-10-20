# `snap`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Setur **hvert tímamerki** — upphaf og endi — á næsta ramma þeirrar
rammatíðni sem er gefin. Ekkert er endurtímasett: hver staðsetning færist um hálfan ramma
í mesta lagi.

## `snap` er ekki `framerate`, og það sést ekki ef þeim er ruglað saman

Skipanirnar tvær hafa sama form — skrár, áfangastaður, rammatíðni — og gera andstæða hluti.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| hvað það reiknar | `t' = t x input rate / output rate` | næsta ramma, á neti `--rate` |
| um hversu mikið það færir | í hlutfalli við tíma — **nokkrar sekúndur** í kvikmynd í fullri lengd | hálfan ramma í mesta lagi, **alls staðar** |
| fyrir hvaða skrá | skrá þar sem **tímasetningin er röng** | skrá þar sem tímasetningin er **rétt** og netið rangt |

**Notkunartilvik `snap`**, útskýrt: kvikmynd með 25 römmum á
sekúndu, skjátextaskrá skrifuð á neti með 24, línur þegar
nokkurn veginn á sínum stað innan nokkurra millisekúndna. Það er ekkert að endurtímasetja
— aðeins að setja tímamerkin aftur á réttu rammana.

**Notkunartilvik `framerate`** er hitt: skrá tímasett fyrir frumeintak á
25 sem maður vill spila á afriti með 23,976, og þar sem tímasetningin skekkist um
nokkrar sekúndur undir lok kvikmyndarinnar.

Að taka annað fyrir hitt **veldur engri villu**: bæði
taka við skránni og skrifa niðurstöðu. Frásagnarlínan er það sem
greinir þau að — `snap` segir alltaf um hversu mikið það færði staðsetningarnar.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
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
| `--rate` | **já** | rammatíðni í römmum á sekúndu, rituð eins og fyrir [`framerate`](framerate.md#að-rita-rammatíðni) | — |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | rofi: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

`--rate` tekur við **hvaða gildri rammatíðni sem er**, og ekki aðeins þeim átta
stöðluðu: valið er notandans, eins og hjá `framerate`. Rammatíðni sem er núll eða neikvæð
er hafnað.

Snið skrárinnar sem er lesin er **varðveitt**: að breyta sniði er verk
[`convert`](convert.md).

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem er skrifuð. Með `--dry-run` heldur ekki neitt: það er engin skrá, og frásagnarlínan segir það — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er).

| Stig | Hvað bætist við á villuúttakið |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — snið, stafakóðun, merki og línulok skrárinnar sem var lesin, sett aftur eins og þau voru |
| 3 | `<path>: N bytes read, M written`, síðan **sérhvert greiningarboð við lestur** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

**Tölurnar tvær eru tilgangur línunnar.** Hálfur rammi á 25 römmum á
sekúndu er tuttugu millisekúndur, og ekkert stærra getur verið skrifað þar. Sá sem
vildi `framerate` og sló inn `snap` les „by at most 20 ms“ þar sem hann bjóst við
sekúndum, og veit það strax.

## Dæmi

Lína á `00:00:01,010`, stillt á 25 ramma á sekúndu — það er á
margfeldi af fjörutíu millisekúndum:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Tveir eiginleikar

**Að stilla tvisvar breytir engu í seinna skiptið.** Staðsetning sem er þegar á
ramma helst þar: það er sama aðgerðin beitt á sömu niðurstöðu.

**Röð skjátextanna er varðveitt.** Námundun að næsta ramma er
einhalla, þannig að tvö upphöf með að minnsta kosti einn ramma á milli haldast þannig. Nær
hvort öðru geta þau runnið saman — sem er frávik sem
[`inspect`](inspect.md#hvað-anomalies-tilkynnir) kann að nefna, og ekki
öfugsnúningur.

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) ber hlutur hverrar skrár `counts.subtitles`, `counts.moved` (staðsetningarnar sem voru færðar) og `counts.furthest_ms` (stærstu færsluna, í millisekúndum). Heildartaflan er í [Ræsing](invocation.md#hvað-hver-undirskipun-setur-í-counts).

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef sérhver skrá var skrifuð, `2` ef engin, `3` ef
aðeins sumar, `1` við notkunarvillu.

## Villur

Þær eru villur rammatíðni, orðrétt þær sömu og hjá [`framerate`](framerate.md#villur)
— bæði lesa sama gildið eftir sömu leið:

| Hvað veldur henni | Skilaboð |
| :------------------ | :------ |
| rammatíðni núll eða neikvæð | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| tugabrotskomma | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| texti sem er ekki tala | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| rammatíðni úr öllu samhengi | `"…" is not a frame rate: no video runs at that many frames per second` |

**Ekkert er skrifað án eins af valkostunum þremur um áfangastað**, og villur
þeirra eru sameiginlegar undirskipununum sem skrifa:
sjá [Ræsing](invocation.md#áfangastaðurinn).
