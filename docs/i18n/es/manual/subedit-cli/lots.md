# Procesar un árbol

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Todo subcomando que lee un lote de archivos — `inspect` incluido, y no
[`append`](append.md), [`split-file`](split-file.md) ni [`pair`](pair.md), cuya aridad es la de un par o
la de una película — acepta un **directorio** en lugar de un archivo, con `--recursive` (`-r`).
Esta página dice qué toma el recorrido, en qué orden y dónde escribe.

**Nada de lo que sigue cambia lo que se hace con un archivo**: cada uno se lee,
se procesa y se escribe de forma independiente, el fallo de uno no detiene a los demás, y los
cuatro códigos de salida son los de [la invocación](invocation.md#códigos-de-salida).
El recorrido produce una lista de rutas, que el resto del lote consume como si se
hubieran tecleado.

## Sin `--recursive`, un directorio se rechaza

Error de uso, código `1`, ningún archivo procesado ni escrito:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Lo que toma el recorrido

El recorrido es **recursivo, determinista y sin sorpresas**:

- **el orden** — las entradas de cada directorio se toman en el orden de sus
  nombres, **byte a byte** (mayúsculas antes que minúsculas, sea cual sea el
  idioma de la máquina), y un subdirectorio se recorre en su lugar, en
  profundidad. Dos ejecuciones dan los mismos bytes, en todas las
  máquinas;
- **los enlaces simbólicos no se siguen** — ni hacia un archivo ni hacia un
  directorio: un árbol que contuviera un enlace a uno de sus padres no tendría
  fin;
- **los archivos y directorios ocultos se ignoran** — aquellos cuyo nombre empieza por
  un punto;
- **solo se conservan las extensiones de los formatos conocidos**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, sin distinguir mayúsculas y minúsculas. **`.txt` queda excluida**:
  designa dos formatos, pero sobre todo todos los `LISEZMOI.txt` del mundo, y un lote que
  fallara en cada uno sería inutilizable.

**Un archivo nombrado en la línea de comandos nunca se filtra.** Nombrarlo es
quererlo: `subedit-cli shift --by 1 --in-place notes.txt` lo procesa, sea cual sea
su extensión, y solo se queja si su contenido no está en ningún formato.

**Lo que se ha dejado de lado se cuenta**, y se indica en el nivel `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Un directorio que no contiene nada que recorrer lo dice en el nivel por defecto
(`<directory>: holds no file in a format this tool walks for`) y no es un error.

## Dónde escribe el recorrido

**El árbol se conserva bajo `--output-dir`**: la ruta de un archivo,
**relativa al directorio dado**, se reproduce bajo el directorio de salida. El nombre
del directorio dado no forma parte de ella — es la semántica de `rsync` con una
barra final, no la de `cp -r`: se nombra el destino y se le añade el
nivel que se quiera (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

El directorio de salida y los del árbol se **crean** si faltan. Con
`--in-place`, cada archivo conservado se vuelve a escribir en su lugar. Una extensión que
`convert` cambia cambia también en el árbol: `films/a/x.srt` convertido
a WebVTT se escribe `sortie/a/x.vtt`.

**El directorio de salida, si está dentro del árbol recorrido, se excluye del recorrido**:
una segunda ejecución no vuelve a leer lo que escribió la primera.

## Dos directorios, y las colisiones

La regla del [destino](invocation.md#el-destino) lo resuelve: dos archivos
que acabarían en la misma ruta se rechazan **antes de que se escriba nada**, código `1`, con las
dos entradas y el destino nombrados.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Lo que cuesta

El lote sigue siendo secuencial: su duración es la suma de las de sus archivos. El
banco de pruebas `make bench` mide el recorrido y el mismo trabajo en un único archivo; las cifras
están en el [registro de rendimiento](../../../../mesures/performances.md).
