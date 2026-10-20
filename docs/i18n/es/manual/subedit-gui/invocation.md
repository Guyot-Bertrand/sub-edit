# Invocación

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-gui [--version] [<file>]
```

| Argumento | Obligatorio | Valor | Por defecto |
| :------- | :------- | :---- | :------ |
| `<file>` | no | la ruta de un archivo de subtítulos que abrir | ninguno — la ventana se abre vacía |
| `--version` | no | — | — |

**Un solo archivo.** Los argumentos siguientes se ignoran: la ventana contiene
un solo proyecto a la vez.

## `--version`

Escribe en la salida estándar el bloque que escribe el `--version` de la línea de comandos, y
termina enseguida, sin abrir ninguna ventana. Código de salida `0`.

**`subedit <version>`, solo, en la primera línea**, después una línea por cada biblioteca con la que
funciona la ventana, con la versión encontrada en tiempo de ejecución y, entre paréntesis, aquella
con la que se compiló; después los idiomas de la interfaz instalados, por código. En
una máquina con Qt 6.4.2:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

Las cifras no se copian aquí: son las de la máquina, y este manual no tiene ningún
bloque generado que mantuviera al día las del propio programa. Las bibliotecas son
las de la línea de comandos, ICU y Enchant, y las dos que solo enlaza la ventana — Qt, y
libmpv, el reproductor, indicada por la versión de su API cliente, la que anuncia
`mpv.pc`. El resto se describe en
[el manual de la línea de comandos](../subedit-cli/invocation.md#--version). Las palabras
`built with` y `Languages` siguen el idioma de la ventana; nada más lo hace.

`Ayuda ▸ Acerca de subedit` muestra las mismas bibliotecas, de la misma lista, en una tabla —
el nombre, la versión cargada, la versión con la que se compiló, una fila cada una — y después los
idiomas debajo; sus encabezados siguen el idioma de la ventana.

## Abrir un archivo

```console
$ subedit-gui film.srt
```

Los formatos leídos son **los nueve** que enumera el filtro de apertura — véase
[Abrir y guardar](files.md#abrir). El formato se reconoce por el contenido y
no por la extensión, y **la codificación también**: un BOM
si el archivo lleva uno; si no, la codificación que sugieren sus bytes.

## Cuando la apertura falla

Un archivo que no existe, uno ilegible, uno cuyos bytes no se decodifican en ninguna
codificación, o uno cuyo contenido no reconoce ningún formato: **la ventana se abre
de todos modos, vacía**, y el fallo se escribe en el flujo de error.

**El mensaje nombra la causa**, con las mismas palabras que la línea de comandos:

| Mensaje | Qué ha ocurrido |
| :------ | :------------ |
| `does not exist` | la ruta no designa ningún archivo |
| `cannot be opened: permission denied` | el sistema se niega a abrirlo |
| `cannot be read` | el sistema se ha negado por otro motivo |
| `cannot be decoded in the chosen encoding` | los bytes no se leen en la codificación elegida |
| `is in no format this tool knows` | ningún formato reconoce el contenido |
| `holds nothing recognisable as a subtitle` | el formato se reconoce, pero nada de lo que contiene es un subtítulo |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Abrirse vacía en lugar de negarse a arrancar: la herramienta sigue sirviendo para
algo, y el fallo de un argumento no es motivo para no lanzarla.

**Un archivo mal formado pero legible se abre**, y lo que la lectura haya detectado
por el camino puede leerse en un botón de la barra de estado — véase
[los diagnósticos de una lectura](files.md#los-diagnósticos-de-una-lectura).

## Códigos de salida

| Código | Cuándo |
| :--- | :--- |
| `0` | la ventana se abrió y después se cerró normalmente, o `--version` respondió |
| `2` | un error inesperado antes de la apertura — falta Qt, pantalla no disponible |

El fallo al abrir un archivo **no cambia el código de salida**: la
ventana se abrió, y eso es lo que indica el código.

## La página de manual

Una instalación coloca `subedit-gui(1)` en `share/man/man1`:

```console
$ man subedit-gui
```

Da la forma de llamada, `--version`, los mensajes de fallo de apertura, los
códigos de salida, las dos variables de entorno y el archivo de configuración.
**No describe la ventana** — eso lo hace este manual, y la página
remite aquí.

## La plataforma Qt

`QT_QPA_PLATFORM` elige sobre qué funciona la ventana. **Si la variable está
definida, `subedit-gui` la respeta sin discusión.**

Si no lo está, deja que Qt elija: el reproductor integrado dibuja la imagen en
la propia ventana, y no pide nada a la plataforma — véase
[El reproductor](player.md).

`QT_QPA_PLATFORM=offscreen` hace funcionar la ventana donde no hay ningún
servidor gráfico — que es lo que hacen la integración continua y las pruebas.

## Idioma

La ventana habla el idioma de su entorno, **elegido una sola vez, al arrancar**: cambiar las
variables de una ventana en marcha no cambia nada hasta el siguiente inicio. Las variables y las reglas
son las de la línea de comandos — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` para
el inglés, el inglés **sin decir nada** cuando el idioma pedido no está instalado — descritas en
[el manual de la línea de comandos](../subedit-cli/invocation.md#idioma). Los catálogos son los mismos
archivos, instalados en `<prefix>/share/subedit/locale`.

**Un idioma elegido en `Editar ▸ Preferencias…` tiene prioridad**, y el entorno solo se lee
cuando es `System default`, como lo es mientras no se elija ninguno: un idioma instalado elegido allí
se impone a cualquier variable, `LC_ALL=C` incluida. La elección también se aplica en el siguiente inicio. Véase
[El idioma de la interfaz](preferences.md#el-idioma-de-la-interfaz).

| Qué | Sigue el idioma |
| :--- | :------------------- |
| los menús y sus entradas, con sus letras subrayadas; la barra de herramientas; las descripciones emergentes | sí |
| las columnas de la tabla y de `Ver ▸ Columnas`, las pestañas, la línea de estado, la barra de reproducción | sí |
| los botones estándar que dibuja el propio Qt (`OK`, `Cancel`) | sí, cuando está instalado el propio catálogo de Qt de ese idioma (`qtbase_<language>.qm`); en inglés en caso contrario |
| los cuadros de diálogo, las páginas de `Correct Texts…`, los títulos y filtros de los diálogos de archivo, los cuadros de mensaje y lo que dicen | sí |
| las líneas que la ventana escribe en su flujo de error, `subedit-gui: …` | sí, salvo el nombre del programa |
| los nombres de los patrones de corrección en `Correct Texts…`, y sus descripciones en las descripciones emergentes | sí, para los patrones que se distribuyen con el programa; un patrón propio se muestra tal como lo escribe su archivo |
| los nombres de codificaciones, finales de línea e idiomas de ortografía (`UTF-8`, `CRLF`, `en_US`) | nunca: son nombres, no palabras |
| los atajos de teclado | nunca: `Ctrl+S` es `Ctrl+S` en todos los idiomas |
| un mensaje que el catálogo no traduce | se muestra en inglés |

Esta versión instala `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT` y `nl` — catalán, checo, alemán, español,
finés, francés, gallego, hebreo, húngaro, interlingue, islandés, italiano y neerlandés — traducidos por completo. El francés lo ha revisado un lector del
idioma; el catalán, el checo, el alemán, el español, el finés, el gallego, el hebreo, el húngaro, el interlingue, el islandés, el italiano y el neerlandés son traducciones automáticas que todavía
no ha revisado ningún lector del idioma.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
En un escritorio en francés, la barra de menús dice
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. Las capturas de pantalla de
este manual se toman en español.
