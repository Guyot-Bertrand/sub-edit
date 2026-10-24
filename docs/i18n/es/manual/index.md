# subedit — manual

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

subedit lee, inspecciona y reajusta los tiempos de archivos de subtítulos. Se presenta como dos
programas que comparten el mismo núcleo y, por tanto, las mismas reglas y las mismas palabras.

| Programa | Para qué | Su manual |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | abrir un archivo, verlo en una tabla, editarlo, mirarlo junto con la película | [el manual de la ventana](subedit-gui/index.md) |
| `subedit-cli` | procesar archivos por lotes, desde un script, sin interfaz | [el manual de la línea de comandos](subedit-cli/index.md) |

Ambos leen y escriben **los nueve formatos** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — y
reconocen el formato por su contenido y no por su extensión. **También la codificación
se reconoce por el contenido**: un archivo en Latin-1, en CP1252 o en UTF-16
se abre, se indica cómo se ha leído y se vuelve a escribir en su propia codificación — un
archivo abierto y luego guardado sin cambios no se mueve ni un byte.

## Por dónde empezar

| Lo que se quiere | Adónde ir |
| :----------------- | :------- |
| instalar la herramienta | [Instalación](subedit-cli/installation.md) — vale para los dos programas |
| abrir un archivo y ver lo que contiene | [La tabla](subedit-gui/table.md) |
| reajustar los tiempos de subtítulos desplazados una cantidad constante | [Las operaciones](subedit-gui/operations.md#shift-positions), o [`shift`](subedit-cli/shift.md) |
| reajustar los tiempos de un archivo escrito para otra velocidad de fotogramas | [`framerate`](subedit-cli/framerate.md), y [`snap`](subedit-cli/snap.md) si la temporización ya es correcta |
| saber para qué velocidad de fotogramas se escribió un archivo | [La rejilla de fotogramas](subedit-gui/frame-grid.md), o [`inspect`](subedit-cli/inspect.md) |
| eliminar las anotaciones para sordos | [Las operaciones](subedit-gui/operations.md#remove-hearing-impaired-mentions), o [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| corregir anotaciones, errores comunes, capitalización y saltos de línea en una sola pasada | [`Correct Texts…`](subedit-gui/correct-texts.md), o [`correct`](subedit-cli/correct.md) |
| comprobar la ortografía palabra por palabra, con los diccionarios del sistema | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| alargar subtítulos demasiado cortos para poder leerse | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations), o [`adjust`](subedit-cli/adjust.md) |
| volver a poner en orden un archivo cuyos subtítulos no lo están | [`sort`](subedit-cli/sort.md) |
| hacer un solo subtítulo con los trozos de un mismo texto que se suceden | [`Join Repeated Subtitles`](subedit-gui/lines.md#unir-los-subtítulos-repetidos), o [`join-repeats`](subedit-cli/join-repeats.md) |
| enderezar de una vez un archivo mal segmentado, desordenado y con solapamientos | [`repair`](subedit-cli/repair.md) |
| eliminar los solapamientos entre subtítulos | [`Make Ranges Disjoint…`](subedit-gui/lines.md#hacer-disjuntos-los-intervalos), o [`disjoint`](subedit-cli/disjoint.md) |
| poner una tras otra las dos partes de una película | [`Append File…`](subedit-gui/operations.md#append-file), o [`append`](subedit-cli/append.md) |
| cortar una película en dos partes | [`Split Project…`](subedit-gui/operations.md#split-project), o [`split-file`](subedit-cli/split-file.md) |
| cambiar la capitalización, la cursiva o las rayas de diálogo de todo un archivo | [Las operaciones](subedit-gui/operations.md#italic), o [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| buscar una palabra y reemplazarla, sin romper las etiquetas | [Buscar y reemplazar](subedit-gui/find-and-replace.md), o [`replace`](subedit-cli/replace.md) |
| escribir el habla de una película como subtítulos | [`Transcribe Audio…`](subedit-gui/video.md#transcribir-el-audio), o [`transcribe`](subedit-cli/transcribe.md) — tras [instalar las herramientas](subedit-cli/install-transcription.md) |
| traducir junto al texto original | [Abrir una traducción](subedit-gui/files.md#abrir-una-traducción), o [`pair`](subedit-cli/pair.md) para reajustar sus tiempos sobre el archivo principal |
| procesar cien archivos de una vez | [Procesar un árbol](subedit-cli/lots.md), y después [Invocación](subedit-cli/invocation.md) |

**Entre `framerate` y `snap`, un error pasa desapercibido.** En caso de
duda, la página [`snap`](subedit-cli/snap.md#snap-no-es-framerate-y-confundirlos-no-se-nota)
empieza con la comparación de los dos.

## Lo que promete este manual

**Describe lo que existe, nunca lo que está previsto.** Lo que vendrá, y en
qué orden, está en la [hoja de ruta](../../../feuille-de-route.md).

**Este manual está escrito en inglés, y traducido.** Sus traducciones lo siguen
página por página, en el directorio `docs/i18n/` del repositorio, un árbol por idioma. Los
programas hablan el idioma de su entorno; los nombres que muestran — subcomandos,
opciones, etiquetas de menú — se citan aquí tal como el programa los escribe en inglés.

**Los ejemplos y las imágenes se generan, el texto no.** Los
bloques `console` se producen ejecutando realmente el comando; las
capturas de pantalla, construyendo la ventana real y fotografiándola. Ninguno
de los dos puede, por tanto, describir una herramienta que ya no existe. Lo que queda sin red
es el texto que los rodea, cuya exactitud depende de la relectura.

**Las capturas de pantalla muestran las dos paletas**, `Light` y `Dark` — las que
ofrecen [las preferencias](subedit-gui/preferences.md#el-tema).

## Las dos superficies dicen las mismas palabras

Una razón para leer cualquiera de los dos manuales cuando se usa el otro programa: los
mensajes de error, los nombres de los formatos, los veredictos sobre la rejilla y las etiquetas
de deshacer se escriben **una sola vez**, en el núcleo, y los dos programas
los citan. Un archivo rechazado da la misma frase en ambos lados.

Lo que difiere es lo que puede hacer cada superficie:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| varios archivos a la vez | sí — una pestaña por proyecto | sí |
| una traducción junto al archivo principal | sí | sí — `-t` en los subcomandos de texto, y [`pair`](subedit-cli/pair.md) para reajustar sus tiempos |
| añadir un archivo, dividir un proyecto | sí | sí — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| deshacer | sí, mil entradas | no aplicable — nada se cambia sobre la propia entrada sin pedirlo |
| editar a mano un texto o una posición | sí | no |
| insertar, eliminar, fusionar y dividir líneas | sí | no |
| buscar y reemplazar | sí | sí — [`replace`](subedit-cli/replace.md) |
| cortar, copiar y pegar textos | sí | no |
| ajustar las duraciones, la cursiva, la capitalización, las rayas | sí | sí — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| corregir los textos mediante patrones | sí — `Correct Texts…` | sí — [`correct`](subedit-cli/correct.md) |
| comprobar la ortografía palabra por palabra | sí — `Check Spelling…` | no |
| mirar la película mientras se temporiza | sí | no |
| escribir sobre la entrada | `Save` | `--in-place` |
