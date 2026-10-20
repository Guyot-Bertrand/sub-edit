# `correct`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

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
deficientes auditivos**, a **união** e a **divisão de palavras**, os **erros comuns**,
o **uso de maiúsculas** e a **quebra de linhas**.

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

**Nada vem marcado de antemão.** A janela abre suas caixas como o usuário as deixou; uma
linha de comando toma suas tarefas, seu código, seu comprimento de linha e seu idioma da própria linha,
nunca do arquivo de configurações: nem as tarefas marcadas um dia na janela, nem as caixas de
padrões ativadas ali (`correction.activations`), nem a lista de substituições do corretor ortográfico são
lidas, e nada é gravado desse lado.

**Quatro configurações vêm do arquivo de configurações, por padrão** — os valores que `Correct Texts…`
lembra, ou que [`config set`](config.md) gravou. **Uma opção na linha sempre prevalece**, depois vem
o arquivo e, por fim, o valor padrão embutido; veja [O arquivo de configurações](invocation.md#o-arquivo-de-configurações)
para saber onde ele fica, `--config` e `--no-config`.

| Chave | O que ela fornece | Opção que a substitui |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | as classes de erros comuns aplicadas | `--classes` |
| `correction.remove-blank` | se as legendas que a correção esvazia são removidas | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | o máximo de linhas de `line-break` | `--max-lines` |

- **Cada uma das quatro tem uma opção que a desfaz**, seja o que for que o arquivo diga: `--classes human,ocr`
  aplica as duas classes, `--remove-blank-subtitles` remove as legendas esvaziadas, `--max-lines 3`
  devolve as linhas do Gaupol. `--keep-blank-subtitles` e `--remove-blank-subtitles` juntas são um
  erro de uso.
- **As duas classes desativadas no arquivo são ignoradas, e isso é informado** no nível 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` não conseguiria escrevê-lo, e ele
  deixaria common-errors sem nada a fazer. As duas classes são aplicadas em seu lugar.
- **Um número de linhas fora do intervalo de 1 a 1000 não é lido do arquivo de modo algum**: o leitor do arquivo
  o recusa — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, no
  nível 1 — e o 3 embutido é usado.

Os exemplos desta página são executados com `--no-config`, para mostrar o que os valores
padrão embutidos fazem, sejam quais forem as configurações da máquina que os executa.

| Opção | Obrigatória | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--tasks` | **sim** | uma lista separada por vírgulas: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | nenhuma tarefa |
| `--code` | **sim** com uma tarefa de padrões, e **não se aplica** nos demais casos | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **sim** com `join-words` ou `split-words`, e **não se aplica** nos demais casos | um código de idioma: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | não | `human`, `ocr` ou `human,ocr` | as duas — ou o que o arquivo de configurações diz |
| `--enable`, `--disable` | não, repetível | o nome em inglês de um padrão, ou `type:name` | os arquivos `.conf` fornecidos |
| `--max-length` | **sim** com `line-break` | um número de **caracteres** maior que zero | **nenhum** — veja abaixo |
| `--max-lines` | não | um número inteiro de linhas, maior que zero | 3 — ou o que o arquivo de configurações diz |
| `--skip-length`, `--skip-lines` | não | um número, ou `off` | os mesmos limites de `--max-length` e `--max-lines` |
| `--keep-blank-subtitles` | não | uma opção sem valor: as legendas esvaziadas permanecem, vazias | as legendas esvaziadas são removidas — a menos que o arquivo de configurações diga o contrário |
| `--remove-blank-subtitles` | não | uma opção sem valor: as legendas esvaziadas são removidas, seja o que for que o arquivo de configurações diga; recusada com `--keep-blank-subtitles` | desativada |
| `--range` | não | `N-M`, ou `N-` até o fim — veja [`adjust`](adjust.md#--range) | o arquivo inteiro |
| `--document` | não | `main` ou `translation` — veja [Uma tradução](invocation.md#uma-tradução) | `main` |
| `-t`, `--translation-file` | com `--document translation` | um arquivo de tradução, para **uma única** entrada | — |
| `--align-method` | não, e somente com `-t` | `position` ou `number` | `position` |
| `--recursive`, `-r` | não | uma opção sem valor | desativada — veja [Processar uma árvore de diretórios](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativada |

### As tarefas

**A ordem da lista não faz diferença**: as tarefas se aplicam na
ordem do Gaupol — as menções, depois a união e a divisão de palavras, depois os erros comuns, depois o uso de maiúsculas e, por fim, a quebra de linhas —, cada uma sobre o
texto que a anterior deixou. Uma legenda que as menções esvaziaram não participa
do que vem depois.

| Tarefa | O que ela faz |
| :---- | :-------------- |
| `mentions` | remove os sons entre colchetes, e entre parênteses quando `Sound in parentheses` está ativado (a varredura de [`hearing-impaired`](hearing-impaired.md)), depois os padrões de letras de música e de locutores |
| `join-words` | cola de volta uma palavra cortada em duas, a partir do dicionário de `--language` |
| `split-words` | separa duas palavras coladas, a partir do mesmo dicionário |
| `common-errors` | corrige os erros comuns: espaços, pontuação, ligaduras, erros de OCR |
| `capitalization` | coloca a maiúscula onde o idioma a exige |
| `line-break` | corta de novo as linhas de uma legenda para que nenhuma ultrapasse `--max-length`, ponderando os padrões de quebra do código |

### O código dos padrões

**`--code` nomeia a cascata**: `Zyyy` para qualquer escrita, depois a escrita, o idioma
e o país — `Latn-en-US` lê os padrões de `Zyyy`, de `Latn`, de `Latn-en` e de
`Latn-en-US`. **Ele é obrigatório**, porque uma linha de comando não tem idioma ambiente:
um valor padrão universal aplicaria *menos* do que se acredita, sem dizer. Ele vale para
todas as tarefas da mesma execução; uma tarefa que queira um código próprio é uma segunda
execução. Um código mal escrito é um erro de uso:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Os padrões que o usuário coloca em `$XDG_DATA_HOME/subedit/patterns` (ou
`~/.local/share/subedit/patterns`) **são acrescentados aos padrões fornecidos**, como na
janela, e um padrão fornecido que não possa ser lido é informado no nível 1: `patterns: <file>, line N (<reason>)`.

### União e divisão de palavras, e o dicionário

**Essas duas tarefas não leem padrão algum**: elas consultam o **dicionário do sistema, por meio do
Enchant** — uma dependência dos pacotes (`libenchant-2-2` no Debian e no Ubuntu, `enchant2` no
Fedora), que o `subedit-cli` compartilha com a janela. Elas não têm, portanto, nem código nem nome a definir:
`--code` não faz sentido sem uma tarefa de padrões, e `--language` não faz sentido sem uma destas duas
— dar um sem o outro é um erro de uso, código `1`.

- **`join-words`** cola uma palavra que o dicionário não conhece àquela que a precede ou
  a segue, **quando uma única direção resulta em uma palavra conhecida**: `hel lo` se torna `hello`; duas possíveis
  direções, ou nenhuma, não decidem nada.
- **`split-words`** separa uma palavra que o dicionário não conhece **quando exatamente uma de suas
  sugestões é essa palavra com um espaço**: `hellothere` se torna `hello there`. Uma palavra com uma inicial
  maiúscula seguida de minúsculas é deixada em paz: na maioria das vezes é um nome.

**Sem um dicionário para o idioma, a execução é recusada**, código `1`, **antes que um arquivo seja
lido** e com as palavras da janela, que esmaece a função e diz isso — aqui a linha de comando
recusa e diz isso, em vez de se calar:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**A lista de substituições da janela não é lida nem gravada.** Ela *se enche* pelas
escolhas do usuário na verificação ortográfica; uma linha de comando não tem tal
escolha, e ler a lista faria um script depender do que seu autor um dia colocou nela.

### Quebra de linhas

**Ela é medida em caracteres, e essa é a diferença em relação à janela.** A janela mede em *ems* —
uma largura calibrada sobre a fonte, a 0.55 em por letra —, e seu valor padrão 24 é um valor
em ems. Uma linha de comando não tem fonte: ela conta caracteres, os do
texto. **Copiar o 24 como caracteres cortaria quase tudo, sem
dizer**; não existe, portanto, **nenhum comprimento padrão**, e `--max-length` é obrigatório com
`line-break`:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` é 3**, o valor do Gaupol, que a unidade não muda; a quebra de linhas toma
mais uma quando três não bastam. **A exceção** — uma legenda já dentro dos limites é
deixada como está — toma por padrão **os mesmos limites** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), como o Gaupol faz; `--skip-length off` e `--skip-lines off`
a desativam, e o texto é então lido de novo como um só e cortado de novo:

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

Os padrões de quebra são os do código: `Latn` não quebra entre um número e sua unidade nem
depois de um travessão de diálogo, `Latn-en` não quebra depois de um artigo, uma preposição, um título
ou um determinante possessivo. **Sob `Zyyy`, não existe nenhum padrão de quebra**: as linhas são
apenas equilibradas. **Essa tarefa nunca é recusada por falta de um padrão ativo**: seus padrões
ponderam onde quebrar, e a quebra acontece sem eles. As quatro opções só fazem sentido com
`line-break`; dadas sem ela, são um erro de uso.

### As classes e a ativação por nome

**`--classes` filtra os erros comuns de vez**: desmarcar `ocr` tira seus
padrões da aplicação — a ligadura `ﬁ` permanece então `ﬁ`. Os outros três tipos de padrões
não têm classe.

**`--enable` e `--disable` definem um padrão pelo seu nome em inglês**, sendo o nome a chave
(ADR 0037); um nome pode designar vários registros, que compartilham então uma única
caixa. É necessário, e é o que deixa de lado os valores padrão da janela: **os arquivos
`.conf` fornecidos desativam os padrões de menções**, de modo que `--tasks mentions` sozinho
não aplicaria nada. Os dois padrões “Sound in brackets” e “Sound in parentheses”
comandam cada um o seu delimitador — os colchetes, os parênteses — e são definidos pelas mesmas
opções: ativar um sozinho não remove o outro.

Três coisas são **erros de uso**, código `1`, informados antes que um arquivo seja lido:

| O que o provoca | Mensagem |
| :------------------ | :------ |
| um nome que não designa nenhum padrão das tarefas dadas, sob o código dado | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| um nome que designa padrões de dois tipos — escreve-se então `type:name`, sendo o tipo `common-error`, `capitalization`, `hearing-impaired` ou `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| um nome dado a `--enable` e a `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**Um erro de digitação é ruidoso**: a janela ignora uma configuração obsoleta porque uma
configuração pode sobreviver a um padrão que desapareceu; uma invocação não tem sobrevivente.

**Uma tarefa sem nenhum padrão ativo é recusada**, código `1` — “nada a fazer” é uma
resposta que se diz, não um sucesso silencioso:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

O mesmo arquivo, com a varredura dos colchetes ativada:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## O que é gravado e o que é dito

O formato, a codificação e os finais de linha do arquivo lido são **mantidos**. **As
legendas que a correção esvazia são removidas**, como na caixa padrão da janela;
`--keep-blank-subtitles` as deixa vazias, e `--remove-blank-subtitles` as remove
mesmo quando o arquivo de configurações as mantém. **Uma tradução esvaziada permanece esvaziada**
e conserva a sua legenda.

**A contagem é a da janela** — “Edited N and removed M subtitles”: textos
alterados e legendas removidas, **nunca correspondências**. Um arquivo em que nada
muda é gravado do mesmo modo: um destino dado é um destino gravado.

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

### Um padrão que não pode ser aplicado

**Um padrão que não pode ser lido, não pode ser traduzido ou não termina é nomeado, com a
legenda em que ele desistiu**, e é aqui que a linha de comando diz mais do que a
janela: a página de confirmação nomeia o padrão uma só vez, sem o texto.

| Motivo | O que significa |
| :----- | :------------------- |
| `cannot be translated` | sua expressão usa o que a tradução para ICU recusa |
| `will not compile` | o motor recusou sua expressão |
| `has an invalid replacement` | sua substituição não é lida como o Python a lê |
| `timed out` | o motor desistiu desse texto |
| `never settled` | `Repeat` ainda alterava o texto depois de cem passagens |
| `grew the text too long` | o texto ultrapassou o que uma legenda pode conter |

No nível 1, no erro padrão: 
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — sem o final quando o padrão é deixado de lado para todos os textos. **O número é o
do arquivo**, sejam quais forem as tarefas que removeram legendas antes. Em JSON, é
um aviso `pattern-failed` do arquivo, cujo `detail` traz a frase. **Um padrão
abandonado não é um arquivo com falha**: o arquivo é gravado, e o código de saída é `0`.

## Saída

**Saída padrão** — nada, sendo o resultado o arquivo gravado. **Com `--dry-run`, são as
alterações propostas**, um bloco por legenda: seu número, o texto antes, o texto depois —
e `(removed)` sem linha `+` para uma legenda que a correção removeria; com
`--keep-blank-subtitles`, ela permanece, vazia. Veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta no erro padrão |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, os padrões que não puderam ser aplicados, os arquivos de padrões que não podem ser lidos — e, antes, o arquivo de configurações lido — `settings: read <path>` — e o que dele foi ignorado |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, depois todos os diagnósticos de leitura |

Em JSON, `counts` traz `corrected` (os textos alterados) e `removed` (as legendas
removidas), e `changes` as legendas tocadas. Com `-t`, a chave `alignment` diz como a
tradução foi posicionada.
`settings` nomeia o arquivo de configurações que foi lido, ou é `null` quando nenhum foi.

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram gravados, `2` se nenhum, `3` se
apenas alguns, `1` em caso de erro de uso.

## Erros

| O que o provoca | O que é escrito, no erro padrão |
| :------------------ | :--------------------------------------- |
| um arquivo indicado por `--config` que não existe — o arquivo do próprio usuário, ausente, resulta nos valores padrão | `settings file not found: <FILE>` |
| `--tasks` ausente | `--tasks is required`, seguido de uma indicação de `--help` |
| uma tarefa desconhecida | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| `--code` ausente | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` sem uma tarefa de padrões | `--code is for the tasks that read patterns, and none was asked for` |
| `--language` ausente com `join-words` ou `split-words` | `--language is required by the tasks that check words: <tasks>` |
| `--language` que não é um código de idioma | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` sem uma dessas duas tarefas | `--language is for the tasks join-words and split-words, which were not asked for` |
| um idioma sem dicionário | `no dictionary for <language>` |
| um código mal escrito | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| `--max-length` ausente com `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length` que não é um número maior que zero | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines` que não são um número inteiro de linhas | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` ou `--skip-lines` ilegível | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| uma das quatro opções sem `line-break` | `--max-length is for the task line-break, which was not asked for` |
| uma classe desconhecida | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` com `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| um nome de padrão defeituoso | veja a tabela acima |
| uma tarefa sem padrão ativo | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, o destino | os de [Invocação](invocation.md#erros) e de [Uma tradução](invocation.md#uma-tradução) |
