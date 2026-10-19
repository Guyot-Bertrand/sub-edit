# `framerate`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

> **Isto non é [`snap`](snap.md).** Esta orde **escala o
> ficheiro**: é para cando a temporización é errada e se desvía varios
> segundos ao final da película. A suborde `snap` volve poñer as marcas de tempo na grade
> máis próxima sen recalcular os tempos de nada, e é para cando a temporización xa é
> correcta. As dúas teñen a mesma forma, unha taxa de fotogramas ou dúas, e confundilas non provoca
> ningún erro.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Recalcula os tempos dun ficheiro temporizado para unha taxa de fotogramas a outra. É
a operación que require un ficheiro de subtítulos cando se colleu para unha copia que
non é a da película que se ten: un DVD PAL fronte a unha transferencia NTSC, por
exemplo.

O formato do ficheiro lido **consérvase**: cambiar o formato é tarefa de
[`convert`](convert.md).

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opcións

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :----- | :------ | :----- |
| `<file>...` | si | unha ou máis rutas | — |
| `--recursive`, `-r` | non | un indicador | desactivado — véxase [Procesar unha árbore](lots.md) |
| `--from` | **si** | unha taxa de fotogramas, véxase máis abaixo | — |
| `--to` | **si** | unha taxa de fotogramas, véxase máis abaixo | — |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

A opción `--from` é a taxa de fotogramas para a que o ficheiro **está** temporizado, e `--to` aquela
para a que se quere. Intercambialas despraza o ficheiro no
outro sentido (uns sete segundos nunha longametraxe entre 25 e
23,976), e nada o sinala antes de ver a película.

## Escribir unha taxa de fotogramas

En fotogramas por segundo, decimais incluídos, co punto como separador:

```
25      24      23.976      29.97      60
```

**Rexéitase unha taxa de fotogramas nula ou negativa**: non é unha taxa de fotogramas.

### As tres taxas de fotogramas NTSC non se poden escribir exactamente

`23.976`, `29.97` e `59.94` son **etiquetas**. As taxas de fotogramas que
nomean son `24000/1001`, `30000/1001` e `60000/1001`, e ningunha delas ten
unha escritura decimal finita. A ferramenta recoñéceas como tales:

| O que se escribe | O que nomea |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — cinema transferido a vídeo NTSC |
| `24` | `24` — cinema |
| `25` | `25` — vídeo PAL e SECAM |
| `29.97` | `30000/1001` — vídeo NTSC |
| `30`, `50`, `60` | elas mesmas |
| `59.94` | `60000/1001` |

O recoñecemento refírese ao **valor e non ás letras**: `23.9760`
nomea a mesma taxa de fotogramas que `23.976`.

Calquera outro decimal tómase literalmente: `23.9` non é o estándar
de ninguén, non hai nada que interpretar. Iso é tamén o que fai necesaria a táboa de
arriba: sen ela, `23.976` sería `23976/1000`, e a liña de ordes
diría algo distinto da xanela coas mesmas palabras.

## En que se converten as posicións

```
t′ = round( t × (from / to) )
```

Unha película masterizada a 23,976 e reproducida a 25 fotogramas por segundo vai máis rápido,
polo que os seus subtítulos chegan antes; no outro sentido, chegan
máis tarde.

**O factor é un racional exacto, e o redondeo prodúcese unha soa vez.** O
camiño que parece equivalente (converter a posición nun número de fotograma á
taxa de entrada, e despois de novo a milisegundos á taxa de saída) non
o é: cuantifica cada posición na grade de entrada e erra ata
medio fotograma, 21 ms medidos. Véxase
[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

Na práctica, de 25 a 23,976, o factor é `1001/960`:

| Posición lida | Escrita | O que daría pasar polos fotogramas |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito. Con `--dry-run`, tampouco nada: non hai ficheiro, e a liña de narración dio — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificación, a marca e os finais de liña do ficheiro lido, repostos tal como estaban |
| 3 | `<path>: N bytes read, M written`, e despois **todos os diagnósticos de lectura** — véxase [Invocación](invocation.md#diagnósticos-de-lectura) |

**A narración nomea a taxa de fotogramas exacta, non a etiqueta tecleada.** Escribir
`23.976` nesa liña informaría dunha conversión que non tivo lugar; é
o único sitio onde se ve cal das dúas se usou. Unha taxa de fotogramas que un
decimal escribe exactamente móstrase alí como decimal, as demais como fracción.

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), o obxecto de cada ficheiro leva `counts.subtitles` (todos os subtítulos do ficheiro). A táboa completa é a de [Invocación](invocation.md#o-que-pon-cada-suborde-en-counts).

## Códigos de saída

Os da ferramenta: `0` se se escribiron todos os ficheiros, `2` se non se escribiu ningún, `3` se
só se escribiron algúns, `1` nun erro de uso.

Ningún rexeitamento depende do contido dun ficheiro: dúas taxas de fotogramas sempre definen
un factor. É a única das subordes que escriben na que todos os
rexeitamentos chegan antes da primeira lectura.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Erros

| O que o provoca | Mensaxe |
| :------------------ | :------ |
| taxa de fotogramas nula ou negativa | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| coma decimal | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| texto que non é un número | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| taxa de fotogramas desmesurada | `"…" is not a frame rate: no video runs at that many frames per second` |

**Non se escribe nada sen unha das tres opcións de destino**, e os seus
erros son comúns ás subordes que escriben:
véxase [Invocación](invocation.md#o-destino).
