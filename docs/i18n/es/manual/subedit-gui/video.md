# El vídeo asociado

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

El menú **Vídeo** lleva primero tres órdenes, y luego los gestos que controlan la película
— véase [El reproductor](player.md#desplazarse-por-la-película).

| Comando | Atajo | Qué hace |
| :------ | :------- | :----------- |
| Seleccionar vídeo… | ninguno | elige el archivo de vídeo que acompaña al documento |
| Transcribir audio… | ninguno | transcribe el audio de la película en subtítulos que sustituyen a los del proyecto — véase [Transcribir el audio](#transcribir-el-audio) |
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

## Transcribir el audio

**`Transcribe Audio…`** escribe el habla de la película como subtítulos. Está desactivada hasta que se abre una película: es
el audio de **la película del proyecto**, y nunca un archivo que se elija — para transcribir un
archivo de audio, use [`transcribe`](../subedit-cli/transcribe.md) en la línea de comandos.

El cuadro de diálogo pide tres cosas, y dice si puede empezar:

- **la pista de audio**, enumerada como la enumera el menú [`Audio`](#la-pista-de-audio), con el idioma
  y el título que declara el archivo;
- **el idioma**, dos letras como `en` o `fr` (`fra` se lee como `fr`). Empieza con el idioma de la pista elegida
  cuando el archivo declara uno, sigue la pista mientras no haya escrito ninguno, y
  **una línea bajo el campo dice de dónde viene**. Cuando el archivo no declara ningún idioma para la pista,
  la línea lo dice, y advierte: el modelo lo detectará entonces a partir de los primeros treinta segundos de sonido, algo que solo hace un modelo
  multilingüe, y **una película que empieza con música o silencio puede tomarse por otro idioma y su habla traducirse
  a él**. Escríbalo si lo conoce. Con un modelo solo para inglés, el campo queda fijo en `en` y no
  se puede cambiar, y la línea dice que la película saldrá en inglés sea cual sea el idioma que hable;
- **la parte de la película**, de un tiempo a otro — véase [Probar primero una parte](#probar-primero-una-parte). Con los dos
  campos vacíos, es toda la película.

**También dice lo que falta.** Tres líneas dicen si el motor, `ffmpeg` y el modelo están, y
dónde; cuando uno no está, `OK` queda apagado, y el cuadro lo dice con la referencia a
[Instalar las herramientas de transcripción](../subedit-cli/install-transcription.md). Un botón `Preferences…` abre
las [preferencias](preferences.md#transcripción), donde se definen el programa y el modelo — son hechos
sobre la máquina, no elecciones que haya que rehacer en cada transcripción — y el cuadro vuelve a mirar cuando se cierran.

El idioma se conserva para la próxima vez. El motor mismo y los límites de los subtítulos son
[ajustes](preferences.md) (`transcription.*`).

**Cuando el proyecto ya contiene subtítulos, la ventana pregunta antes de sustituirlos**, y dice
cuántos desaparecen. Pregunta antes del trabajo, de modo que declinar no cuesta nada. Cuando el proyecto está vacío no
hay nada que preguntar.

El trabajo se ejecuta **en segundo plano**, con un cuadro y un botón `Cancel`; la ventana sigue siendo utilizable alrededor. **El cuadro
dice en qué punto está el trabajo**, para que sepa que está vivo y cuánto va a tardar:

- **el paso**: primero “Extrayendo el audio…” — el cuadro está entonces ocupado, sin cifra, pues extraer no dice nada de hasta dónde ha
  llegado —, luego “Transcribiendo el audio…”, con una barra;
- **la posición**: “0:12:30 de 0:50:00”, hasta dónde de la película ha llegado el motor, en cuanto ha producido su primer texto;
- **el tiempo restante**, “quedan unos 38 min”: “estimando el tiempo restante…” al principio, sustituido cuando se ha hecho trabajo suficiente para extrapolar, y calculado sobre la
  velocidad del último minuto; es una precisión de unos minutos, no una promesa;
- **el tiempo transcurrido**, “transcurrido: 0:05:12”, que avanza incluso cuando nada más se mueve;
- **el último texto** obtenido, para que vea que el trabajo tiene sentido.

En una película larga con un modelo grande en un procesador, los primeros minutos pueden ser largos: el motor carga el modelo y trabaja en sus primeras
ventanas de treinta segundos. La posición y el último texto se mueven antes que el por ciento propio del motor, que solo llega en pasos
del 5 % de la película. Cuando termina, los subtítulos del proyecto se sustituyen
por la transcripción **como una sola operación**: un solo `Undo` devuelve lo que había, textos de
traducción incluidos. Una transcripción que falla, o que usted cancela, no cambia nada — el motivo se
dice en un cuadro, salvo en una cancelación, que solo dice que se canceló en la barra de estado.

**Cuando la mayor parte de lo que el motor ha escrito son notas entre corchetes** — `[MUSIC PLAYING]`, `[SPEAKING FRENCH]` —, un cuadro
lo dice después del trabajo: el idioma empleado probablemente no es el que se habla, y el motor puede haber traducido el habla.
Vuelva a ejecutarlo escribiendo el idioma. Los subtítulos están en el proyecto de todos modos, y un `Undo` devuelve lo que había.

### Probar primero una parte

Un intervalo convierte la transcripción en una **prueba**, para juzgar un modelo o un idioma en dos minutos antes de comprometerse con
dos horas. Los campos “Parte de la película” admiten un inicio y un final, cada uno escrito `2:00`, `1:02:03`, `2:00.5` o un número
de segundos, `90`; se permite solo uno (desde ahí hasta el final, o desde el principio hasta ahí). `OK` queda apagado mientras un campo
no es un tiempo o el final no va después del inicio, y una línea bajo los campos dice cuál.

- **Los subtítulos conservan el tiempo de la película**: lo que se dice en 2:10 está en `0:02:10` en el resultado.
- **No se sustituye nada y no se pregunta nada.** El resultado se abre como **un proyecto aparte**, en una pestaña nueva, sin
  archivo; el proyecto del que viene, y sus subtítulos, quedan como estaban. Guárdelo, o ciérrelo.

**Lo que necesita no se entrega**: un motor (whisper.cpp), un modelo y `ffmpeg` para extraer el audio.
Véase [Instalar las herramientas de transcripción](../subedit-cli/install-transcription.md).

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

**La transcripción es lo único que sí necesita `ffmpeg`**: extrae el audio de la película. Véase
[Instalar las herramientas de transcripción](../subedit-cli/install-transcription.md).
No es, por tanto, un modo degradado, sino **el funcionamiento normal** de una máquina
que nunca ha instalado `ffmpeg`: el dato lo aporta el usuario, y `ffprobe`
lo propone cuando está.

Una velocidad de fotogramas leída es un **racional exacto** — `24000/1001`, no `23.976`.
Es la razón por la que se conserva `ffprobe` para esa respuesta: el
reproductor sabe lo mismo como número de coma flotante, y una velocidad de fotogramas aproximada
reajustaría los tiempos de todo el archivo con un ligero desvío.
