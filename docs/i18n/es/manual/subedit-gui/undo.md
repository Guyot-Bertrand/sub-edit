# Deshacer y rehacer

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

Cada operación entra en un historial y puede deshacerse. Una celda editada cuenta
como **una** entrada: lo que se teclea no se registra pulsación a pulsación, solo
cuenta la validación.

| Acción | Atajo | Dónde |
| :----- | :------- | :---- |
| Deshacer | `Ctrl+Z` | menú **Editar**, barra de herramientas |
| Rehacer | `Ctrl+Shift+Z` | menú **Editar**, barra de herramientas |

## La acción nombra lo que va a deshacer

El menú no dice «Deshacer», sino **«Deshacer: edición de un texto»**, según la
operación que se va a deshacer. El botón de la barra de herramientas conserva la palabra sola —
de lo contrario su anchura seguiría a la última operación y se desplazaría bajo el
puntero —, pero su información sobre herramientas lleva la etiqueta completa.

| Operación | Lo que indica la acción |
| :-------- | :-------------------- |
| una celda de texto editada | `editing a text` |
| un inicio, un fin | `editing a start`, `editing an end` |
| una duración | `editing a duration` |
| una inserción de líneas | `inserting` |
| una eliminación de líneas | `removing` |
| un desplazamiento, `Shift Whole File onto Grid` incluido | `shifting` |
| una transformación | `transforming` |
| una conversión de la velocidad de fotogramas | `converting the frame rate` |
| una alineación sobre una velocidad de fotogramas | `aligning on the frame rate` |
| una eliminación de anotaciones | `removing hearing-impaired mentions` |
| poner en cursiva | `putting in italics` |
| quitar la cursiva | `taking italics out` |
| un cambio de capitalización | `changing the case` |
| rayas añadidas, eliminadas | `adding dialogue dashes`, `removing dialogue dashes` |
| una fusión de líneas | `merging` |
| una división de una línea | `splitting` |
| textos cortados, pegados | `cutting texts`, `pasting texts` |
| un ajuste de duraciones | `adjusting durations` |
| la apertura de una traducción | `opening a translation` |
| la adición de un archivo | `appending a file` |
| la división del proyecto | `splitting the project` |
| una corrección aceptada en `Correct Texts…`, o hecha por `Check Spelling…` | `correcting texts` |
| un reemplazo, todos los reemplazos | `replacing`, `replacing all` |

**Veintisiete etiquetas, y es todo lo que la ventana puede producir.** El núcleo nombra
una vigésima octava — `sorting` — a la que no llega ninguna acción de la ventana: solo se
ordena bajo una política de orden estricta, mientras que la ventana abre sus documentos
bajo la política tolerante, que informa del desorden en lugar de repararlo.

**`Shift Whole File onto Grid` no se distingue de un desplazamiento**, y es
exacto: es uno, cuya cantidad se ha medido en lugar de teclearse.

Son las mismas palabras que las de la línea de comandos: se escriben una sola vez,
en el núcleo.

Cuando no hay nada que deshacer, la acción dice simplemente «Deshacer» **y está
desactivada**: su atajo no hace nada.

## La marca de modificación

El título de la ventana lleva una marca mientras el archivo difiera del que está en el
disco — un asterisco, o lo que la plataforma use en su lugar. **Lo mismo ocurre con la etiqueta
de la pestaña**, `premier.srt*`: el título solo habla del proyecto que se está
mirando; la etiqueta informa de los que no se están mirando.

**Cada pestaña tiene su propio historial.** Deshacer solo deshace lo que ha recibido el proyecto de la
pestaña actual — incluido un `Replace All` hecho sobre todos los proyectos: deja
una entrada en cada proyecto que ha tocado.

**Desaparece si se deshace hasta el punto de partida.** No es un detalle de
visualización: el número de modificaciones se cuenta, no se registra como un sí o un no, así que
volver atrás puede devolverlo a cero, y la marca desaparece.

## Dos cosas que conviene saber

**El punto de referencia es el último guardado**, o la apertura del archivo si no
lo ha habido. Guardar lo desplaza: la marca desaparece, y deshacer después
la hace volver.

Una operación hecha después de deshacer **borra lo que había por rehacer**: rehacer
reproduciría una orden cuyo estado de partida ya no existe. Es el
comportamiento de todos los editores, y es deliberado.

**Mientras una celda está abierta**, `Undo` y `Redo` — desde el menú o la barra de herramientas —
validan primero la entrada, como los demás gestos de
[Editar una celda](editing.md). Lo que se acaba de teclear pasa entonces a ser la última
entrada del historial: `Undo` lo deshace, no la operación anterior; `Redo` no tiene
nada que rehacer, ya que una entrada borra lo que había por rehacer. Una celda abierta
en la que no se ha tecleado nada no deja ninguna entrada: `Undo` deshace entonces la
operación anterior.

El historial conserva mil entradas y, más allá, olvida las más antiguas.
