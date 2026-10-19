# subedit-cli — příručka

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

Nástroj příkazového řádku programu subedit: práce se soubory titulků bez grafického
rozhraní, v dávkách nebo ze skriptu. O druhém programu a o tom,
kde začít, viz
[příručku](../index.md).

> **Současný stav.** Sedmnáct podpříkazů `subedit-cli` existuje: kontrola,
> převod, posun, transformace, přečasování snímkové frekvence, zarovnání na mřížku snímků, odstranění
> popisků pro neslyšící, úprava dob trvání, nahrazení textu, změna velikosti písmen,
> kurzívy a dialogových pomlček, opětovné seřazení titulků, oprava
> textů pomocí vzorů Gaupolu, spojení souborů za sebe, rozdělení
> souboru na dvě části a zápis překladu na pozice jeho hlavního souboru. Tato příručka
> popisuje to, co existuje, nikdy to, co se plánuje; co přijde dál, je
> v [plánu vývoje](../../../../feuille-de-route.md).

## Oddíly

| Oddíl | Obsah |
| :------ | :------ |
| [Instalace](installation.md) | sestavení a instalace nástroje |
| [Spuštění](invocation.md) | globální volby, výstupy, návratové kódy |
| [Soubor nastavení](invocation.md#soubor-nastavení) | co `adjust` a `correct` čtou ze souboru nastavení okna, `--config`, `--no-config` |
| [Zpracování stromu](lots.md) | několik souborů, celý adresář, `--recursive` |
| [`inspect`](inspect.md) | vypsat, z čeho se soubor skládá |
| [`convert`](convert.md) | zapsat soubor v jiném formátu nebo v jiné podobě |
| [`shift`](shift.md) | posunout každou pozici o stejnou hodnotu |
| [`transform`](transform.md) | opravit každou pozici podle dvou bodů, o kterých se ví, že jsou správně |
| [`framerate`](framerate.md) | přečasovat soubor z jedné snímkové frekvence na jinou |
| [`snap`](snap.md) | vrátit časové údaje na snímky dané snímkové frekvence |
| [`hearing-impaired`](hearing-impaired.md) | odstranit popisky pro neslyšící |
| [`adjust`](adjust.md) | upravit doby trvání podle rychlosti čtení a mezí |
| [`replace`](replace.md) | nahradit text, aniž by se porušila značka |
| [`case`](case.md) | převést texty na velikost písmen jako v nadpisu, ve větě, na velká nebo malá písmena |
| [`italics`](italics.md) | převést texty na kurzívu, nebo kurzívu odebrat |
| [`dialogue-dashes`](dialogue-dashes.md) | přidat dialogové pomlčky, nebo je odebrat |
| [`sort`](sort.md) | znovu seřadit titulky podle jejich začátku |
| [`join-repeats`](join-repeats.md) | spojit do jediného titulku po sobě jdoucí kusy téhož textu |
| [`disjoint`](disjoint.md) | zkrátit překrývající se titulky tak, aby se už žádný rozsah nepřekrýval s jiným |
| [`repair`](repair.md) | zřetězit `join-repeats`, `sort` a `disjoint` na souboru, který je špatně rozdělený, neseřazený a s překryvy |
| [`correct`](correct.md) | opravit texty pomocí vzorů Gaupolu: popisky, spojování a rozdělování slov, běžné chyby, velká písmena, zalamování řádků |
| [`append`](append.md) | připojit soubory za první, do jediného souboru |
| [`split-file`](split-file.md) | rozdělit soubor na dvě části, druhou vrácenou na počátek |
| [`pair`](pair.md) | zapsat překlad na pozice jeho hlavního souboru a sdělit, jak se jeho řádky zarovnaly |
| [`config`](config.md) | vypsat, zobrazit a změnit nastavení operací sdílená s oknem |
