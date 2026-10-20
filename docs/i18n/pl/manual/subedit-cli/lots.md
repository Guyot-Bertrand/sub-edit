# Przetwarzanie drzewa katalogów

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Każde podpolecenie, które odczytuje partię plików — łącznie z `inspect`, a bez
[`append`](append.md), [`split-file`](split-file.md) i [`pair`](pair.md), których liczba argumentów to para plików lub
film — przyjmuje **katalog** zamiast pliku, razem z opcją `--recursive` (`-r`).
Ta strona mówi, co obejście drzewa uwzględnia, w jakiej kolejności i gdzie zapisuje wyniki.

**Nic z poniższych zasad nie zmienia tego, co dzieje się z plikiem**: każdy plik jest odczytywany,
przetwarzany i zapisywany niezależnie, porażka jednego nie zatrzymuje pozostałych, a
cztery kody wyjścia są takie, jak w sekcji [uruchamianie](invocation.md#kody-wyjścia).
Obejście drzewa tworzy listę ścieżek, którą reszta partii wykorzystuje tak, jakby
zostały wpisane ręcznie.

## Bez `--recursive` katalog jest odrzucany

Błąd użycia, kod `1`, żaden plik nie jest przetwarzany ani zapisywany:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Co obejmuje obejście drzewa

Obejście drzewa jest **rekurencyjne, deterministyczne i pozbawione niespodzianek**:

- **kolejność** — wpisy każdego katalogu są brane w kolejności ich
  nazw, **bajt po bajcie** (wielkie litery przed małymi, niezależnie od
  języka systemu), a podkatalog jest przechodzony w miejscu, w którym się znajduje, najpierw w głąb. Dwa uruchomienia dają te same bajty na każdym
  komputerze;
- **dowiązania symboliczne nie są śledzone** — ani do pliku, ani do
  katalogu: drzewo zawierające dowiązanie do jednego ze swoich rodziców nie miałoby
  końca;
- **ukryte pliki i katalogi są pomijane** — te, których nazwa zaczyna się
  od kropki;
- **zachowywane są tylko rozszerzenia znanych formatów**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, bez względu na wielkość liter. **`.txt` jest wyłączone**: oznacza
  dwa formaty, ale przede wszystkim każdy `LISEZMOI.txt` na świecie, a partia, która
  zawodziłaby na każdym z nich, byłaby bezużyteczna.

**Plik wskazany w wierszu poleceń nigdy nie jest filtrowany.** Wskazanie go oznacza,
że się go chce: `subedit-cli shift --by 1 --in-place notes.txt` przetwarza go, niezależnie od
rozszerzenia, i zgłasza zastrzeżenia tylko wtedy, gdy jego zawartość nie należy do żadnego formatu.

**To, co pominięto, jest liczone** i zgłaszane na poziomie `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Katalog, który nie zawiera niczego do przejścia, mówi o tym na poziomie domyślnym
(`<directory>: holds no file in a format this tool walks for`) i nie jest to błąd.

## Gdzie obejście drzewa zapisuje

**Drzewo jest zachowywane w katalogu `--output-dir`**: ścieżka pliku,
**względna wobec podanego katalogu**, jest odtwarzana w katalogu wyjściowym. Nazwa
podanego katalogu nie wchodzi w jej skład — to semantyka `rsync` z ukośnikiem na
końcu, a nie `cp -r`: podaje się miejsce docelowe i dodaje do niego
poziom, którego się chce (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

Katalog wyjściowy i katalogi drzewa są **tworzone**, jeśli ich brakuje. Z opcją
`--in-place` każdy zachowany plik jest zapisywany z powrotem w swoim miejscu. Rozszerzenie, które
zmienia `convert`, zmienia się także w drzewie: `films/a/x.srt` skonwertowany
do WebVTT jest zapisywany jako `sortie/a/x.vtt`.

**Katalog wyjściowy, jeśli leży wewnątrz przechodzonego drzewa, jest wyłączony z obejścia**:
drugie uruchomienie nie odczytuje ponownie tego, co zapisało pierwsze.

## Dwa katalogi i kolizje

Zasada [miejsca docelowego](invocation.md#miejsce-docelowe) rozstrzyga sprawę: dwa pliki,
które trafiłyby pod tę samą ścieżkę, są odrzucane **zanim cokolwiek zostanie zapisane**, kod `1`,
z nazwaniem obu danych wejściowych i miejsca docelowego.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Ile to kosztuje

Partia pozostaje sekwencyjna: jej czas jest sumą czasów jej plików. Test wydajności
`make bench` mierzy obejście drzewa i tę samą pracę na pojedynczym pliku; wyniki
są w [dzienniku wydajności](../../../../mesures/performances.md).
