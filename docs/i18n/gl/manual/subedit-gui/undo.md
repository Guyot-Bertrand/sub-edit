# Desfacer e refacer

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

Cada operación entra nun historial, e pódese desfacer. Unha cela editada conta
como **unha** entrada: a escritura non se rexistra tecla a tecla, só conta a
validación.

| Acción | Atallo | Onde |
| :----- | :------- | :---- |
| Desfacer | `Ctrl+Z` | menú **Editar**, barra de ferramentas |
| Refacer | `Ctrl+Shift+Z` | menú **Editar**, barra de ferramentas |

## A acción nomea o que vai desfacer

O menú non di «Desfacer» senón **«Desfacer: edición dun texto»**, segundo a
operación que se vai desfacer. O botón da barra de ferramentas conserva a palabra soa — o seu
ancho seguiría doutro xeito a última operación, e moveríase baixo o
punteiro —, pero a súa axuda emerxente leva a etiqueta completa.

| Operación | O que le a acción |
| :-------- | :-------------------- |
| unha cela de texto editada | `editing a text` |
| un inicio, un final | `editing a start`, `editing an end` |
| unha duración | `editing a duration` |
| unha inserción de liñas | `inserting` |
| unha eliminación de liñas | `removing` |
| un desprazamento, `Shift Whole File onto Grid` incluído | `shifting` |
| unha transformación | `transforming` |
| unha conversión da taxa de fotogramas | `converting the frame rate` |
| un aliñamento a unha taxa de fotogramas | `aligning on the frame rate` |
| unha eliminación de mencións | `removing hearing-impaired mentions` |
| poñer en cursiva | `putting in italics` |
| quitar a cursiva | `taking italics out` |
| un cambio de maiúsculas e minúsculas | `changing the case` |
| trazos engadidos, eliminados | `adding dialogue dashes`, `removing dialogue dashes` |
| unha fusión de liñas | `merging` |
| unha división dunha liña | `splitting` |
| textos cortados, pegados | `cutting texts`, `pasting texts` |
| un axuste das duracións | `adjusting durations` |
| abrir unha tradución | `opening a translation` |
| engadir un ficheiro | `appending a file` |
| dividir o proxecto | `splitting the project` |
| unha corrección aceptada en `Correct Texts…`, ou feita por `Check Spelling…` | `correcting texts` |
| unha substitución, todas as substitucións | `replacing`, `replacing all` |

**Vinte e sete etiquetas, e iso é todo o que pode producir a xanela.** O núcleo nomea
unha vixésimo oitava — `sorting` — á que non chega ningunha acción da xanela: a ordenación
só se produce cunha política de orde estrita, mentres que a xanela abre os seus documentos
coa política tolerante, que indica a desorde en lugar de reparala.

**`Shift Whole File onto Grid` non se distingue dun desprazamento**, e iso é
exacto: é un, cuxa cantidade se mediu en lugar de escribirse.

Son as mesmas palabras que as da liña de ordes: escríbense unha soa vez,
no núcleo.

Cando non hai nada que desfacer, a acción di simplemente «Desfacer» **e está
desactivada**: o seu atallo non fai nada.

## A marca de modificación

O título da xanela leva unha marca mentres o ficheiro difire do do
disco — un asterisco, ou o que use a plataforma no seu lugar. **Tamén a etiqueta
da lapela**, `premier.srt*`: o título só fala do proxecto que se está
a mirar, a etiqueta informa dos que non se están a mirar.

**Cada lapela ten o seu propio historial.** Desfacer só desfai o que recibiu o proxecto da
lapela actual — incluído un `Replace All` feito sobre todos os proxectos: este
deixa unha entrada en cada proxecto que tocou.

**Desaparece se se desfai ata o punto de partida.** Non é un detalle de
visualización: o número de modificacións cóntase, non se rexistra como un si ou non, así que
volver atrás pode levalo de novo a cero e a marca desaparece.

## Dúas cousas que hai que saber

**O punto de referencia é o último gardado**, ou a apertura do ficheiro se non
houbo ningún. Gardar móveo: a marca desaparece, e desfacer despois
tráea de volta.

Unha operación feita despois de desfacer **borra o que había para refacer**: refacer
reproduciría unha orde cuxo estado de partida xa non existe. É o
comportamento de todos os editores, e é deliberado.

**Mentres hai unha cela aberta**, `Undo` e `Redo` — desde o menú ou a barra de ferramentas —
validan primeiro a entrada, como os outros xestos de
[Editar unha cela](editing.md). O que se acaba de escribir pasa entón a ser a última
entrada do historial: `Undo` desfaino, non a operación anterior; `Redo` non ten
nada que refacer, xa que unha entrada borra o que había para refacer. Unha cela
aberta onde non se escribiu nada non deixa ningunha entrada: `Undo` desfai entón a
operación anterior.

O historial conserva mil entradas e esquece as máis antigas a partir de aí.
