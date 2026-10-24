# `config`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Lista, mostra e altera as **configurações das operações**: a parte do arquivo de configurações que a
janela e a linha de comando compartilham. `Adjust Durations…` e `Correct Texts…` lembram seus
valores ali, e [`adjust`](adjust.md) e [`correct`](correct.md) os leem por padrão — veja
[O arquivo de configurações](invocation.md#o-arquivo-de-configurações).

**O que ele não faz:** nunca lê nem grava uma **preferência de exibição** (a geometria da
janela, as colunas, o tema, as opções de pesquisa, a verificação ortográfica, o vídeo…). Elas continuam
sendo assunto da janela, definidas na janela e descritas em [o seu manual](../subedit-gui/preferences.md#o-arquivo).
Também nunca toca em um arquivo de legendas.

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

| Subcomando | O que faz | Arquivo |
| :---------- | :----------- | :--- |
| `config list` | escreve cada chave de operação, com sua finalidade, o que ela aceita, seu valor padrão e quem a usa — **nenhum valor** | não lê nenhum |
| `config show` | escreve o arquivo usado e se ele existe, depois cada chave de operação com seu valor, a origem do valor e o valor padrão | lê o arquivo |
| `config set KEY VALUE` | verifica `VALUE`, depois reescreve o arquivo com `KEY = VALUE`, mantendo todas as outras configurações | lê e grava o arquivo |
| `config unset KEY` | devolve `KEY` ao valor padrão, escrito como comentário, como o arquivo sempre escreve um valor padrão | lê e grava o arquivo |

| Opção | Obrigatório | Valores | Padrão |
| :----- | :------- | :----- | :------ |
| `KEY` | sim, para `set` e `unset` | uma chave de operação, como `config list` a nomeia | — |
| `VALUE` | sim, para `set` | o que `config list` diz que a chave aceita | — |

`config` não tem nenhuma opção própria. As opções globais escritas **antes** de `config` escolhem o
arquivo e a forma da saída:

| Opção global | Efeito sobre `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` e `unset` atuam sobre `FILE` em vez do arquivo do usuário, mesmo que ele ainda não exista; `list` a ignora |
| `--no-config` | `list` funciona como de costume; `show` mostra os valores padrão, com `File: none (--no-config)`; `set` e `unset` são recusados: não há arquivo onde gravar |
| `--format json` | `list` e `show` escrevem um objeto JSON por linha; `set` e `unset` não escrevem nada na saída padrão |
| `-q`, `--quiet` | `set` e `unset` deixam de confirmar o que gravaram |

## Qual arquivo

Sem `--config`, o arquivo do usuário:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

ou `~/.config/subedit/settings.conf` quando `XDG_CONFIG_HOME` não está definida, está vazia ou não é um caminho
absoluto. É o arquivo que a janela lê e grava; `config show` escreve seu caminho na primeira linha:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Quando nem `XDG_CONFIG_HOME` nem `HOME` indicam um local, não há arquivo do usuário: `show`, `set`
e `unset` são recusados, e `--config FILE` nomeia um.

## `config list`

**O catálogo das chaves de operação**, na ordem do arquivo: cada chave, depois quatro linhas
recuadas — sua finalidade, `Accepts:` (o conjunto fechado, ou a forma e os limites), `Default:` e
`Used by:` (os comandos de `subedit-cli` e as ações da janela que a leem). Ele não lê arquivo algum e
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

Ele lista **33 chaves**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
40
```

**Treze delas são lidas pela linha de comando**; as outras vinte são lidas somente pela janela,
e sua linha `Used by:` diz isso, com a opção de `correct` que toma o seu lugar:

| Chave | Lida por |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) e `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` e `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` e `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` e `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) e `Correct Texts…` |
| `correction.remove-blank` | `correct` e `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` e `Correct Texts…` |
| todas as outras chaves `correction.*` | somente `Correct Texts…` — `correct` toma `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` e `--disable` de sua própria linha |

Os números de linhas, `correction.line-break.max-lines` e `correction.line-break.skip-max-lines`,
são números inteiros de 1 a 1000 — os limites de `correct --max-lines` —, e os comprimentos de linha,
`correction.line-break.max-length` e `correction.line-break.skip-max-length`, são números maiores que
0: `set` recusa o resto, e um arquivo editado manualmente que contenha um deles mantém o valor padrão.

`correct` toma as tarefas, o código e o comprimento de linha de sua própria linha de propósito: nada é
marcado de antemão em uma linha de comando, e a janela mede as linhas em ems onde a linha de comando
conta caracteres. `correction.activations`, a lista de caixas de padrões que a janela gerencia, não é
uma chave do catálogo: `correct` tem `--enable` e `--disable`.

Com `--format json`, um objeto por chave — `key`, `description`, `accepts`, `default`, `used_by`
e `surface`, `"both"` ou `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**O arquivo usado e depois cada chave de operação com seu valor**: `File: <path> (found)`,
`(not found)` ou `(unreadable: <reason>)`, depois uma linha por chave —
`[file]` e o valor padrão quando o arquivo a define, `[default]` quando não. Uma chave que o arquivo
contém com um valor ilegível mostra o valor padrão. As preferências de exibição não são listadas.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Um arquivo ausente não é um erro: `show` escreve os valores padrão, e o código é `0` — vale tanto para o
arquivo do usuário quanto para um `FILE` indicado por `--config`, o que [`adjust`](adjust.md) e [`correct`](correct.md),
ao contrário, recusam (veja [O arquivo de configurações](invocation.md#o-arquivo-de-configurações)). Com
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

**`set` verifica o valor com o leitor do próprio arquivo** — o leitor que a janela usa — e o recusa
quando ele não pode ser lido, antes que qualquer coisa seja gravada. **`unset`** grava de volta o valor padrão,
como comentário: a chave passa então a seguir o valor padrão mesmo que uma versão posterior o altere. Remover a definição de uma
chave que já está no valor padrão tem sucesso e não altera nada.

Ambos **reescrevem o arquivo inteiro e de forma atômica**: todas as preferências de exibição que já estão nele são mantidas,
o bloco das operações vem primeiro e o bloco de exibição em segundo, cada um sob o seu cabeçalho. **Um valor
que o arquivo não conseguiu ler, e uma chave desconhecida, não são mantidos** — a reescrita é a da janela, que
também os descarta. **Cada valor descartado é informado**, no erro padrão no nível padrão, incluindo as preferências de
exibição: `<key>: unreadable value '<value>' replaced by its default`. O arquivo e
seu diretório são criados quando não existem.

A confirmação vai para o erro padrão, no nível padrão: `KEY = VALUE` para `set`,
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
| `0` | concluído — inclusive `show` em um arquivo ausente e `unset` em uma chave que está no valor padrão |
| `1` | erro de uso: nenhum subcomando, um argumento ausente, uma chave ou um valor recusado, `--no-config` com `set` ou `unset`, `--config` com um nome vazio, nenhum arquivo do usuário e nenhum `--config`; nada é gravado |
| `2` | o arquivo não pode ser lido ou gravado; nada é gravado |

## Erros

| O que o provoca | O que é escrito, no erro padrão |
| :--------------- | :------------------------------------- |
| uma preferência de exibição | `<key> is a display preference; set it in the window` |
| uma chave desconhecida | `unknown setting <key>; subedit-cli config list names them` |
| um valor que a chave não aceita | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` ou `unset` com `--no-config` | `--no-config has nothing to write to` |
| nenhum arquivo do usuário pode ser localizado, e nenhum `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` com `--no-config` | `--config excludes --no-config` |
| `--config` com um nome vazio | `--config needs a file name` |
| um arquivo que existe e não pode ser lido, para `set` ou `unset` | `<path>: cannot be read; nothing written` |
| um arquivo que não pode ser gravado | `<path>: cannot be written`, ou `<path>: cannot be written: permission denied` |

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
