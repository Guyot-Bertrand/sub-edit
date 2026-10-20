# `append`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Ustawia pliki **za pierwszym** w **jednym** pliku: film w dwóch częściach, zamieniony w jeden
zestaw napisów. Odpowiada to `Append File…` w oknie — [ten sam silnik](../subedit-gui/operations.md#append-file),
te same liczniki — zastosowanemu tyle razy, ile jest plików do dołączenia.

- **Pierwszy plik jest bazą.** Daje wynikowi **swój format, swoje kodowanie, swój znacznik
  kolejności bajtów i swoje końce linii**.
- **Między dwoma plikami nic nie jest wstawiane**: każdy z nich jest przesuwany o **koniec ostatniego napisu tego, co
  go poprzedza**, a jego pozycje zachowują kolejność i odstępy. Trzeci następuje po drugim, a nie po
  bazie: pliki następują po sobie w kolejności, w jakiej je podano.
- **Każdy dołączany plik przechodzi konwersję** do formatu bazy: jego znaczniki są tłumaczone,
  a to, czego ten format nie potrafi zapisać, jest zgłaszane plik po pliku, słowami okna.
- **Dołączane pliki nie mają tłumaczenia**: plik otwarty osobno nie ma go do przekazania.
- **Plik można podać kilka razy** albo dołączyć do samego siebie: nic tego nie zabrania.

**`append` nie jest przetwarzaniem partii.** N plików wejściowych, **jeden** wynik: pętla pozostałych podpoleceń — jeden wynik
na plik i błąd, który nie przerywa pozostałych — nie ma tu sensu. Wynikają z tego trzy rzeczy:

- **miejscem docelowym jest `--output` i nic innego.** `--output-dir` i `--in-place` nie istnieją w
  `append` i są odrzucane jak każda nieznana opcja, kodem `1`; `--recursive` tak samo: kolejność
  plików jest taka, w jakiej je podano, a przeszukiwane drzewo żadnej nie ma;
- **wszystko albo nic**: pierwszy plik, którego nie da się odczytać, zatrzymuje przebieg i **nic nie jest zapisywane**.
  Film w trzech częściach, z których jednej brakuje, nie jest filmem w dwóch;
- **jedna linia na przebieg** dla wyniku, a w JSON — **jeden obiekt**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Argumenty i opcje

| Opcja | Wymagane | Wartości | Domyślnie |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | tak, **co najmniej dwa** | ścieżki: baza, a potem to, co jest za nią ustawiane, po kolei | — |
| `--output` | **tak**, z wyjątkiem `--dry-run` | ścieżka pliku; jego katalog jest tworzony, jeśli go brak | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nic nie zapisuje — zob. [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączone |
| `--sort` | nie | flaga: ustawia całość w kolejności początków **po dołączeniu wszystkiego**, zob. [Sortowanie wyniku](#sortowanie-wyniku) | wyłączone |
| `--disjoint` | nie | flaga: obcina nakładające się napisy **po dołączeniu i posortowaniu wszystkiego**, zob. [Usuwanie nakładania się zakresów](#usuwanie-nakładania-się-zakresów) | wyłączone |

**Miejscem docelowym nie może być żaden z plików wejściowych**, łącznie z bazą: zapis nad
odczytywanym plikiem jest odrzucany, zanim cokolwiek zostanie odczytane, kodem `1`. Istniejący `--output`, różny od plików wejściowych, jest
nadpisywany, jak wszędzie.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

Ostatni napis z `partie1.srt` kończy się w 4 s; napis z `partie2.srt`, który zaczynał się w 0,5 s, zaczyna się w
4,5 s.

## Sortowanie wyniku

Każdy plik jest przesuwany **od końca tego, co go poprzedza**: kolejność, która może się zepsuć, to więc kolejność samych plików. `--sort`
ustawia wynik w kolejności początków, **za jednym razem, po ostatnim dołączeniu**, tak jak zrobiłoby to [`sort`](sort.md) — stabilnie i bez zmiany czegokolwiek
w zawartości.

Nigdy domyślnie. Z tą opcją baza mówi o tym na pierwszym poziomie komentarza: `<base>: N subtitles moved` lub `<base>: already in order`, a
rekord `--format json` zawiera licznik `moved`, **nieobecny bez tej opcji**.

## Usuwanie nakładania się zakresów

Nigdy domyślnie. Opcja robi to, co robi [`disjoint`](disjoint.md), **z jego ustawieniami domyślnymi**, po dołączeniu wszystkiego, na całości. Wymaga pliku we właściwej kolejności: w przeciwnym razie jest odrzucana, a `--sort` — które działa wcześniej — wcześniej tę kolejność ustawia. Plik mówi o tym na pierwszym poziomie komentarza (`<path>: N subtitles split, M merged`), a rekord `--format json` zawiera liczniki `split` i `merged`, **nieobecne bez tej opcji**.

## Gdy formaty się różnią

Wynik jest w **formacie bazy**, niezależnie od formatów pozostałych. To, co kosztuje takie przejście, jest
zgłaszane plik po pliku, słowami okna — tym samym zdaniem co w `Append File…` i przy
[wklejaniu z innego formatu](../subedit-gui/clipboard.md#znaczniki-innego-formatu-są-tłumaczone):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

To, co zostaje utracone, zależy wyłącznie od formatu **każdego** dołączanego pliku: baza w Advanced SSA przyjmuje
plik SubRip, niczego nie tracąc, ale nie odwrotnie. Aby wybrać format wyniku, ustaw na początku plik,
który już jest w tym formacie, albo później przekonwertuj za pomocą [`convert`](convert.md).

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik.

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | jedna linia na dołączany plik — `<path>: appended N subtitles`, a gdy format coś kosztował, dalej `; <source format> into <base format>: <what is lost>` — potem `<base>: N subtitles from M files -> <destination>` (z `--dry-run` zamiast `-> <destination>` stoi `(dry run, nothing written)`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — format, kodowanie, znacznik i końce linii bazy, przywrócone takie, jakie były |
| 3 | `<base>: N bytes read, M written` (wszystkie pliki razem), a następnie **każdy komunikat diagnostyczny odczytu** — zob. [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

### W formacie JSON

Z [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) **jeden obiekt** na cały przebieg,
którego `file` to **baza**. Jego `counts`:

| Licznik | Co mówi |
| :----- | :----------- |
| `files` | pliki przebiegu, łącznie z bazą |
| `subtitles` | napisy wyniku |
| `appended` | te, które pochodzą z dołączanych plików |
| `lost_ends`, `lost_header` | `0` lub `1`: jeden z dołączanych plików to utracił |
| `joined_lines`, `lost_tags`, `lost_fields` | suma tego, co utraciły dołączane pliki |
| `furthest_ms` | największe przesunięcie pozycji, wszystkie pliki razem |

**Tablica `inputs`** towarzyszy `counts`, poza nimi: jeden element na dołączany plik, po kolei,
`{"file", "counts"}` — `counts` zawiera `appended`, a następnie te same straty, **dla tego jednego pliku**. To ona
mówi, w przebiegu z dziesięciu plików, który z nich ile kosztował. Ostrzeżenia z odczytu wszystkich plików
są zebrane w `warnings`.

Błąd to również obiekt, **jeden**: ten dotyczący pierwszego pliku, którego nie dało się odczytać.

## Kody wyjścia

| Kod | Kiedy |
| :--- | :---- |
| `0` | wynik jest zapisany — albo obliczony, z `--dry-run` |
| `1` | błąd użycia: mniej niż dwa pliki, brak `--output`, opcja, której `append` nie ma, miejsce docelowe będące jednym z plików wejściowych |
| `2` | **nie dało się odczytać pliku** albo nie dało się zapisać miejsca docelowego: nic nie jest zapisywane. Nie ma kodu `3`: przebieg, który zapisuje tylko jeden plik, kończy się powodzeniem albo nie |

## Błędy

| Co je wywołuje | Komunikat |
| :------------------- | :------ |
| podano pojedynczy plik | `append needs a base file and at least one file to put after it` |
| ani `--output`, ani `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` wskazuje plik wejściowy | `<destination>: would be written over the input <input>` |
| plik nieistniejący, nieczytelny lub w nieznanym formacie | `<path>: does not exist`, `<path>: is in no format this tool knows`… — takie same jak wszędzie |

**Pusty plik nie jest dołączany**: żaden format nie odczytuje pliku bez co najmniej jednego napisu, a plik jest
odrzucany jak każdy plik, którego nie da się odczytać — `<path>: holds nothing recognisable as a subtitle` —, co
zatrzymuje przebieg i nic nie zapisuje.
