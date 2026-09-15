# Phase 10 — Opérations complémentaires

Cadrage de l'issue [#370](https://github.com/Guyot-Bertrand/sub-edit/issues/370),
après l'initialisation [#369](https://github.com/Guyot-Bertrand/sub-edit/issues/369).

## Ce que la feuille de route promet, et ce qui est déjà là

Ajustement des durées, casse, italiques, tirets de dialogue, fusion, scission,
recherche et remplacement, presse-papiers.

**C'est la première phase où le sujet n'est pas une propriété du fichier.** Les
phases 8 et 9 répondaient à des questions qui ont une bonne réponse : dans quel
encodage ce fichier est-il, ce format porte-t-il une fin. Ici, la plupart des
opérations n'ont pas de bonne réponse unique — elles ont un résultat qu'un
utilisateur trouve acceptable ou non. Le cadrage doit donc dire **qui décide**,
et pas seulement quoi calculer.

Quatre choses sont déjà faites quand ce cadrage s'écrit, et elles changent son
contenu :

| Issue | Ce qu'elle a livré |
| :---- | :----------------- |
| [#365](https://github.com/Guyot-Bertrand/sub-edit/issues/365) | l'italique — **une entrée, pas trois** |
| [#371](https://github.com/Guyot-Bertrand/sub-edit/issues/371) | le recensement des contradictions de durée |
| [#372](https://github.com/Guyot-Bertrand/sub-edit/issues/372) | les cas de la recherche balisée, et la règle qu'ils fixent |
| [#373](https://github.com/Guyot-Bertrand/sub-edit/issues/373) | les deux fixtures de grille que la phase 16 avait laissées |

## Analyse préalable — ce que Gaupol fait

Lue dans `aeidon/agents/position.py`, `format.py`, `edit.py`, `clipboard.py`,
`parser.py` et `finder.py`, et **exécutée** là où la lecture ne suffisait pas.
L'inventaire porte le détail ; trois constats gouvernent ce qui suit.

**Un parseur conscient des balises est au centre.** `aeidon/parser.py` retire
les balises, retient leurs positions, laisse transformer le texte nu, puis les
remet. La casse et les tirets passent par lui. Ce
n'est pas le pivot de l'ADR 0031, qui traduit un vocabulaire en un autre et
retire ce qu'il ne sait pas porter ; l'ADR le dit elle-même — « ce sont deux
pièces différentes ».

**`adjust_durations` n'a pas de priorités, il a un ordre de lignes.** Vitesse,
minimum, maximum, écart ; seule la fin bouge ; la dernière contrainte appliquée
gagne et défait la précédente, sans le dire.

**Le presse-papiers ne transporte que des textes.** `aeidon/clipboard.py` est
une liste de chaînes, une par sous-titre. Ni positions, ni format, ni sous-titre
entier — ce que l'issue de cadrage croyait, et que la lecture a démenti.

## D1 — Le parseur conscient des balises, et ce qu'il décide

**La pièce que l'ADR 0009 annonce depuis la phase 1 est écrite ici.** Trois
opérations l'attendent, et aucune ne peut s'en passer : appliquer une expression
au texte source casse les balises, et l'ADR en donne l'exemple — `\[.*?\]` qui
avale une ouvrante et laisse sa fermante orpheline.

**Sa règle est écrite cas par cas, et le corpus est la spécification.**
`src/test/data/textes/recherche.cas` en porte vingt-quatre dans le vocabulaire
HTML, `recherche-accolades.cas` huit en Sub Station Alpha, et la règle est en
tête du premier, en six points. Les deux qui décident :

1. **Une frontière de balise qui coupe un mot est poussée au bord du mot**,
   vers l'extérieur, avant qu'on cherche quoi que ce soit. `<i>bon</i>jour` vaut
   `<i>bonjour</i>`.
2. **Tout style qui touche la correspondance couvre le remplacement entier.**

> **Tranché en #402 : ces deux règles sont celles du remplacement, et de lui
> seul.** Le parseur les appliquait dès sa construction, donc à toute opération
> qui passait par lui : la casse déplaçait une balise qui coupait un mot, un
> remplacement réécrivait des balises qu'il ne touchait pas, deux `<font>` de
> deux couleurs fusionnaient sur leur nom, et une espace insécable ou un
> guillemet comptaient pour des lettres. Désormais :
>
> | Opération | Ce que deviennent les balises |
> | :-------- | :---------------------------- |
> | remplacement | celles que la correspondance atteint sont poussées au bord du mot, puis couvrent le remplacement ; les autres ne bougent pas |
> | transformation — casse, tirets | **aucune ne bouge** : `<i>Bon</i>jour` en capitales donne `<i>BON</i>JOUR` |
>
> Deux plages ne fusionnent que si **la balise entière** est la même, attributs
> compris, et un mot est fait de **lettres et de chiffres**, lus par point de
> code.

> **Écrit en relecture de fin de phase.** Le corpus a bougé une fois après le
> cadrage : #378 a corrigé une réponse attendue de `recherche.cas` — deux styles
> voisins sur un même mot, `<i>Bon</i><b>jour</b>` — et ajouté en tête du
> fichier une règle d'ordre que les six points ne portaient pas. La réponse
> écrite avant le code se contredisait elle-même ; c'est le corpus qui a été
> corrigé, et il reste la spécification.

**Ce que cela écarte, et pourquoi.** Gaupol n'a pas de règle sur le premier
point, seulement une arithmétique de décalage, et la frontière retombe à une
place sans rapport avec le texte neuf — `<i>Bon</i>jour` remplacé par
`Bonjour tout le monde` lui rend `<i>Bonjour tout le m</i>onde`. Mesuré en
exécutant son parseur. C'est le seul endroit où cette phase s'écarte de lui
délibérément, et l'inventaire le porte.

> **Corrigé en relecture de fin de phase : l'écart est plus large.** La
> recherche de Gaupol **ne passe pas par son parseur** : `SearchAgent` cherche
> dans le texte stocké, balises comprises, avec un `Finder` nu. Chez lui `<i>` se
> trouve, et `Bonjour` ne trouve pas `<i>Bon</i>jour`. L'exemple ci-dessus décrit
> ce que son parseur ferait, pas ce que son dialogue fait. Chercher dans le texte
> visible est donc un choix de cette phase, pas une reprise corrigée — c'est
> celui du point difficile que la feuille de route posait. Et la cible des
> écarts délibérés compte d'autres lignes, que D4, D5 et D7 portent désormais.

La règle « le remplacement prend le style de son premier caractère » a été
écartée pour une raison qui se voit : elle n'est pas symétrique. Elle répond
`<i>Salut Sophie</i>` ou `Salut Sophie` pour la même forme, selon laquelle des
deux moitiés portait l'italique.

**Conséquence d'implémentation, et elle est ferme :** le parseur ne peut pas
être bâti sur `decodeAs`/`encodeAs`. Une balise laissée ouverte doit le rester —
un décodage suivi d'un ré-encodage la refermerait et changerait des octets que
personne n'a demandé de changer. Et une balise de mise en page, `{\pos(x,y)}`,
doit traverser intacte, là où le pivot la compte comme perdue.

## D2 — L'ordre des quatre contraintes de durée, et ce qui est sacrifié

**L'ordre est celui de Gaupol** — vitesse de lecture, durée minimale, durée
maximale, écart au suivant — et **seule la fin bouge**. Le début n'est jamais
touché ; le déplacer serait déplacer le sous-titre, ce qui est le travail de
`Shift Positions…`.

**L'écart gagne, et les trois autres sont des vœux.** Il est appliqué en dernier
et peut ramener la durée sous le minimum qu'on venait de poser. Ce n'est pas un
défaut hérité : c'est la seule hiérarchie défendable. Deux sous-titres à l'écran
en même temps sont un défaut que le spectateur voit ; un sous-titre un peu court
ne l'est pas.

**Ce qui est sacrifié est dit**, dans l'esprit de l'[ADR 0008](../adr/0008-lecture-au-mieux-avec-diagnostics.md) —
rapporter plutôt que taire.
C'est le seul point où nous ajoutons quelque chose à Gaupol, qui viole en
silence, et c'est le même geste que l'ADR 0031 pour la conversion.

**La contradiction n'est pas un cas de bord, et c'est mesuré.** Sur le corpus
privé, avec les défauts de Gaupol — minimum 1,5 s, maximum 6 s, écart 0 s,
vitesse 15 car/s :

| Ce qu'aucune fin ne peut satisfaire ensemble | Part des sous-titres |
| :------------------------------------------- | -------------------: |
| minimum contre écart | 4,5 % |
| vitesse contre écart | 9,2 % |
| vitesse contre maximum | 0,0 % |
| **au moins l'une des trois** | **12,9 %** |

Rejouable par `./src/scripts/measure-duration-constraints.py`, issue #371. Un
sous-titre sur huit : refuser l'opération sur ces cas-là la rendrait inutile, et
les violer en silence rendrait son résultat incompréhensible.

**Une contrainte inactive est absente, pas nulle.** Chez Gaupol, un minimum de
zéro désactive le minimum par accident — `minimum and …` est faux pour zéro, et
l'écart, testé par `gap is not None`, n'a pas ce défaut. Trois paramètres, deux
conventions. Ici elles s'expriment en `std::optional`, et zéro vaut zéro.

**Le dernier sous-titre n'a pas de suivant, et n'a pas de borne pour autant.**
Gaupol lui donne 360 000 s, soit cent heures ; un nombre en dur qui vaut « pas
de contrainte » s'écrit comme tel.

**La longueur du texte se compte hors balises**, comme `get_text_length` : la
vitesse de lecture parle de ce que le spectateur lit.

## D3 — Une durée saisie déplace la fin

**Renvoi du cadrage de la phase 5.** La colonne `Duration` s'affiche et ne se
saisit pas, faute d'une commande dans le noyau ; elle naît ici.

> **Corrigé en relecture de fin de phase.** Le cadrage ajoutait « dont D2 a
> besoin de toute façon » : c'est faux, l'ajustement pose la fin par la commande
> de position qui existait déjà, et `SetDurationCommand` n'a que la cellule pour
> appelant.

**La fin, et jamais le début.** C'est la borne que l'ajustement déplace, la
colonne `Start` se saisit déjà, et déplacer le début déplacerait le sous-titre.

**Un chevauchement créé par le geste est permis, et il est déjà dit.**
`scanAnomalies` relève `OverlappingSubtitles` depuis la phase 1 et la table le
teinte. Le cadrage écrivait « le panneau de diagnostics l'affiche » : ce panneau
ne parle que de la lecture du fichier et n'est alimenté qu'à l'ouverture —
corrigé en relecture de fin de phase. Refuser une saisie dont la conséquence est visible serait
refuser ce que l'utilisateur peut vouloir — c'est la décision D4 de la phase 5,
un avis et jamais un refus.

## D4 — La casse et les tirets passent par le parseur, et la casse passe par ICU

**Par le parseur, comme chez Gaupol.** Mettre un sous-titre en minuscules ne
doit pas transformer `<I>` en `<i>`, et un tiret de dialogue se pose devant le
texte, pas devant une balise ouvrante. **Et aucune balise ne change de place** —
tranché en #402 : la règle du bord du mot, en D1, est celle du remplacement.

**ICU porte les quatre casses.** `std::toupper` travaille octet par octet et
couperait une lettre accentuée en deux ; ICU est une dépendance du projet depuis
l'[ADR 0027](../adr/0027-icu-pour-les-encodages.md) et sait faire les quatre, la
casse de titre comprise. Aucune dépendance nouvelle, et la seule alternative
sérieuse — une table écrite à la main — serait fausse pour toutes les langues
qu'on n'aurait pas prévues.

> **Précisé en #401 : dans la locale racine, et en suivant ce qu'ICU réécrit.**
> Appelé sans locale, ICU casait selon celle du processus — `istanbul` en
> capitales rendait `İSTANBUL` sous `LANG=tr_TR`. Et une casse change la longueur
> d'une lettre — `ﬁ` devient `FI`, `İ` deux points de code — si bien que remettre
> les balises à leur décalage en octets en écrivait une au milieu d'un caractère,
> ou en perdait une. Les balises suivent désormais les éditions qu'ICU
> enregistre. Casser selon la langue du document est une question de la
> phase 15.

**La casse ne touche pas ce qui précède la première lettre.** Gaupol cherche le
premier caractère alphanumérique et n'applique la transformation qu'à partir de
là : un tiret de dialogue, un guillemet ouvrant, une parenthèse survivent.

**Le tiret est un basculement qui regarde toute la cible** : si une seule ligne
n'en a pas, toutes en gagnent un ; sinon toutes le perdent. C'est la règle que
l'italique de #365 a déjà posée — une entrée, deux sens, et appuyer deux fois
rend la cible telle qu'elle était.

> **Précisé en relecture de fin de phase : pour une cible homogène.** Le geste
> normalise ce qu'il touche. Une cible panachée part d'un bloc et ne revient pas
> panachée, et un cadratin `—` revient trait d'union. C'est le prix d'un bouton
> unique ; l'historique, lui, rend l'état exact.

> **Deux écarts à Gaupol, inscrits en relecture de fin de phase.** Sa règle des
> tirets ne reconnaît que le trait d'union, et une ligne vide y compte comme une
> ligne sans tiret — il en pose donc sur les lignes vides. Ici les trois tirets
> sont reconnus et les lignes vides ignorées : une cible toute en cadratins gagne
> des tirets chez lui et les perd ici. Et sa casse de titre est `str.title`, qui
> rend `L'Été` ; celle d'ICU rend `L'été`.

## D5 — La fusion recolle, la scission coupe au milieu de la durée

**Fusionner** prend le début du premier, la fin du dernier, et recolle les
textes non vides par un saut de ligne.

**Scinder** coupe au milieu de la durée : le premier garde tout le texte, le
second naît vide. C'est `split_subtitle`, retenu tel quel — **à une différence
près, relevée en relecture de fin de phase** : Gaupol remet aux défauts les
champs propres au format des deux moitiés, et la première les garde ici, comme
la ligne fusionnée.

> **Couper le texte à son saut de ligne a été écarté.** C'est tentant — un
> sous-titre de deux lignes en ferait deux d'une ligne — et c'est faux dès qu'il
> en a une ou trois. Une règle qui marche sur le cas fréquent et invente sur les
> autres est pire qu'une règle prévisible.

**Une commande par geste, chacune annulable d'un coup.** La phase 2 a posé que
l'historique est un compte fidèle de ce qui a été fait : fusionner cinq
sous-titres est une entrée, pas cinq.

**Trois choix faits par #380, inscrits en relecture de fin de phase :**

- **la fusion ne prend qu'un bloc de lignes voisines** — le noyau reçoit un
  intervalle, pas une `Selection` : fusionner 1 et 3 sans 2 recouvrirait la 2 ;
- **le style, la couche et les coordonnées de la première sont gardés**, là où
  Gaupol les perd ;
- **pas de raccourcis `M` et `S`** : une lettre seule serait prise à la table
  avant d'atteindre une cellule en cours d'édition.

## D6 — Le presse-papiers porte des textes, et traduit ce qu'il dépose

**Des textes, et rien d'autre.** C'est ce que Gaupol transporte, et il n'y a pas
de raison d'en transporter plus : coller des positions écraserait un calage que
personne n'a demandé de toucher.

**Coller ne déplace aucune position et crée des lignes s'il en manque.** C'est
`paste_texts`, qui insère des sous-titres vides à la fin pour recevoir ce qui
dépasse, et dit combien.

**Un texte collé depuis un document d'un autre format voit ses balises traduites,
et la perte est dite.** Gaupol ne le fait pas et dépose du `{\i1}` dans un
SubRip — la salissure que l'ADR 0031 a été écrite pour finir et que #364 a
fermée à l'écriture. Le presse-papiers en est la porte d'à côté.

Concrètement : le presse-papiers interne retient de quel format viennent ses
textes, un collage vers un autre passe par `convertMarkup`, et la perte s'annonce
dans les mots que `Save As…` emploie déjà. **Un texte venu de l'extérieur de
`subedit` n'a pas de format**, et se colle tel quel : il n'y a rien à traduire
depuis nulle part.

**La perte est dite après le collage, pas demandée avant**, à la différence de
`Save As…` — question ouverte par #370, tranchée par #382 : un collage s'annule
d'un `Ctrl+Z`, un fichier écrasé non. **Et les trois entrées exigent une
sélection** : couper sans sélection viderait tous les textes du document.

## D7 — La recherche porte sur la cible, avec deux options

**Deux options, celles de Gaupol** : expression régulière ou texte simple,
sensible ou non à la casse. Ses défauts sont retenus — texte simple, insensible
à la casse.

**La portée est la cible habituelle** : la sélection, ou tout le document si
rien n'est sélectionné. C'est la règle du menu `Tools` depuis la phase 5, et
elle remplace les portées de Gaupol.

> **Corrigé en relecture de fin de phase.** Le cadrage écrivait « les trois
> cibles de Gaupol ». Son dialogue a **deux portées** — le projet courant ou tous
> les projets ouverts — et **deux champs** — texte principal et traduction — et il
> **ne regarde jamais la sélection**, qui ne lui donne que la ligne de départ.
> Restreindre à la sélection est donc un ajout de cette phase.

**`Replace All` est une entrée d'historique, pas cent.**

**Un écart à Gaupol, inscrit en relecture de fin de phase : la syntaxe du
remplacement.** Les expressions sont lues par ICU, et un groupe s'écrit `$1` ;
Gaupol lit les siennes en Python, où il s'écrit `\1`. Taper `\1` ici met un `1`.
Le manuel le dit.

## D8 — Les deux surfaces, et une seule est livrée

**La phase livre le noyau et la fenêtre.** La ligne de commande est la phase 13,
« CLI complète », et aucune des huit opérations n'y apparaît ici.

Ce n'est pas un oubli mais une conséquence : ces opérations sont des gestes
d'édition interactifs — on met en italique ce qu'on regarde, on cherche ce qu'on
vient de lire. Les quatre opérations que la ligne de commande porte déjà —
décaler, transformer, convertir, retirer les mentions — ont en commun de
s'appliquer à un fichier entier sans qu'on le regarde, ce qui est exactement ce
qu'un traitement par lot demande.

**Les commandes du noyau sont écrites pour les deux surfaces malgré tout**, comme
`removeHearingImpaired` l'a été : une `Selection` reçue plutôt que déduite, et
un compte lu sur la commande plutôt que recompté. La phase 13 n'aura que du
câblage à faire.

> **Tenu dans l'esprit, pas à la lettre — relecture de fin de phase.** Aucune
> commande ne déduit sa cible de l'état de la fenêtre. Mais la fusion reçoit un
> intervalle, la scission et le collage un index, parce qu'une `Selection`
> discontinue n'y a pas de sens ; et le presse-papiers, l'ajustement et
> `Replace All` rendent leur compte **avec** la commande, dans une structure
> calculée à sa construction, plutôt que de le faire lire sur elle.

## Ce que la phase ne livre pas

Chacun avec une phase ou une issue, parce qu'un renvoi sans destinataire finit
par désigner une phase déjà passée.

- **Le correcteur orthographique**, et la jonction ou scission de mots qu'il
  permet — phase 12.
- **La correction d'erreurs courantes par motifs** — phase 12, dont c'est le
  cœur.
- **Le multi-projets**, donc la portée « tous les projets ouverts » d'une
  recherche et le texte de traduction comme champ cherchable — phase 11.
- **Retrouver la paire d'une conversion faite à la mauvaise fréquence** —
  [#386](https://github.com/Guyot-Bertrand/sub-edit/issues/386), phase 14. Rien
  de ce que cette phase apporte ne rend ce mécanisme plus sûr : c'est une
  recherche sur les rationnels, et ni jusqu'où chercher ni comment dire sa
  confiance n'ont de réponse aujourd'hui.
- **La prévisualisation dans un lecteur externe** — **écartée, et pas
  renvoyée.** Gaupol écrit un fichier temporaire et lance MPlayer ou VLC parce
  qu'il n'a pas eu de lecteur intégré pendant quinze ans. `subedit` en a un
  depuis la phase 6, et la phase 14 lui ajoute le pilotage. Reconstruire le
  détour serait livrer une régression.

## Exigences

| Identifiant | Ce qu'il promet |
| :---------- | :-------------- |
| `GUI-ITALIC-01` | une entrée met la sélection en italique et l'en retire, dans les balises du format ouvert |
| `GUI-ITALIC-02` | un format qui ne porte aucun style éteint l'entrée sans la cacher |
| `GUI-CASE-01` | les quatre casses s'appliquent à la cible sans toucher aux balises |
| `GUI-DASH-01` | les tirets de dialogue se posent et se retirent d'un même geste |
| `GUI-SPLIT-01` | fusionner et scinder, et l'annulation rend le document tel qu'il était |
| `GUI-DURATION-01` | la colonne `Duration` se saisit, et déplace la fin |
| `GUI-ADJUST-01` | l'ajustement applique les quatre contraintes dans l'ordre spécifié |
| `GUI-ADJUST-02` | ce qu'aucune fin ne peut satisfaire est dit, et compté |
| `GUI-SEARCH-01` | chercher dans le texte visible, remplacer dans le source, sans casser les balises |
| `GUI-SEARCH-02` | la recherche porte sur la sélection, ou sur tout le document |
| `GUI-CLIP-01` | copier, couper, coller des textes, et des lignes naissent s'il en manque |
| `GUI-CLIP-02` | coller depuis un document d'un autre format traduit les balises et dit la perte |

**Le parseur de D1 n'a pas d'exigence, et c'est la règle du registre** : une
exigence est ce que le binaire montre. Une garantie interne du noyau est tenue
par ses tests unitaires et par la couverture — et ici, par trente-deux cas
écrits avant lui.

## Découpage

L'ordre va du fondateur au coûteux, et **le parseur passe avant tout le reste** :
trois des sept issues l'attendent.

| Issue | Ce qu'elle fait | Ce dont elle dépend |
| :---- | :-------------- | :------------------ |
| [#378](https://github.com/Guyot-Bertrand/sub-edit/issues/378) | le parseur conscient des balises | — |
| [#379](https://github.com/Guyot-Bertrand/sub-edit/issues/379) | la casse et les tirets de dialogue | #378, ICU |
| [#380](https://github.com/Guyot-Bertrand/sub-edit/issues/380) | fusionner et scinder | — |
| [#381](https://github.com/Guyot-Bertrand/sub-edit/issues/381) | la durée se saisit | — |
| [#382](https://github.com/Guyot-Bertrand/sub-edit/issues/382) | le presse-papiers | `convertMarkup` |
| [#383](https://github.com/Guyot-Bertrand/sub-edit/issues/383) | l'ajustement des durées | #378 — la longueur se compte hors balises |
| [#384](https://github.com/Guyot-Bertrand/sub-edit/issues/384) | rechercher et remplacer | #378 |

La phase se clôt sur [#385](https://github.com/Guyot-Bertrand/sub-edit/issues/385),
la relecture de fin — la troisième des trois issues qui l'encadrent.

**#380 et #381 peuvent se faire à tout moment**, et elles sont placées tôt parce
qu'elles sont petites : passé le parseur, deux issues courtes éprouvent
l'outillage avant d'attaquer les deux grosses. C'est l'ordre qui a été suivi :
#378, #379, #380, #381, puis le reste.

> **Corrigé en relecture de fin de phase.** Le cadrage donnait #381 pour
> dépendance à #383 ; l'ajustement ne s'en sert pas, et c'est du parseur qu'il
> dépend.

**#383 et #384 sont les deux grosses**, et pour des raisons opposées. La
première porte un dialogue de quatre contraintes et un compte rendu de ce
qu'elle a sacrifié ; la seconde porte quatre gestes, deux options retenues d'une
session à l'autre, et une table qui se déplace toute seule.
