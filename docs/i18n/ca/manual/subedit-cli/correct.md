# `correct`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Corregeix els textos amb els **patrons de Gaupol**, com l'assistent
[`Correct Texts…`](../subedit-gui/correct-texts.md) de la finestra — els mateixos patrons,
el mateix motor, els mateixos recomptes. Aquí hi ha sis tasques: les **mencions per a
persones sordes**, **unir** i **separar paraules**, els **errors comuns**,
les **majúscules** i el **trencament de línies**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Què es tria, i què no

**No hi ha res marcat d'entrada.** La finestra obre les seves caselles tal com l'usuari les va deixar; una
línia d'ordres pren les tasques, el codi, la longitud de línia i la llengua de la seva pròpia línia,
mai del fitxer de configuració: no es llegeixen ni les tasques marcades un dia a la finestra, ni les caselles
de patrons que s'hi van activar (`correction.activations`), ni la llista de substitucions del corrector
ortogràfic, i no s'hi escriu res.

**Quatre paràmetres provenen del fitxer de configuració, per defecte** — els valors que `Correct Texts…`
recorda, o que [`config set`](config.md) va escriure. **Una opció a la línia sempre guanya**; després
el fitxer, i després el valor per defecte integrat; vegeu [El fitxer de configuració](invocation.md#el-fitxer-de-configuració)
per saber on és, `--config` i `--no-config`.

| Clau | Què aporta | Opció que la substitueix |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | les classes d'errors comuns aplicades | `--classes` |
| `correction.remove-blank` | si se suprimeixen els subtítols que la correcció buida | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | el nombre màxim de línies de `line-break` | `--max-lines` |

- **Cadascun dels quatre té una opció que l'anul·la**, digui el que digui el fitxer: `--classes human,ocr`
  aplica les dues classes, `--remove-blank-subtitles` suprimeix els subtítols buidats, `--max-lines 3`
  torna les línies de Gaupol. `--keep-blank-subtitles` i `--remove-blank-subtitles` alhora són un
  error d'ús.
- **Que les dues classes estiguin desactivades al fitxer s'ignora, i es diu** al nivell 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` no ho podria escriure, i
  deixaria common-errors sense res a fer. En lloc d'això s'apliquen les dues classes.
- **Un nombre de línies fora de l'interval d'1 a 1000 no es llegeix del fitxer en absolut**: el lector del fitxer
  el refusa — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, al
  nivell 1 — i s'usa el 3 integrat.

Els exemples d'aquesta pàgina s'executen amb `--no-config`, perquè diguin què fan els valors per
defecte integrats, sigui quina sigui la configuració de la màquina que els executa.

| Opció | Obligatori | Valors | Per defecte |
| :----- | :----- | :------ | :----- |
| `<file>...` | sí | un o més camins | — |
| `--tasks` | **sí** | una llista separada per comes: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | cap tasca |
| `--code` | **sí** amb una tasca de patrons, i **no aplicable** altrament | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **sí** amb `join-words` o `split-words`, i **no aplicable** altrament | un codi de llengua: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | no | `human`, `ocr` o `human,ocr` | totes dues — o el que digui el fitxer de configuració |
| `--enable`, `--disable` | no, repetible | el nom anglès d'un patró, o `type:name` | els fitxers `.conf` distribuïts |
| `--max-length` | **sí** amb `line-break` | un nombre de **caràcters** més gran que zero | **cap** — vegeu més avall |
| `--max-lines` | no | un nombre enter de línies, més gran que zero | 3 — o el que digui el fitxer de configuració |
| `--skip-length`, `--skip-lines` | no | un nombre, o `off` | els mateixos límits que `--max-length` i `--max-lines` |
| `--keep-blank-subtitles` | no | un indicador: els subtítols buidats es queden, buits | els subtítols buidats se suprimeixen — tret que el fitxer de configuració digui el contrari |
| `--remove-blank-subtitles` | no | un indicador: els subtítols buidats se suprimeixen, digui el que digui el fitxer de configuració; refusat amb `--keep-blank-subtitles` | desactivat |
| `--range` | no | `N-M`, o `N-` fins al final — vegeu [`adjust`](adjust.md#--range) | tot el fitxer |
| `--document` | no | `main` o `translation` — vegeu [Una traducció](invocation.md#una-traducció) | `main` |
| `-t`, `--translation-file` | amb `--document translation` | un fitxer de traducció, per a **una sola** entrada | — |
| `--align-method` | no, i només amb `-t` | `position` o `number` | `position` |
| `--recursive`, `-r` | no | un indicador | desactivat — vegeu [Tractar un arbre](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

### Les tasques

**L'ordre de la llista no hi fa res**: les tasques s'apliquen en
l'ordre de Gaupol — les mencions, després unir i separar paraules, després els errors comuns, després les majúscules, després el trencament de línies —, cadascuna sobre el
text que ha deixat l'anterior. Un subtítol que les mencions han buidat no participa
en el que ve després.

| Tasca | Què fa |
| :---- | :-------------- |
| `mentions` | suprimeix els sons entre claudàtors, i entre parèntesis quan `Sound in parentheses` és activat (l'escombrada de [`hearing-impaired`](hearing-impaired.md)), i després els patrons de lletres de cançons i d'interlocutors |
| `join-words` | torna a enganxar una paraula tallada en dues, segons el diccionari de `--language` |
| `split-words` | separa dues paraules enganxades, segons el mateix diccionari |
| `common-errors` | corregeix els errors comuns: espais, puntuació, lligadures, errors d'OCR |
| `capitalization` | posa la majúscula on la vol la llengua |
| `line-break` | torna a tallar les línies d'un subtítol perquè cap no passi de `--max-length`, sospesant els patrons de trencament del codi |

### El codi de patrons

**`--code` anomena la cascada**: `Zyyy` per a qualsevol escriptura, després l'escriptura, la llengua
i el país — `Latn-en-US` llegeix els patrons de `Zyyy`, de `Latn`, de `Latn-en` i de
`Latn-en-US`. **És obligatori**, perquè una línia d'ordres no té llengua ambiental:
un valor per defecte universal aplicaria *menys* del que es creu, sense dir-ho. Val per a
totes les tasques d'una mateixa execució; una tasca que vol un codi propi és una segona
execució. Un codi mal escrit és un error d'ús:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Els patrons que l'usuari posa a `$XDG_DATA_HOME/subedit/patterns` (o a
`~/.local/share/subedit/patterns`) **s'afegeixen als patrons distribuïts**, com a la
finestra, i un patró distribuït que no es pot llegir es diu al nivell 1: `patterns: <file>, line N (<reason>)`.

### Unir i separar paraules, i el diccionari

**Aquestes dues tasques no llegeixen cap patró**: consulten el **diccionari del sistema, a través
d'Enchant** — una dependència dels paquets (`libenchant-2-2` a Debian i Ubuntu, `enchant2` a
Fedora), que `subedit-cli` comparteix amb la finestra. Per tant no tenen ni codi ni nom per definir:
`--code` no té sentit sense una tasca de patrons, ni `--language` sense una d'aquestes dues
— un sense l'altre és un error d'ús, codi `1`.

- **`join-words`** enganxa una paraula que el diccionari no coneix a la que la precedeix o
  la segueix, **quan una sola direcció dona una paraula coneguda**: `hel lo` esdevé `hello`; dues direccions
  possibles, o cap, no decideixen res.
- **`split-words`** separa una paraula que el diccionari no coneix **quan exactament un dels seus
  suggeriments és aquesta paraula amb un espai**: `hellothere` esdevé `hello there`. Una paraula amb una inicial
  majúscula seguida de minúscules es deixa tal com és: sovint és un nom.

**Sense diccionari per a la llengua, l'execució es refusa**, codi `1`, **abans de llegir cap
fitxer** i amb les paraules de la finestra, que desactiva la funció i ho diu — aquí la línia d'ordres
refusa i ho diu, en lloc de callar:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**La llista de substitucions de la finestra no es llegeix ni s'escriu.** Es *va omplint* amb les
eleccions de l'usuari a la correcció ortogràfica; una línia d'ordres no té cap elecció
d'aquest tipus, i llegir la llista faria dependre un script del que el seu autor hi va posar un dia.

### El trencament de línies

**Es mesura en caràcters, i aquesta és la diferència amb la finestra.** La finestra mesura en *ems* —
una amplada calibrada sobre la lletra tipogràfica, a 0,55 em per lletra —, i el seu 24 per defecte és un valor
en ems. Una línia d'ordres no té lletra tipogràfica: compta caràcters, els del
text. **Copiar el 24 com a caràcters ho tallaria gairebé tot, sense
dir-ho**; per tant **no hi ha longitud per defecte**, i `--max-length` és obligatori amb
`line-break`:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` és 3**, el valor de Gaupol, que la unitat no canvia; el trencament de línies en pren
una més quan tres no n'hi ha prou. **L'omissió** — un subtítol que ja és dins dels límits es
deixa tal com és — pren per defecte **els mateixos límits** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), com fa Gaupol; `--skip-length off` i `--skip-lines off`
la desactiven, i aleshores el text es torna a llegir com un de sol i es torna a tallar:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

Els patrons de trencament són els del codi: `Latn` no trenca entre un nombre i la seva unitat ni
després d'un guió de diàleg, `Latn-en` no trenca després d'un article, una preposició, un tractament
o un determinant possessiu. **Sota `Zyyy`, no hi ha cap patró de trencament**: les línies només
s'equilibren. **Aquesta tasca no es refusa mai per manca d'un patró actiu**: els seus patrons
sospesen on trencar, i el trencament es fa sense ells. Les quatre opcions només tenen sentit amb
`line-break`; donades sense aquesta tasca, són un error d'ús.

### Les classes, i l'activació pel nom

**`--classes` filtra els errors comuns del tot**: desmarcar `ocr` treu els seus
patrons de l'aplicació — la lligadura `ﬁ` aleshores es queda `ﬁ`. Els altres tres tipus de patrons
no porten cap classe.

**`--enable` i `--disable` defineixen un patró pel seu nom anglès**, que és la clau
(ADR 0037); un nom pot designar diversos registres, que aleshores comparteixen una sola
casella. Cal fer-ho, i és el que aparta els valors per defecte de la finestra: **els
fitxers `.conf` distribuïts desactiven els patrons de mencions**, de manera que `--tasks mentions` sol
no aplicaria res. Els dos patrons «Sound in brackets» i «Sound in parentheses»
governen cadascun el seu delimitador — els claudàtors, els parèntesis — i es defineixen amb les mateixes
opcions: activar-ne només un no suprimeix l'altre.

Tres coses són **errors d'ús**, codi `1`, que es diuen abans de llegir cap fitxer:

| Què el provoca | Missatge |
| :------------------ | :------ |
| un nom que no designa cap patró de les tasques donades, sota el codi donat | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| un nom que designa patrons de dos tipus — aleshores s'escriu `type:name`, on el tipus és `common-error`, `capitalization`, `hearing-impaired` o `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| un nom donat a `--enable` i a `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**Un error de tecleig es fa sentir**: la finestra ignora un paràmetre caducat perquè un
paràmetre pot sobreviure a un patró que ha desaparegut; una invocació no té cap supervivent.

**Una tasca sense cap patró actiu es refusa**, codi `1` — «res a fer» és una
resposta que es diu, no un èxit silenciós:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

El mateix fitxer, amb l'escombrada de claudàtors activada:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Què s'escriu, i què es diu

El format, la codificació i els finals de línia del fitxer llegit **es conserven**. **Els
subtítols que la correcció buida se suprimeixen**, com la casella per defecte de la
finestra; `--keep-blank-subtitles` els deixa buits, i `--remove-blank-subtitles` els suprimeix
fins i tot quan el fitxer de configuració els conserva. **Una traducció buidada es queda buidada**,
i conserva el seu subtítol.

**El recompte és el de la finestra** — «Edited N and removed M subtitles»: textos
canviats i subtítols suprimits, **mai coincidències**. Un fitxer on no
canvia res s'escriu igualment: una destinació donada és una destinació escrita.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Un patró que no es pot aplicar

**Un patró que no es pot llegir, no es pot traduir o no acaba s'anomena, amb el
subtítol on s'ha rendit**, i aquí és on la línia d'ordres diu més que la
finestra: la pàgina de confirmació anomena el patró una sola vegada, sense el text.

| Motiu | Què significa |
| :----- | :------------------- |
| `cannot be translated` | la seva expressió usa el que la traducció a ICU refusa |
| `will not compile` | el motor ha refusat la seva expressió |
| `has an invalid replacement` | la seva substitució no es llegeix com la llegeix Python |
| `timed out` | el motor s'ha rendit amb aquest text |
| `never settled` | `Repeat` encara canviava el text després de cent passades |
| `grew the text too long` | el text ha superat el que un subtítol pot contenir |

Al nivell 1, a la sortida d'error: 
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — sense el final quan el patró s'aparta per a tots els textos. **El número és el
del fitxer**, siguin quines siguin les tasques que hagin suprimit subtítols abans. En JSON, és
un avís `pattern-failed` del fitxer, el `detail` del qual porta la frase. **Un patró
abandonat no és un fitxer fallit**: el fitxer s'escriu, i el codi de sortida és `0`.

## Sortida

**Sortida estàndard** — res, ja que el resultat és el fitxer escrit. **Amb `--dry-run`, són els
canvis proposats**, un bloc per subtítol: el seu número, el text d'abans, el text de després —
i `(removed)` sense línia `+` per a un subtítol que la correcció suprimiria; amb
`--keep-blank-subtitles`, es queda, buit. Vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure).

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, els patrons que no s'han pogut aplicar, els fitxers de patrons que no es poden llegir — i, primer, el fitxer de configuració llegit — `settings: read <path>` — i el que se n'ha ignorat |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, i després cada diagnòstic de lectura |

En JSON, `counts` porta `corrected` (els textos canviats) i `removed` (els subtítols
suprimits), i `changes` els subtítols tocats. Amb `-t`, la clau `alignment` diu com s'ha
col·locat la traducció.
`settings` anomena el fitxer de configuració que s'ha llegit, o és `null` quan no se n'ha llegit cap.

## Codis de sortida

Els de l'eina: `0` si s'han escrit tots els fitxers, `2` si no se n'ha escrit cap, `3` si només
alguns, `1` en un error d'ús.

## Errors

| Què el provoca | Què s'escriu, a la sortida d'error |
| :------------------ | :--------------------------------------- |
| un fitxer anomenat per `--config` que no existeix — el fitxer propi de l'usuari, si falta, dona els valors per defecte | `settings file not found: <FILE>` |
| falta `--tasks` | `--tasks is required`, seguit d'una indicació cap a `--help` |
| una tasca desconeguda | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| falta `--code` | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` sense cap tasca de patrons | `--code is for the tasks that read patterns, and none was asked for` |
| falta `--language` amb `join-words` o `split-words` | `--language is required by the tasks that check words: <tasks>` |
| un `--language` que no és un codi de llengua | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` sense cap d'aquestes dues tasques | `--language is for the tasks join-words and split-words, which were not asked for` |
| una llengua sense diccionari | `no dictionary for <language>` |
| un codi mal escrit | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| falta `--max-length` amb `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| un `--max-length` que no és un nombre més gran que zero | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines` que no són un nombre enter de línies | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` o `--skip-lines` illegibles | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| una de les quatre opcions sense `line-break` | `--max-length is for the task line-break, which was not asked for` |
| una classe desconeguda | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` amb `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| un nom de patró erroni | vegeu la taula anterior |
| una tasca sense cap patró actiu | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, la destinació | els d'[Invocació](invocation.md#errors) i d'[Una traducció](invocation.md#una-traducció) |
