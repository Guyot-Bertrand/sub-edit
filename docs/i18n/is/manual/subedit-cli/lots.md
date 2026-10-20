# Vinnsla á möpputré

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Sérhver undirskipun sem les runu af skrám — þar með talin `inspect`, en ekki
[`append`](append.md), [`split-file`](split-file.md) né [`pair`](pair.md), sem taka við pari eða
einni kvikmynd — tekur við **möppu** í stað skráar, með `--recursive` (`-r`).
Þessi síða segir hvað gönguferðin um tréð tekur með, í hvaða röð og hvert hún skrifar.

**Ekkert af því sem hér fer á eftir breytir því sem gert er við skrá**: hver skrá er lesin,
unnin og skrifuð sjálfstætt, bilun einnar stöðvar ekki hinar, og
skilakóðarnir fjórir eru þeir sömu og í [ræsingunni](invocation.md#skilakóðar).
Gönguferðin býr til lista yfir slóðir, sem restin af runuvinnslunni notar eins og
þær hefðu verið slegnar inn.

## Án `--recursive` er möppu hafnað

Notkunarvilla, kóði `1`, engin skrá unnin né skrifuð:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Hvað gönguferðin tekur með

Gönguferðin er **endurkvæm, ákvörðuð fyrirfram og án óvæntra uppákoma**:

- **röðin** — færslur hverrar möppu eru teknar í röð
  heita þeirra, **bæti fyrir bæti** (hástafir á undan lágstöfum, óháð
  tungumáli vélarinnar), og undirmappa er gengin á sínum stað, dýpt
  fyrst. Tvær keyrslur skila sömu bætunum, á hverri
  vél;
- **táknrænum tenglum er ekki fylgt** — hvorki að skrá né að
  möppu: tré sem inniheldur tengil á eina af yfirmöppum sínum hefði engan
  endi;
- **földum skrám og möppum er sleppt** — þeim sem heiti byrjar á
  punkti;
- **aðeins endingar þekktra sniða eru teknar með**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, án tillits til stafstöðu. **`.txt` er undanskilið**: það
  á við um tvö snið, en umfram allt um sérhverja `LISEZMOI.txt` í heiminum, og runa sem
  mistækist á hverri þeirra væri ónothæf.

**Skrá sem er nefnd á skipanalínunni er aldrei síuð.** Að nefna hana er
að vilja hana: `subedit-cli shift --by 1 --in-place notes.txt` vinnur hana, hver sem
endingin er, og kvartar aðeins ef innihaldið er á engu sniði.

**Það sem var skilið eftir er talið** og sagt á stiginu `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Mappa sem inniheldur ekkert til að ganga um segir það á sjálfgefna stiginu
(`<directory>: holds no file in a format this tool walks for`) og það er ekki villa.

## Hvert gönguferðin skrifar

**Tréð er varðveitt undir `--output-dir`**: slóð skráar,
**miðað við möppuna sem gefin var**, er endurgerð undir úttaksmöppunni. Heiti
möppunnar sem gefin var er ekki hluti af henni — þetta er merking `rsync` með
skástriki í lokin, ekki merking `cp -r`: maður nefnir áfangastaðinn og bætir við hann
því stigi sem maður vill (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

Úttaksmappan og möppur trésins eru **búnar til** ef þær vantar. Með
`--in-place` er hver skrá sem tekin var með skrifuð aftur á sinn stað. Ending sem
`convert` breytir breytist líka í trénu: `films/a/x.srt` umbreytt
í WebVTT er skrifuð `sortie/a/x.vtt`.

**Úttaksmappan er undanskilin göngunni ef hún er inni í trénu sem gengið er um**:
önnur keyrsla les ekki aftur það sem sú fyrri skrifaði.

## Tvær möppur og árekstrar

Regla [áfangastaðarins](invocation.md#áfangastaðurinn) útkljáir það: tveimur skrám
sem enduðu á sömu slóð er hafnað **áður en nokkuð er skrifað**, kóði `1`, báðar
inntaksskrárnar og áfangastaðurinn nefnd.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Hvað það kostar

Runan er áfram raðbundin: tími hennar er summa tíma skránna. Mælipróf
`make bench` mælir gönguferðina og sömu vinnu á einni skrá; tölurnar
eru í [afkastaskránni](../../../../mesures/performances.md).
