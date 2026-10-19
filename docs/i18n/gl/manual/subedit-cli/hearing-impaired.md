# `hearing-impaired`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Elimina de todo o ficheiro as **mencións para persoas xordas**, os sons descritos entre corchetes. **As parénteses quedan por defecto**: úsanse tanto para unha réplica
murmurada como para un son descrito, e eliminalas borraría diálogo. A opción `--parentheses` elimínaas
tamén, para un ficheiro que describe nelas os seus sons.

O que non fai: as letras de cancións entre cancelos, o nome de quen fala
antes dos dous puntos, volver poñer maiúsculas, corrixir erros de OCR. Eses
padróns son os do asistente
[`Correct Texts…`](../subedit-gui/correct-texts.md) da xanela, que a liña de
ordes ofrece co nome [`correct`](correct.md); e aquí non se pode axustar nada: a
transformación está decidida, non é configurable.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opcións

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :----- | :------ | :----- |
| `<file>...` | si | unha ou máis rutas | — |
| `--parentheses` | non | un indicador: elimina tamén as mencións entre parénteses | desactivado |
| `--recursive`, `-r` | non | un indicador | desactivado — véxase [Procesar unha árbore](lots.md) |
| `--document` | non | `main` ou `translation` — véxase [Unha tradución](invocation.md#unha-tradución) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un ficheiro de tradución, para **unha soa** entrada | — |
| `--align-method` | non, e só con `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

O formato do ficheiro lido **consérvase**: cambiar o formato é tarefa de
[`convert`](convert.md).

**O texto principal por defecto; a tradución con `--document translation -t FILE`.** Entón
é a tradución a que se cambia e se escribe, e só ela — véxase [Unha tradución](invocation.md#unha-tradución).

**Unha mención que baleiraría unha tradución escríbea baleira e conserva o subtítulo**: o subtítulo
queda no ficheiro, sen texto. O texto principal, pola súa banda, perde un subtítulo baleirado por completo, como
se ve máis abaixo.

## O que se elimina e o que queda

Unha mención é un corchete (ou unha paréntese, con `--parentheses`) e o que encerra, **salto
de liña incluído**: nos ficheiros reais, unha mención córtaa a miúdo o
final dunha liña.

| Entrada | Saída |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

A eliminación deixa **exactamente un espazo entre o que a rodeaba, e nada no
bordo dunha liña**. Non é unha limpeza dos espazos do ficheiro: dous espazos
que o texto xa levaba seguen sendo dous espazos.

**Cando a mención está a cabalo do salto de liña, o que queda é o salto**: as
dúas liñas seguen sendo dúas liñas. Este caso non é unha curiosidade: en quince
subtitulacións reais, cada corchete que parecía orfo era unha
mención cortada dese xeito.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**O que non é unha mención e non se move:**

| Caso | O que pasa |
| :-- | :-------------- |
| contido puramente numérico — `[1]`, `(12)`, `[ 156478 ]` | consérvase tal cal, delimitadores incluídos: unha referencia nunca nomeou un son |
| unha paréntese e o que encerra, sen `--parentheses` | consérvase tal cal: `Venez (chut) ici` non cambia |
| un corchete ou unha paréntese que nada pecha | déixase tal cal |
| unha etiqueta de formato — `<i>`, `</i>` | nunca se elimina |

Un espazo en branco **entre** as cifras, unha letra ou un signo (`[1 2]`, `[1a]`,
`[-1]`) convérteo nunha mención. O mesmo pasa co contido baleiro, `[]` ou `( )`: sen
ningunha cifra, non é unha referencia; sen nada que dicir, non hai nada que conservar.

## O que desaparece por completo

**Unha liña que a eliminación baleirou desaparece**, e **un subtítulo baleirado
por completo elimínase do ficheiro**. Os subtítulos seguintes renuméranse.

Unha liña conta como baleira cando todo o que queda nela son espazos en branco, etiquetas
de formato ou un trazo de diálogo solitario: `<i>[Musique]</i>` non deixa un
`<i></i>` na pantalla.

**As etiquetas son as do formato do ficheiro.** `{\i1}[Musique]{\i0}` nun
Advanced SSA, `{Y:i}[Musique]` nun MicroDVD e `/[Musique]` nun MPL2 tampouco deixan
nada. Pola contra, un `<i>` nun formato que non escribe ningunha
etiqueta (TMPlayer, LRC) é texto que le o espectador, e a liña que
o leva non está baleira.

**Un diálogo reducido a unha soa voz xa non é un diálogo**, e o trazo que queda
desaparece. Mentres queden dúas voces, os trazos quedan.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Un ficheiro sen ningunha mención escríbese igualmente

Cópiase idéntico, o código de saída é `0`, e o informe di que non había
nada que eliminar.

É a regra das subordes que escriben: **un destino dado é
un destino escrito**. Facer aquí unha excepción obrigaría un script a saber
cal das subordes produce ás veces un ficheiro e ás veces nada.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito. **Con `--dry-run`, son os cambios propostos**: para cada subtítulo cuxo texto cambiaría, o seu número, o texto anterior e o texto posterior — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, ou `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificación, a marca e os finais de liña do ficheiro lido, repostos tal como estaban |
| 3 | `<path>: N bytes read, M written`, e despois **todos os diagnósticos de lectura** — véxase [Invocación](invocation.md#diagnósticos-de-lectura) |

`N` conta os subtítulos cuxo texto cambiou, e `M` os que a eliminación
baleirou por completo e que, polo tanto, saíron do ficheiro.

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), o obxecto de cada ficheiro leva `counts.cleaned` (os textos reescritos) e `counts.removed` (os subtítulos eliminados); **con `--dry-run`**, a clave `changes` lista ademais os subtítulos cambiados, como a saída estándar da forma de texto. A táboa completa é a de [Invocación](invocation.md#o-que-pon-cada-suborde-en-counts).

## Códigos de saída

Os da ferramenta: `0` se se escribiron todos os ficheiros, `2` se non se escribiu ningún, `3` se
só se escribiron algúns, `1` nun erro de uso.

## Erros

Esta suborde non ten ningún erro propio: non toma ningún
valor que analizar, e ningún texto pode facer que rexeite un ficheiro. O que
pode fallar é a lectura ou a escritura, como nas demais.

**Non se escribe nada sen unha das tres opcións de destino**, e os seus erros
son comúns ás subordes que escriben:
véxase [Invocación](invocation.md#o-destino).
