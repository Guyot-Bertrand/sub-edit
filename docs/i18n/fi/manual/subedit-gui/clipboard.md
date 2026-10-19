# Tekstien leikkaaminen, kopioiminen ja liittäminen

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

Kolme valikon `Edit` kohtaa, kohtien `Undo` ja `Redo` alapuolella ja kohdan
`Find and Replace…` yläpuolella.

| Kohta | Pikanäppäin | Mitä se tekee |
| :---- | :------- | :----------- |
| Leikkaa tekstit | `Ctrl+X` | kopioi valinnan tekstit ja tyhjentää ne sitten |
| Kopioi tekstit | `Ctrl+C` | kopioi valinnan tekstit |
| Liitä tekstit | `Ctrl+V` | kirjoittaa kopioidut tekstit ensimmäisestä valitusta rivistä alkaen |

**Siirtyvät vain tekstit, eikä mikään muu**: ei alkua, ei loppua, ei kokonaista
riviä. Liittäminen ei siis siirrä mitään kohtaa — tekstin vastaanottavan rivin
ajoitus pysyy sen omana. Näin toimii myös Gaupol.

## Mihin ne kohdistuvat

**Kun projektissa on käännös, nykyisen sarakkeen tekstiin**: käännökseen, jos
nykyinen solu on sarakkeessa `Translation`, ja muualla pääasiakirjan tekstiin —
katso [Taulukko](table.md#teksti-johon-toiminnot-kohdistuvat). `Cut Texts` tyhjentää siis
käännöksen ja jättää pääasiakirjan tekstin, ja `Paste Texts` kirjoittaa siihen
sarakkeeseen, jossa olet. **Leikepöytä ei kuljeta mukanaan asiakirjaa**: voit
kopioida pääasiakirjan tekstin solun ja liittää sen käännökseen, ja
päinvastoin.

**Valinta, eikä koskaan koko tiedosto.** Kolme kohtaa eivät ole käytettävissä niin kauan
kuin mitään ei ole valittu. Muualla ikkunassa ”mitään ei ole valittu” tarkoittaa
”koko tiedostoa”; tässä vahingossa painettu `Ctrl+X` tyhjentäisi asiakirjan kaikki
tekstit, eikä liittämisellä olisi riviä, josta aloittaa.

Kun solu on auki, `Ctrl+X`, `Ctrl+C` ja `Ctrl+V` kuuluvat
syöttökentälle: ne leikkaavat, kopioivat ja liittävät merkkejä, aivan kuten `Del`
poistaa siellä merkin eikä kappaletta. Valikosta käynnistettyinä kohdat vahvistavat ensin
syötteen: `Copy Texts` kopioi silloin tekstin sellaisena kuin se on juuri
kirjoitettu. Katso [Solun muokkaaminen](editing.md).

## Kopioiminen

`Copy Texts` kopioi kunkin valitun rivin tekstin ensimmäisestä
viimeiseen. **Epäyhtenäinen valinta säilyttää muotonsa**: valinnan ulkopuolelle jäänyt
rivi muuttuu aukoksi, eikä liittäminen kirjoita sen päälle.

**Valittu rivi, jonka teksti on tyhjä, muuttuu myös aukoksi**:
liitettynä se jättää kohderivin ennalleen. Yksittäisen tyhjän tekstin kopioiminen ei siis
liitä mitään.

Järjestelmän leikepöytä saa nämä tekstit **pelkkänä tekstinä, tyhjällä rivillä
erotettuina** — aukko kirjoitetaan sinne tyhjänä tekstinä, ja siksi tyhjä
teksti ei voi olla muuta kuin aukko: liittäminen antaa saman tuloksen
riippumatta siitä, tuleeko kopio tästä ikkunasta vai toisesta ohjelmasta. Täällä kopioimasi voi
siis liittää mihin tahansa ohjelmaan, ja muualla kopioimasi voi liittää tänne.

Kopioiminen ei muuta asiakirjaa eikä tule historiaan.

## Leikkaaminen

`Cut Texts` kopioi ja sitten **tyhjentää valittujen rivien tekstit**. Rivit
säilyvät kohtineen: leikkaaminen ei ole poistamista — rivit
poistetaan komennolla [`Remove Subtitles`](lines.md#poistaminen).

Jo valmiiksi tyhjien tekstien leikkaaminen ei tee mitään eikä tule
historiaan.

## Liittäminen

`Paste Texts` kirjoittaa leikepöydän tekstit **ensimmäisestä valitusta
rivistä alkaen**, yksi rivi kutakin tekstiä kohden, alaspäin.

| Mitä leikepöytä sisältää | Mitä tapahtuu |
| :-------------------------- | :----------- |
| tekstin | se korvaa rivin tekstin |
| aukon tai kopioidun tyhjän tekstin | rivi säilyttää tekstinsä |
| enemmän tekstejä kuin rivejä on jäljellä | loppuun lisätään rivejä niiden vastaanottamiseksi |

Lisätyt rivit syntyvät kuten viimeisen rivin jälkeen lisätyt rivit: kukin kestää kolme
sekuntia, peräkkäin. **Ikkuna kertoo sen** tietoikkunassa:

```text
inserted 2 subtitles to fit the clipboard
```

Liittämisen jälkeen **kirjoitetut rivit ovat valittuina**.

Liittäminen, joka ei muuta mitään — samat tekstit samoilla riveillä —, ei tule
historiaan.

### Toisen muodon tagit muunnetaan

Tässä ikkunassa tehty kopio **muistaa asiakirjansa muodon**, ja
tämä muisti säilyy toisen tiedoston avaamisen yli. Liittäminen toisen
muodon asiakirjaan muuntaa siis tagit, kuten `Save As…` tekee:
Advanced SSA -tiedostosta kopioitu `{\i1}Bonjour{\i0}` liitetään SubRip-tiedostoon
muodossa `<i>Bonjour</i>`.

**Se, mitä kohdemuoto ei pysty kirjoittamaan, menetetään, ja se kerrotaan**
komennon `Save As…` sanoin:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

Toisin kuin komennossa `Save As…`, mitään ei kysytä etukäteen: liittäminen kumotaan
näppäimillä `Ctrl+Z`, mikä palauttaa tekstit ennalleen.

**Ohjelman `subedit` ulkopuolella kopioidulla tekstillä ei ole muotoa**, ja se liitetään sellaisenaan:
tyhjästä ei ole mitään muunnettavaa. Sama koskee tästä ikkunasta tehtyä kopiota,
jonka järjestelmän leikepöytä on sittemmin korvannut — merkitystä on
järjestelmän leikepöydän sisällöllä.

Kun molemmat viestit on näytettävä, ne mahtuvat yhteen ikkunaan
puolipisteellä erotettuina.

## Mitä kumoamistoiminto siitä kertoo

Jokainen toiminto on **yksi** historiamerkintä riippumatta siitä, kuinka monta
riviä se kirjoittaa tai lisää.

| Toiminto | Mitä `Undo` näyttää |
| :-------- | :---------------- |
| leikkaaminen | `Undo: cutting texts` |
| liittäminen | `Undo: pasting texts` |

Katso [Kumoa ja tee uudelleen](undo.md).
