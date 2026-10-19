# `italics`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Pon os textos **en cursiva**, ou **quítalles a cursiva**, nas etiquetas do formato do ficheiro.
É o `Italic` da [xanela](../subedit-gui/operations.md).

**Dúas palabras e non un conmutador**: o botón da xanela pregúntalle ao núcleo en que sentido
actúa; aquí dino un mesmo. `--on` e `--off` exclúense mutuamente, e fai falta unha delas — omitila, ou dar as dúas,
é un erro de uso (código `1`).

- **`--on`** envolve cada texto nun só par de etiquetas, **despois de eliminar as que
  xa tiña**: un texto xa en cursiva non se envolve dúas veces, e unha cursiva cortada en dous
  refaise nunha soa peza. En varias liñas, un só par cobre o conxunto.
- **`--off`** elimina as etiquetas de cursiva; **o resto dunha etiqueta consérvase** (`{\b1\i1}` dá
  `{\b1}`).
- O texto escríbese **como o formato do ficheiro escribe a cursiva**: `<i>…</i>` para SubRip, WebVTT e
  SubViewer 2, `{\i1}…{\i0}` para os dous Sub Station Alpha, `{Y:i}` para MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
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

## Argumentos e opcións

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :----- | :------ | :----- |
| `<file>...` | si | unha ou máis rutas | — |
| `--recursive`, `-r` | non | un indicador | desactivado — véxase [Procesar unha árbore](lots.md) |
| `--on` / `--off` | **unha das dúas** | indicadores | — |
| `--range` | non | `N-M`, ou `N-` ata o final — véxase [`adjust`](adjust.md#--range) | o ficheiro enteiro |
| `--document` | non | `main` ou `translation` — véxase [Unha tradución](invocation.md#unha-tradución) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un ficheiro de tradución, para **unha soa** entrada | — |
| `--align-method` | non, e só con `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

**O texto principal de forma predeterminada; a tradución con `--document translation -t FILE`.** Entón
é a tradución a que se cambia e se escribe, e só ela — véxase [Unha tradución](invocation.md#unha-tradución).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Un formato sen estilo rexeita

**TMPlayer e LRC non escriben ningún estilo**: non hai cursiva que poñer nin cursiva que quitar. A
xanela atenúa a entrada en lugar de ocultala; a liña de ordes **rexeita, ficheiro por
ficheiro, co motivo**, e os outros ficheiros do lote continúan. O que conta é o que o
formato pode levar, non a forma en que o escribe: SubRip e WebVTT escriben a cursiva do mesmo
xeito, e non din o mesmo sobre a cor.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

En JSON, o fallo leva o identificador **`no-style`**.

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito. Con `--dry-run`, a lista de
cambios, descrita en [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, ou `N subtitles taken out of italics`, ou `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificación, a marca e os finais de liña do ficheiro lido, repostos como estaban |
| 3 | `<path>: N bytes read, M written`, despois **cada diagnóstico de lectura** — véxase [Invocación](invocation.md#diagnósticos-de-lectura) |

`N` é o número de subtítulos **cuxo texto cambiou**, lido da propia operación.

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), o obxecto de cada ficheiro leva
`counts.changed` — o `N` da frase — e `changes`, a lista dos subtítulos cambiados
(`{"subtitle", "document", "before", "after"}`, **baleira** cando non cambiou nada).

## Códigos de saída

Os da ferramenta: `0` se se procesaron todos os ficheiros — un ficheiro onde non cambia nada tamén conta —,
`2` se non se procesou ningún, `3` se só algúns, `1` nun erro de uso.

## Erros

| O que o provoca | Mensaxe |
| :------------------ | :------ |
| nin `--on` nin `--off` | `italics needs --on to put the texts in italics, or --off` |
| as dúas | `--on and --off say opposite things; give one of them` |
| un formato sen estilo | `<path>: TMPlayer writes no style: there are no italics to put on` (ou `take out`) |
| franxa mal escrita ou fóra do ficheiro | véxase [`adjust`](adjust.md#--range) |

Os erros do destino son comúns ás subordes que escriben:
véxase [Invocación](invocation.md#o-destino).
