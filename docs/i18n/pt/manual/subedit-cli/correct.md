# `correct`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Corrige os textos com os **padrões do Gaupol**, como o assistente
[`Correct Texts…`](../subedit-gui/correct-texts.md) da janela — os mesmos padrões,
o mesmo motor, as mesmas contagens. Existem aqui seis tarefas: as **menções para
deficientes auditivos**, **juntar** e **dividir palavras**, os **erros comuns**,
a **utilização de maiúsculas** e a **quebra de linhas**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## O que é escolhido e o que não é

**Nada vem marcado de antemão.** A janela abre as suas caixas como o utilizador as deixou; uma
linha de comandos recebe as suas tarefas, o seu código, o seu comprimento de linha e o seu idioma na própria linha,
nunca do ficheiro de definições: nem as tarefas marcadas um dia na janela, nem as caixas
de padrões ativadas aí (`correction.activations`), nem a lista de substituições do corretor ortográfico são
lidas, e nada é escrito desse lado.

**Quatro definições vêm do ficheiro de definições, por predefinição** — os valores que `Correct Texts…`
memoriza, ou que [`config set`](config.md) escreveu. **Uma opção na linha prevalece sempre**, depois
o ficheiro, depois a predefinição incorporada; ver [O ficheiro de definições](invocation.md#o-ficheiro-de-definições)
para saber onde está, `--config` e `--no-config`.

| Chave | O que fornece | Opção que a anula |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | as classes de erros comuns aplicadas | `--classes` |
| `correction.remove-blank` | se as legendas que a correção esvazia são removidas | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | o máximo de linhas de `line-break` | `--max-lines` |

- **Cada uma das quatro tem uma opção que a anula**, seja o que for que o ficheiro diga: `--classes human,ocr`
  aplica ambas as classes, `--remove-blank-subtitles` remove as legendas esvaziadas, `--max-lines 3`
  repõe as linhas do Gaupol. `--keep-blank-subtitles` e `--remove-blank-subtitles` em conjunto são um
  erro de utilização.
- **As duas classes desativadas no ficheiro são ignoradas, e isso é dito** ao nível 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` não poderia escrevê-lo, e isso
  deixaria common-errors sem nada para fazer. Aplicam-se, em vez disso, as duas classes.
- **Um número de linhas fora do intervalo de 1 a 1000 não é lido de todo do ficheiro**: o leitor do ficheiro
  recusa-o — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, ao
  nível 1 — e usa-se o 3 incorporado.

Os exemplos desta página são executados com `--no-config`, para que mostrem o que as predefinições
incorporadas fazem, sejam quais forem as definições da máquina que os executa.

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--tasks` | **sim** | uma lista separada por vírgulas: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | nenhuma tarefa |
| `--code` | **sim** com uma tarefa de padrões, e **não se aplica** nos outros casos | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **sim** com `join-words` ou `split-words`, e **não se aplica** nos outros casos | um código de idioma: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | não | `human`, `ocr` ou `human,ocr` | ambas — ou o que o ficheiro de definições disser |
| `--enable`, `--disable` | não, repetível | o nome inglês de um padrão, ou `type:name` | os ficheiros `.conf` fornecidos |
| `--max-length` | **sim** com `line-break` | um número de **carateres** superior a zero | **nenhuma** — ver abaixo |
| `--max-lines` | não | um número inteiro de linhas, superior a zero | 3 — ou o que o ficheiro de definições disser |
| `--skip-length`, `--skip-lines` | não | um número, ou `off` | os mesmos limites que `--max-length` e `--max-lines` |
| `--keep-blank-subtitles` | não | um indicador: as legendas esvaziadas ficam, vazias | as legendas esvaziadas são removidas — a menos que o ficheiro de definições diga o contrário |
| `--remove-blank-subtitles` | não | um indicador: as legendas esvaziadas são removidas, seja o que for que o ficheiro de definições diga; recusado com `--keep-blank-subtitles` | desativado |
| `--range` | não | `N-M`, ou `N-` até ao fim — ver [`adjust`](adjust.md#--range) | o ficheiro inteiro |
| `--document` | não | `main` ou `translation` — ver [Uma tradução](invocation.md#uma-tradução) | `main` |
| `-t`, `--translation-file` | com `--document translation` | um ficheiro de tradução, para **uma única** entrada | — |
| `--align-method` | não, e só com `-t` | `position` ou `number` | `position` |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

### As tarefas

**A ordem da lista é indiferente**: as tarefas aplicam-se pela
ordem do Gaupol — as menções, depois juntar e dividir palavras, depois os erros comuns, depois a utilização de maiúsculas e, por fim, a quebra de linhas —, cada uma sobre o
texto que a anterior deixou. Uma legenda que as menções esvaziaram não tem
qualquer papel no que se segue.

| Tarefa | O que faz |
| :---- | :-------------- |
| `mentions` | remove os sons entre parênteses retos, e entre parênteses curvos quando `Sound in parentheses` está ativado (a varredura de [`hearing-impaired`](hearing-impaired.md)), depois os padrões de letras de canções e de falantes |
| `join-words` | volta a colar uma palavra cortada em duas, a partir do dicionário de `--language` |
| `split-words` | divide duas palavras coladas, a partir do mesmo dicionário |
| `common-errors` | corrige os erros comuns: espaços, pontuação, ligaduras, erros de OCR |
| `capitalization` | põe a maiúscula onde o idioma a pede |
| `line-break` | volta a cortar as linhas de uma legenda para que nenhuma ultrapasse `--max-length`, ponderando os padrões de quebra do código |

### O código dos padrões

**`--code` nomeia a cascata**: `Zyyy` para qualquer escrita, depois a escrita, o idioma
e o país — `Latn-en-US` lê os padrões de `Zyyy`, de `Latn`, de `Latn-en` e de
`Latn-en-US`. **É obrigatório**, porque uma linha de comandos não tem idioma ambiente:
uma predefinição universal aplicaria *menos* do que se julga, sem o dizer. Vale para
todas as tarefas da mesma execução; uma tarefa que queira um código próprio é uma segunda
execução. Um código mal escrito é um erro de utilização:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Os padrões que o utilizador põe em `$XDG_DATA_HOME/subedit/patterns` (ou
`~/.local/share/subedit/patterns`) **são acrescentados aos padrões fornecidos**, como na
janela, e um padrão fornecido que não possa ser lido é indicado ao nível 1: `patterns: <file>, line N (<reason>)`.

### Juntar e dividir palavras, e o dicionário

**Estas duas tarefas não leem nenhum padrão**: consultam o **dicionário do sistema, através do
Enchant** — uma dependência dos pacotes (`libenchant-2-2` no Debian e no Ubuntu, `enchant2` no
Fedora), que `subedit-cli` partilha com a janela. Não têm, portanto, nem código nem nome a definir:
`--code` não faz sentido sem uma tarefa de padrões, e `--language` não faz sentido sem uma destas duas
— dar uma sem a outra é um erro de utilização, código `1`.

- **`join-words`** cola uma palavra que o dicionário não conhece à que a precede ou
  segue, **quando uma única direção dá uma palavra conhecida**: `hel lo` torna-se `hello`; duas
  direções possíveis, ou nenhuma, não decidem nada.
- **`split-words`** divide uma palavra que o dicionário não conhece **quando exatamente uma das suas
  sugestões é essa palavra com um espaço**: `hellothere` torna-se `hello there`. Uma palavra com uma inicial
  maiúscula seguida de minúsculas é deixada em paz: na maior parte dos casos é um nome.

**Sem um dicionário para o idioma, a execução é recusada**, código `1`, **antes de qualquer ficheiro ser
lido** e com as palavras da janela, que desativa a função e di-lo — aqui a linha de comandos
recusa e di-lo, em vez de se calar:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**A lista de substituições da janela não é lida nem escrita.** *Enche-se* através das escolhas
do utilizador na verificação ortográfica; uma linha de comandos não tem essa
escolha, e ler a lista faria um script depender do que o seu autor lá pôs um dia.

### Quebra de linhas

**Mede-se em carateres, e é essa a diferença em relação à janela.** A janela mede em *ems* —
uma largura calibrada sobre o tipo de letra, a 0,55 em por letra —, e a sua predefinição de 24 é um valor
em ems. Uma linha de comandos não tem tipo de letra: conta carateres, os do
texto. **Copiar o 24 como carateres cortaria quase tudo, sem o
dizer**; não há, portanto, **nenhum comprimento predefinido**, e `--max-length` é obrigatório com
`line-break`:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` é 3**, o valor do Gaupol, que a unidade não altera; a quebra de linhas toma
mais uma quando três não chegam. **A omissão** — uma legenda que já está dentro dos limites é
deixada como está — adota por predefinição **os mesmos limites** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), como faz o Gaupol; `--skip-length off` e `--skip-lines off`
desativam-na, e o texto é então lido de novo como um só e cortado de novo:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

Os padrões de quebra são os do código: `Latn` não quebra entre um número e a sua unidade nem
depois de um hífen de diálogo, `Latn-en` não quebra depois de um artigo, de uma preposição, de um título
ou de um determinante possessivo. **Sob `Zyyy`, não existe nenhum padrão de quebra**: as linhas são
apenas equilibradas. **Esta tarefa nunca é recusada por falta de um padrão ativo**: os seus padrões
ponderam onde quebrar, e a quebra faz-se sem eles. As quatro opções só fazem sentido com
`line-break`; dadas sem ela, são um erro de utilização.

### As classes e a ativação por nome

**`--classes` filtra definitivamente os erros comuns**: desmarcar `ocr` retira os seus
padrões da aplicação — a ligatura `ﬁ` continua então `ﬁ`. Os outros três tipos de padrões
não têm classe.

**`--enable` e `--disable` definem um padrão pelo seu nome inglês**, sendo o nome a chave
(ADR 0037); um nome pode designar vários registos, que partilham então uma única
caixa. É necessário, e é o que põe de lado as predefinições da janela: **os
ficheiros `.conf` fornecidos desativam os padrões de menções**, pelo que `--tasks mentions` sozinho
não aplicaria nada. Os dois padrões “Sound in brackets” e “Sound in parentheses”
comandam cada um o seu delimitador — os parênteses retos, os parênteses curvos — e são definidos pelas mesmas
opções: ativar um sozinho não remove o outro.

Três coisas são **erros de utilização**, código `1`, ditos antes de qualquer ficheiro ser lido:

| O que os desencadeia | Mensagem |
| :------------------ | :------ |
| um nome que não designa nenhum padrão das tarefas dadas, sob o código dado | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| um nome que designa padrões de dois tipos — escreve-se então `type:name`, sendo o tipo `common-error`, `capitalization`, `hearing-impaired` ou `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| um nome dado a `--enable` e a `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**Um erro de escrita é sonoro**: a janela ignora uma definição obsoleta porque uma
definição pode sobreviver a um padrão que desapareceu; uma invocação não tem sobreviventes.

**Uma tarefa sem nenhum padrão ativo é recusada**, código `1` — “nada a fazer” é uma
resposta que se diz, não um êxito silencioso:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

O mesmo ficheiro, com a varredura dos parênteses retos ativada:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## O que é escrito e o que é dito

O formato, a codificação e os fins de linha do ficheiro lido são **mantidos**. **As
legendas que a correção esvazia são removidas**, como a caixa predefinida da
janela; `--keep-blank-subtitles` deixa-as vazias, e `--remove-blank-subtitles` remove-as
mesmo quando o ficheiro de definições as mantém. **Uma tradução esvaziada continua esvaziada**
e mantém a sua legenda.

**A contagem é a da janela** — “Edited N and removed M subtitles”: textos
alterados e legendas removidas, **nunca correspondências**. Um ficheiro em que nada
muda é escrito na mesma: um destino dado é um destino escrito.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Um padrão que não se pode aplicar

**Um padrão que não pode ser lido, não pode ser convertido ou não termina é nomeado, com a
legenda onde desistiu**, e é aqui que a linha de comandos diz mais do que a
janela: a página de confirmação nomeia o padrão uma única vez, sem o texto.

| Motivo | O que significa |
| :----- | :------------------- |
| `cannot be translated` | a sua expressão usa o que a conversão para ICU recusa |
| `will not compile` | o motor recusou a sua expressão |
| `has an invalid replacement` | a sua substituição não se lê como o Python a lê |
| `timed out` | o motor desistiu desse texto |
| `never settled` | `Repeat` ainda estava a alterar o texto após cem passagens |
| `grew the text too long` | o texto ultrapassou o que uma legenda pode conter |

Ao nível 1, na saída de erros:
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — sem o final quando o padrão é posto de lado para todos os textos. **O número é o
do ficheiro**, sejam quais forem as tarefas que removeram legendas antes. Em JSON, é
um aviso `pattern-failed` do ficheiro, cujo `detail` traz a frase. **Um padrão
abandonado não é um ficheiro falhado**: o ficheiro é escrito, e o código de saída é `0`.

## Saída

**Saída padrão** — nada, sendo o resultado o ficheiro escrito. **Com `--dry-run`, são as
alterações propostas**, um bloco por legenda: o seu número, o texto antes, o texto depois —
e `(removed)` sem linha `+` para uma legenda que a correção removeria; com
`--keep-blank-subtitles`, fica, vazia. Ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, os padrões que não puderam ser aplicados, os ficheiros de padrões que não podem ser lidos — e, primeiro, o ficheiro de definições lido — `settings: read <path>` — e o que dele foi ignorado |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, depois todos os diagnósticos de leitura |

Em JSON, `counts` traz `corrected` (os textos alterados) e `removed` (as legendas
removidas), e `changes` as legendas tocadas. Com `-t`, a chave `alignment` diz como a
tradução foi disposta.
`settings` nomeia o ficheiro de definições que foi lido, ou é `null` quando não houve nenhum.

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram escritos, `2` se nenhum, `3` se apenas
alguns, `1` em caso de erro de utilização.

## Erros

| O que os desencadeia | O que é escrito, na saída de erros |
| :------------------ | :--------------------------------------- |
| um ficheiro indicado por `--config` que não existe — o ficheiro do próprio utilizador, em falta, dá as predefinições | `settings file not found: <FILE>` |
| `--tasks` em falta | `--tasks is required`, seguido de uma indicação para `--help` |
| uma tarefa desconhecida | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| `--code` em falta | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` sem uma tarefa de padrões | `--code is for the tasks that read patterns, and none was asked for` |
| `--language` em falta com `join-words` ou `split-words` | `--language is required by the tasks that check words: <tasks>` |
| `--language` que não é um código de idioma | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` sem uma destas duas tarefas | `--language is for the tasks join-words and split-words, which were not asked for` |
| um idioma sem dicionário | `no dictionary for <language>` |
| um código mal escrito | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| `--max-length` em falta com `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length` que não é um número superior a zero | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines` que não são um número inteiro de linhas | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` ou `--skip-lines` ilegíveis | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| uma das quatro opções sem `line-break` | `--max-length is for the task line-break, which was not asked for` |
| uma classe desconhecida | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` com `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| um nome de padrão errado | ver a tabela acima |
| uma tarefa sem padrão ativo | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, o destino | os de [Invocação](invocation.md#erros) e de [Uma tradução](invocation.md#uma-tradução) |
