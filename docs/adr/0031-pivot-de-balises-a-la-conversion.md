# 0031 — Un pivot de balises, pour la seule conversion

Statut : acceptée — 2026-09-07
Décidée en cadrant la phase 9, issue #337.

## Contexte

L'[ADR 0009](0009-texte-en-chaine-brute.md) a été écrite pour cette phase-ci.
Elle nomme ASS, et elle promet :

> Un modèle structuré reste utilisé, comme chez Gaupol, **uniquement comme pivot
> lors d'une conversion entre formats**, où la perte est inévitable et assumée.

**Ce pivot n'existe pas.** Rien ne l'avait demandé : SubRip et WebVTT écrivent
tous deux `<i>…</i>`, si bien qu'une conversion entre eux n'a jamais eu à
traduire quoi que ce soit. La chaîne brute passait telle quelle, et c'était juste.

Neuf formats, c'est cinq vocabulaires :

| Vocabulaire | Formats | L'italique s'y écrit |
| :---------- | :------ | :------------------- |
| HTML | SubRip, WebVTT, SubViewer 2 | `<i>…</i>` |
| SSA | Sub Station Alpha, Advanced SSA | `{\i1}…{\i0}` |
| MicroDVD | MicroDVD | `{Y:i}` jusqu'au bout du sous-titre |
| MPL2 | MPL2 | `/` en tête de ligne |
| aucun | TMPlayer, LRC | nulle part |

Sans traduction, un SSA converti en SubRip rend `{\i1}Il ne dit rien.{\i0}` — du
texte que l'utilisateur voit, dans un fichier qui ne saura jamais l'interpréter.
**Ce n'est pas une perte, c'est une salissure** : la perte aurait été de rendre
`Il ne dit rien.` sans italique, ce qui est lisible.

## Décision

**Le pivot est construit, et il ne vit que le temps d'une conversion.** Un texte
lu reste une chaîne brute ; ouvrir et enregistrer un fichier dans son propre
format ne le décode pas et ne le touche pas. L'ADR 0009 n'est ni remplacée ni
amendée : elle est honorée.

**Sa portée est celle de Gaupol** — gras, italique, souligné, couleur, police,
taille. Rien d'autre n'est traduit.

**Ce que le format d'arrivée ne sait pas écrire est retiré, et la conversion le
dit.** C'est le seul point où nous ajoutons quelque chose à Gaupol, qui retire
en silence.

## Alternatives écartées

### Ne rien traduire, et laisser passer les balises

C'est ce que ferait une conversion naïve, et le résultat est ci-dessus :
`{\i1}` visible dans un `.srt`. Le fichier reste valide et le texte devient
faux. Écartée sans hésitation.

### Retirer toutes les balises à la conversion

Simple, prévisible, et elle perd ce qui aurait pu être gardé. Convertir un SSA
en SubRip perdrait l'italique alors que SubRip sait l'écrire — une perte
gratuite, que la mesure de #339 compterait comme telle.

### Un modèle structuré permanent

Écartée en phase 1, pour un motif qui n'a pas bougé : le décodage est
destructeur, et le rendre permanent ferait perdre à l'ouverture ce qui n'est
perdu qu'à la conversion. L'ADR 0009 en garde la trace, et sa variante à
« fragments opaques » reste la porte de sortie qu'elle décrit — cette décision
ne la referme pas.

### Traduire aussi la mise en page

`{\pos(x,y)}`, `{\an8}`, `{\t(...)}`, les styles nommés SSA, `<ruby>` et les
horodatages internes de WebVTT. Chacun demanderait un modèle de mise en page que
rien d'autre dans le projet ne réclame, et la plupart n'ont d'équivalent nulle
part. Ils traversent intacts un aller-retour dans leur propre format, et
disparaissent à la conversion — en le disant.

## Conséquences

**Une conversion n'est plus une réécriture, c'est une traduction**, et son coût
en tests est celui d'une traduction : chaque paire de vocabulaires a ses cas.
Cinq vocabulaires font vingt paires ordonnées, dont huit vers ou depuis « aucun »
et donc triviales.

**La perte devient mesurable, et elle l'est déjà** — `docs/mesures/conversion.md`
compte les allers-retours intacts avant que le pivot existe, et le nombre bougera
quand il existera. C'est ce que l'outil de #339 a été écrit pour voir.

**Le parseur conscient des balises de l'ADR 0009 n'est toujours pas écrit**, et
ce pivot ne l'écrit pas. Ce sont deux pièces différentes : le pivot traduit un
vocabulaire en un autre, le parseur protège les balises pendant qu'une opération
transforme le texte autour d'elles. La phase 4 s'est passée du second
([0017](0017-analyseur-de-mentions-ecrit-a-la-main.md)) ; la phase 10 le
retrouvera sur son chemin. Le pivot lui donnera un vocabulaire commun à
reconnaître, ce qui est un acompte, pas la pièce.
