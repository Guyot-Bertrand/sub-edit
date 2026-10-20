# `adjust`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

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
legenda curta demais para ser lida é alongada, uma legenda que sai da tela
tarde demais pode ser encurtada, e duas legendas não se sobrepõem. **Só o
fim se move**: mover o início deslocaria a legenda, o que é trabalho de
[`shift`](shift.md). É a operação `Adjust Durations…` da
[janela](../subedit-gui/operations.md#adjust-durations), com os mesmos valores padrão.

## As quatro restrições e sua ordem

Elas se aplicam **nesta ordem**, cada uma definindo o fim, e a última prevalece:

1. **a velocidade de leitura** — a duração de que o texto precisa, a tantos caracteres por
   segundo (as marcações não contam, as quebras de linha contam);
2. **o mínimo** — uma duração abaixo dele é elevada ao mínimo;
3. **o máximo** — uma duração acima dele é reduzida ao máximo;
4. **o intervalo entre legendas** — o tempo deixado antes da legenda seguinte. Ele vem por último e
   **prevalece**: duas legendas exibidas ao mesmo tempo são uma falha que o espectador vê,
   uma legenda um pouco curta não é.

A última legenda do arquivo não tem legenda seguinte e, portanto, nenhum intervalo a respeitar. A seguinte
de uma legenda é a que vem depois dela no arquivo, **tenha sido escolhida ou não** por `--range`.

| Restrição | Opção | Padrão | Como desativá-la |
| :--------- | :----- | :----- | :------- |
| velocidade | `--speed CPS` | **15** caracteres por segundo | `--speed off` |
| sentido da velocidade | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | só **alonga** | — |
| mínimo | `--minimum TIME` | **1.5 s** | `--minimum off` |
| máximo | `--maximum TIME` | **nenhum** | `--maximum off` |
| intervalo entre legendas | `--gap TIME` | **0** | `--gap off` |

São as configurações do Gaupol e da primeira abertura da janela. **`off` desativa uma
restrição: ela deixa de ser verificada.** Isso não é defini-la como zero — um mínimo de `0`
é um mínimo de zero, um intervalo de `0` proíbe qualquer sobreposição.

## O que o arquivo de configurações fornece

**Os valores padrão acima são os embutidos.** Por padrão, `adjust` lê as configurações da operação no
arquivo de configurações — os valores que `Adjust Durations…` lembra na janela, ou que
[`config set`](config.md) gravou —, e elas tomam o lugar dos valores padrão embutidos. **Uma opção
dada na linha de comando sempre prevalece**, depois vem o arquivo e, por fim, o valor padrão embutido. Veja
[O arquivo de configurações](invocation.md#o-arquivo-de-configurações) para saber onde o arquivo fica, `--config` e
`--no-config`.

| Chave | O que ela fornece | Opção que a substitui |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | a velocidade de leitura, em caracteres por segundo | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | em que sentido a velocidade pode mover um fim; ambas `false` desativam a velocidade | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; dar `--speed CPS` ou um sentido reativa uma velocidade que o arquivo havia desligado, alongando |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | a duração mais curta, ou nenhuma | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | a duração mais longa, ou nenhuma | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | o menor intervalo entre legendas, ou nenhum | `--gap TIME`, `--gap off` |

**Toda chave tem uma opção que a desfaz.** Cada sentido tem um par de opções que o define
seja o que for que o arquivo diga: `--shorten` e `--no-shorten` para encurtar, `--lengthen` e
`--no-lengthen` para alongar; uma opção não dada deixa o sentido do arquivo como está.
`--maximum off` descarta um máximo que o arquivo ativa. Com `--format json`, `constraints` indica
as configurações realmente usadas, de onde quer que venham, e `settings` o arquivo que foi lido:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Os demais exemplos desta página são executados com `--no-config`, para mostrar o que os valores
padrão embutidos fazem, sejam quais forem as configurações da máquina que os executa.

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

| Opção | Obrigatória | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | uma opção sem valor | desativada — veja [Processar uma árvore de diretórios](lots.md) |
| `--speed` | não | um número de caracteres por segundo, **maior que zero** (`15`, `12.5`), ou `off` | `15` |
| `--shorten` | não | uma opção sem valor: a velocidade também pode **trazer** um fim **para mais cedo** | desativada |
| `--no-shorten` | não | uma opção sem valor: a velocidade deixa de **trazer** um fim **para mais cedo**, seja o que for que o arquivo de configurações diga | desativada |
| `--lengthen` | não | uma opção sem valor: a velocidade **move** um fim **para mais tarde**, seja o que for que o arquivo de configurações diga | desativada |
| `--no-lengthen` | não | uma opção sem valor: a velocidade deixa de **mover** um fim **para mais tarde** | desativada |
| `--minimum` | não | uma duração de zero ou mais, ou `off` | `1.5` |
| `--maximum` | não | uma duração **maior que zero**, ou `off` | nenhum |
| `--gap` | não | uma duração de zero ou mais, ou `off` | `0` |
| `--range` | não | `N-M`, ou `N-` até o fim — veja abaixo | o arquivo inteiro |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativada |

Uma duração se escreve como para [`shift`](shift.md#escrever-uma-duração): `2.5`, `00:00:02.500`.

**Sem nenhuma restrição ativa, `adjust` é recusado** (código `1`, nada é gravado): não haveria
nada a que ajustar, e um comando que não faz nada enquanto diz ter tido sucesso é a pior
resposta. É o caso de `--speed off --minimum off --gap off` sem um máximo, e de
`--no-lengthen` sozinho com o mínimo e o intervalo desativados: uma velocidade que não move
nenhum fim não restringe nada.

`--shorten`, `--no-shorten`, `--lengthen` e `--no-lengthen` dizem em que sentido a velocidade move um
fim; só fazem sentido com uma velocidade, e **`--speed off` as recusa**. Uma opção dada com
a sua oposta — `--shorten --no-shorten`, `--lengthen --no-lengthen` — também é recusada.

### `--range`

`--range 3-7` limita o ajuste às legendas de 3 a 7, **contadas a partir de 1, limites
incluídos**; `--range 3-` vai até a última. Uma única legenda se escreve `4-4`.

- Um intervalo **que termina antes de começar**, um `0`, um sinal ou um número isolado são
  recusados como erro de uso (código `1`).
- **Ele vale para todos os arquivos do lote**, o que só faz sentido se os arquivos
  forem parecidos. É julgado **em relação a cada arquivo**: um intervalo que ultrapassa aquele arquivo é
  recusado **antes de ele ser tocado**, com indicação do limite —
  `range 3-9 ends after the last subtitle: the file has 4` —, aquele arquivo falha (código `2` ou `3`, identificador
  `range-out-of-bounds`) e os outros continuam.
- As contagens do relatório só dizem respeito às legendas escolhidas.

## O que é sacrificado

Duas restrições podem se contradizer na mesma legenda: um mínimo de 1.5 s não cabe
nos 0.8 s que a legenda seguinte deixa. **A ordem decide, e o relatório diz
o que cedeu** — o que o Gaupol não faz: ele as quebra em silêncio.

Cada restrição só é nomeada se **pelo menos uma legenda não a obteve**, depois que
as outras foram aplicadas. O máximo não tem contagem: ele é aplicado antes do intervalo entre legendas, que só
pode encurtar, de modo que sempre se mantém. Um mínimo acima do máximo cede ao máximo e é
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

A terceira legenda gostaria de terminar em 15.7 s para durar 1.5 s, mas a quarta começa em
15.0 s: **o intervalo entre legendas prevalece**, ela termina em 15.0 s, e o relatório nomeia o mínimo.

Desativar o intervalo entre legendas deixa o mínimo prevalecer — e as duas legendas se sobrepõem:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

E nada é sacrificado nem ajustado quando o arquivo já se encaixa nos seus limites:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

O arquivo é gravado como está: **um destino dado é um destino gravado**.

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado.

| Nível | O que se acrescenta no erro padrão |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, ou `<path>: no duration to adjust -> <destination>` — e, antes, o arquivo de configurações lido — `settings: read <path>` — veja [O arquivo de configurações](invocation.md#o-arquivo-de-configurações) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os finais de linha do arquivo lido, recolocados como estavam |
| 3 | `<path>: N bytes read, M written`, depois **todos os diagnósticos de leitura** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

A frase é a da janela: `N` é o número de legendas cujo fim se moveu, e o
restante, ausente quando nada cedeu, lista o que não foi mantido — `the reading speed`,
`the minimum duration`, `the gap` — com o número de legendas envolvidas.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada arquivo
traz, além do envelope:

| Chave | Conteúdo |
| :-- | :------ |
| `counts.subtitles` | as legendas **visadas**: o arquivo inteiro, ou o intervalo |
| `counts.adjusted` | aquelas cujo fim se moveu |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | as legendas que ainda não respeitam esta restrição, depois que as outras foram aplicadas, **incluindo aquelas cujo fim não se moveu** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — os caracteres por segundo **como string** (`"15"`, `"12.5"`) — ou `null` se desativada |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | as durações usadas, em **milissegundos inteiros**, ou `null` se desativadas |
| `settings` | o arquivo de configurações que foi lido, ou `null` quando nenhum foi |

`constraints` indica **as configurações realmente usadas**, valores padrão incluídos: dois cálculos
sobre as mesmas legendas se comparam, então, sem ler a linha de comando.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram processados, `2` se nenhum, `3` se apenas
alguns, `1` em caso de erro de uso. Um arquivo que já se encaixa nos seus limites é processado.

## Erros

| O que o provoca | Mensagem |
| :------------------ | :------ |
| um arquivo indicado por `--config` que não existe — o arquivo do próprio usuário, ausente, resulta nos valores padrão | `settings file not found: <FILE>` |
| nenhuma restrição ativa | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| uma velocidade que não é uma velocidade | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| uma opção de sentido com `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| uma opção de sentido com a sua oposta | `--shorten and --no-shorten say opposite things; give one of them` (da mesma forma `--lengthen` e `--no-lengthen`) |
| uma duração que não é uma duração | `--minimum: "…" is not a time: …` (da mesma forma `--maximum`, `--gap`) |
| duração negativa | `--minimum: "-1" is not a duration of zero or more` (da mesma forma `--gap`) |
| máximo zero ou negativo | `--maximum: "0" is not a duration above zero` |
| intervalo mal escrito | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| intervalo que termina antes de começar | `--range: "5-3" is not a range: it ends before it starts` |
| intervalo fora do arquivo | `<path>: range N-M starts after the last subtitle: the file has K`, ou `… ends after …` |

Os erros do destino são comuns aos subcomandos que gravam:
veja [Invocação](invocation.md#o-destino).
