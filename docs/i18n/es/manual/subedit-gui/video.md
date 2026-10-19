# El vídeo asociado

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

El menú **Vídeo** lleva primero dos órdenes, y luego los gestos que controlan la película
— véase [El reproductor](player.md#desplazarse-por-la-película).

| Comando | Atajo | Qué hace |
| :------ | :------- | :----------- |
| Seleccionar vídeo… | ninguno | elige el archivo de vídeo que acompaña al documento |
| Reproducir / Pausa | `Ctrl+P` | reproduce o detiene el vídeo — véase [El reproductor](player.md#reproducir-y-detener) |

El vídeo asociado es **una propiedad del documento abierto**, no un ajuste de
la aplicación: abrir otro archivo de subtítulos vuelve a empezar de cero. No
se escribe en ningún archivo y desaparece con la ventana.

## La pista de audio

Una película puede llevar varias pistas de sonido — la versión original, un doblaje, un comentario. El
submenú **`Vídeo ▸ Audio ▸ Idioma`** las enumera, **una entrada por pista**, y **marca la que suena**:

| Lo que da el archivo | Lo que dice la entrada |
| :------------------ | :------------------ |
| un idioma y un título | `1: fra — Original` |
| solo un idioma | `2: eng` |
| solo un título | `3: Commentary` |
| ninguno de los dos | `1` |

El número es **el lugar de la pista en la película**, de uno en uno, en el orden del archivo: dos pistas
pueden decir `eng`, y un archivo que calla sus etiquetas sigue teniendo una primera y una segunda.

**Elegir una entrada reproduce esa pista.** La marca es la respuesta del reproductor, no el clic: indica la
pista que está sonando.

- **El submenú está desactivado** sin vídeo o sin pista de audio. Con una sola pista, sigue activado y
  la muestra, marcada: indica el idioma de la película. Los dos gestos de volumen, que están a su lado, siguen
  activados en todos los casos: véase [El volumen](player.md#el-volumen).
- **La pista no se recuerda** de un vídeo al siguiente, ni de una sesión a la siguiente: un número de pista es
  el de su archivo, y trasladarlo a la película siguiente elegiría lo que allí lleve ese número.
  Volver a una pestaña reabre su película en la pista que el reproductor elige por sí mismo.
- Sin atajo: se cambia de idioma pocas veces, y el menú basta.

## Elegir un vídeo

El cuadro de diálogo se abre en el directorio del archivo de subtítulos y filtra por
las extensiones de vídeo reconocidas:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

También ofrece mostrar todos los archivos. Cancelar el cuadro de diálogo no cambia
nada.

El archivo elegido **se abre enseguida**: la imagen aparece en la ventana,
en pausa en su primer instante. Véase [El reproductor](player.md).

**Soltar un archivo de vídeo en la ventana hace lo mismo** — véase
[Soltar archivos en la ventana](files.md#soltar-archivos-en-la-ventana).

**El vídeo es el del proyecto de la pestaña actual.** Cada pestaña conserva
el suyo; cambiar de pestaña cambia de película. **Volver a una pestaña reanuda su
película donde se dejó**, en pausa, como cualquier apertura. Otra película
elegida para ese proyecto entretanto empieza, en cambio, desde el principio.

## La propuesta automática

Cuando se abre un archivo de subtítulos, **si no se ha elegido ningún vídeo**,
la ventana busca uno en el mismo directorio: un archivo de vídeo cuyo nombre
sin extensión sea un prefijo del nuestro, hasta un punto.

| Archivo de subtítulos | Vídeo encontrado | Por qué |
| :------------ | :---------- | :-- |
| `film.fr.srt` | `film.mkv` | `film` es un prefijo de `film.fr` |
| `film.en.forced.srt` | `film.mkv` | igualmente, sea cual sea el número de segmentos |
| `film.fr.srt` | `film.fr.mkv` en lugar de `film.mkv` | gana el nombre más largo |
| `film.fr.srt` | ninguno, si están `film.mkv` **y** `film.mp4` | dos candidatos no pueden distinguirse |
| `film.fr.srt` | ninguno, si solo está `fil.mkv` | `fil` no se detiene en un punto |

**Una propuesta no es una elección.** Se deja reemplazar por otra
propuesta — abrir otro archivo, guardar con otro nombre. Una elección
explícita, en cambio, nunca se sobrescribe: una vez usado `Select Video…`, la
propuesta automática ya no vuelve detrás de ella.

Es lo que hace aceptable la suposición: puede corregirse, y su
corrección se mantiene.

## Lo que muestra la barra de estado

A la derecha de la barra de estado, una línea dice qué acompaña al documento.
Es la última de tres: la [codificación](files.md#la-codificación-en-la-barra-de-estado)
del archivo leído va primero, y la [rejilla de fotogramas](frame-grid.md) deducida de las
posiciones a continuación.

| Situación | Lo que se escribe |
| :-------- | :-------------- |
| hay un vídeo asociado | `Video: film.mkv` |
| … y `ffprobe` da su velocidad de fotogramas | `Video: film.mkv, 24000/1001 fps` |
| ninguno | `No video` |

**La velocidad de fotogramas va con la película y no a su lado.** Son un solo hecho — lo que
acompaña a este documento —, y una tercera mención la pondría al mismo nivel que
la [rejilla deducida de las posiciones](frame-grid.md), que es otro.

## Cuando no hay película

La ventana muestra un botón **`Select Video…`** donde iría la imagen, en lugar de
nada en absoluto. Es la misma orden que la del menú `Video`, al alcance
del ratón: una ausencia sobre la que no se puede actuar es difícil de distinguir de una
avería.

El botón desaparece en cuanto se abre una película, y la imagen ocupa **todo** su
lugar: la altura que la tabla deja encima — la que
[`window.table-share`](preferences.md) recuerda de una sesión a la siguiente —, y
nunca menos que la altura mínima de la imagen, que la tabla cede si hace falta.
Cuando la película se va, el botón recupera el mismo lugar.

**El nombre del archivo, nunca su ruta.** Una ruta de doscientos caracteres
expulsaría de la barra todo lo demás; la ruta completa es la que mostró
el cuadro de diálogo.

## `ffmpeg` no es obligatorio

`subedit-gui` consulta a `ffprobe` — que viene con `ffmpeg` — para una sola cosa y
nada más: **la velocidad de fotogramas que declara el contenedor**. Se lee una vez
por película, cuando se asocia.

| Con `ffprobe` | Sin él |
| :------------- | :------ |
| la barra de estado escribe la velocidad de fotogramas junto al nombre de la película | solo escribe el nombre |
| el cuadro de conversión propone la velocidad de fotogramas de la película en el campo inferior | no propone nada, y se abre como antes |
| el cuadro de alineación se abre en ella | se abre en la velocidad de fotogramas del proyecto |

**Nada más depende de él, y ninguna operación se niega.** El reproductor, por su parte,
no necesita `ffprobe` en absoluto: se apoya en `libmpv`, que es una
dependencia de compilación. La duración de la película, la que sirve para señalar lo que
pasa del final, viene del reproductor y no de `ffprobe`.

No es, por tanto, un modo degradado, sino **el funcionamiento normal** de una máquina
que nunca ha instalado `ffmpeg`: el dato lo aporta el usuario, y `ffprobe`
lo propone cuando está.

Una velocidad de fotogramas leída es un **racional exacto** — `24000/1001`, no `23.976`.
Es la razón por la que se conserva `ffprobe` para esa respuesta: el
reproductor sabe lo mismo como número de coma flotante, y una velocidad de fotogramas aproximada
reajustaría los tiempos de todo el archivo con un ligero desvío.
