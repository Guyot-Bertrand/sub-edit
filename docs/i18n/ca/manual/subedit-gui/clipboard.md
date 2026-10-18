# Retallar, copiar i enganxar textos

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

Tres entrades del menú `Edit`, sota `Undo` i `Redo`, i damunt de
`Find and Replace…`.

| Entrada | Drecera | Què fa |
| :---- | :------- | :----------- |
| Retalla els textos | `Ctrl+X` | copia els textos de la selecció i després els buida |
| Copia els textos | `Ctrl+C` | copia els textos de la selecció |
| Enganxa els textos | `Ctrl+V` | escriu els textos copiats a partir de la primera línia seleccionada |

**El que viatja són els textos, i res més**: ni inici, ni final, ni línia
sencera. Enganxar, per tant, no mou cap posició — la sincronització de la línia
que rep un text continua sent la seva. És el que fa Gaupol.

## A què s'apliquen

**Amb una traducció, al text de la columna actual**: la traducció si la cel·la
actual és a la columna `Translation`, el text principal a la resta —
vegeu [La taula](table.md#el-text-a-què-sadrecen-les-operacions). `Cut Texts`, per tant, buida
la traducció i deixa el text principal, i `Paste Texts` escriu a la
columna on sou. **El porta-retalls no porta cap document**: podeu
copiar una cel·la del text principal i enganxar-la a la traducció, i a
l'inrevés.

**La selecció, i mai el fitxer sencer.** Les tres entrades estan desactivades
mentre no hi ha res seleccionat. A la resta de la finestra, «res seleccionat» vol dir
«el fitxer sencer»; aquí, un `Ctrl+X` desafortunat buidaria tots els textos
del document, i un enganxament no tindria cap línia per on començar.

Mentre una cel·la és oberta, `Ctrl+X`, `Ctrl+C` i `Ctrl+V` pertanyen
al camp d'entrada: retallen, copien i enganxen caràcters, igual que `Del`
hi esborra un caràcter i no un subtítol. Llançades des del menú, les entrades
primer validen l'entrada: `Copy Texts` copia llavors el text tal com s'acaba
d'escriure. Vegeu [Editar una cel·la](editing.md).

## Copiar

`Copy Texts` copia el text de cada línia seleccionada, de la primera a
l'última. **Una selecció discontínua conserva la seva forma**: una línia que ha
quedat fora de la selecció esdevé un forat, i l'enganxament no la sobreescriurà.

**Una línia seleccionada amb el text buit també esdevé un forat**:
enganxada, deixa la línia de destinació tal com és. Copiar un sol text buit, per tant,
no enganxa res.

El porta-retalls del sistema rep aquests textos **com a text pla, separats per una
línia buida** — un forat s'hi escriu com un text buit, i per això un text buit
no pot ser altra cosa que un forat: l'enganxament dona el mateix resultat
tant si la còpia ve d'aquesta finestra com d'un altre programa. El que copieu aquí
es pot enganxar, per tant, a qualsevol programa, i el que copieu en un altre lloc
es pot enganxar aquí.

Copiar no modifica el document, i no entra a l'historial.

## Retallar

`Cut Texts` copia i després **buida els textos** de les línies seleccionades. Les línies
es queden, amb les seves posicions: retallar no és suprimir — per suprimir
línies, és [`Remove Subtitles`](lines.md#suprimir).

Retallar textos que ja són tots buits no fa res, i no entra a
l'historial.

## Enganxar

`Paste Texts` escriu els textos del porta-retalls **a partir de la primera línia
seleccionada**, una línia per text, cap avall.

| Què conté el porta-retalls | Què passa |
| :-------------------------- | :----------- |
| un text | substitueix el de la línia |
| un forat, o un text buit copiat | la línia conserva el seu text |
| més textos que línies restants | s'afegeixen línies al final per rebre'ls |

Les línies afegides neixen com les línies inserides després de l'última: tres
segons cadascuna, l'una després de l'altra. **La finestra ho diu**, en un quadre d'informació:

```text
inserted 2 subtitles to fit the clipboard
```

Després de l'enganxament, **les línies escrites queden seleccionades**.

Un enganxament que no canvia res — els mateixos textos a les mateixes línies — no entra a
l'historial.

### Les etiquetes d'un altre format es tradueixen

Una còpia feta en aquesta finestra **recorda el format del seu document**, i
aquest record sobreviu a l'obertura d'un altre fitxer. Enganxar en un document
d'un altre format, per tant, tradueix les etiquetes, com fa `Save As…`:
`{\i1}Bonjour{\i0}` copiat d'un Advanced SSA s'enganxa com a `<i>Bonjour</i>` en un
SubRip.

**El que el format de destinació no pot escriure es perd, i es diu**, amb
les paraules de `Save As…`:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

A diferència de `Save As…`, no es pregunta res abans: l'enganxament es desfà amb
`Ctrl+Z`, que torna els textos tal com eren.

**Un text copiat fora de `subedit` no té format**, i s'enganxa tal com és:
no hi ha res a traduir des d'enlloc. També és el cas d'una còpia d'aquesta
finestra que el porta-retalls del sistema ha substituït després — el que compta
és el contingut del porta-retalls del sistema.

Quan s'escauen tots dos missatges, caben en un sol quadre, separats per
un punt i coma.

## Què en diu l'acció de desfer

Cada gest és **una** entrada de l'historial, sigui quin sigui el nombre de
línies que escriu o afegeix.

| Operació | Què llegeix `Undo` |
| :-------- | :---------------- |
| retallar | `Undo: cutting texts` |
| enganxar | `Undo: pasting texts` |

Vegeu [Desfer i refer](undo.md).
