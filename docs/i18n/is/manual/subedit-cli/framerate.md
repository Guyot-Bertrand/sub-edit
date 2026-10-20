# `framerate`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

> **Þetta er ekki [`snap`](snap.md).** Þessi skipun **kvarðar
> skrána**: hún er fyrir þegar tímasetningin er röng og skeikar um nokkrar
> sekúndur undir lok kvikmyndarinnar. `snap` setur tímamerkin aftur á næsta
> net án þess að tímasetja neitt upp á nýtt og er fyrir þegar tímasetningin er þegar
> rétt. Báðar hafa sömu mynd, eina rammatíðni eða tvær, og að ruglast á þeim veldur
> engri villu.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Tímasetur skrá sem er tímasett fyrir eina rammatíðni upp á nýtt fyrir aðra. Þetta er
aðgerðin sem skjátextaskrá kallar á þegar hún var sótt fyrir eintak sem
er ekki það sama og kvikmyndin sem maður hefur — til dæmis PAL-DVD á móti NTSC-yfirfærslu.

Snið skrárinnar sem er lesin er **varðveitt**: að skipta um snið er verk
[`convert`](convert.md).

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
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
| `--from` | **já** | rammatíðni, sjá hér á eftir | — |
| `--to` | **já** | rammatíðni, sjá hér á eftir | — |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | fáni: reiknar og tilkynnir, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

Valkosturinn `--from` er rammatíðnin sem skráin **er** tímasett fyrir, `--to` sú sem
hún á að verða fyrir. Ef þeim er víxlað færist skráin
í öfuga átt — um sjö sekúndur á kvikmynd í fullri lengd milli 25 og
23,976 — og ekkert varar við því fyrr en horft er á myndina.

## Að rita rammatíðni

Í römmum á sekúndu, með aukastöfum, og punktur er aðgreinir:

```
25      24      23.976      29.97      60
```

**Rammatíðni sem er núll eða neikvæð er hafnað**: hún er ekki rammatíðni.

### Rammatíðnirnar þrjár í NTSC verða ekki ritaðar nákvæmlega

`23.976`, `29.97` og `59.94` eru **merkimiðar**. Rammatíðnirnar sem þeir
nefna eru `24000/1001`, `30000/1001` og `60000/1001`, og engin þeirra hefur
endanlega ritun með aukastöfum. Tólið þekkir þær sem slíkar:

| Það sem er ritað | Hvað það nefnir |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — kvikmyndir yfirfærðar á NTSC-myndband |
| `24` | `24` — kvikmyndir |
| `25` | `25` — PAL- og SECAM-myndband |
| `29.97` | `30000/1001` — NTSC-myndband |
| `30`, `50`, `60` | þær sjálfar |
| `59.94` | `60000/1001` |

Þekkingin snýst um **gildið en ekki stafina**: `23.9760`
nefnir sömu rammatíðni og `23.976`.

Hver önnur tala með aukastöfum er tekin bókstaflega — `23.9` er enginn staðall,
það er engu að lesa úr henni. Það er líka ástæða þess að taflan hér að ofan er
nauðsynleg: án hennar yrði `23.976` að `23976/1000`, og skipanalínan
segði annað en glugginn um sömu orðin.

## Hvað staðsetningarnar verða

```
t′ = round( t × (from / to) )
```

Kvikmynd sem er frumunnin í 23,976 og spiluð í 25 römmum á sekúndu gengur hraðar,
svo skjátextar hennar koma fyrr; í hina áttina koma þeir
seinna.

**Stuðullinn er nákvæm almenn tala og námundunin gerist aðeins einu sinni.** Leiðin
sem virðist jafngild — að umbreyta staðsetningunni í rammanúmer á
inntakstíðninni og svo aftur í millisekúndur á úttakstíðninni — er það
ekki: hún skammtar hverja staðsetningu á inntaksnetið og skeikar um allt að
hálfan ramma, 21 ms mælt. Sjá
[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

Í reynd, frá 25 til 23,976, er stuðullinn `1001/960`:

| Staðsetning lesin | Skrifuð | Hvað leiðin um ramma myndi gefa |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem var skrifuð. Með `--dry-run` er það ekkert heldur: engin skrá er til og frásagnarlínan segir það — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er).

| Stig | Það sem bætist við á villuúttakinu |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — snið, stafakóðun, merki og línulok skrárinnar sem var lesin, sett aftur eins og þau voru |
| 3 | `<path>: N bytes read, M written`, síðan **öll greiningarboð lesturs** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

**Frásögnin nefnir nákvæmu rammatíðnina, ekki merkimiðann sem var sleginn inn.** Ef
`23.976` stæði á þeirri línu myndi hún tilkynna umbreytingu sem varð ekki; hún er
eini staðurinn þar sem sést hvor þeirra tveggja var notuð. Rammatíðni sem
aukastafatala ritar nákvæmlega er sýnd þar sem aukastafatala, hinar sem almennt brot.

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) ber hlutur hverrar skrár `counts.subtitles` (hver skjátexti skrárinnar). Fulla töflu er að finna í [Ræsing](invocation.md#hvað-hver-undirskipun-setur-í-counts).

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef hver skrá var skrifuð, `2` ef engin, `3` ef
aðeins sumar, `1` við notkunarvillu.

Engin höfnun veltur á efni skrár: tvær rammatíðnir skilgreina alltaf
stuðul. Þetta er eina undirskipunin af þeim sem skrifa þar sem allar
höfnunir koma á undan fyrsta lestri.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Villur

| Hvað kallar þær fram | Skilaboð |
| :------------------ | :------ |
| rammatíðni sem er núll eða neikvæð | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| kommu sem aukastafaskil | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| texti sem er ekki tala | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| rammatíðni úr öllu samhengi | `"…" is not a frame rate: no video runs at that many frames per second` |

**Ekkert er skrifað án eins af áfangastaðarvalkostunum þremur**, og villur þeirra
eru sameiginlegar undirskipunum sem skrifa:
sjá [Ræsing](invocation.md#áfangastaðurinn).
