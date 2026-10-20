# Wycinanie, kopiowanie i wklejanie tekstów

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

Trzy pozycje menu `Edit`, pod `Undo` i `Redo` oraz nad
`Find and Replace…`.

| Pozycja | Skrót | Co robi |
| :------ | :---- | :------ |
| Wytnij teksty | `Ctrl+X` | kopiuje teksty zaznaczenia, a następnie je opróżnia |
| Kopiuj teksty | `Ctrl+C` | kopiuje teksty zaznaczenia |
| Wklej teksty | `Ctrl+V` | zapisuje skopiowane teksty, zaczynając od pierwszej zaznaczonej linii |

**Przenoszą się teksty i nic więcej**: żaden początek, żaden koniec, żadna cała
linia. Wklejanie nie przesuwa więc żadnej pozycji — synchronizacja linii, która
otrzymuje tekst, pozostaje jej własna. Tak działa Gaupol.

## Na czym działają

**Przy tłumaczeniu jest to tekst bieżącej kolumny**: tłumaczenie, jeśli
bieżąca komórka leży w kolumnie `Translation`, a gdzie indziej tekst główny —
zobacz [Tabela](table.md#tekst-na-którym-działają-operacje). `Cut Texts` opróżnia więc
tłumaczenie i zostawia tekst główny, a `Paste Texts` zapisuje w
kolumnie, w której jesteś. **Schowek nie przenosi żadnego dokumentu**:
można skopiować komórkę tekstu głównego i wkleić ją do tłumaczenia, i
odwrotnie.

**Zaznaczenie, a nigdy cały plik.** Trzy pozycje są wyłączone, dopóki nic nie jest
zaznaczone. Gdzie indziej w oknie „nic nie zaznaczono” oznacza
„cały plik”; tutaj nieszczęsne `Ctrl+X` opróżniłoby wszystkie teksty
dokumentu, a wklejanie nie miałoby linii, od której mogłoby zacząć.

Gdy komórka jest otwarta, `Ctrl+X`, `Ctrl+C` i `Ctrl+V` należą
do pola wprowadzania: wycinają, kopiują i wklejają znaki, tak jak `Del`
kasuje tam znak, a nie napis. Uruchomione z menu, pozycje najpierw zatwierdzają
wpisane dane: `Copy Texts` kopiuje wtedy tekst taki, jaki właśnie
wpisano. Zobacz [Edycja komórki](editing.md).

## Kopiowanie

`Copy Texts` kopiuje tekst każdej zaznaczonej linii, od pierwszej do
ostatniej. **Zaznaczenie nieciągłe zachowuje swój kształt**: linia pominięta w
zaznaczeniu staje się dziurą i wklejanie jej nie nadpisze.

**Zaznaczona linia o pustym tekście też staje się dziurą**:
po wklejeniu zostawia linię docelową bez zmian. Skopiowanie jednego pustego tekstu
nie wkleja więc niczego.

Schowek systemowy otrzymuje te teksty **jako zwykły tekst, oddzielone
pustą linią** — dziura jest tam zapisana jako pusty tekst i dlatego pusty
tekst nie może być niczym innym niż dziurą: wklejanie daje ten sam wynik,
czy kopia pochodzi z tego okna, czy z innego programu. To, co tu skopiujesz, można
więc wkleić do dowolnego programu, a to, co skopiujesz gdzie indziej, można wkleić tutaj.

Kopiowanie nie modyfikuje dokumentu i nie trafia do historii.

## Wycinanie

`Cut Texts` kopiuje, a następnie **opróżnia teksty** zaznaczonych linii. Linie
zostają, wraz ze swoimi pozycjami: wycinanie to nie usuwanie — aby usunąć
linie, służy [`Remove Subtitles`](lines.md#usuwanie).

Wycięcie tekstów, które są już wszystkie puste, nic nie robi i nie trafia do
historii.

## Wklejanie

`Paste Texts` zapisuje teksty ze schowka, **zaczynając od pierwszej zaznaczonej
linii**, jeden tekst na linię, w dół.

| Co zawiera schowek | Co się dzieje |
| :----------------- | :------------ |
| tekst | zastępuje tekst linii |
| dziura albo skopiowany pusty tekst | linia zachowuje swój tekst |
| więcej tekstów, niż zostało linii | na końcu dodawane są linie, które je przyjmą |

Dodane linie powstają jak linie wstawione po ostatniej: po trzy
sekundy każda, jedna po drugiej. **Okno o tym mówi**, w polu informacyjnym:

```text
inserted 2 subtitles to fit the clipboard
```

Po wklejeniu **zapisane linie są zaznaczone**.

Wklejenie, które niczego nie zmienia — te same teksty w tych samych liniach — nie trafia
do historii.

### Znaczniki innego formatu są tłumaczone

Kopia wykonana w tym oknie **pamięta format swojego dokumentu**, a ta
pamięć przetrwa otwarcie innego pliku. Wklejenie do dokumentu
innego formatu tłumaczy więc znaczniki, tak jak robi to `Save As…`:
`{\i1}Bonjour{\i0}` skopiowane z Advanced SSA zostaje wklejone jako `<i>Bonjour</i>` w
SubRip.

**To, czego format docelowy nie potrafi zapisać, zostaje utracone i jest to powiedziane**,
słowami okna `Save As…`:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

W odróżnieniu od `Save As…` nic nie jest pytane wcześniej: wklejenie cofa się
przez `Ctrl+Z`, który oddaje teksty takimi, jakie były.

**Tekst skopiowany poza `subedit` nie ma formatu** i jest wklejany bez zmian:
nie ma czego tłumaczyć z nicości. Dotyczy to także kopii z
tego okna, którą schowek systemowy od tamtej pory zastąpił — liczy się
zawartość schowka systemowego.

Gdy należą się oba komunikaty, mieszczą się w jednym polu, rozdzielone
średnikiem.

## Co mówi o tym akcja cofania

Każdy gest to **jeden** wpis w historii, bez względu na liczbę
linii, które zapisuje lub dodaje.

| Operacja | Co odczytuje `Undo` |
| :------- | :------------------ |
| wycinanie | `Undo: cutting texts` |
| wklejanie | `Undo: pasting texts` |

Zobacz [Cofanie i ponawianie](undo.md).
