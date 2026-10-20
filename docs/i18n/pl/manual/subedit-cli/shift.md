# `shift`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Przesuwa **każdą** pozycję pliku o tę samą wartość, w jedną stronę lub w
drugą. Żaden napis nie zostaje na ekranie ani dłużej, ani krócej:
na tym polega przesunięcie.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumenty i opcje

| Opcja | Wymagane | Wartości | Domyślnie |
| :----- | :----- | :------ | :----- |
| `<file>...` | tak | jedna lub więcej ścieżek | — |
| `--recursive`, `-r` | nie | flaga | wyłączona — zobacz [Przetwarzanie drzewa katalogów](lots.md) |
| `--by` | **jedna z dwóch** | czas trwania ze znakiem, zobacz niżej | — |
| `--to-grid` | **jedna z dwóch** | flaga: wartość jest mierzona, zobacz niżej | — |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zobacz [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |

Format odczytanego pliku jest **zachowywany**: zmiana formatu to zadanie
[`convert`](convert.md).

## `--to-grid`: wartość zmierzona, a nie podana

Plik, którego pozycje leżą na siatce klatek **z dokładnością do
stałej**, został przesunięty, a tę stałą można zmierzyć na samych pozycjach.
`--to-grid` mierzy ją i stosuje, bez konieczności jej znania.

Wartość jest **właściwa dla każdego pliku**: dwa pliki przesunięte względem tej samej
siatki o dwie różne wielkości wracają każdy o swoją. Jest
zapisywana w komentarzu wraz ze znalezioną liczbą klatek na sekundę.

**Droga jest najkrótsza.** Po przekroczeniu połowy klatki najbliższa linia siatki
jest następną: korekta idzie do przodu, zamiast cofać się o
prawie całą klatkę.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Plik, który nie leży na żadnej znanej siatce, jest odrzucany z podaniem powodu.** Nie ma
do czego wracać, a faza zmierzona na szumie przesunęłaby plik
losowo. Zobacz [`inspect`](inspect.md#co-zgłasza-frame-rate-grid), aby
wiedzieć, co plik daje, zanim spróbujesz tej operacji.

`--by` i `--to-grid` mówią obie, o ile przesunąć: podanie ich razem
to błąd użycia, tak samo jak niepodanie żadnej.

**`--to-grid` to nie [`snap`](snap.md).** Przesuwa cały plik
o tę samą wielkość i zachowuje względną synchronizację **dokładnie**; `snap`
przesuwa każdą pozycję niezależnie i przy okazji pochłania te, które
zostały poprawione ręcznie. Na czystym pliku oba dają ten sam
wynik; na pliku częściowym — nie.

## Zapis czasu trwania

Dwie postaci i tylko dwie:

| Postać | Przykłady |
| :---- | :------- |
| sekundy | `30`, `2.999`, `-7.001`, `+1.5` |
| kod czasowy | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Obie mówią to samo: `--by 7.001` i `--by 00:00:07.001` dają
ten sam plik.

**Separatorem dziesiętnym jest kropka**, bo wiersz poleceń mówi po angielsku.
Przecinek jest odrzucany, a nie interpretowany: połowa świata zapisuje `7,001`
jako siedem tysięcy jeden, a odczytanie go jako siedmiu sekund byłoby zgadywaniem —
błędnym dla tej połowy.

**Rozdzielczością jest milisekunda.** Dokładniejsza wartość jest odrzucana, a nie po cichu
zaokrąglana: milczące zaokrąglenie dałoby wynik, który nie pasuje do niczego z tego, o
co poproszono.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Przesunięcie przed początkiem osi czasu jest odrzucane

Cofnięcie poza zero daje kod `2` i **niczego nie zapisuje**. Komunikat wskazuje
napis, który nie mógł przyjąć przesunięcia, co mówi jednocześnie,
jak daleko można się posunąć.

Powód nie jest zasadą: pozycja przed początkiem osi czasu zapisałaby się jako
`-00:00:01,000`, postać, której nie definiuje żaden z dziewięciu formatów i której nie przyjmuje żaden czytnik.
Rdzeń takie pozycje dopuszcza — odmowa uczyniłaby z
operacji edycji przypadek szczególny — ale plik to coś innego niż
model w pamięci.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik. Z `--dry-run` też nic: nie ma pliku, a wiersz komentarza to mówi — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem).

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — format, kodowanie, znacznik i końce linii odczytanego pliku, przywrócone takie, jakie były |
| 3 | `<path>: N bytes read, M written`, potem **każdy komunikat diagnostyczny odczytu** — zobacz [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

### W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) obiekt każdego pliku zawiera `counts.subtitles` (każdy napis pliku) i `counts.shifted_by_ms` (przesunięcie w pełnych milisekundach, **ze znakiem**). Pełna tabela znajduje się w [Uruchamianie](invocation.md#co-każde-podpolecenie-umieszcza-w-counts).

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli każdy plik został zapisany, `2`, jeśli żaden, `3`, jeśli
tylko niektóre, `1` przy błędzie użycia.

## Błędy

| Co to wywołuje | Komunikat |
| :------------------ | :------ |
| przecinek dziesiętny | `"…" is not a time: use a decimal point and not a comma, in every language` |
| dokładniej niż do milisekundy | `"…" is not a time: positions are held to the millisecond, no finer` |
| tekst, który nie jest czasem trwania | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| przesunięcie przed początkiem osi czasu | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**Nic nie jest zapisywane bez jednej z trzech opcji miejsca docelowego**, a ich
błędy są wspólne dla podpoleceń, które zapisują:
zobacz [Uruchamianie](invocation.md#miejsce-docelowe).
