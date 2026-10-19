# Spuštění

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-gui [--version] [<file>]
```

| Argument | Povinný | Hodnota | Výchozí |
| :------- | :------- | :---- | :------ |
| `<file>` | ne | cesta k souboru titulků, který se má otevřít | žádná — okno se otevře prázdné |
| `--version` | ne | — | — |

**Jediný soubor.** Další argumenty se ignorují: okno drží
v jednu chvíli jen jeden projekt.

## `--version`

Zapíše na standardní výstup blok, který zapisuje `--version` příkazového řádku, a
okamžitě skončí, aniž by otevřel okno. Návratový kód `0`.

**`subedit <version>` samotné na prvním řádku**, pak jeden řádek pro každou knihovnu, se kterou
okno běží, s verzí nalezenou za běhu a v závorce s verzí,
se kterou bylo sestaveno; pak nainstalované jazyky rozhraní podle kódu. Na
počítači s Qt 6.4.2:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

Čísla se sem neopisují: patří počítači a tato příručka nemá žádný
generovaný blok, který by udržoval aktuální čísla samotného programu. Knihovny jsou
ty z příkazového řádku, ICU a Enchant, a dvě, které linkuje jen okno — Qt a
libmpv, přehrávač, uváděný verzí svého klientského API, tou, kterou ohlašuje
`mpv.pc`. Zbytek je popsán v
[příručce příkazového řádku](../subedit-cli/invocation.md#--version). Slova
`built with` a `Languages` se řídí jazykem okna; nic jiného ne.

`Nápověda ▸ O aplikaci subedit` ukazuje tytéž knihovny ze stejného seznamu v tabulce —
název, načtenou verzi, verzi, se kterou bylo sestaveno, každou na jednom řádku — a pod ní
jazyky; její záhlaví se řídí jazykem okna.

## Otevření souboru

```console
$ subedit-gui film.srt
```

Čtené formáty jsou **těch devět**, které uvádí filtr otevírání — viz
[Otevírání a ukládání](files.md#otevírání). Formát se pozná podle obsahu, a
ne podle přípony, a **stejně tak kódování**: podle BOM,
pokud ji soubor nese, jinak podle kódování, které naznačují jeho bajty.

## Když otevření selže

Chybějící soubor, nečitelný soubor, soubor, jehož bajty nelze dekódovat v žádném
kódování, nebo soubor, jehož obsah nepozná žádný formát: **okno se
přesto otevře, prázdné**, a chyba se zapíše na chybový výstup.

**Hlášení uvádí příčinu**, stejnými slovy jako příkazový řádek:

| Hlášení | Co se stalo |
| :------ | :------------ |
| `does not exist` | cesta neoznačuje žádný soubor |
| `cannot be opened: permission denied` | systém ho odmítá otevřít |
| `cannot be read` | systém odmítl z jiného důvodu |
| `cannot be decoded in the chosen encoding` | bajty nejdou přečíst ve zvoleném kódování |
| `is in no format this tool knows` | obsah nepozná žádný formát |
| `holds nothing recognisable as a subtitle` | formát se pozná, ale nic v něm není titulek |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Otevřít se prázdné místo odmítnutí spuštění: nástroj je stále k něčemu
dobrý a selhání jednoho argumentu není důvod nespustit se.

**Špatně utvořený, ale čitelný soubor se otevře** a to, co čtení
cestou zachytilo, lze přečíst v tlačítku stavového řádku — viz
[hlášení při čtení](files.md#hlášení-při-čtení).

## Návratové kódy

| Kód | Kdy |
| :--- | :--- |
| `0` | okno se otevřelo a pak bylo normálně zavřeno, nebo odpověděl `--version` |
| `2` | neočekávaná chyba před otevřením — chybí Qt, displej není k dispozici |

Selhání otevření souboru **nemění návratový kód**:
okno se otevřelo, a to kód hlásí.

## Manuálová stránka

Instalace umístí `subedit-gui(1)` do `share/man/man1`:

```console
$ man subedit-gui
```

Uvádí tvar volání, `--version`, hlášení o selhání otevření,
návratové kódy, dvě proměnné prostředí a soubor nastavení.
**Okno nepopisuje** — to dělá tato příručka a stránka
odkazuje sem.

## Platforma Qt

`QT_QPA_PLATFORM` volí, na čem okno běží. **Pokud je proměnná
nastavená, `subedit-gui` ji bez diskuse respektuje.**

Pokud není, nechá volbu na Qt: vestavěný přehrávač kreslí obraz
přímo v okně a od platformy nic nežádá — viz
[Přehrávač](player.md).

`QT_QPA_PLATFORM=offscreen` spouští okno tam, kde není žádný
grafický server — což dělá průběžná integrace a testy.

## Jazyk

Okno mluví jazykem svého prostředí, **zvoleným jednou, při spuštění**: změna
proměnných běžícího okna nic nemění až do příštího spuštění. Proměnné a pravidla
jsou tytéž jako u příkazového řádku — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` pro
angličtinu, angličtina **bez jediného slova**, když požadovaný jazyk není nainstalován — popsané v
[příručce příkazového řádku](../subedit-cli/invocation.md#jazyk). Katalogy jsou tytéž
soubory, nainstalované v `<prefix>/share/subedit/locale`.

**Jazyk zvolený v `Úpravy ▸ Předvolby…` má přednost** a prostředí se čte jen
tehdy, když je nastaveno `System default`, což platí, dokud se žádný nezvolí: nainstalovaný jazyk zvolený tam
vítězí nad každou proměnnou, včetně `LC_ALL=C`. I tato volba platí od příštího spuštění. Viz
[Jazyk rozhraní](preferences.md#jazyk-rozhraní).

| Co | Řídí se jazykem |
| :--- | :------------------- |
| nabídky a jejich položky, s podtrženými písmeny; panel nástrojů; bublinová nápověda | ano |
| sloupce tabulky a `Zobrazit ▸ Sloupce`, karty, řádek stavu, lišta přehrávání | ano |
| standardní tlačítka, která kreslí samo Qt (`OK`, `Cancel`) | ano, je-li nainstalován vlastní katalog Qt pro daný jazyk (`qtbase_<language>.qm`); jinak anglicky |
| dialogy, stránky `Correct Texts…`, titulky a filtry dialogů souborů, okna se zprávami a to, co říkají | ano |
| řádky, které okno zapisuje na svůj chybový výstup, `subedit-gui: …` | ano, kromě názvu programu |
| názvy opravných vzorů v `Correct Texts…` a jejich popisy v bublinové nápovědě | ano, pro vzory dodávané s programem; vlastní vzor se zobrazuje tak, jak ho zapisuje jeho soubor |
| názvy kódování, konců řádků a jazyků pravopisu (`UTF-8`, `CRLF`, `en_US`) | nikdy: jsou to názvy, ne slova |
| klávesové zkratky | nikdy: `Ctrl+S` je `Ctrl+S` v každém jazyce |
| hlášení, které katalog nepřekládá | zobrazí se anglicky |

Tato verze instaluje `ca`, `cs`, `de`, `es`, `fi`, `fr` a `gl` — katalánštinu, češtinu, němčinu,
španělštinu, finštinu, francouzštinu a galicijštinu — v úplném překladu. Francouzštinu zkontroloval
čtenář tohoto jazyka; katalánština, čeština, němčina, španělština, finština a galicijština
jsou strojově přeložené a dosud je nezkontroloval žádný čtenář daného jazyka.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
Ve francouzském pracovním prostředí nese řádek nabídek
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. Snímky obrazovky
v této příručce jsou pořízeny v češtině.
