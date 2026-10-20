# `hearing-impaired`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Fjarlægir **tilvísanir fyrir heyrnarskerta** — hljóðin sem eru lýst innan hornklofa — úr allri
skránni. **Svigar haldast sjálfgefið**: þeir eru jafnoft notaðir fyrir hvíslaða
línu og fyrir lýst hljóð, og að fjarlægja þá myndi eyða samtali. Valkosturinn `--parentheses` fjarlægir þá
líka, fyrir skrá sem lýsir hljóðum sínum í þeim.

Það sem skipunin gerir ekki: söngtextar milli myllumerkja, nafn mælandans
á undan tvípunkti, að setja hástafi aftur, að leiðrétta OCR-villur. Þessi
mynstur eru þau sem aðstoðarmaðurinn
[`Correct Texts…`](../subedit-gui/correct-texts.md) í glugganum notar, og skipanalínan
býður hann undir nafninu [`correct`](correct.md); og hér er ekkert stillanlegt: umbreytingin er
ákveðin, ekki stillanleg.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Röksemdir og valkostir

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :----- | :------ | :----- |
| `<file>...` | já | ein eða fleiri slóðir | — |
| `--parentheses` | nei | fáni: fjarlægir líka tilvísanir milli sviga | slökkt |
| `--recursive`, `-r` | nei | fáni | slökkt — sjá [Vinnsla á möpputré](lots.md) |
| `--document` | nei | `main` eða `translation` — sjá [Þýðing](invocation.md#þýðing) | `main` |
| `-t`, `--translation-file` | með `--document translation` | þýðingarskrá, fyrir **eina** inntaksskrá | — |
| `--align-method` | nei, og aðeins með `-t` | `position` eða `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | fáni: reiknar og tilkynnir, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

Snið skrárinnar sem er lesin er **varðveitt**: að skipta um snið er verk
[`convert`](convert.md).

**Aðaltextinn sjálfgefið; þýðingin með `--document translation -t FILE`.** Þá er það
þýðingin sem er breytt og skrifuð, og hún ein — sjá [Þýðing](invocation.md#þýðing).

**Tilvísun sem myndi tæma þýðingu skrifar hana tóma og heldur skjátextanum**: skjátextinn
helst í skránni, án texta. Aðaltextinn missir hins vegar skjátexta sem er tæmdur að fullu, eins og
hér á eftir.

## Hvað er fjarlægt og hvað helst

Tilvísun er hornklofi — eða svigi, með `--parentheses` — og það sem hann umlykur, **þar með talið
línuskil**: í raunverulegum skrám er tilvísun oft skorin í sundur við
línulok.

| Inntak | Úttak |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

Fjarlægingin skilur eftir **nákvæmlega eitt bil milli þess sem umlék hana, og ekkert við
jaðar línu**. Hún er ekki hreinsun á bilum skrárinnar: tvö bil
sem textinn bar þegar haldast tvö bil.

**Þegar tilvísunin liggur yfir línuskil er það skilið sem verður eftir** —
línurnar tvær haldast tvær línur. Þetta tilvik er ekki forvitnilegt aukaatriði: í fimmtán
raunverulegum skjátextaskrám var hver hornklofi sem virtist munaðarlaus
tilvísun sem var skorin á þennan hátt.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**Það sem er ekki tilvísun og hreyfist ekki:**

| Tilvik | Hvað gerist |
| :-- | :-------------- |
| eingöngu töluefni — `[1]`, `(12)`, `[ 156478 ]` | haldið eins og það er, afmörkurum meðtöldum: tilvísun nefndi aldrei hljóð |
| svigi og það sem hann umlykur, án `--parentheses` | haldið eins og það er: `Venez (chut) ici` breytist ekki |
| hornklofi eða svigi sem ekkert lokar | látinn vera eins og hann er |
| merki sniðs — `<i>`, `</i>` | aldrei fjarlægt |

Bil **milli** talna, stafur eða tákn — `[1 2]`, `[1a]`,
`[-1]` — gerir þetta að tilvísun. Það gerir tómt efni líka, `[]` eða `( )`: engin
tala, svo ekki tilvísun; ekkert að segja, svo ekkert að halda.

## Það sem hverfur alveg

**Lína sem fjarlægingin tæmdi hverfur**, og **skjátexti sem er tæmdur
að fullu er fjarlægður úr skránni**. Skjátextarnir sem á eftir koma eru endurnúmeraðir.

Lína telst tóm þegar það eina sem eftir er í henni eru bil, merki sniðs
eða stakt þankastrik — `<i>[Musique]</i>` skilur ekki eftir
`<i></i>` á skjánum.

**Merkin eru þau sem snið skrárinnar notar.** `{\i1}[Musique]{\i0}` í
Advanced SSA, `{Y:i}[Musique]` í MicroDVD og `/[Musique]` í MPL2 skilja
ekkert eftir heldur. Öfugt, `<i>` í sniði sem ritar ekkert
merki — TMPlayer, LRC — er texti sem áhorfandinn les, og línan sem
ber hann er ekki tóm.

**Samtal sem er skert niður í eina rödd er ekki lengur samtal**, og þankastrikið sem eftir er
hverfur. Svo lengi sem tvær raddir eru eftir haldast þankastrikin.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Skrá án nokkurrar tilvísunar er samt skrifuð

Hún er afrituð óbreytt, skilakóðinn er `0` og skýrslan segir að ekkert
hafi verið til að fjarlægja.

Þetta er regla undirskipananna sem skrifa: **áfangastaður sem er gefinn er
áfangastaður sem er skrifaður**. Undantekning hér myndi neyða skriftu til að vita
hvaða undirskipanir framleiða stundum skrá og stundum enga.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem var skrifuð. **Með `--dry-run` eru það fyrirhugaðar breytingar**: fyrir hvern skjátexta þar sem textinn myndi breytast, númer hans, textinn fyrir og textinn eftir — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er).

| Stig | Það sem bætist við á villuúttakinu |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, eða `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — snið, stafakóðun, merki og línulok skrárinnar sem var lesin, sett aftur eins og þau voru |
| 3 | `<path>: N bytes read, M written`, síðan **öll greiningarboð lesturs** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

Talan `N` telur skjátextana þar sem textinn breyttist, `M` þá sem fjarlægingin
tæmdi að fullu og fóru því úr skránni.

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) ber hlutur hverrar skrár `counts.cleaned` (textarnir sem voru endurskrifaðir) og `counts.removed` (skjátextarnir sem voru fjarlægðir); **með `--dry-run`** telur lykillinn `changes` líka upp skjátextana sem breyttust, eins og staðalúttak textasniðsins. Fulla töflu er að finna í [Ræsing](invocation.md#hvað-hver-undirskipun-setur-í-counts).

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef hver skrá var skrifuð, `2` ef engin, `3` ef
aðeins sumar, `1` við notkunarvillu.

## Villur

Þessi undirskipun hefur enga villu sem er hennar eigin: hún tekur ekkert
gildi til að þátta og enginn texti getur fengið hana til að hafna skrá. Það sem
getur bilað er lesturinn eða ritunin, eins og hjá hinum.

**Ekkert er skrifað án eins af áfangastaðarvalkostunum þremur**, og villur þeirra
eru sameiginlegar undirskipunum sem skrifa:
sjá [Ræsing](invocation.md#áfangastaðurinn).
