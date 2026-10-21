# Uruchamianie

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-gui [--version] [<file>]
```

| Argument | Wymagane | Wartość | Domyślnie |
| :------- | :------- | :---- | :------ |
| `<file>` | nie | ścieżka pliku napisów do otwarcia | brak — okno otwiera się puste |
| `--version` | nie | — | — |

**Jeden plik.** Kolejne argumenty są ignorowane: okno przechowuje
tylko jeden projekt naraz.

## `--version`

Zapisuje na standardowym wyjściu blok, który zapisuje `--version` wiersza poleceń, i
kończy działanie od razu, bez otwierania okna. Kod wyjścia `0`.

**Sam `subedit <version>` w pierwszym wierszu**, potem po jednym wierszu na każdą bibliotekę, z którą
działa okno, z wersją znalezioną w czasie działania programu i, w nawiasie, tą,
z którą został skompilowany; potem zainstalowane języki interfejsu, według kodów. Na
komputerze z Qt 6.4.2:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

Numerów tu nie kopiujemy: są to numery danego komputera, a ten podręcznik nie ma
generowanego bloku, który nadążałby za numerami samego programu. Biblioteki to
te z wiersza poleceń, ICU i Enchant, oraz dwie, które łączy tylko okno — Qt i
libmpv, odtwarzacz, o którym mówi wersja jego API klienta, ta, którą ogłasza plik
`mpv.pc`. Resztę opisuje
[podręcznik wiersza poleceń](../subedit-cli/invocation.md#--version). Słowa
`built with` i `Languages` zmieniają się zgodnie z językiem okna; nic poza nimi.

`Pomoc ▸ O programie subedit` pokazuje te same biblioteki, z tej samej listy, w tabeli —
nazwa, wersja załadowana, wersja, z którą zbudowano, po jednym wierszu na bibliotekę — a pod nią
języki; jej nagłówki zmieniają się zgodnie z językiem okna.

## Otwieranie pliku

```console
$ subedit-gui film.srt
```

Odczytywanych jest **dziewięć** formatów, które wymienia filtr otwierania — zob.
[Otwieranie i zapisywanie](files.md#otwieranie). Format jest rozpoznawany po zawartości, a
nie po rozszerzeniu, **i tak samo kodowanie**: znacznik BOM,
jeśli plik go zawiera, a w przeciwnym razie kodowanie, które sugerują jego bajty.

## Gdy otwarcie się nie powiedzie

Brakujący plik, nieczytelny, taki, którego bajtów nie da się zdekodować w żadnym
kodowaniu, albo taki, którego zawartości nie rozpoznaje żaden format: **okno otwiera się
mimo to, puste**, a niepowodzenie zostaje zapisane na wyjściu błędów.

**Komunikat wskazuje przyczynę**, tymi samymi słowami co wiersz poleceń:

| Komunikat | Co się stało |
| :------ | :------------ |
| `does not exist` | ścieżka nie wskazuje żadnego pliku |
| `cannot be opened: permission denied` | system odmawia otwarcia |
| `cannot be read` | system odmówił z innego powodu |
| `cannot be decoded in the chosen encoding` | bajtów nie da się odczytać w wybranym kodowaniu |
| `is in no format this tool knows` | żaden format nie rozpoznaje zawartości |
| `holds nothing recognisable as a subtitle` | format jest rozpoznany, ale nic w nim nie jest napisem |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Otwieranie pustego okna zamiast odmowy uruchomienia: narzędzie nadal do czegoś służy,
a niepowodzenie argumentu nie jest powodem, by go nie uruchamiać.

**Plik źle sformułowany, ale czytelny, zostaje otwarty**, a to, co odczyt napotkał
po drodze, można przeczytać w przycisku na pasku stanu — zob.
[komunikaty diagnostyczne odczytu](files.md#komunikaty-diagnostyczne-odczytu).

## Kody wyjścia

| Kod | Kiedy |
| :--- | :--- |
| `0` | okno otworzyło się, a potem zostało zamknięte normalnie, albo odpowiedziało `--version` |
| `2` | nieoczekiwany błąd przed otwarciem — brak Qt, niedostępny wyświetlacz |

Niepowodzenie otwarcia pliku **nie zmienia kodu wyjścia**: okno
otworzyło się, a to właśnie zgłasza kod.

## Strona podręcznika man

Instalacja umieszcza `subedit-gui(1)` w `share/man/man1`:

```console
$ man subedit-gui
```

Podaje ona postać wywołania, `--version`, komunikaty o niepowodzeniu otwarcia, kody
wyjścia, dwie zmienne środowiskowe i plik ustawień.
**Nie opisuje okna** — robi to ten podręcznik, a strona odsyła tutaj.

## Platforma Qt

`QT_QPA_PLATFORM` wybiera, na czym działa okno. **Jeśli zmienna jest
ustawiona, `subedit-gui` respektuje ją bez żadnego argumentu.**

Jeśli nie jest, wybór pozostawia Qt: wbudowany odtwarzacz rysuje obraz
w samym oknie i niczego nie wymaga od platformy — zob.
[Odtwarzacz](player.md).

`QT_QPA_PLATFORM=offscreen` uruchamia okno tam, gdzie nie ma
serwera graficznego — tak robią testy i ciągła integracja.

## Język

Okno mówi językiem swojego środowiska, **wybieranym raz, przy uruchomieniu**: zmiana
zmiennych działającego okna nie zmienia niczego aż do następnego uruchomienia. Zmienne i reguły
są te z wiersza poleceń — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` dla
angielskiego, angielski **bez słowa**, gdy żądany język nie jest zainstalowany — opisane w
[podręczniku wiersza poleceń](../subedit-cli/invocation.md#język). Katalogi są tymi samymi
plikami, zainstalowanymi w `<prefix>/share/subedit/locale`.

**Język wybrany w `Edycja ▸ Preferencje…` ma pierwszeństwo**, a środowisko jest czytane tylko
wtedy, gdy ustawiono `System default`, czyli do czasu wyboru innego: zainstalowany język wybrany tam
wygrywa z każdą zmienną, także z `LC_ALL=C`. Wybór również zaczyna obowiązywać przy następnym uruchomieniu. Zob.
[Język interfejsu](preferences.md#język-interfejsu).

| Co | Podąża za językiem |
| :--- | :------------------- |
| menu i ich pozycje, z podkreślonymi literami; pasek narzędzi; podpowiedzi | tak |
| kolumny tabeli i `View ▸ Columns`, karty, wiersz stanu, pasek odtwarzania | tak |
| standardowe przyciski, które rysuje sam Qt (`OK`, `Cancel`) | tak, gdy zainstalowany jest własny katalog Qt dla tego języka (`qtbase_<language>.qm`); w przeciwnym razie angielski |
| okna dialogowe, strony `Correct Texts…`, tytuły i filtry okien plików, okna komunikatów i to, co mówią | tak |
| wiersze, które okno zapisuje na wyjściu błędów, `subedit-gui: …` | tak, z wyjątkiem nazwy programu |
| nazwy wzorców poprawek w `Correct Texts…` i ich opisy w podpowiedziach | tak, dla wzorców dostarczanych z programem; własny wzorzec jest pokazany tak, jak zapisuje go jego plik |
| nazwy kodowań, końców linii i języków sprawdzania pisowni (`UTF-8`, `CRLF`, `en_US`) | nigdy: to nazwy, nie słowa |
| skróty klawiszowe | nigdy: `Ctrl+S` to `Ctrl+S` w każdym języku |
| komunikat, którego katalog nie tłumaczy | pokazany po angielsku |

Ta wersja instaluje języki `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt` i `pt_BR` — kataloński, czeski, niemiecki,
hiszpański, fiński, francuski, galicyjski, hebrajski, węgierski, interlingue, islandzki, włoski, niderlandzki, polski, portugalski i brazylijski portugalski — w pełni przetłumaczone.
Francuski został sprawdzony przez osobę władającą językiem; kataloński, czeski, niemiecki, hiszpański, fiński, galicyjski, hebrajski, węgierski, interlingue, islandzki, włoski, niderlandzki, polski, portugalski i brazylijski portugalski są przetłumaczone maszynowo i nie zostały jeszcze
sprawdzone przez osobę władającą językiem.
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
Na francuskim pulpicie pasek menu ma postać
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. Zrzuty ekranu w tym podręczniku wykonano w języku polskim.
