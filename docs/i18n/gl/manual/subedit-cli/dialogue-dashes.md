# `dialogue-dashes`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Pon un **trazo de diálogo** ao comezo das liñas, ou quítao. É o `Dialogue Dashes` da
[xanela](../subedit-gui/operations.md).

**Dúas palabras, como para [`italics`](italics.md)**: `--add` e `--remove` exclúense mutuamente, e fai falta unha delas.

- **`--add`** pon un trazo diante de cada liña que non o ten: `Bonjour` dá `- Bonjour`, e
  `Bonjour\nMarie` dá `- Bonjour\n- Marie`. **Un trazo que xa está non se duplica.**
- **Recoñécense tres trazos** — o guión do teclado e os dous trazos longos da tipografía —
  **e o que se escribe é o guión**: `— Bonjour` pasa a ser `- Bonjour`. Os espazos sobrantes
  despois do trazo desaparecen.
- O trazo vai **diante do texto, non diante dunha etiqueta de apertura**: `<i>Bonjour</i>` dá
  `<i>- Bonjour</i>`. Ningunha etiqueta se move.
- **`--remove`** quita os trazos iniciais, nas tres formas.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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
| `--add` / `--remove` | **unha das dúas** | indicadores | — |
| `--range` | non | `N-M`, ou `N-` ata o final — véxase [`adjust`](adjust.md#--range) | o ficheiro enteiro |
| `--document` | non | `main` ou `translation` — véxase [Unha tradución](invocation.md#unha-tradución) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un ficheiro de tradución, para **unha soa** entrada | — |
| `--align-method` | non, e só con `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

**O texto principal de forma predeterminada; a tradución con `--document translation -t FILE`.** Entón
é a tradución a que se cambia e se escribe, e só ela — véxase [Unha tradución](invocation.md#unha-tradución).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito. Con `--dry-run`, a lista de
cambios, descrita en [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, ou `N subtitles undashed`, ou `nothing to change` |
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
| nin `--add` nin `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| as dúas | `--add and --remove say opposite things; give one of them` |
| franxa mal escrita ou fóra do ficheiro | véxase [`adjust`](adjust.md#--range) |

Os erros do destino son comúns ás subordes que escriben:
véxase [Invocación](invocation.md#o-destino).
