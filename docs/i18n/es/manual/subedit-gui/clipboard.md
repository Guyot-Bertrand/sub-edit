# Cortar, copiar y pegar textos

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

Tres entradas del menú `Edit`, debajo de `Undo` y `Redo`, y encima de
`Find and Replace…`.

| Entrada | Atajo | Qué hace |
| :---- | :------- | :----------- |
| Cortar textos | `Ctrl+X` | copia los textos de la selección y luego los vacía |
| Copiar textos | `Ctrl+C` | copia los textos de la selección |
| Pegar textos | `Ctrl+V` | escribe los textos copiados a partir de la primera línea seleccionada |

**Lo que viaja son textos, y nada más**: ni inicio, ni fin, ni línea
entera. Pegar no mueve, por tanto, ninguna posición — la temporización de la línea que
recibe un texto sigue siendo la suya. Es lo que hace Gaupol.

## Sobre qué actúan

**Con una traducción, sobre el texto de la columna actual**: la traducción si la
celda actual está en la columna `Translation`, el texto principal en los demás casos —
véase [La tabla](table.md#el-texto-al-que-se-aplican-las-operaciones). `Cut Texts` vacía por tanto
la traducción y deja el texto principal, y `Paste Texts` escribe en la
columna en la que se está. **El portapapeles no lleva ningún documento**: se
puede copiar una celda del texto principal y pegarla en la traducción, y
al revés.

**La selección, y nunca el archivo entero.** Las tres entradas están desactivadas mientras
no haya nada seleccionado. En el resto de la ventana, «nada seleccionado» significa
«el archivo entero»; aquí, un `Ctrl+X` desafortunado vaciaría todos los textos
del documento, y un pegado no tendría ninguna línea por la que empezar.

Mientras una celda está abierta, `Ctrl+X`, `Ctrl+C` y `Ctrl+V` pertenecen
al campo de entrada: cortan, copian y pegan caracteres, igual que `Del`
borra ahí un carácter y no un subtítulo. Lanzadas desde el menú, las entradas validan primero
la entrada: `Copy Texts` copia entonces el texto tal como se acaba de
teclear. Véase [Editar una celda](editing.md).

## Copiar

`Copy Texts` copia el texto de cada línea seleccionada, de la primera a la
última. **Una selección discontinua conserva su forma**: una línea que queda fuera
de la selección se convierte en un hueco, y el pegado no la sobrescribirá.

**Una línea seleccionada cuyo texto está vacío también se convierte en un hueco**:
al pegarse, deja la línea de destino como está. Copiar un único texto vacío no
pega, por tanto, nada.

El portapapeles del sistema recibe estos textos **como texto plano, separados por una
línea vacía** — un hueco se escribe ahí como un texto vacío, y por eso un texto vacío
no puede ser otra cosa que un hueco: el pegado da el mismo resultado
tanto si la copia viene de esta ventana como de otro programa. Lo que se copia aquí puede
pegarse, por tanto, en cualquier programa, y lo que se copia en otro sitio puede pegarse aquí.

Copiar no modifica el documento y no entra en el historial.

## Cortar

`Cut Texts` copia y luego **vacía los textos** de las líneas seleccionadas. Las líneas
se quedan, con sus posiciones: cortar no es eliminar — para eliminar
líneas, está [`Remove Subtitles`](lines.md#eliminar).

Cortar textos que ya están todos vacíos no hace nada y no entra en el
historial.

## Pegar

`Paste Texts` escribe los textos del portapapeles **a partir de la primera línea
seleccionada**, una línea por texto, hacia abajo.

| Lo que contiene el portapapeles | Lo que ocurre |
| :-------------------------- | :----------- |
| un texto | reemplaza al de la línea |
| un hueco, o un texto vacío copiado | la línea conserva su texto |
| más textos de los que quedan líneas | se añaden líneas al final para recibirlos |

Las líneas añadidas nacen como líneas insertadas después de la última: tres
segundos cada una, una tras otra. **La ventana lo dice**, en un cuadro de información:

```text
inserted 2 subtitles to fit the clipboard
```

Después del pegado, **las líneas escritas quedan seleccionadas**.

Un pegado que no cambia nada — los mismos textos en las mismas líneas — no entra
en el historial.

### Las etiquetas de otro formato se traducen

Una copia hecha en esta ventana **recuerda el formato de su documento**, y
ese recuerdo sobrevive a la apertura de otro archivo. Pegar en un documento de
otro formato traduce, por tanto, las etiquetas, como hace `Save As…`:
`{\i1}Bonjour{\i0}` copiado de un Advanced SSA se pega como `<i>Bonjour</i>` en un
SubRip.

**Lo que el formato de destino no puede escribir se pierde, y se dice**, con
las palabras de `Save As…`:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

A diferencia de `Save As…`, no se pregunta nada antes: el pegado se deshace con
`Ctrl+Z`, que devuelve los textos tal como estaban.

**Un texto copiado fuera de `subedit` no tiene formato**, y se pega tal cual:
no hay nada que traducir desde ninguna parte. Es también el caso de una copia de
esta ventana que el portapapeles del sistema ha reemplazado desde entonces — lo que cuenta es el
contenido del portapapeles del sistema.

Cuando corresponden los dos mensajes, caben en un solo cuadro, separados por
un punto y coma.

## Lo que dice de ello la acción de deshacer

Cada gesto es **una** entrada del historial, sea cual sea el número de
líneas que escribe o añade.

| Operación | Lo que indica `Undo` |
| :-------- | :---------------- |
| cortar | `Undo: cutting texts` |
| pegar | `Undo: pasting texts` |

Véase [Deshacer y rehacer](undo.md).
