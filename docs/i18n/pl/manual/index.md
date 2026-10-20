# subedit — podręcznik

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

subedit odczytuje i analizuje pliki napisów oraz zmienia ich synchronizację. Składa się z dwóch
programów, które mają wspólny rdzeń, a więc te same reguły i te same słowa.

| Program | Do czego służy | Jego podręcznik |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | otworzyć plik, zobaczyć go w tabeli, edytować go, śledzić go na tle filmu | [podręcznik okna](subedit-gui/index.md) |
| `subedit-cli` | przetwarzać pliki partiami, ze skryptu, bez interfejsu | [podręcznik wiersza poleceń](subedit-cli/index.md) |

Oba programy odczytują i zapisują **dziewięć formatów** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — i
rozpoznają format po zawartości, a nie po rozszerzeniu. **Kodowanie również
jest rozpoznawane po zawartości**: plik w Latin-1, w CP1252 lub w UTF-16
da się otworzyć, program mówi, jako jakie kodowanie go odczytał, a zapisuje go z powrotem w jego własnym — plik
otwarty, a następnie zapisany bez zmian, nie zmienia się nawet o jeden bajt.

## Od czego zacząć

| Co chcesz zrobić | Dokąd przejść |
| :----------------- | :------- |
| zainstalować narzędzie | [Instalacja](subedit-cli/installation.md) — dotyczy obu programów |
| otworzyć plik i zobaczyć, co zawiera | [Tabela](subedit-gui/table.md) |
| zmienić synchronizację napisów przesuniętych o stałą wartość | [Operacje](subedit-gui/operations.md#shift-positions) lub [`shift`](subedit-cli/shift.md) |
| zmienić synchronizację pliku napisanego dla innej liczby klatek na sekundę | [`framerate`](subedit-cli/framerate.md), a [`snap`](subedit-cli/snap.md), jeśli synchronizacja jest już poprawna |
| dowiedzieć się, względem jakiej liczby klatek na sekundę napisano plik | [Siatka klatek](subedit-gui/frame-grid.md) lub [`inspect`](subedit-cli/inspect.md) |
| usunąć opisy dla niesłyszących | [Operacje](subedit-gui/operations.md#remove-hearing-impaired-mentions) lub [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| poprawić opisy, typowe błędy, wielkość liter i łamanie linii jednym przebiegiem | [`Correct Texts…`](subedit-gui/correct-texts.md) lub [`correct`](subedit-cli/correct.md) |
| sprawdzić pisownię wyraz po wyrazie za pomocą słowników systemowych | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| wydłużyć napisy zbyt krótkie, by dało się je przeczytać | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations) lub [`adjust`](subedit-cli/adjust.md) |
| przywrócić porządek w pliku, w którym napisy są ułożone nieprawidłowo | [`sort`](subedit-cli/sort.md) |
| zrobić jeden napis z kolejnych kawałków tego samego tekstu | [`Join Repeated Subtitles`](subedit-gui/lines.md#łączenie-powtórzonych-napisów) lub [`join-repeats`](subedit-cli/join-repeats.md) |
| za jednym razem uporządkować plik, który jest źle podzielony, ma napisy w złej kolejności i nakładające się | [`repair`](subedit-cli/repair.md) |
| usunąć nakładanie się napisów | [`Make Ranges Disjoint…`](subedit-gui/lines.md#usuwanie-nakładania-się-zakresów) lub [`disjoint`](subedit-cli/disjoint.md) |
| połączyć dwie części filmu jedną po drugiej | [`Append File…`](subedit-gui/operations.md#append-file) lub [`append`](subedit-cli/append.md) |
| podzielić film na dwie części | [`Split Project…`](subedit-gui/operations.md#split-project) lub [`split-file`](subedit-cli/split-file.md) |
| zmienić wielkość liter, kursywę lub myślniki dialogowe w całym pliku | [Operacje](subedit-gui/operations.md#italic) lub [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| znaleźć wyraz i go zastąpić, nie psując znaczników | [Znajdowanie i zastępowanie](subedit-gui/find-and-replace.md) lub [`replace`](subedit-cli/replace.md) |
| tłumaczyć obok tekstu oryginalnego | [Otwieranie tłumaczenia](subedit-gui/files.md#otwieranie-tłumaczenia) lub [`pair`](subedit-cli/pair.md), aby zmienić jego synchronizację według pliku głównego |
| przetworzyć sto plików naraz | [Przetwarzanie drzewa katalogów](subedit-cli/lots.md), a następnie [Uruchamianie](subedit-cli/invocation.md) |

**Między `framerate` a `snap` pomyłka przechodzi niezauważona.** Jeśli
się wahasz, [`snap`](subedit-cli/snap.md#snap-to-nie-framerate-a-pomylenie-ich-nie-rzuca-się-w-oczy)
zaczyna się od porównania obu.

## Co obiecuje ten podręcznik

**Opisuje to, co istnieje, nigdy to, co jest planowane.** To, co nastąpi, i w jakiej
kolejności, znajduje się w [planie rozwoju](../../../feuille-de-route.md).

**Ten podręcznik jest napisany po angielsku i tłumaczony.** Jego tłumaczenia podążają za nim
strona po stronie, w katalogu `docs/i18n/` repozytorium, po jednym drzewie na język. Programy
mówią językiem swojego środowiska; nazwy, które wyświetlają — podpolecenia,
opcje, etykiety menu — są tu przytaczane tak, jak program zapisuje je po angielsku.

**Przykłady i obrazy są generowane, tekst — nie.**
Bloki `console` powstają przez faktyczne uruchomienie polecenia; zrzuty
ekranu — przez zbudowanie prawdziwego okna i sfotografowanie go. Żadne
z nich nie może więc opisywać narzędzia, które już nie istnieje. Bez zabezpieczenia pozostaje
tekst wokół nich, którego poprawność opiera się na korekcie.

**Zrzuty ekranu pokazują obie palety**, `Light` i `Dark` — te, które
oferują [preferencje](subedit-gui/preferences.md#motyw).

## Oba interfejsy mówią tymi samymi słowami

Jeden powód, by czytać któryś z podręczników, używając drugiego programu:
komunikaty o błędach, nazwy formatów, werdykty dotyczące siatki i etykiety
cofania są zapisane **raz**, w rdzeniu, i oba programy
je przytaczają. Odrzucony plik daje to samo zdanie po obu stronach.

Różni je to, co każdy z interfejsów potrafi:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| kilka plików naraz | tak — jedna karta na projekt | tak |
| tłumaczenie obok pliku głównego | tak | tak — `-t` w podpoleceniach tekstowych oraz [`pair`](subedit-cli/pair.md), aby zmienić jego synchronizację |
| dołączanie pliku, dzielenie projektu | tak | tak — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| cofanie | tak, tysiąc pozycji | nie dotyczy — nic nie jest zmieniane w miejscu bez pytania |
| ręczna edycja tekstu lub pozycji | tak | nie |
| wstawianie, usuwanie, łączenie i dzielenie linii | tak | nie |
| znajdowanie i zastępowanie | tak | tak — [`replace`](subedit-cli/replace.md) |
| wycinanie, kopiowanie i wklejanie tekstów | tak | nie |
| dostosowywanie czasów trwania, kursywy, wielkości liter, myślników | tak | tak — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| poprawianie tekstów według wzorców | tak — `Correct Texts…` | tak — [`correct`](subedit-cli/correct.md) |
| sprawdzanie pisowni wyraz po wyrazie | tak — `Check Spelling…` | nie |
| oglądanie filmu podczas synchronizacji | tak | nie |
| zapis nad plikiem wejściowym | `Save` | `--in-place` |
