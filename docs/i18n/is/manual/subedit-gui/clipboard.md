# Að klippa, afrita og líma texta

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

Þrjár færslur í valmyndinni `Edit`, undir `Undo` og `Redo`, og fyrir ofan
`Find and Replace…`.

| Færsla | Flýtilykill | Hvað hún gerir |
| :----- | :---------- | :------------- |
| Klippa texta | `Ctrl+X` | afritar texta valsins og tæmir þá síðan |
| Afrita texta | `Ctrl+C` | afritar texta valsins |
| Líma texta | `Ctrl+V` | skrifar afrituðu textana frá fyrstu völdu línu |

**Það eru textar sem ferðast, og ekkert annað**: ekkert upphaf, enginn endir,
engin heil lína. Líming færir því enga tímasetningu — tímasetning línunnar sem
tekur við texta helst óbreytt. Þannig gerir Gaupol þetta.

## Á hvað þær verka

**Með þýðingu, á texta núverandi dálks**: þýðinguna ef núverandi reitur er í
dálkinum `Translation`, annars aðaltextann —
sjá [Taflan](table.md#textinn-sem-aðgerðirnar-beinast-að). `Cut Texts` tæmir því
þýðinguna og lætur aðaltextann í friði, og `Paste Texts` skrifar í dálkinn sem
þú ert í. **Klippispjaldið ber ekkert skjal**: þú getur afritað reit úr
aðaltextanum og límt hann í þýðinguna, og öfugt.

**Valið, en aldrei öll skráin.** Færslurnar þrjár eru óvirkar svo lengi sem
ekkert er valið. Annars staðar í glugganum þýðir „ekkert valið“ „öll skráin“;
hér myndi óheppilegt `Ctrl+X` tæma alla texta skjalsins, og líming hefði enga
línu til að byrja á.

Meðan reitur er opinn tilheyra `Ctrl+X`, `Ctrl+C` og `Ctrl+V` innsláttarreitnum:
þau klippa, afrita og líma stafi, rétt eins og `Del` eyðir staf þar en ekki
skjátexta. Þegar færslurnar eru ræstar úr valmyndinni staðfesta þær fyrst
innsláttinn: `Copy Texts` afritar þá textann eins og hann var rétt í þessu
sleginn inn. Sjá [Að breyta reit](editing.md).

## Afritun

`Copy Texts` afritar texta hverrar valinnar línu, frá þeirri fyrstu til þeirrar
síðustu. **Slitrótt val heldur lögun sinni**: lína sem er utan valsins verður
að gati, og límingin skrifar ekki yfir hana.

**Valin lína með tómum texta verður einnig að gati**:
þegar hún er límd skilur hún marklínuna eftir óbreytta. Afritun eins tóms texta
límir því ekkert.

Kerfisklippispjaldið fær þessa texta **sem hreinan texta, aðskilda með tómri
línu** — gat er skrifað þar sem tómur texti, og þess vegna getur tómur texti
ekki verið neitt annað en gat: líming gefur sömu niðurstöðu hvort sem afritunin
kemur úr þessum glugga eða úr öðru forriti. Það sem þú afritar hér má því líma
í hvaða forrit sem er, og það sem þú afritar annars staðar má líma hér.

Afritun breytir ekki skjalinu og fer ekki í ferilinn.

## Klipping

`Cut Texts` afritar og **tæmir síðan textana** í völdu línunum. Línurnar
haldast, með tímasetningum sínum: að klippa er ekki að fjarlæga — til að
fjarlægja línur er [`Remove Subtitles`](lines.md#fjarlæging).

Klipping texta sem eru þegar allir tómir gerir ekkert og fer ekki í
ferilinn.

## Líming

`Paste Texts` skrifar texta klippispjaldsins **frá fyrstu völdu línu**, eina
línu fyrir hvern texta, niður á við.

| Hvað klippispjaldið inniheldur | Hvað gerist |
| :----------------------------- | :---------- |
| texti | hann kemur í stað texta línunnar |
| gat, eða afritaður tómur texti | línan heldur texta sínum |
| fleiri textar en línur eru eftir | línum er bætt við í lokin til að taka við þeim |

Línurnar sem bætt er við fæðast eins og línur sem settar eru inn á eftir þeirri
síðustu: þrjár sekúndur hver, hver á eftir annarri. **Glugginn segir frá því**,
í upplýsingaglugga:

```text
inserted 2 subtitles to fit the clipboard
```

Eftir límingu eru **línurnar sem voru skrifaðar valdar**.

Líming sem breytir engu — sömu textar á sömu línum — fer ekki í
ferilinn.

### Merki úr öðru sniði eru þýdd

Afritun í þessum glugga **man snið skjalsins síns**, og það minni lifir af
opnun annarrar skrár. Líming í skjal með öðru sniði þýðir því merkin, eins og
`Save As…` gerir:
`{\i1}Bonjour{\i0}` afritað úr Advanced SSA er límt sem `<i>Bonjour</i>` í
SubRip.

**Það sem markniðið getur ekki skrifað tapast, og frá því er sagt**, með
orðum `Save As…`:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

Ólíkt `Save As…` er ekkert spurt fyrirfram: límingin er afturkölluð með
`Ctrl+Z`, sem skilar textunum eins og þeir voru.

**Texti sem er afritaður utan `subedit` hefur ekkert snið** og er límdur
óbreyttur: ekkert er til að þýða úr. Sama á við um afritun úr þessum glugga
sem kerfisklippispjaldið hefur síðan skipt út — það er innihald
kerfisklippispjaldsins sem gildir.

Þegar bæði skilaboðin eiga við rúmast þau í einum glugga, aðskilin með
semíkommu.

## Hvað afturköllunaraðgerðin segir um það

Hver aðgerð er **ein** færsla í ferlinum, hversu margar
línur sem hún skrifar eða bætir við.

| Aðgerð | Hvað `Undo` sýnir |
| :----- | :---------------- |
| klipping | `Undo: cutting texts` |
| líming | `Undo: pasting texts` |

Sjá [Afturkalla og endurgera](undo.md).
