# subedit — handbók

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

subedit les, skoðar og tímasetur skjátextaskrár upp á nýtt. Það kemur sem tvö
forrit sem deila sama kjarna og þar með sömu reglum og sömu orðum.

| Forrit | Til hvers | Handbók þess |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | opna skrá, sjá hana í töflu, breyta henni og bera hana saman við kvikmyndina | [handbók gluggans](subedit-gui/index.md) |
| `subedit-cli` | vinna skrár í runum, úr skriftu, án viðmóts | [handbók skipanalínunnar](subedit-cli/index.md) |

Bæði lesa og skrifa **sniðin níu** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — og
þekkja sniðið á innihaldinu fremur en á skráarendingunni. **Stafakóðunin er
einnig greind á innihaldinu**: skrá í Latin-1, í CP1252 eða í UTF-16
opnast, sagt er hvernig hún var lesin og hún er skrifuð aftur í sinni eigin kóðun — skrá
sem er opnuð og síðan vistuð án breytingar haggast ekki um eitt einasta bæti.

## Hvar á að byrja

| Það sem þú vilt | Hvert á að fara |
| :----------------- | :------- |
| setja upp verkfærið | [Uppsetning](subedit-cli/installation.md) — gildir fyrir bæði forritin |
| opna skrá og sjá hvað hún geymir | [Taflan](subedit-gui/table.md) |
| tímasetja upp á nýtt skjátexta sem hafa færst um fasta stærð | [Aðgerðirnar](subedit-gui/operations.md#shift-positions) eða [`shift`](subedit-cli/shift.md) |
| tímasetja upp á nýtt skrá sem var skrifuð fyrir aðra rammatíðni | [`framerate`](subedit-cli/framerate.md), og [`snap`](subedit-cli/snap.md) ef tímasetningin er þegar rétt |
| vita við hvaða rammatíðni skrá var skrifuð | [Rammanetið](subedit-gui/frame-grid.md) eða [`inspect`](subedit-cli/inspect.md) |
| fjarlægja tilvísanir fyrir heyrnarskerta | [Aðgerðirnar](subedit-gui/operations.md#remove-hearing-impaired-mentions) eða [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| leiðrétta tilvísanir, algengar villur, stafstöðu og línuskiptingu í einni umferð | [`Correct Texts…`](subedit-gui/correct-texts.md) eða [`correct`](subedit-cli/correct.md) |
| athuga stafsetninguna orð fyrir orð með orðabókum kerfisins | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| lengja skjátexta sem eru of stuttir til að hægt sé að lesa þá | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations) eða [`adjust`](subedit-cli/adjust.md) |
| koma skrá aftur í röð þar sem skjátextarnir eru það ekki | [`sort`](subedit-cli/sort.md) |
| gera einn skjátexta úr bútum sama texta sem fylgja hver öðrum | [`Join Repeated Subtitles`](subedit-gui/lines.md#að-sameina-endurtekna-skjátexta) eða [`join-repeats`](subedit-cli/join-repeats.md) |
| laga í einu vetfangi skrá sem er illa skipt, í ólagi og með skörun | [`repair`](subedit-cli/repair.md) |
| fjarlægja skörun milli skjátexta | [`Make Ranges Disjoint…`](subedit-gui/lines.md#að-losa-tímabilin-við-skörun) eða [`disjoint`](subedit-cli/disjoint.md) |
| setja tvo hluta kvikmyndar saman enda í enda | [`Append File…`](subedit-gui/operations.md#append-file) eða [`append`](subedit-cli/append.md) |
| skipta kvikmynd í tvo hluta | [`Split Project…`](subedit-gui/operations.md#split-project) eða [`split-file`](subedit-cli/split-file.md) |
| breyta stafstöðu, skáletri eða þankastrikum í heilli skrá | [Aðgerðirnar](subedit-gui/operations.md#italic) eða [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| leita að orði og skipta því út án þess að brjóta merki | [Leita og skipta út](subedit-gui/find-and-replace.md) eða [`replace`](subedit-cli/replace.md) |
| skrifa tal kvikmyndar sem skjátexta | [`Transcribe Audio…`](subedit-gui/video.md#umritun-hljóðsins) eða [`transcribe`](subedit-cli/transcribe.md) — eftir að hafa [sett upp tækin](subedit-cli/install-transcription.md) |
| tímastilla skjátextaskrá eftir hljóði kvikmyndar | [`sync`](subedit-cli/sync.md) — eftir að hafa [sett upp tækin](subedit-cli/install-transcription.md) |
| þýða samhliða frumtextanum | [Að opna þýðingu](subedit-gui/files.md#að-opna-þýðingu) eða [`pair`](subedit-cli/pair.md) til að tímasetja hana upp á nýtt eftir aðalskránni |
| vinna hundrað skrár í einu | [Vinnsla á möpputré](subedit-cli/lots.md), síðan [Ræsing](subedit-cli/invocation.md) |

**Milli `framerate` og `snap` fer mistök fram hjá manni.** Ef þú ert í vafa
byrjar [`snap`](subedit-cli/snap.md#snap-er-ekki-framerate-og-það-sést-ekki-ef-þeim-er-ruglað-saman)
á samanburði þeirra tveggja.

## Hvað þessi handbók lofar

**Hún lýsir því sem er til, aldrei því sem er fyrirhugað.** Það sem kemur og
í hvaða röð er í [vegvísinum](../../../feuille-de-route.md).

**Þessi handbók er skrifuð á ensku og þýdd.** Þýðingarnar fylgja henni
síðu fyrir síðu, undir möppunni `docs/i18n/` í hugbúnaðarsafninu, eitt tré fyrir hvert tungumál. Forritin
tala tungumál umhverfisins; nöfnin sem þau sýna — undirskipanir,
valkostir, valmyndaheiti — eru höfð hér eins og forritið skrifar þau á ensku.

**Dæmin og myndirnar eru búin til, textinn í kring ekki.** Blokkirnar af gerðinni
`console` eru framleiddar með því að keyra skipunina í raun og veru;
skjámyndirnar með því að smíða raunverulega gluggann og mynda hann. Hvorugt
getur því lýst verkfæri sem er ekki lengur til. Það sem stendur eftir án öryggisnets
er textinn í kring, en réttleiki hans byggist á yfirlestri.

**Skjámyndirnar sýna báðar litapalettur**, `Light` og `Dark` — þær sem
[kjörstillingarnar](subedit-gui/preferences.md#þemað) bjóða upp á.

## Fletirnir tveir segja sömu orðin

Ein ástæða til að lesa hvora handbókina sem er þegar hin er notuð: villuskilaboðin,
sniðheitin, úrskurðir netsins og heiti afturköllunar eru skrifuð **einu sinni**, í kjarnanum,
og bæði forritin vitna í þau. Hafnað skrá gefur sömu setninguna á báðum hliðum.

Munurinn liggur í því sem hvor flötur getur gert:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| margar skrár í einu | já — einn flipi fyrir hvert verkefni | já |
| þýðing samhliða aðalskránni | já | já — `-t` á textaundirskipununum, og [`pair`](subedit-cli/pair.md) til að tímasetja hana upp á nýtt |
| bæta við skrá, skipta verkefni | já | já — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| afturköllun | já, þúsund færslur | á ekki við — engu er breytt á staðnum án þess að beðið sé um það |
| breyta texta eða staðsetningu í höndunum | já | nei |
| setja inn, eyða, sameina og skipta línum | já | nei |
| leita og skipta út | já | já — [`replace`](subedit-cli/replace.md) |
| klippa, afrita og líma texta | já | nei |
| laga tímalengdir, skáletur, stafstöðu, þankastrik | já | já — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| leiðrétta textana með mynstrum | já — `Correct Texts…` | já — [`correct`](subedit-cli/correct.md) |
| athuga stafsetninguna orð fyrir orð | já — `Check Spelling…` | nei |
| horfa á kvikmyndina meðan tímasett er | já | nei |
| skrifa yfir inntakið | `Save` | `--in-place` |
