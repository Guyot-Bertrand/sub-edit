# `config`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Lista, mostra e altera as **definições das operações**: a parte do ficheiro de definições que a
janela e a linha de comandos partilham. `Adjust Durations…` e `Correct Texts…` memorizam
aí os seus valores, e [`adjust`](adjust.md) e [`correct`](correct.md) leem-nos por predefinição — ver
[O ficheiro de definições](invocation.md#o-ficheiro-de-definições).

**O que não faz:** nunca lê nem escreve uma **preferência de apresentação** (a
geometria da janela, as colunas, o tema, as opções de pesquisa, a verificação ortográfica, o vídeo…). Essas continuam a ser
assunto da janela, definidas na janela e descritas em [o seu manual](../subedit-gui/preferences.md#o-ficheiro).
Também nunca toca num ficheiro de legendas.

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

## Subcomandos

| Subcomando | O que faz | Ficheiro |
| :---------- | :----------- | :--- |
| `config list` | escreve cada chave de operação, com a sua finalidade, o que aceita, a sua predefinição e quem a usa — **nenhum valor** | não lê nenhum |
| `config show` | escreve o ficheiro usado e se existe, depois cada chave de operação com o seu valor, de onde vem o valor e a sua predefinição | lê-o |
| `config set KEY VALUE` | verifica `VALUE`, depois reescreve o ficheiro com `KEY = VALUE`, mantendo todas as outras definições | lê-o e escreve-o |
| `config unset KEY` | repõe `KEY` na sua predefinição, escrita como comentário, tal como o ficheiro escreve sempre uma predefinição | lê-o e escreve-o |

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :------- | :----- | :------ |
| `KEY` | sim, para `set` e `unset` | uma chave de operação, tal como `config list` a nomeia | — |
| `VALUE` | sim, para `set` | o que `config list` diz que a chave aceita | — |

`config` não tem nenhuma opção própria. As opções globais escritas **antes** de `config` escolhem o
ficheiro e a forma da saída:

| Opção global | Efeito sobre `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` e `unset` atuam sobre `FILE` em vez do ficheiro do utilizador, mesmo que ainda não exista; `list` ignora-a |
| `--no-config` | `list` funciona como de costume; `show` mostra as predefinições, com `File: none (--no-config)`; `set` e `unset` são recusados: não há ficheiro onde escrever |
| `--format json` | `list` e `show` escrevem um objeto JSON por linha; `set` e `unset` não escrevem nada na saída padrão |
| `-q`, `--quiet` | `set` e `unset` deixam de confirmar o que escreveram |

## Que ficheiro

Sem `--config`, o ficheiro do utilizador:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

ou `~/.config/subedit/settings.conf` quando `XDG_CONFIG_HOME` não está definida, está vazia ou não é um caminho
absoluto. É o ficheiro que a janela lê e escreve; `config show` escreve o seu caminho na primeira linha:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Quando nem `XDG_CONFIG_HOME` nem `HOME` indicam um local, não há ficheiro do utilizador: `show`, `set`
e `unset` são recusados, e `--config FILE` designa um.

## `config list`

**O catálogo das chaves de operação**, pela ordem do ficheiro: cada chave, depois quatro linhas
indentadas — a finalidade, `Accepts:` (o conjunto fechado, ou a forma e os limites), `Default:` e
`Used by:` (os comandos de `subedit-cli` e as ações da janela que a leem). Não lê nenhum ficheiro e
não precisa de nenhum, e funciona com `--no-config`.

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

Lista **45 chaves**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
45
```

**Vinte e cinco delas são lidas pela linha de comandos**; as outras vinte são lidas apenas pela janela,
e a sua linha `Used by:` di-lo, com a opção de `correct` que ocupa o seu lugar:

| Chave | Lida por |
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
| todas as outras chaves `correction.*` | só `Correct Texts…` — `correct` recebe `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` e `--disable` na sua própria linha |

Os números de linhas, `correction.line-break.max-lines` e `correction.line-break.skip-max-lines`,
são números inteiros de 1 a 1000 — os limites de `correct --max-lines` —, e os comprimentos de linha,
`correction.line-break.max-length` e `correction.line-break.skip-max-length`, são números superiores a
0: `set` recusa o resto, e um ficheiro editado à mão que contenha um valor desses mantém a predefinição.

`correct` recebe as tarefas, o código e o comprimento de linha na sua própria linha de propósito: nada é
marcado de antemão numa linha de comandos, e a janela mede as linhas em ems, onde a linha de comandos
conta carateres. `correction.activations`, a lista das caixas de padrões que a janela gere, não é
uma chave do catálogo: `correct` tem `--enable` e `--disable`.

Com `--format json`, um objeto por chave — `key`, `description`, `accepts`, `default`, `used_by`,
e `surface`, `"both"` ou `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**O ficheiro usado, depois cada chave de operação com o seu valor**: `File: <path> (found)`,
`(not found)` ou `(unreadable: <reason>)`, depois uma linha por chave —
`[file]` e a predefinição quando o ficheiro a define, `[default]` quando não. Uma chave que o ficheiro
contém com um valor que não consegue ler mostra a sua predefinição. As preferências de apresentação não são listadas.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Um ficheiro em falta não é um erro: `show` escreve as predefinições, e o código é `0` — o ficheiro do utilizador
como um `FILE` indicado por `--config`, que [`adjust`](adjust.md) e [`correct`](correct.md), pelo
contrário, recusam (ver [O ficheiro de definições](invocation.md#o-ficheiro-de-definições)). Com
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

Com `--format json`, um primeiro objeto `{"file", "exists"}` — `file` é `null` com `--no-config` —
depois um objeto por chave: `key`, `value`, `source` (`"file"` ou `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` e `config unset`

**`set` verifica o valor com o próprio leitor do ficheiro** — o leitor que a janela usa — e recusa-o
quando não pode ser lido, antes de escrever seja o que for. **`unset`** volta a escrever a predefinição,
como comentário: a chave passa então a seguir a predefinição, mesmo que uma versão posterior a altere. Repor uma
chave que já está na predefinição tem êxito e não altera nada.

Ambos **reescrevem o ficheiro por inteiro e de forma atómica**: todas as preferências de apresentação já presentes são mantidas,
o bloco das operações vem primeiro e o bloco de apresentação em segundo, cada um sob o seu cabeçalho. **Um valor
que o ficheiro não conseguiu ler, e uma chave desconhecida, não são mantidos** — a reescrita é a da janela, que
também os descarta. **Cada valor descartado é dito**, na saída de erros ao nível predefinido, preferências
de apresentação incluídas: `<key>: unreadable value '<value>' replaced by its default`. O ficheiro e
a sua pasta são criados quando faltam.

A confirmação vai para a saída de erros, ao nível predefinido: `KEY = VALUE` para `set`,
`KEY reset to its default (DEFAULT)` para `unset`.

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

## Códigos de saída

| Código | Significado |
| :--- | :------ |
| `0` | concluído — incluindo `show` num ficheiro em falta e `unset` numa chave na sua predefinição |
| `1` | erro de utilização: nenhum subcomando, um argumento em falta, uma chave ou um valor recusado, `--no-config` com `set` ou `unset`, `--config` com um nome vazio, nenhum ficheiro do utilizador e nenhum `--config`; nada é escrito |
| `2` | o ficheiro não pode ser lido nem escrito; nada é escrito |

## Erros

| O que os desencadeia | O que é escrito, na saída de erros |
| :--------------- | :------------------------------------- |
| uma preferência de apresentação | `<key> is a display preference; set it in the window` |
| uma chave desconhecida | `unknown setting <key>; subedit-cli config list names them` |
| um valor que a chave não aceita | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` ou `unset` com `--no-config` | `--no-config has nothing to write to` |
| não é possível localizar nenhum ficheiro do utilizador, e nenhum `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` com `--no-config` | `--config excludes --no-config` |
| `--config` com um nome vazio | `--config needs a file name` |
| um ficheiro que existe e não pode ser lido, para `set` ou `unset` | `<path>: cannot be read; nothing written` |
| um ficheiro que não pode ser escrito | `<path>: cannot be written`, ou `<path>: cannot be written: permission denied` |

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
