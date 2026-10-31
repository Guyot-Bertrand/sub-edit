# `config`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Wyświetla, pokazuje i zmienia **ustawienia operacji**: tę część pliku ustawień, którą
okno i wiersz poleceń mają wspólną. `Adjust Durations…` i `Correct Texts…` zapamiętują tam swoje
wartości, a [`adjust`](adjust.md) i [`correct`](correct.md) odczytują je domyślnie — zob.
[Plik ustawień](invocation.md#plik-ustawień).

**Czego nie robi:** nigdy nie odczytuje ani nie zapisuje **preferencji wyświetlania** (geometrii
okna, kolumn, motywu, opcji wyszukiwania, sprawdzania pisowni, wideo…). Te pozostają sprawą
okna, ustawiane w oknie i opisane w [jego podręczniku](../subedit-gui/preferences.md#plik).
Nie dotyka też żadnego pliku napisów.

<!-- example: subedit-cli config --help -->
```console
$ subedit-cli config --help
List, show and change the operation settings
Usage: subedit-cli config [OPTIONS] SUBCOMMAND

Options:
  -h,--help                   Print this help message and exit

Subcommands:
  list                        Write every operation key, what it is for, what it accepts and its default
  show                        Write the settings file used, then every operation key with its value
  set                         Write KEY = VALUE into the settings file, keeping every other setting
  unset                       Put KEY back to its default in the settings file, keeping every other setting
```

## Podpolecenia

| Podpolecenie | Co robi | Plik |
| :---------- | :----------- | :--- |
| `config list` | wypisuje każdy klucz operacji, z jego przeznaczeniem, tym, co przyjmuje, wartością domyślną i tym, kto go używa — **bez wartości** | nie odczytuje żadnego |
| `config show` | wypisuje używany plik i informację, czy istnieje, a następnie każdy klucz operacji z jego wartością, skąd ta wartość pochodzi, i wartością domyślną | odczytuje go |
| `config set KEY VALUE` | sprawdza `VALUE`, a następnie zapisuje plik od nowa z `KEY = VALUE`, zachowując wszystkie pozostałe ustawienia | odczytuje go i zapisuje |
| `config unset KEY` | przywraca `KEY` do wartości domyślnej, zapisanej jako komentarz, tak jak plik zawsze zapisuje wartość domyślną | odczytuje go i zapisuje |

| Opcja | Wymagane | Wartości | Domyślnie |
| :----- | :------- | :----- | :------ |
| `KEY` | tak, dla `set` i `unset` | klucz operacji, tak jak nazywa go `config list` | — |
| `VALUE` | tak, dla `set` | to, co według `config list` klucz przyjmuje | — |

`config` nie ma własnej opcji. Opcje globalne zapisane **przed** `config` wybierają
plik i postać wyniku:

| Opcja globalna | Skutek dla `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` i `unset` działają na `FILE` zamiast na pliku użytkownika, nawet gdy ten jeszcze nie istnieje; `list` ją ignoruje |
| `--no-config` | `list` działa jak zwykle; `show` pokazuje wartości domyślne, z `File: none (--no-config)`; `set` i `unset` są odrzucane: nie ma pliku, do którego można zapisać |
| `--format json` | `list` i `show` zapisują jeden obiekt JSON w linii; `set` i `unset` nic nie zapisują na standardowym wyjściu |
| `-q`, `--quiet` | `set` i `unset` nie potwierdzają już tego, co zapisały |

## Który plik

Bez `--config` jest to plik użytkownika:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

albo `~/.config/subedit/settings.conf`, gdy `XDG_CONFIG_HOME` nie jest ustawione, jest puste lub nie jest ścieżką
bezwzględną. To plik, który okno odczytuje i zapisuje; `config show` zapisuje jego ścieżkę w pierwszej linii:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Gdy ani `XDG_CONFIG_HOME`, ani `HOME` nie wskazuje miejsca, pliku użytkownika nie ma: `show`, `set`
i `unset` są odrzucane, a `--config FILE` wskazuje jakiś plik.

## `config list`

**Katalog kluczy operacji**, w kolejności pliku: każdy klucz, a potem cztery wcięte
linie — jego przeznaczenie, `Accepts:` (zbiór zamknięty albo postać i granice), `Default:` oraz
`Used by:` (polecenia `subedit-cli` i akcje okna, które go odczytują). Nie odczytuje żadnego pliku i
żadnego nie potrzebuje, i działa z `--no-config`.

<!-- example: subedit-cli config list | head -10 -->
```console
$ subedit-cli config list | head -10
duration-adjust.speed
    Reading speed the durations are brought to.
    Accepts: a number of characters per second, above 0, decimals allowed
    Default: 15
    Used by: subedit-cli adjust; Adjust Durations…
duration-adjust.lengthen
    Let the reading speed move an end later.
    Accepts: true or false
    Default: true
    Used by: subedit-cli adjust; Adjust Durations…
```

Wypisuje klucze — **45** w sumie:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
45
```

**Dwadzieścia pięć z nich odczytuje wiersz poleceń**; pozostałe dwadzieścia odczytuje tylko okno,
a mówi o tym ich linia `Used by:`, wraz z opcją `correct`, która zajmuje ich miejsce:

| Klucz | Odczytywany przez |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) i `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` i `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` i `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` i `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) i `Correct Texts…` |
| `correction.remove-blank` | `correct` i `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` i `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.vad-model`, `transcription.language`, `transcription.suppress-notes`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) i `Transcribe Audio…` |
| `alignment.engine`, `alignment.min-match`, `alignment.doubt-gap` | [`sync`](sync.md) i `Synchronize with Audio…` |
| każdy inny klucz `correction.*` | tylko `Correct Texts…` — `correct` pobiera `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` i `--disable` z własnego wiersza poleceń |

Liczby linii, `correction.line-break.max-lines` i `correction.line-break.skip-max-lines`,
są liczbami całkowitymi od 1 do 1000 — to granice `correct --max-lines` —, a długości linii,
`correction.line-break.max-length` i `correction.line-break.skip-max-length`, są liczbami większymi
od 0: `set` odrzuca resztę, a plik edytowany ręcznie, który zawiera taką wartość, zachowuje wartość domyślną.

`correct` celowo pobiera zadania, kod i długość linii z własnego wiersza poleceń: nic nie jest
zaznaczone z góry w wierszu poleceń, a okno mierzy linie w em, podczas gdy wiersz poleceń
liczy znaki. `correction.activations`, lista pól wzorców zarządzana przez okno, nie jest
kluczem katalogu: `correct` ma `--enable` i `--disable`.

Z `--format json` jeden obiekt na klucz — `key`, `description`, `accepts`, `default`, `used_by`
oraz `surface`, `"both"` lub `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**Używany plik, a następnie każdy klucz operacji z jego wartością**: `File: <path> (found)`,
`(not found)` lub `(unreadable: <reason>)`, a potem jedna linia na klucz —
`[file]` i wartość domyślna, gdy plik ją ustawia, `[default]`, gdy nie. Klucz, który
plik zawiera z wartością niemożliwą do odczytania, pokazuje swoją wartość domyślną. Preferencje wyświetlania nie są wypisywane.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Brak pliku nie jest błędem: `show` wypisuje wartości domyślne, a kod wyjścia to `0` — dotyczy to pliku użytkownika
oraz `FILE` podanego w `--config`, który [`adjust`](adjust.md) i [`correct`](correct.md)
przeciwnie, odrzucają (zob. [Plik ustawień](invocation.md#plik-ustawień)). Z
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

Z `--format json` pierwszy obiekt to `{"file", "exists"}` — `file` ma wartość `null` z `--no-config` —,
a potem jeden obiekt na klucz: `key`, `value`, `source` (`"file"` lub `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` i `config unset`

**`set` sprawdza wartość własnym czytnikiem pliku** — tym samym, którego używa okno — i odrzuca
ją, gdy nie da się jej odczytać, zanim cokolwiek zostanie zapisane. **`unset`** zapisuje z powrotem wartość domyślną,
jako komentarz: klucz podąża wtedy za wartością domyślną, nawet jeśli zmieni ją późniejsza wersja. Usunięcie ustawienia
klucza, który już ma wartość domyślną, kończy się powodzeniem i nic nie zmienia.

Oba **zapisują plik w całości od nowa i atomowo**: każda preferencja wyświetlania, która w nim już jest, zostaje zachowana,
blok operacji idzie pierwszy, a blok wyświetlania drugi, każdy pod swoim nagłówkiem. **Wartość,
której plik nie potrafił odczytać, oraz nieznany klucz nie są zachowywane** — zapis od nowa robi to, co robi okno, które
też je odrzuca. **Każda odrzucona wartość jest zgłaszana** na standardowym wyjściu błędów na poziomie domyślnym, łącznie
z preferencjami wyświetlania: `<key>: unreadable value '<value>' replaced by its default`. Plik i
jego katalog są tworzone, jeśli ich brak.

Potwierdzenie trafia na standardowe wyjście błędów, na poziomie domyślnym: `KEY = VALUE` dla `set`,
`KEY reset to its default (DEFAULT)` dla `unset`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf
duration-adjust.gap-ms = 100
# Operation settings: shared by the window and the command line (subedit-cli reads these).
duration-adjust.gap-ms = 100
# Display preferences: the window's own, the command line never reads them.
duration-adjust.gap-ms reset to its default (0)
#duration-adjust.gap-ms = 0
#transcription.merge-gap-ms = 400
```

## Kody wyjścia

| Kod | Znaczenie |
| :--- | :------ |
| `0` | gotowe — także `show` dla brakującego pliku i `unset` dla klucza o wartości domyślnej |
| `1` | błąd użycia: brak podpolecenia, brakujący argument, odrzucony klucz lub odrzucona wartość, `--no-config` z `set` lub `unset`, `--config` z pustą nazwą, brak pliku użytkownika i brak `--config`; nic nie jest zapisywane |
| `2` | pliku nie da się odczytać ani zapisać; nic nie jest zapisywane |

## Błędy

| Co to wywołuje | Co jest zapisywane na standardowym wyjściu błędów |
| :--------------- | :------------------------------------- |
| preferencja wyświetlania | `<key> is a display preference; set it in the window` |
| nieznany klucz | `unknown setting <key>; subedit-cli config list names them` |
| wartość, której klucz nie przyjmuje | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` lub `unset` z `--no-config` | `--no-config has nothing to write to` |
| nie można ustalić położenia pliku użytkownika i brak `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` z `--no-config` | `--config excludes --no-config` |
| `--config` z pustą nazwą | `--config needs a file name` |
| plik, który istnieje i nie da się go odczytać, dla `set` lub `unset` | `<path>: cannot be read; nothing written` |
| plik, którego nie da się zapisać | `<path>: cannot be written` lub `<path>: cannot be written: permission denied` |

<!-- example: subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?" -->
```console
$ subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?"
general.theme is a display preference; set it in the window
code=1
cannot read 'fast' for duration-adjust.speed; subedit-cli config list says what it accepts
code=1
--no-config has nothing to write to
code=1
```
