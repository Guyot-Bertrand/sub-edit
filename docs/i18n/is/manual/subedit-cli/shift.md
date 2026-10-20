# `shift`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Færir **allar** staðsetningar skrárinnar um sama magn, í aðra áttina eða
hina. Enginn skjátexti verður lengur eða skemur á skjánum:
það er það sem gerir færslu að færslu.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **annað af tveimur** | tímalengd með formerki, sjá hér að neðan | — |
| `--to-grid` | **annað af tveimur** | rofi: magnið er mælt, sjá hér að neðan | — |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | rofi: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

Snið skrárinnar sem er lesin er **varðveitt**: að breyta sniði er verk
[`convert`](convert.md).

## `--to-grid`: magnið mælt frekar en gefið

Skrá þar sem staðsetningarnar eru á neti ramma **að viðbættum eða frádregnum
fasta** var færð, og þennan fasta má mæla á staðsetningunum sjálfum.
`--to-grid` mælir hann og beitir honum, án þess að nokkur þurfi að þekkja hann.

Magnið er **sérstakt fyrir hverja skrá**: tvær skrár sem voru færðar af sama
neti um tvö ólík magn koma hvor aftur um sitt eigið. Það er
skrifað í frásögninni, ásamt rammatíðninni sem fannst.

**Leiðin er sú stysta.** Handan hálfs ramma er næsta netlína
sú næsta: leiðréttingin fer fram á við í stað þess að færa til baka um
nánast heilan ramma.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Skrá á engu þekktu neti er hafnað og sagt hvers vegna.** Það
er ekkert til að tengja saman á ný, og fasi mældur á suði myndi færa skrána
af handahófi. Sjá [`inspect`](inspect.md#hvað-frame-rate-grid-tilkynnir) til að
vita hvað skrá gefur áður en aðgerðin er reynd.

`--by` og `--to-grid` segja bæði um hversu mikið á að færa: að gefa þau saman
er notkunarvilla, og það er líka að gefa hvorugt.

**`--to-grid` er ekki [`snap`](snap.md).** Það færir alla skrána
um sama magn og heldur afstæðri tímasetningu **nákvæmlega**; `snap`
færir hverja staðsetningu óháð öðrum og gleypir á leiðinni þær sem höfðu
verið leiðréttar í höndunum. Á hreinni skrá gefa þau tvö sömu
niðurstöðu; á skrá að hluta gera þau það ekki.

## Að rita tímalengd

Tveir rithættir, og aðeins tveir:

| Ritháttur | Dæmi |
| :---- | :------- |
| sekúndur | `30`, `2.999`, `-7.001`, `+1.5` |
| tímamerki | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Báðir segja það sama: `--by 7.001` og `--by 00:00:07.001` gefa
sömu skrána.

**Aukastafaskiljan er punkturinn**, því skipanalínan talar ensku.
Kommu er hafnað frekar en að túlka hana: helmingur heimsins skrifar `7,001`
fyrir sjö þúsund og einn, og að lesa það sem sjö sekúndur væri ágiskun —
röng fyrir þann helming.

**Upplausnin er millisekúnda.** Fínni gildum er hafnað frekar en að
námunda þau í kyrrþey: þögul námundun gæfi niðurstöðu sem passar við ekkert af því sem
var beðið um.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Færslu fyrir upphafspunkt er hafnað

Að fara aftur fyrir núll gefur kóðann `2` og **skrifar ekkert**. Skilaboðin nefna
skjátextann sem gat ekki tekið við færslunni, og það segir um leið
hversu langt má fara.

Ástæðan er ekki grundvallaratriði: staðsetning fyrir upphafspunkt er skrifuð
`-00:00:01,000`, ritháttur sem ekkert af sniðunum níu skilgreinir og enginn lesari
tekur við. Kjarninn leyfir slíkar staðsetningar — að hafna þeim myndi gera
ritunaraðgerð að sértilviki — en skrá er annað en
líkan í minni.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem er skrifuð. Með `--dry-run` heldur ekki neitt: það er engin skrá, og frásagnarlínan segir það — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er).

| Stig | Hvað bætist við á villuúttakið |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — snið, stafakóðun, merki og línulok skrárinnar sem var lesin, sett aftur eins og þau voru |
| 3 | `<path>: N bytes read, M written`, síðan **sérhvert greiningarboð við lestur** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) ber hlutur hverrar skrár `counts.subtitles` (hvern skjátexta skrárinnar) og `counts.shifted_by_ms` (færsluna, í heilum millisekúndum, **með formerki**). Heildartaflan er í [Ræsing](invocation.md#hvað-hver-undirskipun-setur-í-counts).

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef sérhver skrá var skrifuð, `2` ef engin, `3` ef
aðeins sumar, `1` við notkunarvillu.

## Villur

| Hvað veldur henni | Skilaboð |
| :------------------ | :------ |
| tugabrotskomma | `"…" is not a time: use a decimal point and not a comma, in every language` |
| fínna en millisekúnda | `"…" is not a time: positions are held to the millisecond, no finer` |
| texti sem er ekki tímalengd | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| færsla fyrir upphafspunkt | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**Ekkert er skrifað án eins af valkostunum þremur um áfangastað**, og villur
þeirra eru sameiginlegar undirskipununum sem skrifa:
sjá [Ræsing](invocation.md#áfangastaðurinn).
