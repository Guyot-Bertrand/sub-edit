# `case`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Pon os textos en **maiúsculas de título, maiúsculas de frase, maiúsculas ou minúsculas**, sen tocar ningunha
etiqueta. É o `Ferramentas ▸ Maiúsculas` da [xanela](../subedit-gui/operations.md), coas súas regras.

| `--to` | Efecto | `bonjour marie` pasa a ser |
| :----- | :---- | :---------------------- |
| `title` | cada palabra leva maiúscula inicial, o resto pasa a minúsculas | `Bonjour Marie` |
| `sentence` | a primeira letra do texto, e só ela; o resto pasa a minúsculas | `Bonjour marie` |
| `upper` | todo en maiúsculas | `BONJOUR MARIE` |
| `lower` | todo en minúsculas | `bonjour marie` |

- **O que vai antes da primeira letra ou da primeira cifra non se toca**: un trazo de
  diálogo, unhas comiñas de apertura ou unha paréntese sobreviven (`- bonjour` dá `- Bonjour`).
- **As etiquetas non son texto**: non cambian de maiúsculas a minúsculas — `<I>` segue sendo `<I>` — nin
  cambian de sitio: unha etiqueta que corta unha palabra séguea cortando despois.
- **ICU decide onde comeza unha palabra**: `l'été sera chaud` dá `L'été Sera Chaud`, e os acentos
  mantéñense (`éléonore` dá `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **si** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | non | `N-M`, ou `N-` ata o final — véxase [`adjust`](adjust.md#--range) | o ficheiro enteiro |
| `--document` | non | `main` ou `translation` — véxase [Unha tradución](invocation.md#unha-tradución) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un ficheiro de tradución, para **unha soa** entrada | — |
| `--align-method` | non, e só con `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

**O texto principal de forma predeterminada; a tradución con `--document translation -t FILE`.** Entón
é a tradución a que se cambia e se escribe, e só ela — véxase [Unha tradución](invocation.md#unha-tradución).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Un ficheiro onde **non cambia ningún texto** — todo está xa nas maiúsculas ou minúsculas pedidas — escríbese tal como está e
dio:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito. Con `--dry-run`, a lista de
cambios, descrita en [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, ou `nothing to change` |
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
| falta `--to` | `--to is required` |
| un valor que non é ningún dos catro | `--to: … not in {title,sentence,upper,lower}` |
| franxa mal escrita ou fóra do ficheiro | véxase [`adjust`](adjust.md#--range) |

Os erros do destino son comúns ás subordes que escriben:
véxase [Invocación](invocation.md#o-destino).
