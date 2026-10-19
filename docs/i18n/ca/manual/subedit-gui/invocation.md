# Invocació

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-gui [--version] [<file>]
```

| Argument | Obligatori | Valor | Per defecte |
| :------- | :------- | :---- | :------ |
| `<file>` | no | el camí d'un fitxer de subtítols per obrir | cap — la finestra s'obre buida |
| `--version` | no | — | — |

**Un sol fitxer.** Els arguments següents s'ignoren: la finestra només conté
un projecte alhora.

## `--version`

Escriu a la sortida estàndard el bloc que escriu el `--version` de la línia d'ordres, i
torna immediatament, sense obrir cap finestra. Codi de sortida `0`.

**`subedit <version>` sol a la primera línia**, després una línia per biblioteca amb què
funciona la finestra, amb la versió trobada en temps d'execució i, entre parèntesis, aquella
amb què es va compilar; després les llengües de la interfície instal·lades, per codi. En
una màquina amb Qt 6.4.2:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

Els números no es copien aquí: són els de la màquina, i aquest manual no té cap
bloc generat que mantingui al dia els del programa. Les biblioteques són
les de la línia d'ordres, ICU i Enchant, i les dues que només enllaça la finestra — Qt, i
libmpv, el reproductor, indicada per la versió de la seva API de client, la que anuncia
`mpv.pc`. La resta es descriu al
[manual de la línia d'ordres](../subedit-cli/invocation.md#--version). Les paraules
`built with` i `Languages` segueixen la llengua de la finestra; res més no ho fa.

`Ajuda ▸ Quant a subedit` mostra les mateixes biblioteques, de la mateixa llista, en una taula —
el nom, la versió carregada, la versió amb què es va compilar, una fila cadascuna — i després
les llengües a sota; els seus encapçalaments segueixen la llengua de la finestra.

## Obrir un fitxer

```console
$ subedit-gui film.srt
```

Els formats que es llegeixen són **els nou** que enumera el filtre d'obertura — vegeu
[Obrir i desar](files.md#obrir). El format es reconeix pel contingut i
no per l'extensió, i **també la codificació**: una BOM
si el fitxer en porta una, si no la codificació que suggereixen els seus bytes.

## Quan falla l'obertura

Un fitxer que no existeix, un d'il·legible, un els bytes del qual no es descodifiquen en cap
codificació, o un el contingut del qual no reconeix cap format: **la finestra s'obre
igualment, buida**, i l'error s'escriu a la sortida d'error.

**El missatge anomena la causa**, amb les mateixes paraules que la línia d'ordres:

| Missatge | Què ha passat |
| :------ | :------------ |
| `does not exist` | el camí no designa cap fitxer |
| `cannot be opened: permission denied` | el sistema es nega a obrir-lo |
| `cannot be read` | el sistema s'hi ha negat per un altre motiu |
| `cannot be decoded in the chosen encoding` | els bytes no es llegeixen en la codificació triada |
| `is in no format this tool knows` | cap format no reconeix el contingut |
| `holds nothing recognisable as a subtitle` | el format es reconeix, però res del que conté no és un subtítol |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Obrir-se buida en lloc de negar-se a arrencar: l'eina encara serveix per a
alguna cosa, i l'error d'un argument no és cap motiu per no iniciar-se.

**Un fitxer mal format però llegible s'obre**, i el que la lectura ha detectat
pel camí es pot llegir en un botó de la barra d'estat — vegeu
[els diagnòstics d'una lectura](files.md#els-diagnòstics-duna-lectura).

## Codis de sortida

| Codi | Quan |
| :--- | :--- |
| `0` | la finestra s'ha obert i després s'ha tancat normalment, o `--version` ha respost |
| `2` | un error inesperat abans d'obrir-se — Qt absent, pantalla no disponible |

L'error en obrir un fitxer **no canvia el codi de sortida**: la
finestra s'ha obert, i això és el que indica el codi.

## La pàgina de manual

Una instal·lació deixa `subedit-gui(1)` a `share/man/man1`:

```console
$ man subedit-gui
```

Dona la forma d'invocació, `--version`, els missatges d'error d'obertura, els
codis de sortida, les dues variables d'entorn i el fitxer de configuració.
**No descriu la finestra** — ho fa aquest manual, i la pàgina
remet aquí.

## La plataforma Qt

`QT_QPA_PLATFORM` tria sobre què funciona la finestra. **Si la variable està
definida, `subedit-gui` la respecta sense discutir.**

Si no ho està, deixa que Qt triï: el reproductor integrat dibuixa la imatge a la
mateixa finestra, i no demana res a la plataforma — vegeu
[El reproductor](player.md).

`QT_QPA_PLATFORM=offscreen` fa funcionar la finestra on no hi ha cap
servidor gràfic — que és el que fan la integració contínua i les proves.

## Llengua

La finestra parla la llengua del seu entorn, **triada una sola vegada, en arrencar**: canviar les
variables d'una finestra en funcionament no canvia res fins al següent inici. Les variables i les regles
són les de la línia d'ordres — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` per a
l'anglès, l'anglès **sense dir res** quan la llengua demanada no està instal·lada — descrites al
[manual de la línia d'ordres](../subedit-cli/invocation.md#llengua). Els catàlegs són els mateixos
fitxers, instal·lats a `<prefix>/share/subedit/locale`.

**Una llengua triada a `Edita ▸ Preferències…` té prioritat**, i l'entorn només es llegeix
quan val `System default`, que és el cas mentre no se n'hagi triat cap: una llengua instal·lada triada allà
s'imposa a totes les variables, `LC_ALL=C` inclosa. L'elecció, també, s'aplica al següent inici. Vegeu
[La llengua de la interfície](preferences.md#la-llengua-de-la-interfície).

| Què | Segueix la llengua |
| :--- | :------------------- |
| els menús i les seves entrades, amb les lletres subratllades; la barra d'eines; els indicadors de funció | sí |
| les columnes de la taula i de `Visualitza ▸ Columnes`, les pestanyes, la línia d'estat, la barra de reproducció | sí |
| els botons estàndard que dibuixa el mateix Qt (`OK`, `Cancel`) | sí, quan el catàleg propi de Qt per a aquesta llengua està instal·lat (`qtbase_<language>.qm`); si no, en anglès |
| els diàlegs, les pàgines de `Correct Texts…`, els títols i els filtres dels diàlegs de fitxers, els quadres de missatge i el que diuen | sí |
| les línies que la finestra escriu a la sortida d'error, `subedit-gui: …` | sí, excepte el nom del programa |
| els noms dels patrons de correcció a `Correct Texts…`, i les seves descripcions als indicadors de funció | sí, per als patrons distribuïts amb el programa; un patró vostre es mostra tal com l'escriu el seu fitxer |
| els noms de les codificacions, dels finals de línia i de les llengües d'ortografia (`UTF-8`, `CRLF`, `en_US`) | mai: són noms, no paraules |
| les dreceres de teclat | mai: `Ctrl+S` és `Ctrl+S` en totes les llengües |
| un missatge que el catàleg no tradueix | es mostra en anglès |

Aquesta versió instal·la `ca`, `cs`, `de` i `fr` — el català, el txec, l'alemany i el francès — traduïts
íntegrament. El francès l'ha revisat un lector de la llengua; el català, el txec i l'alemany són
traduccions automàtiques que encara no ha revisat cap lector de la llengua.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
En un escriptori francès, la barra de menús mostra
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. Les captures
d'aquest manual es fan en català.
