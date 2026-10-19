# Temporizar a partir del vídeo

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

*Se da por supuesto que la película está abierta y que se conoce el reproductor: véase [El reproductor](player.md).*

Se mira la película, se detiene en el fotograma donde debe empezar una línea, y **esa posición pasa a ser
el inicio del subtítulo**. Cinco gestos, en el menú **Vídeo**, con los nombres que les da Gaupol. Se
apoyan en lo que ya existe: poner una posición es **la misma orden que teclear el valor en la
celda**, e insertar es **lo mismo que `Editar ▸ Insertar subtítulos…`**.

| Comando | Atajo | Qué hace |
| :------ | :------- | :----------- |
| Vídeo ▸ Poner el inicio en la posición del vídeo | `Alt+U` | pone el **inicio** del subtítulo seleccionado en la posición de la película |
| Vídeo ▸ Poner el fin en la posición del vídeo | `Alt+K` | pone su **fin** en la posición de la película |
| Vídeo ▸ Insertar subtítulo en la posición del vídeo | `Alt+J` | inserta un subtítulo que **empieza** en la posición, y lo selecciona |
| Vídeo ▸ Seleccionar el anterior a la posición del vídeo | `Alt+Up` | selecciona el último subtítulo que **empieza antes** de la posición |
| Vídeo ▸ Seleccionar el siguiente a la posición del vídeo | `Alt+Down` | selecciona el primero que **empieza después** de la posición |

**Todos están desactivados sin vídeo abierto**, como los gestos del reproductor. `Set Start…` y `Set End…`
también están desactivados mientras no haya nada seleccionado: actúan sobre la **primera línea** de la
selección.

**Los atajos son los de Gaupol, con `Alt`**: allí son `U`, `K` y `J` — letras solas, que
la ventana tomaría antes que la celda en edición, de modo que teclear una `k` en un texto
pondría un fin. `Select Previous…` y `Select Next…` son `Ctrl+Y` y `Ctrl+U` en Gaupol; `Ctrl+Y`
es un *rehacer* en algunos sistemas, y las flechas con `Alt` dicen *antes* y *después* mejor que
dos letras. Una celda en edición se **valida** antes de `Set Start…`, `Set End…` o
`Insert Subtitle…`, como antes de cualquier modificación.

## Poner un extremo

- **Una entrada del historial por gesto**: `Editar ▸ Deshacer` devuelve la posición anterior, y nada más.
- **Las mismas reglas que la celda.** Un inicio puesto después del fin del subtítulo, o un fin antes de su
  inicio, **no se rechaza**: se pone, y la tabla marca el subtítulo como marca aquel
  cuyo valor se tecleó a mano — véase [Las anomalías](table.md#las-anomalías). Nada se corrige en silencio.
- **Una posición que ya es la del extremo no hace nada**, y no añade nada al historial.
- **La posición es siempre el inicio de un fotograma**: el reproductor nunca indica una posición
  dentro de un fotograma. Poner un extremo y luego volver a él — con `Seek Selection Start`, por ejemplo —
  **muestra el mismo fotograma**.

## Insertar en la posición

El nuevo subtítulo **empieza en la posición** y dura **tres segundos**, o hasta el inicio del siguiente
cuando este llega antes. Su texto está vacío. Ocupa su lugar en el orden del archivo: después de
todos los que empiezan en esa posición o antes de ella. La inserción es **una entrada del historial**,
y el subtítulo insertado queda seleccionado: se puede teclear enseguida.

## Seleccionar a partir de la posición

Los dos siguen el orden del archivo, no la selección actual. **Sin vecino por ese lado**, la
selección pasa al último subtítulo del archivo para `Select Next…`, al primero para
`Select Previous…` — el comportamiento de Gaupol. Estos dos gestos no mueven la película.

## Fotograma a fotograma

Avanzar y retroceder **un fotograma** — o N, véase más abajo —, para poner un extremo **en el fotograma exacto**.
Dos gestos, y dos botones en la barra de reproducción, a cada lado del botón de reproducción:

| Comando | Atajo | Qué hace |
| :------ | :------- | :----------- |
| Vídeo ▸ Retroceder un fotograma | `Alt+Left` | hace retroceder la película el paso de fotograma |
| Vídeo ▸ Avanzar un fotograma | `Alt+Right` | la hace avanzar el paso de fotograma |

- **El paso es un número de fotogramas, uno como mínimo**: es el ajuste `Frame step` de las
  [preferencias](preferences.md#el-reproductor). Un paso de N fotogramas muestra el fotograma **N más allá**, a 25 fotogramas por
  segundo como a 23,976; **nunca se expresa en milisegundos**.
- **La película se detiene en sus límites**: retroceder desde el primer fotograma la deja ahí, avanzar desde el último
  también. La reproducción se detiene tras un paso.
- **Los botones de la barra** — dos iconos, a cada lado de reproducir/pausa — **lanzan las mismas acciones que el menú**: mismo paso, desactivados
  sin vídeo como las entradas del menú, y la misma información sobre herramientas, que da el atajo. **Un dedo que se queda en un
  botón, o en la tecla, encadena los pasos.**
- **Un paso cada vez.** Una tecla mantenida se repite más deprisa de lo que se da un paso: la primera repetición sale
  enseguida, las que llegan mientras se da el paso **no se acumulan** — comparten un solo lugar, el
  último, que sale después. Soltar la tecla detiene, por tanto, la imagen donde se está, y no deja que la película
  siga por su impulso.

## Mover un extremo un fotograma

Para ajustar un extremo con el teclado: **el inicio o el fin del subtítulo seleccionado**, un paso, antes o después.

| Comando | Atajo | Qué hace |
| :------ | :------- | :----------- |
| Vídeo ▸ Adelantar el inicio | `Alt+Q` | el subtítulo **empieza un paso antes** |
| Vídeo ▸ Retrasar el inicio | `Alt+W` | **empieza un paso después** |
| Vídeo ▸ Adelantar el fin | `Alt+Shift+Q` | **termina un paso antes** |
| Vídeo ▸ Retrasar el fin | `Alt+Shift+W` | **termina un paso después** |

`Q` y `W` son las de Gaupol para *antes* y *después*, con `Alt` por la razón dada más arriba; el fin
toma `Alt+Shift`, porque la `E` de Gaupol es aquí la letra de `Edit`.

- **Una entrada del historial por desplazamiento**, que se puede deshacer. El subtítulo vecino y el solapamiento se señalan
  **como al teclear**: el extremo se pone, y la tabla marca lo que ya no se cumple — véase
  [Las anomalías](table.md#las-anomalías).
- **El mismo paso que el reproductor**, así que un fotograma por defecto. **Lo que vale un fotograma** se lee, en este orden,
  de **la velocidad de fotogramas que declara el vídeo**, de la del **documento cuando cuenta en fotogramas** (MicroDVD), y luego de
  **la rejilla que dibujan las posiciones**. **Sin ninguna de las tres, el gesto se niega y lo dice**: elegir una
  velocidad de fotogramas al azar desplazaría cualquier extremo algo que no es un fotograma.
- **No necesita vídeo**, solo una selección: sirve también sobre un archivo contado en fotogramas, sin
  película. Cuando hay una película abierta, **la reproducción va a la nueva posición**, para mostrar el fotograma que el extremo
  acaba de tomar.
- Un extremo no pasa por delante del origen: retroceder un paso desde menos de un fotograma lo pone a cero.
