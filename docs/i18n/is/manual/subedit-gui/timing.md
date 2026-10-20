# Tímasetning út frá myndskeiðinu

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

*Gert er ráð fyrir að kvikmyndin sé opin og spilarinn þekktur: sjá [Spilarinn](player.md).*

Þú horfir á kvikmyndina, stöðvar á rammanum þar sem lína á að hefjast og **sú staða verður
upphaf skjátextans**. Fimm aðgerðir, í valmyndinni **Myndskeið**, undir þeim heitum sem Gaupol gefur þeim. Þær
byggja á því sem er þegar til: að setja stöðu er **sama skipun og að slá gildið inn í
reitinn**, og innsetning er **sú sama og `Breyta ▸ Setja inn skjátexta…`**.

| Skipun | Flýtilykill | Hvað hún gerir |
| :----- | :---------- | :------------- |
| Myndskeið ▸ Setja upphaf frá stöðu myndskeiðs | `Alt+U` | setur **upphaf** valda skjátextans á stöðu kvikmyndarinnar |
| Myndskeið ▸ Setja endi frá stöðu myndskeiðs | `Alt+K` | setur **endi** hans á stöðu kvikmyndarinnar |
| Myndskeið ▸ Setja inn skjátexta á stöðu myndskeiðs | `Alt+J` | setur inn skjátexta sem **hefst** á stöðunni, og velur hann |
| Myndskeið ▸ Velja fyrri út frá stöðu myndskeiðs | `Alt+Up` | velur síðasta skjátextann sem **hefst á undan** stöðunni |
| Myndskeið ▸ Velja næsta út frá stöðu myndskeiðs | `Alt+Down` | velur þann fyrsta sem **hefst á eftir** stöðunni |

**Allar eru óvirkar án opins myndskeiðs**, eins og aðgerðir spilarans. `Set Start…` og `Set End…`
eru einnig óvirkar svo lengi sem ekkert er valið: þær verka á **fyrstu línu**
valsins.

**Flýtilyklarnir eru Gaupol, með `Alt`**: þar eru þeir `U`, `K` og `J` — berir stafir, sem
glugginn myndi taka á undan reitnum sem verið er að breyta, þannig að `k` sem slegið er inn í texta
myndi setja endi. `Select Previous…` og `Select Next…` eru `Ctrl+Y` og `Ctrl+U` í Gaupol; `Ctrl+Y`
er *endurgera* á sumum kerfum, og örvarnar með `Alt` segja *á undan* og *á eftir* betur en
tveir stafir. Reitur sem er í breytingu er **staðfestur** áður en `Set Start…`, `Set End…` eða
`Insert Subtitle…` er framkvæmd, eins og fyrir allar breytingar.

## Brún sett

- **Ein færsla í ferlinum fyrir hverja aðgerð**: `Breyta ▸ Afturkalla` skilar fyrri stöðunni og engu öðru.
- **Sömu reglur og í reitnum.** Upphaf sem er sett á eftir enda skjátextans, eða endir á undan
  upphafi hans, **er ekki neitað**: það er sett, og taflan merkir skjátextann eins og þann
  sem gildið var slegið inn á með hendi — sjá [Frávikin](table.md#frávikin). Ekkert er leiðrétt í kyrrþey.
- **Staða sem er þegar brún skjátextans gerir ekkert** og bætir engu við ferilinn.
- **Staðan er alltaf upphaf ramma**: spilarinn tilkynnir aldrei stöðu
  inni í ramma. Að setja brún og fara svo aftur á hana — með `Seek Selection Start`, til dæmis —
  **sýnir sama rammann**.

## Innsetning á stöðunni

Nýi skjátextinn **hefst á stöðunni** og varir í **þrjár sekúndur**, eða þar til sá næsti hefst
ef hann kemur fyrr. Texti hans er tómur. Hann tekur sæti sitt í röð skrárinnar: á eftir
öllum þeim sem hefjast á stöðunni eða á undan henni. Innsetningin er **ein færsla í ferlinum**
og innsetti skjátextinn er valinn: þú getur skrifað hann strax.

## Val út frá stöðunni

Báðar fylgja röð skrárinnar, ekki núverandi vali. **Þegar enginn nágranni er á þeirri hlið**
fer valið á síðasta skjátexta skrárinnar fyrir `Select Next…` og á þann fyrsta fyrir
`Select Previous…` — hegðun Gaupol. Þessar tvær aðgerðir færa ekki kvikmyndina.

## Ramma fyrir ramma

Að fara áfram og aftur **um einn ramma** — eða um N, sjá hér að neðan —, til að setja brún **á nákvæman ramma**.
Tvær aðgerðir, og tveir hnappar í spilunarstikunni, sitt hvorum megin við spilunarhnappinn:

| Skipun | Flýtilykill | Hvað hún gerir |
| :----- | :---------- | :------------- |
| Myndskeið ▸ Skref afturábak | `Alt+Left` | færir kvikmyndina aftur um rammaskrefið |
| Myndskeið ▸ Skref áfram | `Alt+Right` | færir hana áfram um rammaskrefið |

- **Skrefið er fjöldi ramma, einn að minnsta kosti**: það er stillingin `Frame step` í
  [kjörstillingunum](preferences.md#spilarinn). Skref upp á N ramma sýnir rammann **N ramma framar**, við 25 ramma á
  sekúndu eins og við 23,976; það er **aldrei gefið upp í millisekúndum**.
- **Kvikmyndin stöðvast við mörk sín**: skref aftur á bak frá fyrsta ramma skilur hana eftir þar, skref áfram frá þeim síðasta
  líka. Spilun er stöðvuð eftir skref.
- **Hnappar stikunnar** — tvö tákn, sitt hvorum megin við spila/hlé — **ræsa sömu aðgerðir og valmyndin**: sama skref, óvirkir
  án myndskeiðs eins og færslur valmyndarinnar, og sama ábending, sem gefur upp flýtilykilinn. **Fingur sem helst á
  hnappi, eða á lyklinum, keðjar skrefin.**
- **Eitt skref í einu.** Lykill sem er haldið niðri endurtekur hraðar en skref er tekið: fyrsta endurtekningin fer út
  strax, þær sem koma meðan skrefið er tekið **hlaðast ekki upp** — þær deila einu sæti, því
  síðasta, sem fer út á eftir. Að sleppa lyklinum stöðvar því myndina þar sem þú ert og lætur kvikmyndina
  ekki halda áfram af hreyfiorku sinni.

## Brún hnikað um einn ramma

Til að laga brún með lyklaborðinu: **upphaf eða endi valda skjátextans**, um eitt skref, fyrr eða síðar.

| Skipun | Flýtilykill | Hvað hún gerir |
| :----- | :---------- | :------------- |
| Myndskeið ▸ Hnika upphafi fyrr | `Alt+Q` | skjátextinn **hefst einu skrefi fyrr** |
| Myndskeið ▸ Hnika upphafi síðar | `Alt+W` | hann **hefst einu skrefi síðar** |
| Myndskeið ▸ Hnika endi fyrr | `Alt+Shift+Q` | hann **endar einu skrefi fyrr** |
| Myndskeið ▸ Hnika endi síðar | `Alt+Shift+W` | hann **endar einu skrefi síðar** |

`Q` og `W` eru Gaupol fyrir *fyrr* og *síðar*, með `Alt` af ástæðunni sem er gefin hér að ofan; endirinn
fær `Alt+Shift`, því `E` Gaupol er hér stafur `Edit`.

- **Ein færsla í ferlinum fyrir hvert hnik**, hægt að afturkalla. Nágrannaskjátextinn og skörunin eru tilkynnt
  **eins og við innslátt**: brúnin er sett, og taflan merkir það sem heldur ekki lengur — sjá
  [Frávikin](table.md#frávikin).
- **Sama skref og hjá spilaranum**, því einn rammi sjálfgefið. **Hvers virði rammi er** er lesið, í þessari röð,
  úr **rammatíðninni sem myndskeiðið lýsir yfir**, úr þeirri sem **skjalið hefur þegar það telur í römmum** (MicroDVD), síðan úr
  **netinu sem staðsetningarnar draga**. **Með ekkert af þessum þremur neitar aðgerðin og segir frá því**: að velja
  rammatíðni af handahófi myndi færa hverja brún um eitthvað sem er ekki rammi.
- **Hún krefst ekki myndskeiðs**, aðeins vals: hún er einnig notuð á skrá sem er talin í römmum, án
  kvikmyndar. Þegar kvikmynd er opin **fer spilunin á nýju stöðuna**, til að sýna rammann sem brúnin
  tók nú á sig.
- Brún fer ekki fyrir upphafspunkt: að stíga aftur um eitt skref frá minna en einum ramma setur hana á núll.
