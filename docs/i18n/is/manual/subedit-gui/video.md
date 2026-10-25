# Tengda myndskeiðið

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

Í valmyndinni **Myndskeið** eru fyrst þrjár skipanir, og aðgerðirnar sem stýra
kvikmyndinni — sjá [Spilarinn](player.md#að-fara-um-kvikmyndina).

| Skipun | Flýtilykill | Hvað hún gerir |
| :----- | :---------- | :------------- |
| Velja myndskeið… | enginn | velur myndskeiðsskrána sem skjalið fylgir |
| Umrita hljóð… | enginn | umritar hljóð kvikmyndarinnar í skjátexta sem koma í stað skjátexta verkefnisins — sjá [Umritun hljóðsins](#umritun-hljóðsins) |
| Spila / Hlé | `Ctrl+P` | spilar eða stöðvar myndskeiðið — sjá [Spilarinn](player.md#spilun-og-stöðvun) |

Tengda myndskeiðið er **eiginleiki opna skjalsins**, ekki stilling
forritsins: þegar önnur skjátextaskrá er opnuð er byrjað frá grunni. Það
er ekki skrifað í neina skrá og hverfur með glugganum.

## Hljóðrásin

Kvikmynd getur borið margar hljóðrásir — upprunalegu útgáfuna, talsetningu, skýringar. Undirvalmyndin
**`Myndskeið ▸ Hljóð ▸ Tungumál`** telur þær upp, **eina færslu fyrir hverja rás**, og **merkir þá sem er í spilun**:

| Það sem skráin gefur | Það sem færslan segir |
| :------------------- | :-------------------- |
| tungumál og titil | `1: fra — Original` |
| tungumál eitt og sér | `2: eng` |
| titil einan og sér | `3: Commentary` |
| hvorugt | `1` |

Talan er **sæti rásarinnar í kvikmyndinni**, talið eitt af öðru, í röð skrárinnar: tvær rásir
geta sagt `eng`, og skrá sem þegir um merkingar sínar hefur samt fyrstu og aðra rás.

**Að velja færslu spilar þá rás.** Merkið er svar spilarans, ekki smellsins: það segir til um
rásina sem er í spilun.

- **Undirvalmyndin er óvirk** án myndskeiðs eða án hljóðrásar. Með einni rás er hún áfram virk og
  sýnir hana, merkta: hún segir tungumál kvikmyndarinnar. Hljóðstyrksaðgerðirnar tvær, sem eru við hliðina, eru áfram
  virkar í öllum tilvikum: sjá [Hljóðstyrkurinn](player.md#hljóðstyrkurinn).
- **Rásin er ekki munuð** frá einu myndskeiði til annars, né frá einni lotu til annarrar: rásarnúmer er
  það sem skráin hefur, og að flytja það yfir á næstu kvikmynd myndi velja það sem ber það númer þar.
  Þegar farið er aftur í flipa opnast kvikmynd hans aftur á rásinni sem spilarinn velur sjálfur.
- Enginn flýtilykill: þú skiptir sjaldan um tungumál og valmyndin nægir.

## Myndskeið valið

Glugginn opnast á möppu skjátextaskrárinnar og síar á
þekktar endingar myndskeiða:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

Hann býður einnig upp á að sýna allar skrár. Að hætta við gluggann breytir
engu.

Valda skráin er **opnuð samstundis**: myndin birtist í glugganum,
í hléi á fyrsta augnabliki sínu. Sjá [Spilarinn](player.md).

**Að sleppa myndskeiðsskrá á gluggann gerir það sama** — sjá
[Að sleppa skrám á gluggann](files.md#að-sleppa-skrám-á-gluggann).

**Myndskeiðið er það sem tilheyrir verkefni núverandi flipa.** Hver flipi geymir
sitt eigið; að skipta um flipa skiptir um kvikmynd. **Þegar farið er aftur í flipa heldur kvikmynd hans
áfram þar sem þú skildir við hana**, í hléi, eins og við hverja opnun. Önnur kvikmynd
sem var valin fyrir það verkefni í millitíðinni byrjar hins vegar frá upphafi.

## Sjálfvirka tillagan

Þegar skjátextaskrá er opnuð **og ekkert myndskeið hefur verið valið**
leitar glugginn að einu í sömu möppu: myndskeiðsskrá þar sem nafnið
án endingar er forskeyti á okkar, upp að punkti.

| Skjátextaskrá | Myndskeið fundið | Hvers vegna |
| :------------ | :--------------- | :---------- |
| `film.fr.srt` | `film.mkv` | `film` er forskeyti á `film.fr` |
| `film.en.forced.srt` | `film.mkv` | á sama hátt, hvað sem hlutarnir eru margir |
| `film.fr.srt` | `film.fr.mkv` frekar en `film.mkv` | lengsta nafnið vinnur |
| `film.fr.srt` | ekkert, ef bæði `film.mkv` **og** `film.mp4` eru til staðar | ekki er hægt að greina tvo kandídata í sundur |
| `film.fr.srt` | ekkert, ef aðeins `fil.mkv` er til staðar | `fil` endar ekki á punkti |

**Tillaga er ekki val.** Hún lætur aðra tillögu koma í sinn stað
— með því að opna aðra skrá eða vista undir öðru nafni. Skýrt
val er hins vegar aldrei yfirskrifað: eftir að `Select Video…` hefur verið notað kemur
sjálfvirka tillagan ekki aftur á eftir því.

Það er það sem gerir ágiskunina ásættanlega: hægt er að leiðrétta hana og leiðréttingin
helst.

## Hvað stöðulínan sýnir

Hægra megin í stöðulínunni segir lína við hvað skjalið fylgir. Hún
er sú síðasta af þremur: [stafakóðun](files.md#stafakóðunin-í-stöðulínunni)
skrárinnar sem var lesin kemur fyrst, [rammanetið](frame-grid.md) sem er dregið af
staðsetningunum næst.

| Staða | Það sem er skrifað |
| :---- | :----------------- |
| myndskeið er tengt | `Video: film.mkv` |
| … og `ffprobe` gefur rammatíðni þess | `Video: film.mkv, 24000/1001 fps` |
| ekkert | `No video` |

**Rammatíðnin fylgir kvikmyndinni frekar en að standa við hliðina á henni.** Þetta er ein staðreynd — við hvað
þetta skjal fylgir — og þriðja tilvísunin myndi setja hana í sama sæti og
[netið sem er dregið af staðsetningunum](frame-grid.md), sem er önnur.

## Umritun hljóðsins

**`Transcribe Audio…`** skrifar tal kvikmyndarinnar sem skjátexta. Hún er óvirk þar til kvikmynd er opin: hún umritar
hljóð **kvikmyndar verkefnisins**, aldrei skrá sem þú velur — til að umrita
hljóðskrá skaltu nota [`transcribe`](../subedit-cli/transcribe.md) á skipanalínunni.

Glugginn biður um fernt og segir hvort hann geti byrjað:

- **hljóðrásina**, taldar upp eins og valmyndin [`Audio`](#hljóðrásin) telur þær upp, með tungumálinu
  og titlinum sem skráin gefur upp;
- **tungumálið**, valið í **lista**: fyrst færsla sem greinir það, „Greina af fyrstu 30 sekúndum hljóðsins“, síðan tungumálin sem
  vélin býður fyrir líkanið, með heiti á tungumáli gluggans og kóða þeirra á eftir — „Franska (fr)“. Listinn er vélarinnar sjálfrar:
  um hundrað með fjöltyngdu líkani; með líkani fyrir ensku eingöngu hefur hann eina færslu, ensku, og ekki er hægt að breyta henni,
  og lína segir að kvikmyndin komi út á ensku hvaða tungumál sem hún talar. Listinn byrjar á tungumáli valinnar rásar þegar skráin
  gefur það upp og vélin býður það, fylgir rásinni svo lengi sem þú hefur ekki valið eitt, og
  **lína undir honum segir hvaðan það kemur**. Þegar skráin gefur ekkert tungumál upp fyrir rásina segir línan það, og varar við:
  líkanið greinir það þá af fyrstu þrjátíu sekúndum hljóðsins, og **kvikmynd sem byrjar á tónlist eða þögn getur verið tekin fyrir annað tungumál og talmálið þá þýtt yfir á það**.
  Veldu tungumálið ef þú veist það;
- **hljóð skrifuð sem athugasemdir**: reitur, „Ekki skrifa hljóð sem athugasemdir“, hakaður sjálfgefið. Vélinni er þá sagt að skrifa ekki hljóð sem
  athugasemdir, eins og `(phone ringing)` eða `[MUSIC PLAYING]`; þar sem hún hefur ekkert tal til að skrifa skrifar hún `...`, og bútar sem hafa hvorki staf né tölustaf
  eru felldir niður í eftirvinnslunni. Taktu hakið af reitnum til að halda athugasemdunum, fyrir skjátexta fyrir heyrnarskerta. Reiturinn er grár, með ástæðunni,
  fyrir vél sem ekki er hægt að segja það. Þetta er stillingin `transcription.suppress-notes`, sem [kjörstillingarnar](preferences.md#umritun)
  geyma líka;
- **hluta kvikmyndarinnar**, frá einum tíma til annars — sjá [Að prófa hluta fyrst](#að-prófa-hluta-fyrst). Séu báðir reitirnir
  tómir er átt við alla kvikmyndina.

**Hann segir líka hvað vantar.** Þrjár línur segja hvort vélin, `ffmpeg` og líkanið séu til staðar, og
hvar; þegar eitthvað vantar helst `OK` óvirkt, og glugginn segir það með tilvísun á
[Uppsetning umritunartækjanna](../subedit-cli/install-transcription.md). Hnappurinn `Preferences…` opnar
[kjörstillingarnar](preferences.md#umritun), þar sem forritið og líkanið eru stillt — þetta eru staðreyndir
um tölvuna, ekki val sem þarf að endurtaka við hverja umritun — og glugginn skoðar aftur þegar þeim er lokað.

Tungumálið er geymt til næsta skiptis. Vélin sjálf og mörk skjátextanna eru
[stillingar](preferences.md) (`transcription.*`).

Vinnan fer fram **í bakgrunni**, með glugga og hnappi sem heitir `Cancel`; glugginn er áfram nothæfur í kringum hana. **Glugginn
segir hvar vinnan er stödd**, svo að þú vitir að hún lifir og hve lengi hún tekur:

- **skrefið**: fyrst „Dreg út hljóðið…“ — glugginn er þá upptekinn, án tölu, því útdráttur segir ekkert um hve langt er komið —,
  síðan „Umrita hljóðið…“, með stiku;
- **tíminn sem eftir er**, „um 38 mín. eftir“: fjarverandi í upphafi, birtist þegar vélin hefur skilað nægri framvindu til að framreikna út frá, og reiknaður út frá hraða hennar frá byrjun og fram að síðustu mínútu; nákvæmnin er nokkrar mínútur, ekki loforð;
- **tíminn sem liðinn er**, „liðið: 0:05:12“, sem líður jafnvel þegar ekkert annað hreyfist;
- **staðan**, „0:12:30 af 0:50:00“: hve langt inn í kvikmyndina vélin er komin, um leið og hún hefur skilað fyrsta textanum sínum, birt **rétt fyrir ofan síðasta textann** sem hún á við;
- **síðasti textinn** sem fékkst, svo að þú sjáir að vinnan er skynsamleg.

Á langri kvikmynd með stóru líkani á örgjörva geta fyrstu mínúturnar verið langar: vélin er að hlaða og vinna í fyrstu
þrjátíu sekúndna gluggunum sínum. Staðan og síðasti textinn hreyfast á undan eigin prósentu vélarinnar, sem kemur aðeins í skrefum
upp á 5 % af kvikmyndinni.

### Forskoðunin

**Ekkert skiptir út skjátextum verkefnisins fyrr en þú hefur séð þá.** Þegar vinnunni lýkur — og líka þegar þú hættir við hana, eða hún mistekst
eftir að hafa skilað einhverju — sýnir gluggi niðurstöðuna í tveimur töflum hlið við hlið, hvor með **sitt eigið upphaf og endi**:

- **til vinstri, „Eins og vélin skrifaði það“**: allt sem vélin skilaði, eins og hún skar það, án nokkurrar eftirvinnslu — það sem
  [`--raw`](../subedit-cli/transcribe.md) myndi skrifa —, staðgenglar (`...`) meðtaldir;
- **til hægri, „Eins og þeir verða virkjaðir“**: skjátextarnir eftir eftirvinnsluna — staðgenglar felldir niður, nálægir bútar sameinaðir, langir
  bútar klipptir —, nákvæmlega það sem fer í verkefnið.

Töflurnar tvær raðast ekki línu fyrir línu, og þess vegna hefur hvor sína eigin tíma. **Þegar skjátexti til hægri er valinn eru bútarnir sem hann var gerður úr
auðkenndir til vinstri**, og vinstri hliðin skrunar að þeim; einn skjátexti í einu, og aðeins í þá átt. Hvorug taflan er breytanleg.

Tveir hnappar loka glugganum. **„Virkja“** skiptir út skjátextum verkefnisins **í einni aðgerð**: ein `Undo` skilar því sem var, þýðingartextum
meðtöldum. **„Hunsa“** hendir niðurstöðunni, og hún glatast. Það er eina staðfestingin — ekkert er spurt áður en vinnan hefst. „Virkja“ er óvirkt þegar
ekkert er til að virkja.

**Þegar keyrslan var stöðvuð** segir glugginn það — „Hlutaniðurstaða: vinnan stöðvaðist við …“ — og býður það sem fékkst fram að því, til að virkja
eða hunsa, svo að löng keyrsla sem stöðvuð er seint tapi ekki vinnu sinni óumbeðið. Þessir bútar hafa engin tímasett orð (vélin skrifar þau í lokin), og
eftirvinnslan klippir á mörkum bútanna sjálfra. Bilun er fyrst sögð í glugga, eins og hver bilun; ekkert slíkt er sagt þegar hætt er við.

### Að prófa hluta fyrst

Bil gerir þér kleift að **meta líkan eða tungumál á tveimur mínútum áður en lagt er í tvær klukkustundir**. Reitirnir „Hluti kvikmyndarinnar“ taka upphaf og endi, hvort um sig skrifað `HH:MM:SS`, eins og reitirnir segja (`2:00` og fjöldi sekúndna, `90`,
eru líka lesin); nóg er að fylla út annan (þaðan til enda, eða frá upphafi þangað). `OK` helst óvirkt meðan reitur
er ekki tími eða endirinn kemur ekki á eftir upphafinu, og lína undir reitunum segir hvor það er.

- **Skjátextarnir halda tíma kvikmyndarinnar**: það sem sagt er á 2:10 er á `0:02:10` í niðurstöðunni.
- **Það fer um forskoðunina eins og hver umritun**, og skiptir út skjátextum verkefnisins þegar það hefur verið virkjað; ein `Undo` skilar þeim.
  Til að prófa bil án þess að snerta vinnuna þína skaltu hunsa forskoðunina, eða vista fyrst.

**Það sem hún þarf fylgir ekki forritinu**: vél (whisper.cpp), líkan og `ffmpeg` til að draga hljóðið út.
Sjá [Uppsetning umritunartækjanna](../subedit-cli/install-transcription.md).

## Þegar engin kvikmynd er

Glugginn sýnir hnappinn **`Select Video…`** þar sem myndin væri, frekar en
ekkert. Það er sama skipun og í valmyndinni `Video`, innan seilingar
músarinnar: fjarvera sem þú getur ekkert gert við er erfitt að greina frá
bilun.

Hnappurinn hverfur um leið og kvikmynd er opin og myndin tekur **allt** sitt
pláss: hæðina sem taflan skilur eftir fyrir ofan hana — þá sem
[`window.table-share`](preferences.md) mundi frá einni lotu til annarrar —, og
aldrei minna en lágmarkshæð myndarinnar, sem taflan lætur eftir ef þarf.
Þegar kvikmyndin fer tekur hnappurinn aftur sama stað.

**Nafn skrárinnar, aldrei slóðin.** Slóð upp á tvö hundruð stafi
myndi ýta öllu öðru út úr stikunni; heila slóðin er sú sem
glugginn sýndi.

## `ffmpeg` er ekki nauðsynlegt

`subedit-gui` spyr `ffprobe` — sem fylgir `ffmpeg` — um eitt og aðeins eitt:
**rammatíðnina sem gámurinn lýsir yfir**. Hún er lesin einu sinni
fyrir hverja kvikmynd, þegar hún er tengd.

| Með `ffprobe` | Án þess |
| :------------ | :------ |
| stöðulínan skrifar rammatíðnina við hlið nafns kvikmyndarinnar | hún skrifar aðeins nafnið |
| umbreytingarglugginn stingur upp á rammatíðni kvikmyndarinnar í neðsta reitnum | hann stingur ekki upp á neinu og opnast eins og áður |
| jöfnunarglugginn opnast á henni | hann opnast á rammatíðni verkefnisins |

**Ekkert annað er háð því og engin aðgerð neitar.** Spilarinn
þarf aftur á móti alls ekki á `ffprobe` að halda: hann reiðir sig á `libmpv`, sem er
háð við smíði. Lengd kvikmyndarinnar, sú sem er notuð til að tilkynna það sem
fer út fyrir endann, kemur frá spilaranum en ekki frá `ffprobe`.

**Umritunin er það eina sem þarf í raun `ffmpeg`**: það dregur út hljóð kvikmyndarinnar. Sjá
[Uppsetning umritunartækjanna](../subedit-cli/install-transcription.md).

Þetta er því ekki skert ástand heldur **eðlileg virkni** á vél
sem hefur aldrei sett upp `ffmpeg`: gögnin koma frá notandanum og `ffprobe`
stingur upp á þeim þegar það er til staðar.

Rammatíðni sem er lesin er **nákvæmt hlutfall** — `24000/1001`, ekki `23.976`.
Það er ástæðan fyrir því að `ffprobe` er áfram notað fyrir það svar: spilarinn
þekkir það sama sem fleytitölu, og áætluð rammatíðni
myndi tímastilla alla skrána örlítið skakkt.
