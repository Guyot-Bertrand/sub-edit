# `config`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Elenca, mostra e modifica le **impostazioni delle operazioni**: la parte del file delle impostazioni che la
finestra e la riga di comando condividono. `Adjust Durations…` e `Correct Texts…` vi ricordano i
propri valori, e [`adjust`](adjust.md) e [`correct`](correct.md) li leggono per impostazione predefinita — vedi
[Il file delle impostazioni](invocation.md#il-file-delle-impostazioni).

**Ciò che non fa:** non legge né scrive mai una **preferenza di visualizzazione** (la
geometria della finestra, le colonne, il tema, le opzioni di ricerca, il controllo ortografico, il video…). Quelle restano
affare della finestra, si impostano nella finestra e sono descritte nel [suo manuale](../subedit-gui/preferences.md#il-file).
Non tocca nemmeno mai un file di sottotitoli.

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

## Sottocomandi

| Sottocomando | Che cosa fa | File |
| :---------- | :----------- | :--- |
| `config list` | scrive ogni chiave delle operazioni, con la sua funzione, ciò che accetta, il valore predefinito e chi la usa — **nessun valore** | non ne legge nessuno |
| `config show` | scrive il file usato e se esiste, poi ogni chiave delle operazioni con il suo valore, la provenienza del valore e il valore predefinito | lo legge |
| `config set KEY VALUE` | verifica `VALUE`, poi riscrive il file con `KEY = VALUE`, conservando ogni altra impostazione | lo legge e lo scrive |
| `config unset KEY` | riporta `KEY` al valore predefinito, scritto come commento come il file scrive sempre un valore predefinito | lo legge e lo scrive |

| Opzione | Obbligatorio | Valori | Predefinito |
| :----- | :------- | :----- | :------ |
| `KEY` | sì, per `set` e `unset` | una chiave delle operazioni, come la nomina `config list` | — |
| `VALUE` | sì, per `set` | ciò che `config list` dice che la chiave accetta | — |

`config` non ha alcuna opzione propria. Le opzioni globali scritte **prima** di `config` scelgono il
file e la forma dell'output:

| Opzione globale | Effetto su `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` e `unset` agiscono su `FILE` invece che sul file dell'utente, anche quando non esiste ancora; `list` la ignora |
| `--no-config` | `list` funziona come al solito; `show` mostra i valori predefiniti, con `File: none (--no-config)`; `set` e `unset` vengono rifiutati: non c'è alcun file in cui scrivere |
| `--format json` | `list` e `show` scrivono un oggetto JSON per riga; `set` e `unset` non scrivono nulla sull'output standard |
| `-q`, `--quiet` | `set` e `unset` non confermano più ciò che hanno scritto |

## Quale file

Senza `--config`, il file dell'utente:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

oppure `~/.config/subedit/settings.conf` quando `XDG_CONFIG_HOME` non è impostata, è vuota o non è un percorso
assoluto. È il file che la finestra legge e scrive; `config show` ne scrive il percorso sulla prima riga:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Quando né `XDG_CONFIG_HOME` né `HOME` indicano un posto, non c'è alcun file dell'utente: `show`, `set`
e `unset` vengono rifiutati, e `--config FILE` ne indica uno.

## `config list`

**Il catalogo delle chiavi delle operazioni**, nell'ordine del file: ogni chiave, poi quattro righe
rientrate — la sua funzione, `Accepts:` (l'insieme chiuso, oppure la forma e i limiti), `Default:` e
`Used by:` (i comandi di `subedit-cli` e le azioni della finestra che la leggono). Non legge alcun file e
non ne richiede, e funziona con `--no-config`.

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

Elenca **45 chiavi**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
45
```

**Venticinque sono lette dalla riga di comando**; le altre venti sono lette solo dalla finestra,
e la loro riga `Used by:` lo dice, indicando l'opzione di `correct` che ne prende il posto:

| Chiave | Letta da |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) e `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` e `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` e `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` e `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) e `Correct Texts…` |
| `correction.remove-blank` | `correct` e `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` e `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.vad-model`, `transcription.language`, `transcription.suppress-notes`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) e `Transcribe Audio…` |
| `alignment.engine`, `alignment.min-match`, `alignment.doubt-gap` | [`sync`](sync.md) e `Synchronize with Audio…` |
| ogni altra chiave `correction.*` | solo `Correct Texts…` — `correct` prende `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` e `--disable` dalla propria riga |

I numeri di righe, `correction.line-break.max-lines` e `correction.line-break.skip-max-lines`,
sono numeri interi da 1 a 1000 — i limiti di `correct --max-lines` —, e le lunghezze di riga,
`correction.line-break.max-length` e `correction.line-break.skip-max-length`, sono numeri maggiori di
0: `set` rifiuta il resto, e un file modificato a mano che ne contiene uno mantiene il valore predefinito.

`correct` prende le operazioni da eseguire, il codice e la lunghezza di riga dalla propria riga di comando di proposito: nulla è
spuntato in anticipo su una riga di comando, e la finestra misura le righe in em mentre la riga di comando
conta i caratteri. `correction.activations`, l'elenco delle caselle dei modelli che la finestra gestisce, non è
una chiave del catalogo: `correct` ha `--enable` e `--disable`.

Con `--format json`, un oggetto per chiave — `key`, `description`, `accepts`, `default`, `used_by`
e `surface`, `"both"` oppure `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**Il file usato, poi ogni chiave delle operazioni con il suo valore**: `File: <path> (found)`,
`(not found)` oppure `(unreadable: <reason>)`, poi una riga per chiave —
`[file]` e il valore predefinito quando il file lo imposta, `[default]` quando no. Una chiave che il file
contiene con un valore che non si può leggere mostra il valore predefinito. Le preferenze di visualizzazione non sono elencate.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Un file mancante non è un errore: `show` scrive i valori predefiniti, e il codice è `0` — il file
dell'utente come un `FILE` indicato da `--config`, che [`adjust`](adjust.md) e [`correct`](correct.md), al
contrario, rifiutano (vedi [Il file delle impostazioni](invocation.md#il-file-delle-impostazioni)). Con
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

Con `--format json`, un primo oggetto `{"file", "exists"}` — `file` è `null` con `--no-config` —
poi un oggetto per chiave: `key`, `value`, `source` (`"file"` oppure `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` e `config unset`

**`set` verifica il valore con il lettore proprio del file** — quello che usa la finestra — e lo
rifiuta se non può essere letto, prima di scrivere qualsiasi cosa. **`unset`** riscrive il valore predefinito,
come commento: la chiave segue allora il valore predefinito anche se una versione successiva lo cambia. Annullare
l'impostazione di una chiave già al valore predefinito riesce e non cambia nulla.

Entrambi **riscrivono il file per intero e in modo atomico**: ogni preferenza di visualizzazione già presente viene conservata,
il blocco delle operazioni viene per primo e il blocco di visualizzazione per secondo, ciascuno sotto la propria intestazione. **Un valore
che il file non ha potuto leggere, e una chiave sconosciuta, non vengono conservati** — la riscrittura è quella della finestra, che
li elimina a sua volta. **Ogni valore eliminato viene segnalato**, sull'errore standard al livello predefinito, comprese le preferenze
di visualizzazione: `<key>: unreadable value '<value>' replaced by its default`. Il file e
la sua directory vengono creati se mancano.

La conferma va sull'errore standard, al livello predefinito: `KEY = VALUE` per `set`,
`KEY reset to its default (DEFAULT)` per `unset`.

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

## Codici di uscita

| Codice | Significato |
| :--- | :------ |
| `0` | fatto — compreso `show` su un file mancante e `unset` su una chiave al valore predefinito |
| `1` | errore di utilizzo: nessun sottocomando, un argomento mancante, una chiave o un valore rifiutati, `--no-config` con `set` o `unset`, `--config` con un nome vuoto, nessun file dell'utente e nessun `--config`; nulla viene scritto |
| `2` | il file non può essere letto o scritto; nulla viene scritto |

## Errori

| Che cosa lo provoca | Che cosa viene scritto, sull'errore standard |
| :--------------- | :------------------------------------- |
| una preferenza di visualizzazione | `<key> is a display preference; set it in the window` |
| una chiave sconosciuta | `unknown setting <key>; subedit-cli config list names them` |
| un valore che la chiave non accetta | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` o `unset` con `--no-config` | `--no-config has nothing to write to` |
| nessun file dell'utente può essere individuato, e nessun `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` con `--no-config` | `--config excludes --no-config` |
| `--config` con un nome vuoto | `--config needs a file name` |
| un file che esiste e non può essere letto, per `set` o `unset` | `<path>: cannot be read; nothing written` |
| un file che non può essere scritto | `<path>: cannot be written`, oppure `<path>: cannot be written: permission denied` |

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
