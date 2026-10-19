# `replace`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Substitúe un texto nos subtítulos **sen romper ningunha etiqueta**. É a
operación `Replace All` da [busca](../subedit-gui/find-and-replace.md) da xanela, coas súas regras:

- **a busca faise no texto visible**, o que le o espectador. As etiquetas non forman parte
  del: buscar `<i>` non atopa nada, e buscar `Bonjour` atopa un «Bonjour» que ten a metade
  en cursiva;
- **a substitución faise no texto fonte**, o do ficheiro, mantendo as etiquetas onde
  estaban. Unha etiqueta que corta a palabra buscada encerra toda a substitución; un estilo que
  toca a coincidencia cobre toda a substitución; o que a coincidencia non toca
  non se move. As regras, caso por caso, son as da busca da xanela.

**Non hai ningunha suborde `find`**: para ver onde está un padrón sen cambiar nada,
é [`--dry-run`](invocation.md#ver-antes-de-escribir) o que lista cada subtítulo afectado.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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
| `<pattern>` | si | o texto que buscar no que se mostra; **non baleiro** | — |
| `<replacement>` | si | o que o substitúe, etiquetas permitidas; pode estar baleiro | — |
| `<file>...` | si | unha ou máis rutas | — |
| `--recursive`, `-r` | non | un indicador | desactivado — véxase [Procesar unha árbore](lots.md) |
| `--regex` | non | un indicador: o padrón é unha expresión regular ICU | texto simple |
| `--case-sensitive` | non | un indicador: distingue maiúsculas de minúsculas | ignóranse |
| `--range` | non | `N-M`, ou `N-` ata o final — véxase [`adjust`](adjust.md#--range) | todo o ficheiro |
| `--document` | non | `main` ou `translation` — véxase [Unha tradución](invocation.md#unha-tradución) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un ficheiro de tradución, para **unha soa** entrada | — |
| `--align-method` | non, e só con `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

**Un padrón que comeza por un guión** escríbese despois de `--`: `subedit-cli replace -- -- "—" film.srt`.

**O texto principal por defecto; a tradución con `--document translation -t FILE`.** Entón
é a tradución a que se cambia e se escribe, e só ela — véxase [Unha tradución](invocation.md#unha-tradución).

### Texto simple ou expresión

Por defecto o padrón **lese tal como está**: `a.b` busca un `a`, un punto, un `b`. As maiúsculas
e minúsculas ignóranse coa mesma finura que na xanela: `É` e `é` son unha soa letra.

Con `--regex`, o padrón é unha expresión regular **ICU**, lida como a le Gaupol: `.` cruza
un salto de liña, `^` e `$` valen en cada liña dun subtítulo. Na substitución, e
só nela:

| Escribir | Para dicir |
| :----- | :-------- |
| `$1` a `$9` | os grupos da coincidencia |
| `$0` | toda a coincidencia |
| `\n` | un salto de liña |
| `\$`, `\\` | un `$`, unha barra invertida |

Coidado coas comiñas do shell: `'$1 EUR'`, e non `"$1 EUR"`, que deixaría que o shell lese `$1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Etiquetas

Un texto no que unha etiqueta corta a palabra buscada:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

No primeiro subtítulo, a etiqueta cortaba «Bonjour»: agora encerra «Salut».
No segundo, non se toca e queda onde está. **Unha etiqueta que quedou aberta
segue aberta**: reparar un marcado inestable sería outra operación.

A substitución lese nas **etiquetas do formato do ficheiro**: `<i>…</i>` para SubRip,
`{\i1}…` para Advanced SSA. É a única maneira de dar formato ao que se escribe.

## Nada atopado, nada que cambiar

Dúas respostas que non son fallos: o código de saída é `0` e **o ficheiro escríbese
igualmente**, porque un destino dado é un destino escrito:

| Situación | Frase |
| :-------- | :----- |
| o padrón non está en ningures | `"zzz" not found` |
| está, pero cada coincidencia substitúese por si mesma | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

A saída estándar lista **cada subtítulo cuxo texto cambiaría**, antes e despois, tal como
están no ficheiro, etiquetas incluídas:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

A forma da lista descríbese en [Ver antes de escribir](invocation.md#ver-antes-de-escribir);
en JSON, é `changes`.

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito. Con `--dry-run`, a lista de
cambios de máis arriba.

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, ou `"<pattern>" not found`, ou `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificación, a marca e os finais de liña do ficheiro lido, repostos tal como estaban |
| 3 | `<path>: N bytes read, M written`, e despois **todos os diagnósticos de lectura** — véxase [Invocación](invocation.md#diagnósticos-de-lectura) |

`N` conta **as coincidencias substituídas nos subtítulos cuxo texto cambiou**: é o
número que tamén di a xanela.

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), o obxecto de cada ficheiro leva:

| Clave | Contido |
| :-- | :------ |
| `counts.replaced` | as coincidencias substituídas, nos subtítulos cuxo texto cambiou: o `N` da frase |
| `counts.matched` | as coincidencias atopadas, **tanto se a súa substitución cambiou o texto como se non**; iso é o que distingue «nada atopado» (`0`) de «nada que cambiar» (`replaced` a `0`, `matched` positivo) |
| `changes` | os subtítulos cambiados: `{"subtitle", "document", "before", "after"}`, **baleiro** cando non cambiou nada |

## Códigos de saída

Os da ferramenta: `0` se se procesaron todos os ficheiros (un ficheiro no que non se atopou nada
procésase), `2` se non se procesou ningún, `3` se só algúns, `1` nun erro de uso.

## Erros

| O que o provoca | Mensaxe |
| :------------------ | :------ |
| padrón baleiro | `pattern: nothing to look for` |
| expresión ilexible | `pattern: not a regular expression (<ICU's reason>)` |
| franxa mal escrita ou fóra do ficheiro | véxase [`adjust`](adjust.md#--range) |

**O padrón lese antes do primeiro ficheiro**: unha expresión ilexible é un erro de uso
(código `1`), non se le nin se escribe nada.

Os erros do destino son comúns ás subordes que escriben:
véxase [Invocación](invocation.md#o-destino).
