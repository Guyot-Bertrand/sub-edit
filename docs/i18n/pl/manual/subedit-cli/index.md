# subedit-cli — podręcznik

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

Narzędzie wiersza poleceń programu subedit: praca na plikach napisów bez graficznego
interfejsu, partiami lub ze skryptu. Informacje o drugim programie i o tym, od czego
zacząć, znajdziesz w
[podręczniku](../index.md).

> **Stan obecny.** Istnieje siedemnaście podpoleceń `subedit-cli`: analiza,
> konwersja, przesunięcie, przekształcenie, zmiana synchronizacji przy zmianie liczby klatek na sekundę, dopasowanie do siatki klatek, usuwanie
> opisów dla niesłyszących, dostosowanie czasów trwania, zastępowanie tekstu, zmiana wielkości liter,
> kursywy i myślników dialogowych, przywracanie kolejności napisów, poprawianie
> tekstów według wzorców Gaupola, ustawianie plików jeden po drugim, podział
> pliku na dwie części oraz zapis tłumaczenia w pozycjach jego pliku głównego. Ten podręcznik
> opisuje to, co istnieje, nigdy to, co jest planowane; dalsze plany znajdują się w
> [planie rozwoju](../../../../feuille-de-route.md).

## Sekcje

| Sekcja | Zawartość |
| :------ | :------ |
| [Instalacja](installation.md) | budowanie i instalowanie narzędzia |
| [Uruchamianie](invocation.md) | opcje globalne, wyjścia, kody wyjścia |
| [Plik ustawień](invocation.md#plik-ustawień) | co `adjust` i `correct` odczytują z pliku ustawień okna, `--config`, `--no-config` |
| [Przetwarzanie drzewa katalogów](lots.md) | kilka plików, cały katalog, `--recursive` |
| [`inspect`](inspect.md) | zgłasza, z czego składa się plik |
| [`convert`](convert.md) | zapisuje plik w innym formacie lub w innej postaci |
| [`shift`](shift.md) | przesuwa każdą pozycję o tę samą wartość |
| [`transform`](transform.md) | koryguje każdą pozycję na podstawie dwóch punktów, o których wiadomo, że są poprawne |
| [`framerate`](framerate.md) | zmienia synchronizację pliku z jednej liczby klatek na sekundę na inną |
| [`snap`](snap.md) | ustawia znaczniki czasu z powrotem na klatkach danej liczby klatek na sekundę |
| [`hearing-impaired`](hearing-impaired.md) | usuwa opisy dla niesłyszących |
| [`adjust`](adjust.md) | dostosowuje czasy trwania do szybkości czytania i do limitów |
| [`replace`](replace.md) | zastępuje tekst, nie psując znaczników |
| [`case`](case.md) | zmienia teksty na wielkie litery w wyrazach, wielką literę w zdaniu, wielkie lub małe litery |
| [`italics`](italics.md) | dodaje do tekstów kursywę lub ją usuwa |
| [`dialogue-dashes`](dialogue-dashes.md) | dodaje myślniki dialogowe lub je usuwa |
| [`sort`](sort.md) | przywraca napisom kolejność według ich początku |
| [`join-repeats`](join-repeats.md) | robi jeden napis z kolejnych kawałków tego samego tekstu |
| [`disjoint`](disjoint.md) | obcina nakładające się napisy tak, aby żaden zakres nie nakładał się już na inny |
| [`repair`](repair.md) | łączy w ciąg `join-repeats`, `sort` i `disjoint` w pliku, który jest źle podzielony, ma napisy w złej kolejności i nakładające się |
| [`correct`](correct.md) | poprawia teksty według wzorców Gaupola: opisy, łączenie i dzielenie wyrazów, typowe błędy, wielkie litery, łamanie linii |
| [`append`](append.md) | ustawia pliki za pierwszym, w jednym pliku |
| [`split-file`](split-file.md) | dzieli plik na dwie części, a drugą cofa do początku |
| [`pair`](pair.md) | zapisuje tłumaczenie w pozycjach jego pliku głównego i mówi, jak ułożyły się jego linie |
| [`config`](config.md) | wyświetla, pokazuje i zmienia ustawienia operacji wspólne z oknem |
