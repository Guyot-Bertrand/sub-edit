# Invocation

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-gui [--version] [<file>]
```

| Argument | Obligatori | Valore | Predefinit |
| :------- | :------- | :---- | :------ |
| `<file>` | no | li via de un file de subtitules a aperter | null — li fenestre aperte vacui |
| `--version` | no | — | — |

**Un sol file.** Li argumentes sequent es ignorat: li fenestre tene solmen un
projecte a un tempore.

## `--version`

Scri sur li exeunte standard li bloc quel `--version` del linea de comande scri, e
retorna strax, sin aperter un fenestre. Code de exeunte `0`.

**`subedit <version>` sol sur li prim linea**, tande un linea per biblioteca con quel li
fenestre funge, con li version trovat durant li execution e, inter parentheses, ti
con quel it esset compilat; tande li lingues de interfacie installat, per code. Sur
un machine con Qt 6.4.2:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

Li númeres ne es copiat hic: ili es ti del machine, e ca manual ha null bloc
generat quel mantenerea ti del programma self actual. Li bibliotecas es ti del
linea de comande, ICU e Enchant, e li du queles solmen li fenestre liga — Qt, e
libmpv, li reproductor, indicat per li version de su API de client, ti quel
`mpv.pc` anuncia. Li resta es descrit in
[li manual del linea de comande](../subedit-cli/invocation.md#--version). Li paroles
`built with` e `Languages` seque li lingue del fenestre; nullcos altri fa it.

`Auxilie ▸ Pri subedit` monstra li sam bibliotecas, ex li sam liste, in un tabelle —
li nómine, li version cargat, li version con quel it esset compilat, un rangea chascun —
tande sub it li lingues; su titules seque li lingue del fenestre.

## Aperter un file

```console
$ subedit-gui film.srt
```

Li formates leet es **li nin** quel li filtre de apertion lista — vide
[Aperter e salvar](files.md#aperter). Li formate es recognoscet per li contenete e
ne per li extension, e **li codification anc**: un BOM
si li file have un, altrimen li codification quel su bytes suggere.

## Quande li apertion falie

Un file mancant, un file ineleibil, un file queles bytes ne decode in null
codification, o un file quel contenete null formate recognosce: **li fenestre aperte
nonobstant, vacui**, e li fallimente es scrit sur li canal de errores.

**Li missage nomina li causa**, con li sam paroles quam li linea de comande:

| Missage | Quo evenit |
| :------ | :------------ |
| `does not exist` | li via designa null file |
| `cannot be opened: permission denied` | li sistema refusa aperter it |
| `cannot be read` | li sistema refusat por un altri rason |
| `cannot be decoded in the chosen encoding` | li bytes ne leet se in li codification retenet |
| `is in no format this tool knows` | null formate recognosce li contenete |
| `holds nothing recognisable as a subtitle` | li formate es recognoscet, ma nullcos in it es un subtitul |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Aperter vacui vice refusar lansar se: li instrument resta bon por alquo, e
li fallimente de un argument ne es rason por ne lansar it.

**Un file mal format ma leibil aperte se**, e quo li lectura ha captat
durant li via posse esser leet in un button del barre de statu — vide
[li diagnoses de un lectura](files.md#li-diagnoses-de-un-lectura).

## Codes de exeunte

| Code | Quande |
| :--- | :--- |
| `0` | li fenestre ha aperit se e tande ha esset clúdet normalmen, o `--version` ha respondet |
| `2` | un error inexpectat ante li apertion — Qt mancant, ecran ne disponibil |

Li fallimente de aperter un file **ne changea li code de exeunte**: li
fenestre ha aperit se, e to es quo li code reporta.

## Li págine de manual

Un installation pone `subedit-gui(1)` sub `share/man/man1`:

```console
$ man subedit-gui
```

It dona li forme de appelle, `--version`, li missages de fallimente de apertion, li
codes de exeunte, li du variabiles de ambiente e li file de configuration.
**It ne descri li fenestre** — ca manual fa it, e li págine
refere se a it.

## Li platteforme Qt

`QT_QPA_PLATFORM` elige sur quo li fenestre funge. **Si li variabile es
definit, `subedit-gui` respecte it sin argument.**

Si it ne es definit, it lassa Qt eliger: li reproductor integrat designa li image in
li fenestre self, e demanda nullcos al platteforme — vide
[Li reproductor](player.md).

`QT_QPA_PLATFORM=offscreen` fa funger li fenestre u null servitor
grafic existe — to quo li integration continu e li tests fa.

## Lingue

Li fenestre parla li lingue de su ambiente, **eliget un vice, quande it comensa**: changear li
variabiles de un fenestre in execution changea nullcos til li sequent lancement. Li variabiles e li regules
es ti del linea de comande — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` por
anglesi, anglesi **sin un parole** quande li lingue demandat ne es installat — descrit in
[li manual del linea de comande](../subedit-cli/invocation.md#lingue). Li catalogos es li sam
files, installat sub `<prefix>/share/subedit/locale`.

**Un lingue eliget in `Redacter ▸ Preferenties…` va ante**, e li ambiente es leet solmen
quande it es `System default`, quo it es til que un lingue es eliget: un lingue installat eliget
prevale sur omni variabile, `LC_ALL=C` includet. Li election anc aplica se al sequent lancement. Vide
[Li lingue del interfacie](preferences.md#li-lingue-del-interfacie).

| Quo | Seque li lingue |
| :--- | :------------------- |
| li menus e lor entratas, con lor líttere suslineat; li barre de instrumentes; li bulles de auxilie | yes |
| li columnes del tabelle e de `Vision ▸ Columnes`, li linguetas, li linea de statu, li barre de reproduction | yes |
| li buttones standard quel Qt designa se self (`OK`, `Cancel`) | yes, quande li catalogo propri de Qt por ti lingue es installat (`qtbase_<language>.qm`); anglesi altrimen |
| li casses de dialog, li págines de `Correct Texts…`, li titules e filtres de casses de dialog de files, li casses de missage e quo ili dice | yes |
| li lineas quel li fenestre scri sur su canal de errores, `subedit-gui: …` | yes, except li nómine del programma |
| li nómines de patrones de correction in `Correct Texts…`, e lor descritiones in li bulles de auxilie | yes, por li patrones livrat con li programma; un patrone de vos es monstrat quam su file scri it |
| li nómines de codificationes, de fines de linea e de lingues de ortografie (`UTF-8`, `CRLF`, `en_US`) | nequande: ili es nómines, ne paroles |
| li vias rapid | nequande: `Ctrl+S` es `Ctrl+S` in chascun lingue |
| un missage quel li catalogo ne traduce | monstrat in anglesi |

Ca version installa `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt` e `pt_BR` — catalan, cec, german,
hispan, finnesi, francesi, galician, hebraic, hungarian, interlingue, islandesi, italian, hollandesi, polonesi, portugalesi e portugalesi brasilian — traductet integralmen. Francesi ha esset revidet de un lector del
lingue; catalan, cec, german, hispan, finnesi, galician, hebraic, hungarian, interlingue, islandesi, italian, hollandesi, polonesi, portugalesi e portugalesi brasilian es traductet machinalmen e ancor ne revidet de un lector del
lingue.
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
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
Sur un table de labor francesi, li barre de menus lee
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. Li capturas de ecran de
ca manual es prendet in interlingue.
