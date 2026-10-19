# Ajoitus videon avulla

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

*Oletetaan, että elokuva on auki ja soitin tuttu: katso [Soitin](player.md).*

Katsot elokuvaa, pysäytät sen ruutuun, josta rivin on määrä alkaa, ja **tästä kohdasta tulee
kappaleen alku**. Viisi toimintoa valikossa **Video**, Gaupolin antamilla nimillä. Ne
perustuvat siihen, mikä on jo olemassa: kohdan asettaminen on **sama komento kuin arvon kirjoittaminen
soluun**, ja lisääminen on **sama kuin `Muokkaa ▸ Lisää kappaleita…`**.

| Komento | Pikanäppäin | Mitä se tekee |
| :------ | :------- | :----------- |
| Video ▸ Aseta alku videon kohdasta | `Alt+U` | asettaa valitun kappaleen **alun** elokuvan kohtaan |
| Video ▸ Aseta loppu videon kohdasta | `Alt+K` | asettaa sen **lopun** elokuvan kohtaan |
| Video ▸ Lisää kappale videon kohtaan | `Alt+J` | lisää kappaleen, joka **alkaa** kohdasta, ja valitsee sen |
| Video ▸ Valitse edellinen videon kohdasta | `Alt+Up` | valitsee viimeisen kappaleen, joka **alkaa ennen** kohtaa |
| Video ▸ Valitse seuraava videon kohdasta | `Alt+Down` | valitsee ensimmäisen kappaleen, joka **alkaa kohdan jälkeen** |

**Mikään niistä ei ole käytettävissä, kun videota ei ole avattu**, kuten soittimen toiminnot. `Set Start…` ja `Set End…`
eivät ole käytettävissä myöskään niin kauan kuin mitään ei ole valittu: ne kohdistuvat valinnan
**ensimmäiseen riviin**.

**Pikanäppäimet ovat Gaupolin pikanäppäimiä, ja niissä on `Alt`**: Gaupolissa ne ovat `U`, `K` ja `J` — pelkkiä kirjaimia, jotka
ikkuna sieppaisi ennen muokattavana olevaa solua, niin että kirjaimen `k` kirjoittaminen tekstiin
asettaisi lopun. `Select Previous…` ja `Select Next…` ovat Gaupolissa `Ctrl+Y` ja `Ctrl+U`; `Ctrl+Y`
on joissakin järjestelmissä *tee uudelleen*, ja nuolinäppäimet yhdessä näppäimen `Alt` kanssa ilmaisevat *ennen* ja *jälkeen* paremmin kuin
kaksi kirjainta. Muokattavana oleva solu **vahvistetaan** ennen toimintoja `Set Start…`, `Set End…` ja
`Insert Subtitle…`, kuten ennen mitä tahansa muutosta.

## Reunan asettaminen

- **Yksi historiamerkintä toimintoa kohden**: `Muokkaa ▸ Kumoa` palauttaa edellisen kohdan eikä mitään muuta.
- **Samat säännöt kuin solussa.** Kappaleen lopun jälkeen asetettua alkua tai ennen alkua asetettua
  loppua **ei hylätä**: se asetetaan, ja taulukko merkitsee kappaleen samoin kuin kappaleen,
  jonka arvo on kirjoitettu käsin — katso [Poikkeamat](table.md#poikkeamat). Mitään ei korjata hiljaa.
- **Kohta, joka on jo reunan kohta, ei tee mitään** eikä lisää historiaan mitään.
- **Kohta on aina ruudun alku**: soitin ei koskaan ilmoita kohtaa
  ruudun keskeltä. Kun reuna asetetaan ja siihen palataan — esimerkiksi toiminnolla `Seek Selection Start` —,
  **näkyy sama ruutu**.

## Lisääminen kohtaan

Uusi kappale **alkaa kohdasta** ja kestää **kolme sekuntia** tai seuraavan kappaleen alkuun asti,
jos se tulee aiemmin. Sen teksti on tyhjä. Se asettuu paikalleen tiedoston järjestyksessä: kaikkien
niiden jälkeen, jotka alkavat tästä kohdasta tai sitä ennen. Lisäys on **yksi historiamerkintä**,
ja lisätty kappale valitaan: voit kirjoittaa sen heti.

## Valitseminen kohdasta

Molemmat noudattavat tiedoston järjestystä eivätkä nykyistä valintaa. **Kun tällä puolella ei ole naapuria**,
valinta siirtyy tiedoston viimeiseen kappaleeseen toiminnolla `Select Next…` ja ensimmäiseen toiminnolla
`Select Previous…` — Gaupolin tapaan. Nämä kaksi toimintoa eivät siirrä elokuvaa.

## Ruutu ruudulta

Siirtyminen eteen- ja taaksepäin **yhden ruudun** — tai N ruudun, katso alempaa — verran, jotta reuna asettuu **tarkalleen oikeaan ruutuun**.
Kaksi toimintoa ja kaksi painiketta toistopalkissa toistopainikkeen molemmin puolin:

| Komento | Pikanäppäin | Mitä se tekee |
| :------ | :------- | :----------- |
| Video ▸ Ruutu taaksepäin | `Alt+Left` | siirtää elokuvaa taaksepäin ruutuaskeleen verran |
| Video ▸ Ruutu eteenpäin | `Alt+Right` | siirtää sitä eteenpäin ruutuaskeleen verran |

- **Askel on ruutujen määrä, vähintään yksi**: se on [asetusten](preferences.md#soitin) asetus
  `Frame step`. N ruudun askel näyttää ruudun, joka on **N ruutua edempänä**, niin nopeudella 25 ruutua
  sekunnissa kuin nopeudella 23,976; sitä **ei koskaan ilmaista millisekunteina**.
- **Elokuva pysähtyy rajoilleen**: askel taaksepäin ensimmäisestä ruudusta jättää sen siihen, samoin askel eteenpäin
  viimeisestä. Toisto pysäytetään askeleen jälkeen.
- **Palkin painikkeet** — kaksi kuvaketta toista/tauko-painikkeen molemmin puolin — **käynnistävät samat toiminnot kuin valikko**: sama askel, ei käytettävissä
  ilman videota kuten valikon kohdat, ja sama työkaluvihje, joka kertoo pikanäppäimen. **Kun sormi pysyy
  painikkeella tai näppäimellä, askeleet seuraavat toisiaan.**
- **Yksi askel kerrallaan.** Pohjassa pidetty näppäin toistuu nopeammin kuin askel ehditään ottaa: ensimmäinen toisto lähtee
  heti, ja ne, jotka saapuvat askeleen ollessa käynnissä, **eivät kasaannu** — ne jakavat yhden paikan, viimeisen,
  joka lähtee sen jälkeen. Näppäimen vapauttaminen pysäyttää siis kuvan siihen, missä olet, eikä anna elokuvan
  jatkaa vauhdillaan.

## Reunan siirtäminen yhdellä ruudulla

Reunan säätämiseen näppäimistöllä: **valitun kappaleen alku tai loppu** yhden askeleen verran aiemmaksi tai myöhemmäksi.

| Komento | Pikanäppäin | Mitä se tekee |
| :------ | :------- | :----------- |
| Video ▸ Aikaista alkua | `Alt+Q` | kappale **alkaa askeleen aiemmin** |
| Video ▸ Myöhennä alkua | `Alt+W` | se **alkaa askeleen myöhemmin** |
| Video ▸ Aikaista loppua | `Alt+Shift+Q` | se **päättyy askeleen aiemmin** |
| Video ▸ Myöhennä loppua | `Alt+Shift+W` | se **päättyy askeleen myöhemmin** |

`Q` ja `W` ovat Gaupolin kirjaimet sanoille *aiemmin* ja *myöhemmin*, ja niissä on `Alt` edellä mainitusta syystä; loppu
saa yhdistelmän `Alt+Shift`, koska Gaupolin `E` on tässä valikon `Edit` kirjain.

- **Yksi historiamerkintä siirtoa kohden**, kumottavissa. Naapurikappale ja päällekkäisyys ilmoitetaan
  **samoin kuin kirjoitettaessa**: reuna asetetaan, ja taulukko merkitsee sen, mikä ei enää ole johdonmukaista — katso
  [Poikkeamat](table.md#poikkeamat).
- **Sama askel kuin soittimessa**, siis oletuksena yksi ruutu. **Ruudun pituus** luetaan tässä järjestyksessä:
  **videon ilmoittamasta ruutunopeudesta**, **asiakirjan ruutunopeudesta, kun se laskee ruutuina** (MicroDVD), ja sitten
  **kohtien muodostamasta ruudukosta**. **Jos mitään näistä kolmesta ei ole, toiminto kieltäytyy ja kertoo sen**:
  sattumanvaraisen ruutunopeuden valitseminen siirtäisi reunaa määrällä, joka ei ole ruutu.
- **Se ei vaadi videota**, ainoastaan valinnan: sitä käytetään myös ruutuina lasketussa tiedostossa ilman
  elokuvaa. Kun elokuva on auki, **toisto siirtyy uuteen kohtaan** näyttämään ruudun, jonka reuna
  juuri sai.
- Reuna ei mene alkupisteen eteen: askel taaksepäin, kun matkaa on alle ruudun, asettaa sen nollaan.
