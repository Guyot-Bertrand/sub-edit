# `pair`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Escribe **a tradución cos tempos recalculados sobre as posicións do ficheiro principal**, e di como atoparon as súas liñas
o seu subtítulo. É o `Open Translation…` da xanela — [o mesmo emparellamento](../subedit-gui/files.md#abrir-unha-tradución),
a mesma frase — seguido da escritura do resultado, que a xanela fai mediante `Save Translation`.

**É o único que fai o emparellamento e que non fai un ficheiro procesado só.** As outras subordes
que aceptan `-t` cambian a tradución que len — [`case`](case.md), [`replace`](replace.md)… —; `pair` non cambia
nada no texto: **pousa** a tradución sobre o ficheiro principal e escribe o que produciu ese pousado. O ficheiro principal
lese para recibir as liñas, **nunca se escribe**.

- **O que se escribe é a tradución**, na **súa propia ruta e no seu propio formato**: o formato, a codificación e
  os finais de liña do ficheiro de tradución. Os seus subtítulos están **nas posicións do ficheiro principal**: unha liña que
  caeu ao lado da marca recupera os límites do subtítulo que traduce.
- **`-t` é obrigatoria.** `pair` sen tradución non ten nada que pousar: é un erro de uso, código `1`.
- **Un só ficheiro principal**, e sen `--recursive`: un par son dous ficheiros, e unha árbore non ten
  forma de dicir que tradución vai con que ficheiro principal. Para un lote, unha invocación por par.
- **`--output` nomea o ficheiro da tradución escrita, `--output-dir` pon alí o seu nome base, `--in-place`
  reescríbeo**, como para calquera suborde que acepte `-t` — vexa [Unha tradución](invocation.md#unha-tradución).
- **`--document` non existe**: só hai un documento obxectivo, a tradución.

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

## Argumentos e opcións

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :----- | :------ | :----- |
| `<main>` | si | **unha soa** ruta | — |
| `-t`, `--translation-file` | **si** | o ficheiro de tradución | — |
| `--align-method` | non | `position` ou `number` — vexa [Unha tradución](invocation.md#unha-tradución) | `position` |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | vexa [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — vexa [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

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

A tradución ía 0,3 s atrasada: cada liña atopou o subtítulo co que se solapa o seu punto medio, e o
ficheiro escrito leva as posicións do ficheiro principal.

## O que di o aliñamento

A frase é **a mesma que di a xanela** cando abre unha tradución, cada parte só cando non é
cero — vexa [`inspect -t`](inspect.md#unha-tradución--t) para a táboa das súas partes. **Escríbese
desde o nivel 1**, e non no nivel 2 como para as outras subordes: aquí o aliñamento non é un
detalle do procesamento, é o resultado.

**O que lle fai o aliñamento ao ficheiro escrito**:

- **un subtítulo que ninguén traduciu escríbese como un bloque sen texto**, que se le sen ningunha anomalía;
- **un subtítulo nacido dunha liña** — unha liña á que non chegou nada — escríbese coas **súas propias posicións**, sexa cal
  sexa o método: polo tanto, o ficheiro escrito pode contar máis subtítulos ca o ficheiro principal;
- **o que o documento levase como tradución bórrase** antes do pousado, o que non se aplica na liña de ordes:
  o ficheiro principal lese só.

**Os dous métodos** dan o mesmo ficheiro cando as liñas caen cada unha no seu subtítulo, e non
cando falta unha liña ou a tradución está desprazada — por iso o método é unha elección, e por iso
a posición é o predeterminado. Os oito casos de `src/test/data/paires/` enuméranos, co ficheiro esperado para
cada método.

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito.

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **a ruta é a da tradución**, que é o ficheiro escrito; con `--dry-run`, `(dry run, nothing written)` en lugar de `-> <destination>` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificación, a marca e os finais de liña **da tradución**, repostos como estaban |
| 3 | `<translation>: N bytes read, M written`, despois **cada diagnóstico de lectura** do ficheiro principal e da tradución — vexa [Invocación](invocation.md#diagnósticos-de-lectura) |

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), **un só obxecto**, cuxo `file` é a tradución e
cuxo `destination` é o ficheiro escrito. `counts.subtitles` conta **os subtítulos do ficheiro escrito**, incluídos os que
fixo nacer unha liña. A clave `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
leva os mesmos recontos ca a frase, igual que a clave `translation` de [`inspect`](inspect.md#en-json).

## Códigos de saída

Os da ferramenta: `0` se a tradución se escribe — ou se calcula, con `--dry-run` —, `2` se un dos dous ficheiros non se puido
ler ou se o destino non se puido escribir, `1` nun erro de uso. **Unha tradución mal aliñada non é
un fallo**: escríbese, e a frase di o que non atopou o seu sitio.

## Erros

| O que os provoca | Mensaxe |
| :------------------- | :------ |
| sen `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — non existe para `pair` |
| máis dun ficheiro principal | `The following argument was not expected: <path>` |
| a tradución é o ficheiro principal | `<translation>: the file is already open as the main document`, código `2` |
| un dos ficheiros falta, non se pode ler ou ten un formato descoñecido | o mesmo que en todas partes, coa ruta do ficheiro culpable |
