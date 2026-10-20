# Ræsing

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-gui [--version] [<file>]
```

| Röksemd | Nauðsynlegt | Gildi | Sjálfgefið |
| :------- | :------- | :---- | :------ |
| `<file>` | nei | slóð skjátextaskrár sem á að opna | engin — glugginn opnast tómur |
| `--version` | nei | — | — |

**Ein skrá.** Röksemdunum sem á eftir fylgja er sleppt: glugginn heldur aðeins
einu verkefni í einu.

## `--version`

Skrifar á staðalúttak blokkina sem `--version` skipanalínunnar skrifar, og
skilar strax, án þess að opna glugga. Skilakóði `0`.

**`subedit <version>` einni sér í fyrstu línu**, síðan ein lína fyrir hvert bókasafn sem
glugginn keyrir með, með útgáfunni sem fannst við keyrslu og, innan sviga, þeirri
sem hann var þýddur með; síðan uppsettu viðmótstungumálin, eftir kóða. Á
vél með Qt 6.4.2:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

Tölurnar eru ekki afritaðar hér: þær eru vélarinnar, og þessi handbók hefur enga
myndaða blokk sem héldi þeirri sem forritið sjálft skrifar réttri. Bókasöfnin eru
þau sem skipanalínan hefur, ICU og Enchant, og þau tvö sem aðeins glugginn tengir — Qt, og
libmpv, spilarinn, gefið til kynna með útgáfu biðlaraviðmóts hans, þeirri sem `mpv.pc`
tilkynnir. Afgangurinn er lýst í
[handbók skipanalínunnar](../subedit-cli/invocation.md#--version). Orðin
`built with` og `Languages` fylgja tungumáli gluggans; ekkert annað gerir það.

`Hjálp ▸ Um subedit` sýnir sömu bókasöfn, af sama lista, í töflu —
heitið, útgáfuna sem er hlaðin, útgáfuna sem var smíðað með, eina röð hvert — síðan
tungumálin undir henni; fyrirsagnir hennar fylgja tungumáli gluggans.

## Að opna skrá

```console
$ subedit-gui film.srt
```

Sniðin sem eru lesin eru **þau níu** sem síuvalmyndin í opnunarglugganum telur upp — sjá
[Að opna og vista](files.md#opnun). Sniðið er greint af innihaldinu en
ekki af skráarendingunni, og **það gildir líka um stafakóðunina**: bætaröðunarmerki
ef skráin hefur slíkt, annars sú stafakóðun sem bæti hennar benda til.

## Þegar opnun mistekst

Skrá sem vantar, skrá sem ekki er hægt að lesa, skrá þar sem bætin afkóðast í engri
stafakóðun, eða skrá þar sem ekkert snið þekkir innihaldið: **glugginn opnast
samt, tómur**, og bilunin er skrifuð á villustrauminn.

**Skilaboðin nefna orsökina**, með sömu orðum og skipanalínan:

| Skilaboð | Hvað gerðist |
| :------ | :------------ |
| `does not exist` | slóðin vísar á enga skrá |
| `cannot be opened: permission denied` | kerfið neitar að opna hana |
| `cannot be read` | kerfið neitaði af annarri ástæðu |
| `cannot be decoded in the chosen encoding` | bætin lesast ekki í stafakóðuninni sem var valin |
| `is in no format this tool knows` | ekkert snið þekkir innihaldið |
| `holds nothing recognisable as a subtitle` | sniðið er þekkt, en ekkert í skránni er skjátexti |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Að opnast tómt frekar en að neita að ræsa: tólið gagnast enn til
einhvers, og að röksemd mistekst er engin ástæða til að ræsa ekki.

**Skrá sem er illa gerð en læsileg opnast**, og það sem lesturinn tók eftir
á leiðinni má lesa í hnappi í stöðulínunni — sjá
[greiningarboð lesturs](files.md#greiningarboð-lesturs).

## Skilakóðar

| Kóði | Hvenær |
| :--- | :----- |
| `0` | glugginn opnaðist og var síðan lokað eðlilega, eða `--version` svaraði |
| `2` | óvænt villa áður en opnað var — Qt vantar, skjár ekki tiltækur |

Það að skrá opnast ekki **breytir ekki skilakóðanum**: glugginn
opnaðist, og það er það sem kóðinn segir frá.

## Man-síðan

Uppsetning leggur `subedit-gui(1)` undir `share/man/man1`:

```console
$ man subedit-gui
```

Hún gefur kallsniðið, `--version`, skilaboðin þegar opnun mistekst,
skilakóðana, umhverfisbreyturnar tvær og stillingaskrána.
**Hún lýsir ekki glugganum** — það gerir þessi handbók, og síðan
vísar hingað.

## Qt-vettvangurinn

`QT_QPA_PLATFORM` velur það sem glugginn keyrir á. **Ef breytan er
stillt virðir `subedit-gui` hana án röksemdar.**

Ef hún er það ekki lætur hann Qt velja: innbyggði spilarinn teiknar myndina í
gluggann sjálfan og biður vettvanginn ekki um neitt — sjá
[Spilarinn](player.md).

`QT_QPA_PLATFORM=offscreen` keyrir gluggann þar sem enginn
myndrænn þjónn er — sem er það sem samfelld samþætting og prófanir gera.

## Tungumál

Glugginn talar tungumál umhverfis síns, **valið einu sinni, þegar hann ræsist**: að breyta
breytum glugga sem er í gangi breytir engu fyrr en við næstu ræsingu. Breyturnar og reglurnar
eru þær sömu og hjá skipanalínunni — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` fyrir
ensku, ensku **án eins orðs** þegar tungumálið sem beðið er um er ekki uppsett — lýst í
[handbók skipanalínunnar](../subedit-cli/invocation.md#tungumál). Þýðingaskrárnar eru sömu
skrárnar, uppsettar undir `<prefix>/share/subedit/locale`.

**Tungumál sem er valið í `Breyta ▸ Kjörstillingar…` gengur fyrir**, og umhverfið er aðeins lesið
þegar það er `System default`, sem það er þar til annað er valið: uppsett tungumál sem er valið þar
sigrar allar breytur, `LC_ALL=C` meðtalið. Valið tekur líka gildi við næstu ræsingu. Sjá
[Viðmótstungumálið](preferences.md#tungumál-viðmótsins).

| Hvað | Fylgir tungumálinu |
| :--- | :------------------- |
| valmyndirnar og færslur þeirra, með undirstrikuðu stöfunum; verkfærastikan; ábendingarnar | já |
| dálkar töflunnar og `Skoða ▸ Dálkar`, flipar, stöðulínan, spilunarstikan | já |
| venjulegu hnapparnir sem Qt teiknar sjálft (`OK`, `Cancel`) | já, þegar eigin þýðingaskrá Qt fyrir það tungumál er uppsett (`qtbase_<language>.qm`); annars enska |
| gluggarnir, síður `Correct Texts…`, titlar og síur skráagluggana, skilaboðagluggarnir og það sem þeir segja | já |
| línurnar sem glugginn skrifar á villustraum sinn, `subedit-gui: …` | já, nema heiti forritsins |
| heiti leiðréttingarmynstranna í `Correct Texts…`, og lýsingar þeirra í ábendingunum | já, fyrir mynstrin sem fylgja forritinu; mynstur sem þú býrð til sjálf er sýnt eins og skráin þess skrifar það |
| heiti stafakóðana, línuloka og stafsetningartungumála (`UTF-8`, `CRLF`, `en_US`) | aldrei: þetta eru heiti, ekki orð |
| flýtilyklarnir | aldrei: `Ctrl+S` er `Ctrl+S` á öllum tungumálum |
| skilaboð sem þýðingaskráin þýðir ekki | sýnd á ensku |

Þessi útgáfa setur upp `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT` og `nl` — katalónsku, tékknesku, þýsku,
spænsku, finnsku, frönsku, galisísku, hebresku, ungversku, interlingue, íslensku, ítölsku og hollensku — þýdd að fullu. Franskan hefur verið yfirfarin af lesanda
tungumálsins; katalónska, tékkneska, þýska, spænska, finnska, galisíska, hebreska, ungverska, interlingue, íslenska, ítalska og hollenska eru vélþýddar og enn ekki
yfirfarnar af lesanda tungumálsins.
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
Á frönsku skjáborði stendur í valmyndastikunni
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. Skjámyndir þessarar handbókar eru teknar á íslensku.
