# `adjust`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Ajusta a **duração** de cada legenda a uma velocidade de leitura e a limites: uma
legenda demasiado curta para ser lida é alongada, uma legenda que sai do ecrã
demasiado tarde pode ser encurtada, e duas legendas não se sobrepõem. **Só o
fim se move**: mover o início seria mover a legenda, o que é trabalho do subcomando
[`shift`](shift.md). É a operação `Adjust Durations…` da
[janela](../subedit-gui/operations.md#adjust-durations), com as mesmas predefinições.

## As quatro restrições e a sua ordem

Aplicam-se **por esta ordem**, cada uma definindo o fim, e a última prevalece:

1. **a velocidade de leitura** — a duração de que o texto precisa, a tantos carateres por
   segundo (as etiquetas não contam, as quebras de linha contam);
2. **o mínimo** — uma duração abaixo dele é elevada até ao mínimo;
3. **o máximo** — uma duração acima dele é reduzida até ao máximo;
4. **o espaço** — o tempo deixado antes da legenda seguinte. Vem em último lugar e
   **prevalece**: duas legendas apresentadas em simultâneo são uma falha que o espectador vê,
   uma legenda um pouco curta não é.

A última legenda do ficheiro não tem seguinte e, portanto, não tem espaço a respeitar. A legenda seguinte
de uma legenda é a que vem no ficheiro, **tenha sido escolhida ou não** por `--range`.

| Restrição | Opção | Predefinição | Desativar |
| :--------- | :----- | :----- | :------- |
| velocidade | `--speed CPS` | **15** carateres por segundo | `--speed off` |
| sentido da velocidade | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | só **alonga** | — |
| mínimo | `--minimum TIME` | **1,5 s** | `--minimum off` |
| máximo | `--maximum TIME` | **nenhum** | `--maximum off` |
| espaço | `--gap TIME` | **0** | `--gap off` |

São as definições do Gaupol e da primeira abertura da janela. **`off` desativa uma
restrição: deixa de ser verificada.** Isso não é pô-la a zero — um mínimo de `0`
é um mínimo de zero, um espaço de `0` proíbe qualquer sobreposição.

## O que o ficheiro de definições fornece

**As predefinições acima são as incorporadas.** Por predefinição, o subcomando `adjust` lê as definições da operação no
ficheiro de definições — os valores que `Adjust Durations…` memoriza na janela, ou que
[`config set`](config.md) escreveu — e estas substituem as predefinições incorporadas. **Uma opção
dada na linha prevalece sempre**, depois o ficheiro, depois a predefinição incorporada. Veja
[O ficheiro de definições](invocation.md#o-ficheiro-de-definições) para saber onde está o ficheiro, `--config` e
`--no-config`.

| Chave | O que fornece | Opção que a anula |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | a velocidade de leitura, em carateres por segundo | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | em que sentido a velocidade pode mover um fim; com ambas a `false`, a velocidade fica desativada | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; dar `--speed CPS` ou um sentido volta a ativar uma velocidade que o ficheiro desativou, alongando |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | a duração mais curta, ou nenhuma | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | a duração mais longa, ou nenhuma | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | o menor espaço, ou nenhum | `--gap TIME`, `--gap off` |

**Cada chave tem uma opção que a anula.** Cada sentido tem um par de indicadores que o definem
seja o que for que o ficheiro diga: `--shorten` e `--no-shorten` para encurtar, `--lengthen` e
`--no-lengthen` para alongar; um indicador não dado deixa o sentido do ficheiro como está.
`--maximum off` abandona um máximo que o ficheiro ativa. Com `--format json`, `constraints` indica
as definições realmente usadas, venham de onde vierem, e `settings` o ficheiro que foi lido:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Os outros exemplos desta página são executados com `--no-config`, para que mostrem o que as predefinições
incorporadas fazem, sejam quais forem as definições da máquina que os executa.

<!-- example: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-shorten                Do not let the reading speed bring an end earlier
  --lengthen                  Let the reading speed move an end later
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME|off          Longest duration, or off (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opções

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--speed` | não | um número de carateres por segundo, **superior a zero** (`15`, `12.5`), ou `off` | `15` |
| `--shorten` | não | um indicador: a velocidade também pode **trazer** um fim **para mais cedo** | desativado |
| `--no-shorten` | não | um indicador: a velocidade deixa de **trazer** um fim **para mais cedo**, seja o que for que o ficheiro de definições diga | desativado |
| `--lengthen` | não | um indicador: a velocidade **move** um fim **para mais tarde**, seja o que for que o ficheiro de definições diga | desativado |
| `--no-lengthen` | não | um indicador: a velocidade deixa de **mover** um fim **para mais tarde** | desativado |
| `--minimum` | não | uma duração de zero ou mais, ou `off` | `1.5` |
| `--maximum` | não | uma duração **superior a zero**, ou `off` | nenhum |
| `--gap` | não | uma duração de zero ou mais, ou `off` | `0` |
| `--range` | não | `N-M`, ou `N-` até ao fim — ver abaixo | o ficheiro inteiro |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

Uma duração escreve-se como para [`shift`](shift.md#escrever-uma-duração): `2.5`, `00:00:02.500`.

**Sem nenhuma restrição ativa, o subcomando `adjust` é recusado** (código `1`, nada é escrito): não haveria
nada a que ajustar, e um comando que não faz nada e diz que teve êxito é a pior
resposta. É o caso de `--speed off --minimum off --gap off` sem máximo, e de
`--no-lengthen` sozinho com o mínimo e o espaço desativados: uma velocidade que não move
nenhum fim não restringe nada.

`--shorten`, `--no-shorten`, `--lengthen` e `--no-lengthen` dizem em que sentido a velocidade move um
fim; só fazem sentido com uma velocidade, e **`--speed off` recusa-os**. Um indicador dado com o seu
oposto — `--shorten --no-shorten`, `--lengthen --no-lengthen` — também é recusado.

### `--range`

`--range 3-7` limita o ajuste às legendas 3 a 7, **contadas a partir de 1, limites
incluídos**; `--range 3-` vai até à última. Uma única legenda escreve-se `4-4`.

- Um intervalo **que termina antes de começar**, um `0`, um sinal ou um número isolado são
  recusados como erro de utilização (código `1`).
- **Vale para todos os ficheiros do lote**, o que só faz sentido se os ficheiros
  se parecerem. É avaliado **em relação a cada ficheiro**: um intervalo que ultrapasse esse ficheiro é
  recusado **antes de lhe tocar**, indicando o limite —
  `range 3-9 ends after the last subtitle: the file has 4` —, esse ficheiro falha (código `2` ou `3`, identificador
  `range-out-of-bounds`) e os outros continuam.
- As contagens do relatório só dizem respeito às legendas escolhidas.

## O que é sacrificado

Duas restrições podem contradizer-se na mesma legenda: um mínimo de 1,5 s não cabe
nos 0,8 s que a legenda seguinte deixa. **A ordem decide, e o relatório diz
o que cedeu** — o que o Gaupol não faz: quebra-as em silêncio.

Cada restrição só é nomeada se **pelo menos uma legenda não a obteve**, depois de
as outras serem aplicadas. O máximo não tem contagem: é aplicado antes do espaço, que só
pode encurtar, pelo que se cumpre sempre. Um mínimo superior ao máximo cede ao máximo e é
contado como mínimo.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

A terceira legenda gostaria de terminar aos 15,7 s para durar 1,5 s, mas a quarta começa
aos 15,0 s: **o espaço prevalece**, termina aos 15,0 s, e o relatório nomeia o mínimo.

Desativar o espaço deixa o mínimo prevalecer — e as duas legendas sobrepõem-se:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

E nada é sacrificado nem ajustado quando o ficheiro já cabe nos seus limites:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

O ficheiro é escrito tal como está: **um destino dado é um destino escrito**.

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito.

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, ou `<path>: no duration to adjust -> <destination>` — e, primeiro, o ficheiro de definições lido — `settings: read <path>` — ver [O ficheiro de definições](invocation.md#o-ficheiro-de-definições) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os fins de linha do ficheiro lido, repostos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **cada diagnóstico de leitura** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

A frase é a da janela: `N` é o número de legendas cujo fim se moveu, e o
resto, ausente quando nada cedeu, enumera o que não foi respeitado — `the reading speed`,
`the minimum duration`, `the gap` — com o número de legendas em causa.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada ficheiro
traz, além do envelope:

| Chave | Conteúdo |
| :-- | :------ |
| `counts.subtitles` | as legendas **visadas**: o ficheiro inteiro, ou o intervalo |
| `counts.adjusted` | aquelas cujo fim se moveu |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | as legendas que continuam a não respeitar esta restrição, depois de as outras serem aplicadas, **incluindo aquelas cujo fim não se moveu** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — os carateres por segundo **como texto** (`"15"`, `"12.5"`) — ou `null` se estiver desativada |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | as durações usadas, em **milissegundos inteiros**, ou `null` se estiverem desativadas |
| `settings` | o ficheiro de definições que foi lido, ou `null` quando não houve nenhum |

`constraints` indica **as definições realmente usadas**, predefinições incluídas: dois cálculos
sobre as mesmas legendas comparam-se então sem ler a linha de comandos.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram processados, `2` se nenhum, `3` se apenas
alguns, `1` em caso de erro de utilização. Um ficheiro que já cabe nos seus limites é processado.

## Erros

| O que os desencadeia | Mensagem |
| :------------------ | :------ |
| um ficheiro indicado por `--config` que não existe — o ficheiro do próprio utilizador, em falta, dá as predefinições | `settings file not found: <FILE>` |
| nenhuma restrição ativa | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| uma velocidade que não o é | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| um indicador de sentido com `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| um indicador de sentido com o seu oposto | `--shorten and --no-shorten say opposite things; give one of them` (do mesmo modo `--lengthen` e `--no-lengthen`) |
| uma duração que não o é | `--minimum: "…" is not a time: …` (do mesmo modo `--maximum`, `--gap`) |
| duração negativa | `--minimum: "-1" is not a duration of zero or more` (do mesmo modo `--gap`) |
| máximo nulo ou negativo | `--maximum: "0" is not a duration above zero` |
| intervalo mal escrito | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| intervalo que termina antes de começar | `--range: "5-3" is not a range: it ends before it starts` |
| intervalo fora do ficheiro | `<path>: range N-M starts after the last subtitle: the file has K`, ou `… ends after …` |

Os erros do destino são comuns aos subcomandos que escrevem:
ver [Invocação](invocation.md#o-destino).
