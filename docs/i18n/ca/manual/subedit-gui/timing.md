# Sincronitzar des del vídeo

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

*Se suposa que la pel·lícula és oberta i que el reproductor és conegut: vegeu [El reproductor](player.md).*

Mireu la pel·lícula, us atureu al fotograma on ha de començar una rèplica, i **aquesta posició esdevé
l'inici del subtítol**. Cinc gestos, al menú **Vídeo**, amb els noms que els dona Gaupol. Es
basen en el que ja existeix: fixar una posició és **la mateixa ordre que escriure el valor a la
cel·la**, i inserir és **el mateix que `Edita ▸ Insereix subtítols…`**.

| Ordre | Drecera | Què fa |
| :------ | :------- | :----------- |
| Vídeo ▸ Fixa l'inici a la posició del vídeo | `Alt+U` | fixa l'**inici** del subtítol seleccionat a la posició de la pel·lícula |
| Vídeo ▸ Fixa el final a la posició del vídeo | `Alt+K` | en fixa el **final** a la posició de la pel·lícula |
| Vídeo ▸ Insereix un subtítol a la posició del vídeo | `Alt+J` | insereix un subtítol que **comença** a la posició, i el selecciona |
| Vídeo ▸ Selecciona l'anterior des de la posició del vídeo | `Alt+Up` | selecciona l'últim subtítol que **comença abans** de la posició |
| Vídeo ▸ Selecciona el següent des de la posició del vídeo | `Alt+Down` | selecciona el primer que **comença després** de la posició |

**Tots estan desactivats sense cap vídeo obert**, com els gestos del reproductor. `Set Start…` i `Set End…`
també estan desactivats mentre no hi ha res seleccionat: actuen sobre la **primera línia** de la
selecció.

**Les dreceres són les de Gaupol, amb `Alt`**: allà són `U`, `K` i `J` — lletres soles, que
la finestra prendria abans que la cel·la en edició, de manera que escriure una `k` en un text
fixaria un final. `Select Previous…` i `Select Next…` són `Ctrl+Y` i `Ctrl+U` a Gaupol; `Ctrl+Y`
és un *refer* en alguns sistemes, i les fletxes amb `Alt` diuen *abans* i *després* millor que
dues lletres. Una cel·la en edició es **valida** abans de `Set Start…`, `Set End…` o
`Insert Subtitle…`, com abans de qualsevol modificació.

## Fixar una vora

- **Una entrada de l'historial per gest**: `Edita ▸ Desfés` torna la posició d'abans, i res més.
- **Les mateixes regles que la cel·la.** Un inici fixat després del final del subtítol, o un final abans del seu
  inici, **no es rebutja**: es fixa, i la taula marca el subtítol com marca aquell
  el valor del qual s'ha escrit a mà — vegeu [Les anomalies](table.md#les-anomalies). No es corregeix res en silenci.
- **Una posició que ja és la de la vora no fa res**, i no afegeix res a l'historial.
- **La posició sempre és l'inici d'un fotograma**: el reproductor mai no indica una posició
  dins d'un fotograma. Fixar una vora i després tornar-hi — amb `Seek Selection Start`, per exemple —
  **mostra el mateix fotograma**.

## Inserir a la posició

El subtítol nou **comença a la posició**, i dura **tres segons**, o fins a l'inici del següent
quan aquest arriba abans. El seu text és buit. Ocupa el seu lloc en l'ordre del fitxer: després de
tots els que comencen en aquesta posició o abans. La inserció és **una entrada de l'historial**,
i el subtítol inserit queda seleccionat: el podeu escriure de seguida.

## Seleccionar des de la posició

Tots dos segueixen l'ordre del fitxer, no la selecció actual. **Sense cap veí en aquest costat**, la
selecció va a l'últim subtítol del fitxer per a `Select Next…`, al primer per a
`Select Previous…` — el comportament de Gaupol. Aquests dos gestos no mouen la pel·lícula.

## Fotograma a fotograma

Avançar i retrocedir **un fotograma** — o N, vegeu més avall —, per fixar una vora **al fotograma exacte**.
Dos gestos, i dos botons a la barra de reproducció, a banda i banda del botó de reproducció:

| Ordre | Drecera | Què fa |
| :------ | :------- | :----------- |
| Vídeo ▸ Retrocedeix un fotograma | `Alt+Left` | fa retrocedir la pel·lícula el pas en fotogrames |
| Vídeo ▸ Avança un fotograma | `Alt+Right` | la fa avançar el pas en fotogrames |

- **El pas és un nombre de fotogrames, un com a mínim**: és el paràmetre `Frame step` de les
  [preferències](preferences.md#el-reproductor). Un pas de N fotogrames mostra el fotograma **N més enllà**, a 25 fotogrames per
  segon com a 23,976; **mai no s'expressa en mil·lisegons**.
- **La pel·lícula s'atura als seus límits**: retrocedir des del primer fotograma l'hi deixa, avançar des de l'últim
  també. La reproducció s'atura després d'un pas.
- **Els botons de la barra** — dues icones, a banda i banda de reproduir/pausar — **llancen les mateixes accions que el menú**: el mateix pas, desactivats
  sense vídeo com les entrades del menú, i el mateix indicador de funció, que dona la drecera. **Un dit que es queda en un
  botó, o a la tecla, encadena els passos.**
- **Un pas cada vegada.** Una tecla mantinguda es repeteix més de pressa del que es fa un pas: la primera repetició surt
  de seguida, les que arriben mentre es fa el pas **no s'acumulen** — comparteixen un sol lloc, l'últim,
  que surt després. Deixar anar la tecla, per tant, atura la imatge allà on sou, i no deixa que la pel·lícula
  continuï per inèrcia.

## Moure una vora un fotograma

Per ajustar una vora amb el teclat: **l'inici o el final del subtítol seleccionat**, un pas, més d'hora o més tard.

| Ordre | Drecera | Què fa |
| :------ | :------- | :----------- |
| Vídeo ▸ Inici més d'hora | `Alt+Q` | el subtítol **comença un pas més d'hora** |
| Vídeo ▸ Inici més tard | `Alt+W` | **comença un pas més tard** |
| Vídeo ▸ Final més d'hora | `Alt+Shift+Q` | **acaba un pas més d'hora** |
| Vídeo ▸ Final més tard | `Alt+Shift+W` | **acaba un pas més tard** |

`Q` i `W` són les de Gaupol per a *més d'hora* i *més tard*, amb `Alt` pel motiu que s'ha donat més amunt; el final
pren `Alt+Shift`, perquè la `E` de Gaupol aquí és la lletra d'`Edit`.

- **Una entrada de l'historial per moviment**, que es pot desfer. El subtítol veí i l'encavalcament s'indiquen
  **com quan s'escriu**: la vora es fixa, i la taula marca el que ja no es compleix — vegeu
  [Les anomalies](table.md#les-anomalies).
- **El mateix pas que el reproductor**, és a dir, un fotograma per defecte. **Quant val un fotograma** es llegeix, en aquest ordre,
  de **la freqüència de fotogrames que declara el vídeo**, de la del **document quan compta en fotogrames** (MicroDVD), i després
  de **la graella que dibuixen les posicions**. **Sense cap de les tres, el gest es nega i ho diu**: triar una
  freqüència de fotogrames a l'atzar desplaçaria qualsevol vora en una cosa que no és un fotograma.
- **No requereix cap vídeo**, només una selecció: també serveix en un fitxer comptat en fotogrames, sense
  pel·lícula. Quan hi ha una pel·lícula oberta, **la reproducció va a la posició nova**, per mostrar el fotograma que la vora
  acaba de prendre.
- Una vora no va més enllà de l'origen: retrocedir un pas des de menys d'un fotograma la posa a zero.
