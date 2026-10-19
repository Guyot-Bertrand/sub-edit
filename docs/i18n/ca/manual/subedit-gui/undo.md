# Desfer i refer

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

Cada operació entra en un historial, i es pot desfer. Una cel·la editada compta
com **una** entrada: el que s'escriu no s'hi registra tecla per tecla, només en
compta la validació.

| Acció | Drecera | On |
| :----- | :------- | :---- |
| Desfés | `Ctrl+Z` | menú **Edita**, barra d'eines |
| Refés | `Ctrl+Shift+Z` | menú **Edita**, barra d'eines |

## L'acció anomena el que desfarà

El menú no diu «Desfés» sinó **«Desfés: modificació d'un text»**, pel nom de
l'operació que es desfarà. El botó de la barra d'eines conserva la paraula
sola — si no, la seva amplada seguiria l'última operació, i es mouria sota el
punter —, però el seu indicador de funció porta l'etiqueta sencera.

| Operació | Què llegeix l'acció |
| :-------- | :-------------------- |
| una cel·la de text editada | `editing a text` |
| un inici, un final | `editing a start`, `editing an end` |
| una durada | `editing a duration` |
| una inserció de línies | `inserting` |
| una supressió de línies | `removing` |
| un desplaçament, `Shift Whole File onto Grid` inclòs | `shifting` |
| una transformació | `transforming` |
| una conversió de freqüència de fotogrames | `converting the frame rate` |
| una alineació a una freqüència de fotogrames | `aligning on the frame rate` |
| una supressió de mencions | `removing hearing-impaired mentions` |
| posar en cursiva | `putting in italics` |
| treure la cursiva | `taking italics out` |
| un canvi de majúscules i minúscules | `changing the case` |
| guions afegits, suprimits | `adding dialogue dashes`, `removing dialogue dashes` |
| una fusió de línies | `merging` |
| una divisió d'una línia | `splitting` |
| textos retallats, enganxats | `cutting texts`, `pasting texts` |
| un ajust de les durades | `adjusting durations` |
| l'obertura d'una traducció | `opening a translation` |
| afegir un fitxer | `appending a file` |
| dividir el projecte | `splitting the project` |
| una correcció acceptada a `Correct Texts…`, o feta per `Check Spelling…` | `correcting texts` |
| una substitució, totes les substitucions | `replacing`, `replacing all` |

**Vint-i-set etiquetes, i és tot el que la finestra pot produir.** El nucli
n'anomena una vint-i-vuitena — `sorting` — a la qual no arriba cap acció de la
finestra: l'ordenació només té lloc sota una política d'ordre estricta, mentre
que la finestra obre els seus documents sota la política flexible, que indica el
desordre en lloc de reparar-lo.

**`Shift Whole File onto Grid` no es distingeix d'un desplaçament**, i és
exacte: n'és un, la quantitat del qual s'ha mesurat en lloc d'escriure's.

Són les mateixes paraules que les de la línia d'ordres: s'escriuen una sola
vegada, al nucli.

Quan no hi ha res per desfer, l'acció diu simplement «Desfés» **i està
desactivada**: la seva drecera no fa res.

## La marca de modificació

El títol de la finestra porta una marca mentre el fitxer difereix del que hi ha
al disc — un asterisc, o el que la plataforma faci servir en el seu lloc. **També
la porta l'etiqueta de la pestanya**, `premier.srt*`: el títol només parla del
projecte que mireu, l'etiqueta informa dels que no mireu.

**Cada pestanya té el seu propi historial.** Desfer només desfà el que ha rebut
el projecte de la pestanya actual — un `Replace All` fet sobre tots els projectes
inclòs: deixa una entrada a cada projecte que ha tocat.

**Desapareix si desfeu fins al punt de partida.** No és un detall de
visualització: el nombre de modificacions es compta, no es registra com un sí o
un no, de manera que tornar enrere el pot tornar a zero i la marca desapareix.

## Dues coses que cal saber

**El punt de referència és l'últim desament**, o l'obertura del fitxer si no
n'hi ha hagut cap. Desar el desplaça: la marca desapareix, i desfer després la
torna a fer aparèixer.

Una operació feta després d'un desfer **esborra el que hi havia per refer**:
refer tornaria a reproduir una ordre l'estat de partida de la qual ja no existeix.
És el comportament de tots els editors, i és deliberat.

**Mentre una cel·la és oberta**, `Undo` i `Redo` — des del menú o des de la barra
d'eines — primer validen l'entrada, com els altres gestos
d'[Editar una cel·la](editing.md). El que s'acaba d'escriure esdevé llavors
l'última entrada de l'historial: `Undo` el desfà, no l'operació d'abans; `Redo`
ja no té res per refer, perquè una entrada esborra el que hi havia per refer. Una
cel·la oberta on no s'ha escrit res no deixa cap entrada: `Undo` desfà llavors
l'operació anterior.

L'historial conserva mil entrades i oblida les més antigues més enllà d'aquest
nombre.
