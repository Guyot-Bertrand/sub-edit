# `config`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Luettelee, näyttää ja muuttaa **toimintojen asetuksia**: sitä asetustiedoston osaa, joka on
ikkunalle ja komentoriville yhteinen. `Adjust Durations…` ja `Correct Texts…` muistavat arvonsa
siellä, ja alikomennot [`adjust`](adjust.md) ja [`correct`](correct.md) lukevat ne oletuksena — katso
[Asetustiedosto](invocation.md#asetustiedosto).

**Mitä se ei tee:** se ei koskaan lue eikä kirjoita **näyttöasetusta** (ikkunan koko ja sijainti,
sarakkeet, teema, haun asetukset, oikoluku, video…). Ne kuuluvat ikkunalle: ne asetetaan
ikkunassa ja kuvataan [sen käyttöoppaassa](../subedit-gui/preferences.md#tiedosto).
Se ei myöskään koskaan koske tekstitystiedostoon.

<!-- example: subedit-cli config --help -->
```console
$ subedit-cli config --help
List, show and change the operation settings
Usage: subedit-cli config [OPTIONS] SUBCOMMAND

Options:
  -h,--help                   Print this help message and exit

Subcommands:
  list                        Write every operation key, what it is for, what it accepts and its default
  show                        Write the settings file used, then every operation key with its value
  set                         Write KEY = VALUE into the settings file, keeping every other setting
  unset                       Put KEY back to its default in the settings file, keeping every other setting
```

## Alikomennot

| Alikomento | Mitä se tekee | Tiedosto |
| :---------- | :----------- | :--- |
| `config list` | kirjoittaa jokaisen toiminta-avaimen sekä sen, mihin se on tarkoitettu, mitä se hyväksyy, sen oletusarvon ja sen käyttäjät — **ei arvoa** | ei lue mitään |
| `config show` | kirjoittaa käytetyn tiedoston ja sen, onko se olemassa, sitten jokaisen toiminta-avaimen arvoineen, arvon alkuperän ja oletusarvon | lukee sen |
| `config set KEY VALUE` | tarkistaa arvon `VALUE` ja kirjoittaa sitten tiedoston uudelleen rivillä `KEY = VALUE` säilyttäen kaikki muut asetukset | lukee ja kirjoittaa sen |
| `config unset KEY` | palauttaa avaimen `KEY` oletusarvoonsa, joka kirjoitetaan kommentoituna, kuten tiedosto aina kirjoittaa oletusarvon | lukee ja kirjoittaa sen |

| Valitsin | Pakollinen | Arvot | Oletus |
| :----- | :------- | :----- | :------ |
| `KEY` | kyllä, alikomennoille `set` ja `unset` | toiminta-avain sellaisena kuin `config list` sen nimeää | — |
| `VALUE` | kyllä, alikomennolle `set` | mitä `config list` kertoo avaimen hyväksyvän | — |

Alikomennolla `config` ei ole omia valitsimia. Yleiset valitsimet, jotka kirjoitetaan **ennen**
sanaa `config`, valitsevat tiedoston ja tulosteen muodon:

| Yleinen valitsin | Vaikutus alikomentoon `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` ja `unset` käsittelevät käyttäjän tiedoston sijaan tiedostoa `FILE`, vaikka sitä ei vielä olisi olemassa; `list` ei välitä siitä |
| `--no-config` | `list` toimii tavalliseen tapaan; `show` näyttää oletusarvot rivillä `File: none (--no-config)`; `set` ja `unset` hylätään: ei ole tiedostoa, johon kirjoittaa |
| `--format json` | `list` ja `show` kirjoittavat yhden JSON-olion riville; `set` ja `unset` eivät kirjoita mitään vakiotulosteeseen |
| `-q`, `--quiet` | `set` ja `unset` eivät enää vahvista, mitä ne kirjoittivat |

## Mikä tiedosto

Ilman valitsinta `--config` käytetään käyttäjän tiedostoa:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

tai tiedostoa `~/.config/subedit/settings.conf`, kun muuttuja `XDG_CONFIG_HOME` on asettamatta, tyhjä tai
ei ole absoluuttinen polku. Se on tiedosto, jota ikkuna lukee ja kirjoittaa; `config show` kirjoittaa sen polun ensimmäiselle rivilleen:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Kun kumpikaan muuttujista `XDG_CONFIG_HOME` ja `HOME` ei anna paikkaa, käyttäjän tiedostoa ei ole: `show`, `set`
ja `unset` hylätään, ja tiedosto nimetään valitsimella `--config FILE`.

## `config list`

**Toiminta-avainten luettelo** tiedoston järjestyksessä: kukin avain ja sen jälkeen neljä sisennettyä
riviä — mihin se on tarkoitettu, `Accepts:` (suljettu joukko tai muoto ja rajat), `Default:` ja
`Used by:` (sitä lukevat komennot `subedit-cli` ja ikkunan toiminnot). Se ei lue tiedostoa eikä
tarvitse sellaista, ja se toimii valitsimen `--no-config` kanssa.

<!-- example: subedit-cli config list | head -10 -->
```console
$ subedit-cli config list | head -10
duration-adjust.speed
    Reading speed the durations are brought to.
    Accepts: a number of characters per second, above 0, decimals allowed
    Default: 15
    Used by: subedit-cli adjust; Adjust Durations…
duration-adjust.lengthen
    Let the reading speed move an end later.
    Accepts: true or false
    Default: true
    Used by: subedit-cli adjust; Adjust Durations…
```

Avaimia on **41**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
41
```

**Niistä kaksikymmentäyksi luetaan komentoriviltä**; muut kaksikymmentä lukee vain ikkuna, ja niiden
rivi `Used by:` kertoo sen sekä nimeää alikomennon `correct` valitsimen, joka korvaa ne:

| Avain | Lukija |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) ja `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` ja `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` ja `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` ja `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) ja `Correct Texts…` |
| `correction.remove-blank` | `correct` ja `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` ja `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.language`, `transcription.suppress-notes`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) ja `Transcribe Audio…` |
| kaikki muut avaimet `correction.*` | vain `Correct Texts…` — `correct` ottaa valitsimet `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` ja `--disable` omalta komentoriviltään |

Rivimäärät, `correction.line-break.max-lines` ja `correction.line-break.skip-max-lines`,
ovat kokonaislukuja väliltä 1–1 000 — valitsimen `correct --max-lines` rajat —, ja rivipituudet,
`correction.line-break.max-length` ja `correction.line-break.skip-max-length`, ovat nollaa
suurempia lukuja: `set` hylkää muut, ja käsin muokattu tiedosto, jossa on sellainen, säilyttää oletusarvon.

`correct` ottaa tehtävät, koodin ja rivipituuden omalta komentoriviltään tarkoituksella: komentorivillä
mitään ei ole valittu etukäteen, ja ikkuna mittaa rivejä em-yksiköissä, kun taas komentorivi
laskee merkkejä. `correction.activations`, ikkunan hallitsema lausekeruutujen luettelo, ei ole
luettelon avain: alikomennolla `correct` on valitsimet `--enable` ja `--disable`.

Valitsimella `--format json` kirjoitetaan yksi olio avainta kohden — `key`, `description`, `accepts`, `default`, `used_by`
ja `surface`, joka on `"both"` tai `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**Käytetty tiedosto ja sen jälkeen jokainen toiminta-avain arvoineen**: `File: <path> (found)`,
`(not found)` tai `(unreadable: <reason>)`, sitten yksi rivi avainta kohden —
`[file]` ja oletusarvo, kun tiedosto asettaa avaimen, `[default]`, kun se ei aseta. Avain, jonka tiedosto
sisältää arvolla, jota ei voi lukea, näyttää oletusarvonsa. Näyttöasetuksia ei luetella.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Puuttuva tiedosto ei ole virhe: `show` kirjoittaa oletusarvot, ja paluuarvo on `0` — niin käyttäjän
tiedostolle kuin valitsimella `--config` nimetylle tiedostolle `FILE`, jonka alikomennot [`adjust`](adjust.md) ja [`correct`](correct.md)
päinvastoin hylkäävät (katso [Asetustiedosto](invocation.md#asetustiedosto)). Valitsimella
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

Valitsimella `--format json` kirjoitetaan ensin olio `{"file", "exists"}` — `file` on `null` valitsimella `--no-config` —
ja sitten yksi olio avainta kohden: `key`, `value`, `source` (`"file"` tai `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` ja `config unset`

**`set` tarkistaa arvon tiedoston omalla lukijalla** — samalla, jota ikkuna käyttää — ja hylkää
sen, jos sitä ei voi lukea, ennen kuin mitään kirjoitetaan. **`unset`** kirjoittaa oletusarvon takaisin
kommentoituna: avain seuraa silloin oletusarvoa, vaikka myöhempi versio muuttaisi sitä. Jo oletusarvossaan
olevan avaimen palauttaminen onnistuu eikä muuta mitään.

Molemmat **kirjoittavat tiedoston uudelleen kokonaan ja atomisesti**: jokainen siinä jo oleva näyttöasetus säilytetään,
toimintojen lohko tulee ensin ja näytön lohko toisena, kumpikin oman otsikkorivinsä alla. **Arvoa,
jota tiedostosta ei voitu lukea, ja tuntematonta avainta ei säilytetä** — uudelleenkirjoitus on ikkunan, joka
myös hylkää ne. **Jokainen hylätty arvo kerrotaan** virhetulosteessa oletustasolla, näyttöasetukset
mukaan lukien: `<key>: unreadable value '<value>' replaced by its default`. Tiedosto ja
sen hakemisto luodaan, jos ne puuttuvat.

Vahvistus kirjoitetaan virhetulosteeseen oletustasolla: `KEY = VALUE` alikomennolle `set`,
`KEY reset to its default (DEFAULT)` alikomennolle `unset`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf
duration-adjust.gap-ms = 100
# Operation settings: shared by the window and the command line (subedit-cli reads these).
duration-adjust.gap-ms = 100
# Display preferences: the window's own, the command line never reads them.
duration-adjust.gap-ms reset to its default (0)
#duration-adjust.gap-ms = 0
#transcription.merge-gap-ms = 400
```

## Paluuarvot

| Koodi | Merkitys |
| :--- | :------ |
| `0` | valmis — myös `show` puuttuvalle tiedostolle ja `unset` oletusarvossaan olevalle avaimelle |
| `1` | käyttövirhe: ei alikomentoa, puuttuva argumentti, hylätty avain tai arvo, `--no-config` yhdessä alikomennon `set` tai `unset` kanssa, `--config` tyhjällä nimellä, ei käyttäjän tiedostoa eikä valitsinta `--config`; mitään ei kirjoiteta |
| `2` | tiedostoa ei voi lukea tai kirjoittaa; mitään ei kirjoiteta |

## Virheet

| Mikä sen aiheuttaa | Mitä virhetulosteeseen kirjoitetaan |
| :--------------- | :------------------------------------- |
| näyttöasetus | `<key> is a display preference; set it in the window` |
| tuntematon avain | `unknown setting <key>; subedit-cli config list names them` |
| arvo, jota avain ei hyväksy | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` tai `unset` valitsimen `--no-config` kanssa | `--no-config has nothing to write to` |
| käyttäjän tiedostoa ei löydy eikä valitsinta `--config` ole annettu | `no settings file could be located; give one with --config FILE` |
| `--config` yhdessä valitsimen `--no-config` kanssa | `--config excludes --no-config` |
| `--config` tyhjällä nimellä | `--config needs a file name` |
| olemassa oleva tiedosto, jota ei voi lukea, alikomennolle `set` tai `unset` | `<path>: cannot be read; nothing written` |
| tiedosto, jota ei voi kirjoittaa | `<path>: cannot be written` tai `<path>: cannot be written: permission denied` |

<!-- example: subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?" -->
```console
$ subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?"
general.theme is a display preference; set it in the window
code=1
cannot read 'fast' for duration-adjust.speed; subedit-cli config list says what it accepts
code=1
--no-config has nothing to write to
code=1
```
