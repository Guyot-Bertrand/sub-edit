# `pair`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Escribe **la traducción con los tiempos reajustados a las posiciones del archivo principal**, e indica cómo sus líneas
encontraron su subtítulo. Es el `Open Translation…` de la ventana — [el mismo emparejamiento](../subedit-gui/files.md#abrir-una-traducción),
la misma frase — seguido de la escritura del resultado, que la ventana hace con `Save Translation`.

**Es lo único que hace el emparejamiento y que no hace un archivo procesado solo.** Los demás subcomandos
que aceptan `-t` modifican la traducción que leen — [`case`](case.md), [`replace`](replace.md)… —; `pair` no modifica
nada del texto: **superpone** la traducción al archivo principal y escribe lo que la superposición ha producido. El archivo principal se
lee para recibir las líneas y **nunca se escribe**.

- **Lo que se escribe es la traducción**, en **su propia ruta y en su propio formato**: el formato, la codificación y
  los finales de línea del archivo de traducción. Sus subtítulos están **en las posiciones del archivo principal**: una línea que
  había caído al lado de la marca recupera los límites del subtítulo que traduce.
- **`-t` es obligatoria.** `pair` sin traducción no tiene nada que superponer: es un error de uso, código `1`.
- **Un solo archivo principal**, y nada de `--recursive`: un par son dos archivos, y un árbol no tiene
  forma de decir qué traducción va con qué archivo principal. Para un lote, una invocación por par.
- **`--output` nombra el archivo de la traducción escrita, `--output-dir` deposita allí su nombre base, `--in-place`
  la reescribe**, como en cualquier subcomando que acepta `-t` — véase [Una traducción](invocation.md#una-traducción).
- **`--document` no existe**: solo hay un documento al que apuntar, la traducción.

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos y opciones

| Opción | Obligatoria | Valores | Por defecto |
| :----- | :----- | :------ | :----- |
| `<main>` | sí | **una sola** ruta | — |
| `-t`, `--translation-file` | **sí** | el archivo de traducción | — |
| `--align-method` | no | `position` o `number` — véase [Una traducción](invocation.md#una-traducción) | `position` |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

La traducción iba 0,3 s retrasada: cada línea ha encontrado el subtítulo con el que se solapa su punto medio, y el
archivo escrito lleva las posiciones del archivo principal.

## Lo que dice la alineación

La frase es **la que dice la ventana** cuando abre una traducción, cada parte solo cuando no es
cero — véase [`inspect -t`](inspect.md#una-traducción--t) para la tabla de sus partes. **Se escribe
desde el nivel 1**, y no en el nivel 2 como en los demás subcomandos: aquí la alineación no es un
detalle del procesamiento, es el resultado.

**Lo que la alineación hace al archivo escrito**:

- **un subtítulo que nadie ha traducido se escribe como un bloque sin texto**, que se lee sin ninguna anomalía;
- **un subtítulo nacido de una línea** — una línea a la que no ha llegado nada — se escribe con **sus propias posiciones**, sea cual sea
  el método: el archivo escrito puede, por tanto, contar más subtítulos que el archivo principal;
- **lo que el documento llevaba como traducción se vacía** antes de la superposición, lo que no se aplica en la línea de comandos:
  el archivo principal se lee solo.

**Los dos métodos** dan el mismo archivo cuando cada línea cae sobre su subtítulo, y no
cuando falta una línea o la traducción está desplazada — por eso el método es una elección, y por eso
la posición es el valor por defecto. Los ocho casos de `src/test/data/paires/` los enumeran, con el archivo esperado para
cada método.

## Salida

**Salida estándar** — nada: el resultado es el archivo escrito.

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **la ruta es la de la traducción**, que es el archivo escrito; con `--dry-run`, `(dry run, nothing written)` en lugar de `-> <destination>` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — el formato, la codificación, la marca y los finales de línea **de la traducción**, restituidos tal como estaban |
| 3 | `<translation>: N bytes read, M written`, y después **cada diagnóstico de lectura** del archivo principal y de la traducción — véase [Invocación](invocation.md#diagnósticos-de-lectura) |

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), **un solo objeto**, cuyo `file` es la traducción y
`destination` el archivo escrito. `counts.subtitles` cuenta **los subtítulos del archivo escrito**, incluidos los que una línea
ha hecho nacer. La clave `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
lleva los mismos recuentos que la frase, como la clave `translation` de [`inspect`](inspect.md#en-json).

## Códigos de salida

Los de la herramienta: `0` si la traducción se escribe — o se calcula, con `--dry-run` —, `2` si uno de los dos archivos no ha podido
leerse o si el destino no ha podido escribirse, `1` ante un error de uso. **Una traducción mal alineada no es
un fallo**: se escribe, y la frase dice lo que no ha encontrado su sitio.

## Errores

| Qué los provoca | Mensaje |
| :------------------- | :------ |
| sin `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — no existe para `pair` |
| más de un archivo principal | `The following argument was not expected: <path>` |
| la traducción es el archivo principal | `<translation>: the file is already open as the main document`, código `2` |
| uno de los archivos ausente, ilegible o de formato desconocido | lo mismo que en todas partes, con la ruta del archivo en cuestión |
