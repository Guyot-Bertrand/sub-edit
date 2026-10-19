# `pair`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Kirjoittaa **päätiedoston kohtiin uudelleen ajoitetun käännöksen** ja kertoo, miten sen rivit löysivät
kappaleensa. Se on ikkunan `Open Translation…` — [sama parittaminen](../subedit-gui/files.md#käännöksen-avaaminen),
sama lause — ja sen perään tuloksen kirjoittaminen, jonka ikkuna tekee komennolla `Save Translation`.

**Se on ainoa asia, jonka parittaminen tekee ja jota yksin käsitelty tiedosto ei tee.** Muut alikomennot,
jotka ottavat valitsimen `-t`, muuttavat lukemaansa käännöstä — [`case`](case.md), [`replace`](replace.md)… —; `pair` ei muuta
tekstissä mitään: se **asettaa** käännöksen päätiedoston päälle ja kirjoittaa sen, mitä asettaminen tuotti. Päätiedosto
luetaan rivien vastaanottamista varten, **sitä ei koskaan kirjoiteta**.

- **Kirjoitettava tiedosto on käännös**, **sen omaan polkuun ja sen omassa muodossa**: käännöstiedoston muoto, merkistökoodaus ja
  rivinvaihdot. Sen kappaleet ovat **päätiedoston kohdissa**: rivi, joka osui merkin viereen,
  saa takaisin kääntämänsä kappaleen rajat.
- **`-t` on pakollinen.** Ilman käännöstä alikomennolla `pair` ei ole mitään asetettavaa: se on käyttövirhe, koodi `1`.
- **Yksi ainoa päätiedosto**, eikä valitsinta `--recursive`: pari on kaksi tiedostoa, eikä hakemistopuu voi
  mitenkään kertoa, mikä käännös kuuluu millekin päätiedostolle. Erää varten yksi kutsu paria kohden.
- **`--output` nimeää kirjoitettavan käännöksen tiedoston, `--output-dir` sijoittaa sen perusnimen sinne, `--in-place`
  kirjoittaa sen uudelleen**, kuten kaikilla alikomennoilla, jotka ottavat valitsimen `-t` — katso [Käännös](invocation.md#käännös).
- **Valitsinta `--document` ei ole**: kohteena on vain yksi asiakirja, käännös.

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentit ja valitsimet

| Valitsin | Pakollinen | Arvot | Oletus |
| :----- | :----- | :------ | :----- |
| `<main>` | kyllä | **yksi ainoa** polku | — |
| `-t`, `--translation-file` | **kyllä** | käännöstiedosto | — |
| `--align-method` | ei | `position` tai `number` — katso [Käännös](invocation.md#käännös) | `position` |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Käännös oli 0,3 s myöhässä: kukin rivi löysi kappaleen, jonka päälle sen keskikohta osuu, ja
kirjoitettu tiedosto saa päätiedoston kohdat.

## Mitä kohdistus kertoo

Lause on **sama, jonka ikkuna kertoo** avatessaan käännöksen, ja kukin sen osa näytetään vain, kun se ei ole
nolla — sen osien taulukko on sivulla [`inspect -t`](inspect.md#käännös--t). **Se kirjoitetaan
tasolta 1 alkaen** eikä tasolla 2 kuten muilla alikomennoilla: tässä kohdistus ei ole
käsittelyn yksityiskohta, se on tulos.

**Mitä kohdistus tekee kirjoitettavalle tiedostolle**:

- **kappale, jota kukaan ei kääntänyt, kirjoitetaan lohkona ilman tekstiä**, joka luetaan ilman poikkeamaa;
- **rivistä syntynyt kappale** — rivi, johon mikään ei ulottunut — kirjoitetaan **omiin kohtiinsa** menetelmästä
  riippumatta: kirjoitetussa tiedostossa voi siis olla enemmän kappaleita kuin päätiedostossa;
- **kaikki, mitä asiakirjassa oli käännöksenä, tyhjennetään** ennen asettamista, mikä ei koske komentoriviä:
  päätiedosto luetaan yksinään.

**Kaksi menetelmää** tuottavat saman tiedoston, kun rivit osuvat kukin omalle kappaleelleen, eivätkä tuota,
kun rivi puuttuu tai käännös on siirtynyt — siksi menetelmä on valinta, ja siksi
kohta on oletus. Hakemiston `src/test/data/paires/` kahdeksan tapausta luettelevat ne, ja kullekin menetelmälle on
odotettu tiedosto.

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto.

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **polku on käännöksen polku**, sillä käännös on kirjoitettava tiedosto; valitsimella `--dry-run` kohdan `-> <destination>` tilalla on `(dry run, nothing written)` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — **käännöksen** muoto, merkistökoodaus, merkki ja rivinvaihdot, palautettuina sellaisinaan |
| 3 | `<translation>: N bytes read, M written`, sitten **jokainen lukemisen huomautus** päätiedostosta ja käännöksestä — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) **yksi ainoa objekti**, jonka `file` on käännös ja
`destination` kirjoitettu tiedosto. `counts.subtitles` laskee **kirjoitetun tiedoston kappaleet**, myös ne, jotka
rivi herätti henkiin. Avain `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
sisältää samat lukumäärät kuin lause, kuten alikomennon [`inspect`](inspect.md#json-muodossa) avain `translation`.

## Paluuarvot

Työkalun paluuarvot: `0`, jos käännös kirjoitetaan — tai lasketaan valitsimella `--dry-run` —, `2`, jos jompaakumpaa tiedostoa ei voitu
lukea tai jos kohdetta ei voitu kirjoittaa, `1` käyttövirheessä. **Huonosti kohdistunut käännös ei ole
epäonnistuminen**: se kirjoitetaan, ja lause kertoo, mikä ei löytänyt paikkaansa.

## Virheet

| Mikä ne aiheuttaa | Viesti |
| :------------------- | :------ |
| ei valitsinta `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — sitä ei ole alikomennolla `pair` |
| useampi kuin yksi päätiedosto | `The following argument was not expected: <path>` |
| käännös on päätiedosto | `<translation>: the file is already open as the main document`, koodi `2` |
| jompikumpi tiedosto puuttuu, ei ole luettavissa tai on tuntemattomassa muodossa | sama kuin kaikkialla, virheellisen tiedoston polun kera |
