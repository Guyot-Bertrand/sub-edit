# Synchronizacja na podstawie wideo

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

*Zakładamy, że film jest otwarty, a odtwarzacz znany: zobacz [Odtwarzacz](player.md).*

Oglądasz film, zatrzymujesz się na klatce, od której ma się zaczynać linia, i **ta pozycja staje się
początkiem napisu**. Pięć gestów w menu **Wideo**, pod nazwami, które nadał im Gaupol. Opierają
się na tym, co już istnieje: ustawienie pozycji to **to samo polecenie, co wpisanie wartości
w komórkę**, a wstawianie to **to samo, co `Edycja ▸ Wstaw napisy…`**.

| Polecenie | Skrót | Co robi |
| :-------- | :---- | :------ |
| Wideo ▸ Ustaw początek z pozycji wideo | `Alt+U` | ustawia **początek** zaznaczonego napisu na pozycję filmu |
| Wideo ▸ Ustaw koniec z pozycji wideo | `Alt+K` | ustawia jego **koniec** na pozycję filmu |
| Wideo ▸ Wstaw napis w pozycji wideo | `Alt+J` | wstawia napis, który **zaczyna się** w tej pozycji, i zaznacza go |
| Wideo ▸ Zaznacz poprzedni napis według pozycji wideo | `Alt+Up` | zaznacza ostatni napis, który **zaczyna się przed** pozycją |
| Wideo ▸ Zaznacz następny napis według pozycji wideo | `Alt+Down` | zaznacza pierwszy, który **zaczyna się po** pozycji |

**Wszystkie są wyłączone, gdy nie jest otwarte żadne wideo**, tak jak gesty odtwarzacza. `Set Start…` i `Set End…`
są ponadto wyłączone, dopóki nic nie jest zaznaczone: działają na **pierwszą linię**
zaznaczenia.

**Skróty są skrótami Gaupola, z klawiszem `Alt`**: tam są to `U`, `K` i `J` — gołe litery, które
okno przechwytywałoby przed edytowaną komórką, tak że wpisanie litery `k` w tekście
ustawiałoby koniec. `Select Previous…` i `Select Next…` to w Gaupolu `Ctrl+Y` i `Ctrl+U`; `Ctrl+Y`
w niektórych systemach oznacza *ponowienie*, a strzałki z `Alt` mówią *przed* i *po* lepiej niż
dwie litery. Edytowana komórka jest **zatwierdzana** przed `Set Start…`, `Set End…` lub
`Insert Subtitle…`, jak przed każdą modyfikacją.

## Ustawianie krawędzi

- **Jeden wpis w historii na gest**: `Edycja ▸ Cofnij` przywraca poprzednią pozycję i nic więcej.
- **Te same reguły co w komórce.** Początek ustawiony po końcu napisu albo koniec przed jego
  początkiem **nie jest odrzucany**: zostaje ustawiony, a tabela oznacza napis tak, jak oznacza ten,
  którego wartość wpisano ręcznie — zobacz [Anomalie](table.md#anomalie). Nic nie jest po cichu poprawiane.
- **Pozycja, która już jest pozycją krawędzi, nic nie robi** i nic nie dodaje do historii.
- **Pozycja jest zawsze początkiem klatki**: odtwarzacz nigdy nie zgłasza pozycji
  wewnątrz klatki. Ustawienie krawędzi, a potem powrót do niej — na przykład poleceniem `Seek Selection Start` —
  **pokazuje tę samą klatkę**.

## Wstawianie w danej pozycji

Nowy napis **zaczyna się w tej pozycji** i trwa **trzy sekundy**, albo do początku następnego,
gdy ten jest wcześniejszy. Jego tekst jest pusty. Zajmuje miejsce w porządku pliku: po
wszystkich, które zaczynają się w tej pozycji lub przed nią. Wstawienie to **jeden wpis w historii**,
a wstawiony napis jest zaznaczony: można go od razu wpisać.

## Zaznaczanie według pozycji

Oba gesty idą według porządku pliku, a nie bieżącego zaznaczenia. **Gdy po tej stronie nie ma sąsiada**,
zaznaczenie przechodzi na ostatni napis pliku dla `Select Next…`, a na pierwszy dla
`Select Previous…` — tak jak w Gaupolu. Te dwa gesty nie przesuwają filmu.

## Klatka po klatce

Przesuwanie do przodu i do tyłu **o jedną klatkę** — albo o N, zobacz niżej —, by ustawić krawędź **na dokładnej klatce**.
Dwa gesty i dwa przyciski na pasku odtwarzania, po obu stronach przycisku odtwarzania:

| Polecenie | Skrót | Co robi |
| :-------- | :---- | :------ |
| Wideo ▸ Krok wstecz | `Alt+Left` | cofa film o krok określony w klatkach |
| Wideo ▸ Krok naprzód | `Alt+Right` | przesuwa go do przodu o ten sam krok |

- **Krok to liczba klatek, co najmniej jedna**: to ustawienie `Frame step`
  [preferencji](preferences.md#odtwarzacz). Krok N klatek pokazuje klatkę **N dalej**, przy 25 klatkach na
  sekundę tak samo jak przy 23,976; **nigdy nie jest wyrażany w milisekundach**.
- **Film zatrzymuje się na swoich granicach**: cofnięcie od pierwszej klatki zostawia go tam, przesunięcie do przodu od ostatniej
  też. Po kroku odtwarzanie jest zatrzymane.
- **Przyciski paska** — dwie ikony, po obu stronach odtwarzania/pauzy — **uruchamiają te same akcje co menu**: ten sam krok, wyłączone,
  gdy nie ma wideo, jak pozycje menu, i ta sama podpowiedź, która podaje skrót. **Palec, który zostaje na
  przycisku lub na klawiszu, łączy kroki w serię.**
- **Jeden krok naraz.** Przytrzymany klawisz powtarza się szybciej, niż wykonuje się krok: pierwsze powtórzenie wychodzi od
  razu, te, które nadchodzą w trakcie wykonywania kroku, **nie kumulują się** — dzielą jedno miejsce,
  ostatnie, które wychodzi potem. Puszczenie klawisza zatrzymuje więc obraz tam, gdzie jesteś, i nie pozwala, by film
  toczył się dalej siłą rozpędu.

## Przesuwanie krawędzi o jedną klatkę

Aby dostroić krawędź za pomocą klawiatury: **początek lub koniec zaznaczonego napisu**, o jeden krok, wcześniej lub później.

| Polecenie | Skrót | Co robi |
| :-------- | :---- | :------ |
| Wideo ▸ Przesuń początek wcześniej | `Alt+Q` | napis **zaczyna się o krok wcześniej** |
| Wideo ▸ Przesuń początek później | `Alt+W` | **zaczyna się o krok później** |
| Wideo ▸ Przesuń koniec wcześniej | `Alt+Shift+Q` | **kończy się o krok wcześniej** |
| Wideo ▸ Przesuń koniec później | `Alt+Shift+W` | **kończy się o krok później** |

`Q` i `W` pochodzą z Gaupola i oznaczają *wcześniej* oraz *później*, z klawiszem `Alt` z powodu podanego wyżej; koniec
używa `Alt+Shift`, ponieważ `E` z Gaupola jest tu literą menu `Edit`.

- **Jeden wpis w historii na przesunięcie**, z możliwością cofnięcia. Sąsiedni napis i nakładanie się są zgłaszane
  **tak jak przy wpisywaniu**: krawędź zostaje ustawiona, a tabela oznacza to, co przestało do siebie pasować — zobacz
  [Anomalie](table.md#anomalie).
- **Ten sam krok co w odtwarzaczu**, czyli domyślnie jedna klatka. **Ile jest warta klatka**, odczytuje się, w tej kolejności,
  z **liczby klatek na sekundę, którą deklaruje wideo**, z liczby klatek **dokumentu, gdy liczy on w klatkach** (MicroDVD), a następnie
  z **siatki, którą rysują pozycje**. **Gdy brak wszystkich trzech, gest odmawia i mówi o tym**: wybranie
  liczby klatek na sekundę na chybił trafił przesunęłoby każdą krawędź o coś, co nie jest klatką.
- **Nie wymaga wideo**, tylko zaznaczenia: stosuje się go także w pliku liczonym w klatkach, bez
  filmu. Gdy film jest otwarty, **odtwarzanie przechodzi na nową pozycję**, by pokazać klatkę, którą krawędź
  właśnie przyjęła.
- Krawędź nie cofa się przed początek osi czasu: cofnięcie o jeden krok z miejsca bliższego niż klatka ustawia ją na zero.
