# subedit-cli — manual

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

L'eina de línia d'ordres de subedit: treballar amb fitxers de subtítols sense interfície
gràfica, per lots o des d'un script. Per a l'altre programa i per saber
per on començar, vegeu
[el manual](../index.md).

> **Estat actual.** Les disset subordres de `subedit-cli` existeixen: inspeccionar,
> convertir, desplaçar, transformar, resincronitzar la freqüència de fotogrames, alinear a una graella de fotogrames, suprimir
> les mencions per a persones sordes, ajustar les durades, reemplaçar un text, canviar les majúscules i minúscules,
> la cursiva i els guions de diàleg, tornar a posar els subtítols en ordre, corregir els
> textos amb els patrons de Gaupol, posar fitxers l'un darrere l'altre, tallar un
> fitxer en dos i escriure una traducció a les posicions del seu fitxer principal. Aquest manual
> descriu el que existeix, mai el que està previst; el que ve després és al
> [full de ruta](../../../../feuille-de-route.md).

## Seccions

| Secció | Contingut |
| :------ | :------ |
| [Instal·lació](installation.md) | compilar i instal·lar l'eina |
| [Invocació](invocation.md) | opcions globals, sortides, codis de sortida |
| [El fitxer de configuració](invocation.md#el-fitxer-de-configuració) | què llegeixen `adjust` i `correct` del fitxer de configuració de la finestra, `--config`, `--no-config` |
| [Processar un arbre](lots.md) | diversos fitxers, un directori sencer, `--recursive` |
| [`inspect`](inspect.md) | informar de què està fet un fitxer |
| [`convert`](convert.md) | escriure un fitxer en un altre format o una altra forma |
| [`shift`](shift.md) | desplaçar cada posició la mateixa quantitat |
| [`transform`](transform.md) | corregir cada posició a partir de dos punts que se sap que són correctes |
| [`framerate`](framerate.md) | resincronitzar un fitxer d'una freqüència de fotogrames a una altra |
| [`snap`](snap.md) | tornar a posar les marques de temps sobre els fotogrames d'una freqüència de fotogrames |
| [`hearing-impaired`](hearing-impaired.md) | suprimir les mencions per a persones sordes |
| [`adjust`](adjust.md) | ajustar les durades a una velocitat de lectura i a uns límits |
| [`replace`](replace.md) | reemplaçar un text sense trencar cap etiqueta |
| [`case`](case.md) | posar els textos en majúscula inicial a cada paraula, en majúscula de frase, en majúscules o en minúscules |
| [`italics`](italics.md) | posar els textos en cursiva, o treure'ls la cursiva |
| [`dialogue-dashes`](dialogue-dashes.md) | posar guions de diàleg, o treure'ls |
| [`sort`](sort.md) | tornar a posar els subtítols en l'ordre del seu inici |
| [`join-repeats`](join-repeats.md) | fer un sol subtítol dels trossos d'un mateix text que se segueixen |
| [`disjoint`](disjoint.md) | tallar els subtítols que s'encavalquen perquè cap franja no s'encavalqui més amb una altra |
| [`repair`](repair.md) | encadenar `join-repeats`, `sort` i `disjoint` sobre un fitxer mal segmentat, desordenat i amb encavalcaments |
| [`correct`](correct.md) | corregir els textos amb els patrons de Gaupol: mencions, unió i separació de paraules, errors comuns, majúscules, divisió de línies |
| [`append`](append.md) | posar fitxers després del primer, en un sol fitxer |
| [`split-file`](split-file.md) | tallar un fitxer en dos, amb la cua portada a l'origen |
| [`pair`](pair.md) | escriure una traducció a les posicions del seu fitxer principal, i dir com s'han alineat les seves línies |
| [`transcribe`](transcribe.md) | escriure la parla d'un vídeo o d'un fitxer d'àudio en forma de subtítols |
| [`sync`](sync.md) | resincronitzar els subtítols d'un fitxer segons l'àudio d'una pel·lícula |
| [Instal·lació de les eines de transcripció](install-transcription.md) | el motor, el model i `ffmpeg`, que el programa no lliura |
| [`config`](config.md) | llistar, mostrar i canviar la configuració de les operacions compartida amb la finestra |
