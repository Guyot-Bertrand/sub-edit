# Cofanie i ponawianie

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

Każda operacja trafia do historii i można ją cofnąć. Edytowana komórka liczy się
jako **jeden** wpis: pisanie nie jest rejestrowane klawisz po klawiszu, liczy się tylko
zatwierdzenie.

| Akcja | Skrót | Gdzie |
| :---- | :---- | :---- |
| Cofnij | `Ctrl+Z` | menu **Edycja**, pasek narzędzi |
| Ponów | `Ctrl+Shift+Z` | menu **Edycja**, pasek narzędzi |

## Akcja nazywa to, co cofnie

Menu nie mówi „Cofnij”, lecz **„Undo: editing a text”**, od nazwy
operacji, która zostanie cofnięta. Przycisk paska narzędzi zachowuje samo słowo — w przeciwnym razie jego
szerokość zależałaby od ostatniej operacji i przycisk przesuwałby się pod
wskaźnikiem — ale jego podpowiedź zawiera całą etykietę.

| Operacja | Co czyta akcja |
| :------- | :------------- |
| edytowana komórka tekstu | `editing a text` |
| początek, koniec | `editing a start`, `editing an end` |
| czas trwania | `editing a duration` |
| wstawienie linii | `inserting` |
| usunięcie linii | `removing` |
| przesunięcie, w tym `Shift Whole File onto Grid` | `shifting` |
| przekształcenie | `transforming` |
| zmiana liczby klatek na sekundę | `converting the frame rate` |
| dopasowanie do liczby klatek na sekundę | `aligning on the frame rate` |
| usunięcie opisów | `removing hearing-impaired mentions` |
| ustawienie kursywy | `putting in italics` |
| zdjęcie kursywy | `taking italics out` |
| zmiana wielkości liter | `changing the case` |
| dodanie lub usunięcie myślników | `adding dialogue dashes`, `removing dialogue dashes` |
| połączenie linii | `merging` |
| podział linii | `splitting` |
| wycięcie lub wklejenie tekstów | `cutting texts`, `pasting texts` |
| dostosowanie czasów trwania | `adjusting durations` |
| otwarcie tłumaczenia | `opening a translation` |
| dołączenie pliku | `appending a file` |
| podział projektu | `splitting the project` |
| poprawka zaakceptowana w `Correct Texts…` lub wykonana przez `Check Spelling…` | `correcting texts` |
| zastąpienie, wszystkie zastąpienia | `replacing`, `replacing all` |

**Dwadzieścia siedem etykiet i to wszystko, co okno może wytworzyć.** Rdzeń nazywa
dwudziestą ósmą — `sorting` — do której nie dociera żadna akcja okna: sortowanie
zachodzi tylko przy ścisłej polityce kolejności, podczas gdy okno otwiera swoje dokumenty
przy polityce łagodnej, która zgłasza nieporządek zamiast go naprawiać.

**`Shift Whole File onto Grid` nie jest odróżniane od przesunięcia**, i to
trafnie: jest nim, tyle że jego wartość została zmierzona, a nie wpisana.

Są to te same słowa co w wierszu poleceń: zapisano je raz,
w rdzeniu.

Gdy nie ma niczego do cofnięcia, akcja czyta zwykłe „Cofnij” **i jest
wyłączona**: jej skrót nic nie robi.

## Znacznik modyfikacji

Tytuł okna nosi znacznik, dopóki plik różni się od tego na
dysku — gwiazdkę albo to, czego w jej miejsce używa platforma. **Etykieta
karty też**, `premier.srt*`: tytuł mówi tylko o projekcie, na który
patrzysz, a etykieta informuje o tych, na które nie patrzysz.

**Każda karta ma własną historię.** Cofanie cofa tylko to, co otrzymał projekt
bieżącej karty — także `Replace All` wykonane na wszystkich projektach:
zostawia jeden wpis w każdym projekcie, którego dotknęło.

**Znika, gdy cofniesz do punktu wyjścia.** To nie jest szczegół wyświetlania:
liczba modyfikacji jest liczona, a nie zapisywana jako tak lub nie, więc
powrót może sprowadzić ją do zera i znacznik znika.

## Dwie rzeczy, które warto wiedzieć

**Punktem odniesienia jest ostatni zapis** albo otwarcie pliku, jeśli
zapisu nie było. Zapis go przesuwa: znacznik znika, a późniejsze cofnięcie
przywraca go.

Operacja wykonana po cofnięciu **czyści to, co było do ponowienia**: ponowienie
odtworzyłoby polecenie, którego stan wyjściowy już nie istnieje. Tak działa
każdy edytor i jest to zamierzone.

**Gdy komórka jest otwarta**, `Undo` i `Redo` — z menu lub paska narzędzi —
najpierw zatwierdzają wpisane dane, jak inne gesty
[Edycji komórki](editing.md). To, co właśnie wpisano, staje się wtedy ostatnim
wpisem historii: `Undo` cofa je, a nie poprzednią operację; `Redo` nie ma
już nic do ponowienia, ponieważ wpisanie czyści to, co było do ponowienia. Otwarta
komórka, w której nic nie wpisano, nie zostawia wpisu: `Undo` cofa wtedy poprzednią
operację.

Historia zachowuje tysiąc wpisów i zapomina najstarsze ponad ten limit.
