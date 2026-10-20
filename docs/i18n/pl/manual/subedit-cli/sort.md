# `sort`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Przywraca napisom **kolejność ich początków**. Nic innego się nie zmienia: ani teksty, ani
pozycje, ani format pliku. Napisy są **numerowane od nowa** od 1 przy zapisie,
jak w każdym pliku zapisywanym z powrotem.

**Rdzeń nigdy nie sortuje sam**: odczyt pliku pozostawia jego kolejność bez zmian, a
[`inspect`](inspect.md#co-zgłasza-anomalies) zgłasza naruszenie kolejności komunikatem
`subtitle N starts before the previous one starts`. `sort` jest **gestem wykonanym świadomie**, dla pliku
lub partii o zaburzonej kolejności — i można go cofnąć w oknie jak każdą operację.

- **Sortowanie jest stabilne**: dwa napisy, które zaczynają się w tej samej chwili, **zachowują kolejność nadaną im
  przez plik**. Żaden nie poprzedza drugiego; przestawienie ich byłoby decyzją, o którą nikt
  nie prosił.
- **`sort` nie przyjmuje ani zakresu, ani dokumentu**: nie ma czego wybierać, kolejność wynika z początków.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zobacz [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## Co jest liczone

**Liczone są miejsca, w których zmienił się napis**, tak jak zgłasza to rdzeń — to, co
historia okna nazwałaby przeniesieniem. Jeden źle ułożony napis przesuwa kilka: w
powyższym przykładzie żadne z trzech miejsc nie zachowuje swojego napisu, a zdanie mówi `3 subtitles moved`.

**Plik już uporządkowany jest mimo to zapisywany** i mówi o tym: podane miejsce docelowe to
miejsce docelowe zapisane, a uczynienie z `sort` wyjątku zmuszałoby skrypt do wiedzy, które
podpolecenia czasem tworzą plik, a czasem nic.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik. Z `--dry-run` też nic: nie ma
pliku, a wiersz komentarza to mówi — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem).

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, lub `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — format, kodowanie, znacznik i końce linii odczytanego pliku, przywrócone takie, jakie były |
| 3 | `<path>: N bytes read, M written`, potem **każdy komunikat diagnostyczny odczytu** — zobacz [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

### W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) obiekt każdego pliku zawiera
`counts.subtitles` (każdy napis pliku) i `counts.moved` (miejsca, które się zmieniły;
`0` dla pliku już uporządkowanego).

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli każdy plik został przetworzony — plik już uporządkowany też —, `2`, jeśli
żaden, `3`, jeśli tylko niektóre, `1` przy błędzie użycia.

## Błędy

Żadnych właściwych dla `sort`: nieczytelny plik kończy się błędem jak w każdym podpoleceniu, a
błędy miejsca docelowego są wspólne dla podpoleceń, które zapisują:
zobacz [Uruchamianie](invocation.md#miejsce-docelowe).
