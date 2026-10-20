# Afturkalla og endurgera

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

Hver aðgerð fer í feril og er hægt að afturkalla hana. Breyttur reitur telst
sem **ein** færsla: ekki er skráð hvert einasta lyklaborðsslag, aðeins
staðfestingin telur.

| Aðgerð | Flýtilykill | Hvar |
| :----- | :---------- | :--- |
| Afturkalla | `Ctrl+Z` | valmyndin **Breyta**, verkfærastikan |
| Endurgera | `Ctrl+Shift+Z` | valmyndin **Breyta**, verkfærastikan |

## Aðgerðin nefnir það sem hún afturkallar

Valmyndin segir ekki „Afturkalla“ heldur **„Afturkalla: breyting á texta“**, á
eftir heiti aðgerðarinnar sem verður afturkölluð. Hnappur verkfærastikunnar
heldur orðinu einu sér — breidd hans myndi annars fylgja síðustu aðgerð og
hann myndi hreyfast undir bendlinum —, en í ábendingunni stendur allt
heitið.

| Aðgerð | Það sem aðgerðin sýnir |
| :----- | :--------------------- |
| breyttur textareitur | `editing a text` |
| upphaf, endir | `editing a start`, `editing an end` |
| tímalengd | `editing a duration` |
| innsetning lína | `inserting` |
| fjarlæging lína | `removing` |
| færsla, þar með talin `Shift Whole File onto Grid` | `shifting` |
| umbreyting | `transforming` |
| umbreyting rammatíðni | `converting the frame rate` |
| jöfnun á rammatíðni | `aligning on the frame rate` |
| fjarlæging tilvísana | `removing hearing-impaired mentions` |
| skáletrun | `putting in italics` |
| skáletrun tekin af | `taking italics out` |
| breyting á stafstöðu | `changing the case` |
| þankastrikum bætt við eða þau fjarlægð | `adding dialogue dashes`, `removing dialogue dashes` |
| sameining lína | `merging` |
| skipting línu | `splitting` |
| textar klipptir eða límdir | `cutting texts`, `pasting texts` |
| lagfæring tímalengda | `adjusting durations` |
| opnun þýðingar | `opening a translation` |
| skrá bætt aftan við | `appending a file` |
| verkefni skipt upp | `splitting the project` |
| leiðrétting samþykkt í `Correct Texts…` eða gerð með `Check Spelling…` | `correcting texts` |
| skipti, öll skipti | `replacing`, `replacing all` |

**Tuttugu og sjö heiti, og fleiri getur glugginn ekki framleitt.** Kjarninn
nefnir tuttugasta og áttunda — `sorting` — sem engin aðgerð gluggans nær til:
röðun á sér aðeins stað undir strangri reglu um röð, en glugginn opnar skjölin
sín undir mildri reglu, sem tilkynnir óreiðu í stað þess að lagfæra hana.

**`Shift Whole File onto Grid` er ekki aðgreind frá færslu**, og það er rétt:
hún er færsla, þar sem stærðin var mæld en ekki slegin inn.

Þetta eru sömu orð og á skipanalínunni: þau eru rituð einu sinni, í kjarnanum.

Þegar ekkert er til að afturkalla stendur aðeins „Afturkalla“ í aðgerðinni
**og hún er óvirk**: flýtilykill hennar gerir ekkert.

## Breytingamerkið

Í titli gluggans er merki svo lengi sem skráin er frábrugðin þeirri sem er á
disknum — stjarna, eða það sem kerfið notar í hennar stað. **Sama gildir um
heiti flipans**, `premier.srt*`: titillinn talar aðeins um verkefnið sem þú
horfir á, heitið á flipanum segir frá þeim sem þú horfir ekki á.

**Hver flipi hefur sinn eigin feril.** Afturköllun afturkallar aðeins það sem
verkefni núverandi flipa hefur fengið — þar með talið `Replace All` sem gert
var yfir öll verkefni: hún skilur eftir eina færslu í hverju verkefni sem hún
snerti.

**Merkið hverfur ef þú afturkallar alla leið að upphafspunktinum.** Þetta er
ekki útlitsatriði: fjöldi breytinga er talinn en ekki skráður sem já eða nei,
þannig að með því að fara til baka má koma honum niður í núll og þá hverfur
merkið.

## Tvennt sem gott er að vita

**Viðmiðunarpunkturinn er síðasta vistun**, eða opnun skrárinnar ef engin
vistun hefur átt sér stað. Vistun færir hann: merkið hverfur og afturköllun
eftir það kallar það fram aftur.

Aðgerð sem gerð er eftir afturköllun **hreinsar það sem var hægt að
endurgera**: endurgerð myndi spila aftur skipun þar sem upphafsástandið er ekki
lengur til. Þetta er hegðun allra ritla og hún er vísvitandi.

**Meðan reitur er opinn** staðfesta `Undo` og `Redo` — úr valmyndinni eða af
verkfærastikunni — fyrst innsláttinn, eins og aðrar aðgerðir í
[Að breyta reit](editing.md). Það sem var að ljúka við að slá inn verður þá
síðasta færslan í ferlinum: `Undo` afturkallar hana, ekki aðgerðina á undan;
`Redo` hefur ekkert eftir til að endurgera, þar sem innsláttur hreinsar það sem
var hægt að endurgera. Opinn reitur þar sem ekkert var slegið inn skilur enga
færslu eftir: `Undo` afturkallar þá fyrri aðgerð.

Ferillinn geymir þúsund færslur og gleymir þeim elstu umfram það.
