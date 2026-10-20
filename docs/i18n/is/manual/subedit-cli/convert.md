# `convert`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```
Skrifar hverja skrá á því sniði og í þeirri mynd sem beðið er um. **Breytir aldrei
inntakinu**, nema `--in-place` biðji um það berum orðum.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
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
| `--to` | **já** | heiti úr töflunni hér að neðan, og ekkert annað | — |
| `--line-endings` | nei | `unix`, `windows` eða `mac` | þau sem skráin sem var lesin hafði |
| `--to-encoding` | nei | hver sú stafakóðun sem ICU getur skrifað, nema þær sem skrifa sitt eigið merki | sú sem skráin sem var lesin hafði |
| `--bom` / `--no-bom` | nei | fánar, sem útiloka hvor annan | það sem skráin sem var lesin bar |
| `--sort` | nei | fáni: raðar skjátextunum eftir upphafi þeirra **áður en skrifað er**, sjá [Röðun við ritun](#röðun-við-ritun) | slökkt |
| `--join-repeats` | nei | fáni: gerir einn skjátexta úr bútum sama texta sem fylgja hver öðrum, **á undan öllu öðru**, sjá [Að losa tímabilin við skörun](#að-losa-tímabilin-við-skörun) | slökkt |
| `--repair` | nei | fáni: það sama og `--join-repeats --sort --disjoint`, í þeirri röð | slökkt |
| `--disjoint` | nei | fáni: sker skjátexta sem skarast **áður en skrifað er**, sjá [Að losa tímabilin við skörun](#að-losa-tímabilin-við-skörun) | slökkt |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | fáni: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

`mac` nefnir vagnskil eitt og sér (`\r`), línulok gamla Mac OS.

## Sniðin sem `--to` tekur við

| Heiti | Snið | Skráarending sem er skrifuð |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Heiti, en ekki skráarending.** Tvær skráarendingar nefna tvö snið
hvor — `.sub` er einnig MicroDVD, `.txt` er bæði MPL2 og
TMPlayer — svo valkostur sem tæki við skráarendingum vissi ekki hvora
verið er að biðja um. Heitin hér að ofan eru ótvíræð og samsvara
skráarendingunni alls staðar þar sem skráarendingin er líka ótvíræð.

**Listinn er lokaður.** Hann er listi Gaupol, og öll sniðin níu eru í honum.

## Sniðin tvö sem bera engan endi

**TMPlayer og LRC skrifa aðeins eina staðsetningu í hverja línu**, upphafið. Þetta
sést báðum megin.

**Við lestur** eru endarnir dregnir af öðru: endir línu er upphaf
þeirrar næstu, og sú síðasta fær fimm sekúndur. Það er það sem Gaupol gerir.
Við segjum líka frá því, með greiningarboði sem `--verbose` sýnir á undirskipununum
þremur:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**Við ritun** eru endarnir ekki skrifaðir, þar sem ekkert pláss er fyrir þá.
Umbreyting í annað þessara tveggja sniða tapar þeim því, og umferð fram og til baka finnur
þá ekki aftur — hún dregur þá af öðru á ný.

**LRC tapar einu í viðbót**: það getur ekki borið línuskil.
Samtalslína á tveimur línum er skrifuð á einni, línurnar tvær tengdar með
bili. Það tap er ekki heldur hægt að bæta.

Hvorugt hefur orðaforða fyrir skáletur.

## `--frame-rate`, og eina sniðið sem telur í römmum

**MicroDVD telur ekki í sekúndum**: staðsetningar þess *eru* rammanúmer, og engin MicroDVD-skrá gefur upp þá tíðni sem þau voru
talin við. `--frame-rate` gefur hana upp, og gildir báðum megin: það er sú
tíðni sem `.sub`-skrá í römmum er lesin við, og sú sem umbreyting í
MicroDVD telur rammana við.

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | nei | tíðni í römmum á sekúndu: `25`, `23.976`, `29.97` | sjá hér að neðan |

**Við lestur, án `--frame-rate`, er skráin lesin við 23,976** — gildi Gaupol —
og stigið `-vvv` segir frá því:
`counts in frames and states no rate; it was read at`. Þetta er eini staðurinn í verkfærinu þar sem hver staðsetning sem er sýnd
byggist á forsendu sem skráin getur ekki staðfest.

**Við ritun, án `--frame-rate`, eru þrjú tilvik og þrjú svör:**

| Upphafsskráin | Hvað gerist |
| :------------------- | :-------------- |
| er þegar MicroDVD | tíðnin sem las hana skrifar hana aftur, og rammarnir koma eins til baka |
| er tímabundin, og staðsetningar hennar falla á net | netið er tekið, og `-vv` segir hvert |
| er tímabundin, án nets | **höfnun**, kóði 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

Höfnunin er af ásettu ráði. Eina látbragðið sem eftir væri er að búa til tölu, og
hver lína skrárinnar færðist um jafn mikið og talan var röng.

## Hvað umbreyting týnir

**Umbreyting í annað snið er aldrei tapslaus**, og það sem kemst ekki í gegn
fer eftir parinu:

| Það sem kemst ekki í gegn | Hvers vegna |
| :--------------------- | :------- |
| hausinn | hann á aðeins við í sínu eigin sniði: `[INFORMATION]` er ekki WebVTT-haus |
| gögn sem eru sértæk fyrir sniðið | hnit SubRip, auðkenni og stillingar WebVTT-vísbendingar eiga sér enga hliðstæðu annars staðar |
| nákvæmnin | SubViewer 2, báðir Sub Station Alpha og LRC skrifa á hundraðshluta, MPL2 á tíunda hluta, TMPlayer á sekúndu, MicroDVD á ramma; `01:00:00,017` kemur til baka sem `01:00:00,020` |
| endir línu | TMPlayer og LRC hafa ekkert svið fyrir hann; næsti lestur dregur hann af öðru á ný |
| línuskilin | aðeins LRC: línur þess rúmast á einni línu, og þeim tveimur er límt saman með bili |
| merkin sem markasniðið getur ekki skrifað | WebVTT hefur ekkert `<font color>`, TMPlayer og LRC hafa ekkert yfirleitt |
| umbrotið | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: þau lifa af umferð fram og til baka í sínu eigin sniði, og hverfa við umbreytingu |

**Að skrifa skrá aftur á hennar eigin sniði tapar engu** — það er tryggingin
sem allt hitt hvílir á, og `--to srt` á `.srt` skilar sömu bætum. Ekkert
er einu sinni afkóðað: textinn helst óunninn strengur eins og hann var lesinn.

## Merki eru þýdd, ekki afrituð

**Hvert snið skrifar skáletur á sinn hátt**, og að umbreyta án þess að þýða
myndi skilja eftir `{\i1}` í `.srt`: texta sem notandinn sér, í
skrá sem mun aldrei kunna að túlka hann. Það sem kemst í gegn eru **sex
atriði** — feitletur, skáletur, undirstrikun, litur, leturgerð og stærð:

| Snið | Feitletur | Skáletur | Undirstrikun | Litur | Leturgerð | Stærð |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**Strik er tilkynnt tap.** Umbreyting litaðrar `.srt` í WebVTT fjarlægir
litinn og segir frá því; skráin sem er framleidd er WebVTT, ekki SubRip í dulargervi.

**MicroDVD og MPL2 geta ekki stílað hluta línu.** Merki þeirra
hafa engan endi: `{y:i}` hallar öllu sem á eftir kemur út línuna, og
`{Y:i}` út skjátextann. Lína sem aðeins helmingur var
feitletraður er því skrifuð án nokkurs feitleturs, frekar en með feitletri sem flæðir yfir.

## Hvað umbreytingin segir að hafi tapast

**Hún segir frá þegar hún tapar og þegir þegar hún tapar engu.** Skýrsla
sem birtist við hvert kall er skýrsla sem enginn les.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

Línan birtist á sjálfgefnu nákvæmnisstigi, og á hverju stigi þar fyrir ofan.
Atriðin, í þessari röð:

| Atriði | Hvað það segir |
| :---- | :----------- |
| `ends are not carried by <format>` | markasniðið skrifar engan endi |
| `line breaks were joined in N subtitles` | aðeins LRC: N skjátextar voru á mörgum línum |
| `N tags dropped` | N merki var ekki hægt að skrifa, umbrot meðtalið |
| `the header was dropped` | hausinn fer ekki yfir snið |
| `the <format> fields of N subtitles were dropped` | N skjátextar báru gögn sem eru sértæk fyrir snið þeirra |
| `positions moved by up to N ms` | mesta vegalengd sem staðsetning færðist |

## Að breyta stafakóðun

**`--to-encoding` velur stafakóðunina sem er skrifuð; `--encoding` segir þá sem er
lesin.** Saman endurkóða þau skrá:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Án `--to-encoding` er skráin skrifuð í **þeirri stafakóðun sem hún var lesin í**,
eins og með línulok hennar og merki: skrá sem er umbreytt án
fyrirmæla um stafakóðun skilar sömu stöfum, á sömu bætum.

**Staf sem markstafakóðunin getur ekki skrifað stöðvar skrána**, og
ekkert er skrifað. `ł` á sér engan stað í Latin-1; að skipta honum út fyrir `?`
myndi tapa texta milli lesturs og ritunar, án þess að nokkur sæi það.

**Bætaröðunarmerki er aðeins til fyrir Unicode-stafakóðanir.**
`--bom` með `--to-encoding` sem hefur ekkert slíkt er hafnað frekar en hunsað
— sjá villutöfluna hér að neðan.

**Og stafakóðunin verður að nefna bætaröðina.** `UTF-16` og `UTF-32` eru
heiti sem ICU þekkir, og umbreytirinn skrifar **eigið merki**, hvað sem
er beðið um: `--no-bom` yrði þá samþykkt og ekki hlýtt. Báðum er
því hafnað, og skilaboðin nefna úttakið:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` og `UTF-32BE` skrifa nákvæmlega sömu
bæti, merkið meðtalið — en undir stjórn `--bom` og `--no-bom`. Höfnunin
kostar því ekkert nema nákvæmara heiti, og hún gildir um lestur jafnt og um ritun:
`--encoding UTF-16` fær sama svar.

## Áfangastaðurinn

Valkostirnir þrír og reglur þeirra eru sameiginleg undirskipununum sem
skrifa: sjá [Ræsing](invocation.md#áfangastaðurinn). Ekkert er skrifað
án eins þeirra.

Það sem er sértækt fyrir `convert`: **`--output-dir` breytir skráarendingunni með
sniðinu.** `convert --to vtt` á `a.srt` skrifar `a.vtt`, aldrei `a.srt` sem geymir
WebVTT — skrá með nafn sem lýgur flækist fyrir öllum öðrum verkfærum.

## Umbreyting á staðnum er hafnað

`--in-place` með `--to` sem breytir sniðinu gefur kóða `1` og skrifar
ekkert: á staðnum er ekkert annað nafn til að bera nýja sniðið, og
skráin héldist undir skráarendingu sem innihaldið réttlætir ekki lengur.

`--in-place` má þó nota til að breyta **myndinni án sniðsins** —
til dæmis að skrifa `.srt` aftur sem `.srt` með önnur línulok.

Höfnunin er ákveðin á **skráarendingunni einni saman**, áður en nokkuð er lesið: notkunarvilla
má aldrei skilja runu eftir hálfskrifaða.

## Línulok og bætaröðunarmerki

Sjálfgefið **tekur skráin sem er skrifuð upp það sem skráin sem var lesin bar**. Líkanið
geymir hvort tveggja við lestur; að þvinga þau sjálfgefið myndi tapa, við hverja
umbreytingu, upplýsingum sem eru geymdar af ásettu ráði.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Röðun við ritun

`--sort` skrifar skrána **í röð upphafanna**, hvert sem markasniðið er. Röðunin er gerð **á afriti**: skráin sem var lesin er
ekki snert, og það sem er skrifað er það sem [`sort`](sort.md) hefði gert úr henni — stöðugt, tveir skjátextar sem byrja saman halda röð sinni.

**Aldrei sjálfgefið**: röð skráar er höfundar hennar svo lengi sem ekki er beðið um annað. Með valkostinum segir hver skrá frá því, á
fyrsta frásagnarstigi, `<path>: N subtitles moved` eða `<path>: already in order`, og `--format json`-færslan ber
talninguna `moved` — **sem vantar án valkostsins**, til að breyta ekki lögun þess sem núverandi skriftur lesa.

## Að losa tímabilin við skörun

`--disjoint` sker skjátexta sem skarast þannig að ekkert tímabil skarist lengur við annað, **á sama afriti og röðunin**: skráin sem var lesin er ekki snert.

Aldrei sjálfgefið. Valkosturinn gerir það sem [`disjoint`](disjoint.md) gerir, **með sjálfgefnum stillingum þess**, áður en skrifað er, eftir röðunina. Hann krefst skrár í réttri röð: annars er honum hafnað, og `--sort` — sem kemur á undan — kemur henni í röð áður. Skráin segir frá því á fyrsta frásagnarstigi (`<path>: N subtitles split, M merged`), og `--format json`-færslan ber talningarnar `split` og `merged`, **sem vantar án valkostsins**.

**`--join-repeats` kemur á undan röðuninni og skurðinum**: texti sem er skorinn í búta sem fylgja hver öðrum er endurbyggður fyrst (sjá
[`join-repeats`](join-repeats.md)), síðan eru skjátextarnir raðaðir, síðan eru tímabilin gerð án skörunar. **`--repair` er allt þrennt í einu**,
eins og [`repair`](repair.md), með sjálfgefnum stillingum skurðarins. Með `--join-repeats` segir hver skrá
`<path>: N repeated subtitles joined` á fyrsta frásagnarstigi, og `--format json`-færslan ber `counts.joined` — **sem vantar án valkostsins**.

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem er skrifuð. Með `--dry-run` líka ekkert: engin skrá er til, og frásagnarlínan segir frá því — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er).

Á villuúttakinu, eftir stigi:

| Stig | Hverju er bætt við |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — stafakóðunin sem er nefnd er sú sem er skrifuð |
| 3 | `<path>: N bytes read, M written`, síðan **hvert lesgreiningarboð** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) ber hlutur hverrar skrár `counts.subtitles`, síðan það sem umbreytingin týndi: `lost_ends` og `lost_header` (`0` eða `1`), `joined_lines`, `lost_tags`, `lost_fields` og `furthest_ms` — atriði [setningarinnar um tap](#hvað-umbreytingin-segir-að-hafi-tapast). Heildartaflan er í kaflanum [Ræsing](invocation.md#hvað-hver-undirskipun-setur-í-counts).

## Skilakóðar

Þeir sömu og í verkfærinu: `0` ef hver skrá var skrifuð, `2` ef engin, `3` ef
aðeins sumar, `1` við notkunarvillu.

## Villur

| Hvað veldur henni | Skilaboð |
| :------------------ | :------ |
| `--bom` með `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` sem breytir sniðinu | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` á stafakóðun sem hefur ekkert slíkt | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` sem nefnir óþekkta stafakóðun | `no encoding is named "<name>"` |
| `--to-encoding` sem nefnir stafakóðun sem skrifar sitt eigið merki | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| stafur sem vantar í stafakóðunina sem er skrifuð | `<path>: holds a character the chosen encoding cannot write` |

**Að hafna `--bom` á stafakóðun sem hefur ekkert slíkt er ekki
smámunasemi.** Bætaröðunarmerki er til fyrir Unicode-stafakóðanir og
engar aðrar: að biðja um slíkt á skrá í Windows-1252 er að biðja um
eitthvað sem er ekki til. Skráin yrði skrifuð án þess, og enginn
vissi að beiðninni var ekki sinnt.

Villur áfangastaðarins eru sameiginlegar undirskipununum sem skrifa:
sjá [Ræsing](invocation.md#áfangastaðurinn).

Lesvillurnar eru þær sömu og hjá [`inspect`](inspect.md): þær varða
inntaksskrána og hafa sömu skilaboð.
