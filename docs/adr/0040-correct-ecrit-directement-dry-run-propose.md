# 0040 — Écrire directement, et proposer par `--dry-run`

**Date :** 2026-10-01
**Statut :** acceptée

Décidée en cadrant la phase 13, issue
[#542](https://github.com/Guyot-Bertrand/sub-edit/issues/542). Répond à la question que la
[spec de la phase 12](../specs/12-correction.md) (D8) laissait à cette phase : *un mode qui
écrit les changements proposés sans les appliquer remplace-t-il la confirmation ?*

## Contexte

L'assistant `Correct Texts…` de la fenêtre calcule d'abord, sans toucher à aucun projet
(`proposeCorrections`), **puis** montre chaque texte changé avec son original, et n'applique
que ce que l'utilisateur a accepté (`applyCorrections`). C'est cette page de confirmation
qui fait de la correction automatique **une proposition plutôt qu'un fait accompli** — et
c'est la raison pour laquelle on tolère qu'un motif d'erreurs courantes soit parfois trop
zélé.

La ligne de commande n'a pas de page : elle n'a ni écran partagé ni questions en cours de
route, et ses appelants sont des scripts. Elle a pourtant le même besoin, **voir avant
d'écrire** — et elle a, à la différence de la fenêtre, un fichier qu'on peut ne pas
toucher. Les sous-commandes qui réécrivent suivent une règle de la phase 3 : **une
destination donnée est une destination écrite**.

## Décision

**`correct` écrit directement**, comme chaque sous-commande qui réécrit : donnez une
destination, le fichier est écrit. **Il n'y a ni confirmation ni invite.**

**`--dry-run` est la proposition.** Il est accepté par **toute sous-commande qui écrit** et
dit la même chose partout : *lire, calculer, rendre compte, n'écrire aucun fichier*.

- **Il n'exige pas de destination** — c'est tout son intérêt : on ajoute `--dry-run` à la
  ligne qu'on s'apprêtait à lancer, ou on la lance sans destination. **Une destination
  donnée reste vérifiée comme d'habitude** (collisions, entrées écrasées, ADR 0039) : une
  ligne de commande qui serait refusée sans `--dry-run` l'est avec.
- **Sur les sous-commandes qui changent des textes** — `correct`, `replace`, `case`,
  `italics`, `dialogue-dashes`, `hearing-impaired` —, **la sortie standard porte les
  changements proposés : pour chaque sous-titre changé, son numéro, le texte d'avant et le
  texte d'après**, ou « supprimé » quand le sous-titre le serait. C'est la même liste que la
  page de confirmation montre ; elle vient de la même fonction du noyau.
- **Sur les autres** (`shift`, `adjust`, `snap`…), elle porte ce que porterait la sortie
  d'un vrai lancement — les comptes —, avec la mention `dry_run` et `destination: null` en
  JSON, et une ligne de narration qui ne ment pas : `… (dry run, nothing written)` à la
  place de `-> <destination>`.
- **Le code de retour est celui d'un vrai lancement** : `0` quand tout s'est calculé,
  `2` ou `3` pour des échecs de lecture, `1` pour l'usage. **Que des changements existent ne
  change pas le code**, au contraire de `diff` : les quatre codes disent si l'outil a
  réussi, pas si le fichier est conforme, et le compte est dans la sortie.
- **En `--format json`**, l'objet du fichier porte les changements : un tableau
  `changes` de `{"subtitle", "document", "before", "after"}` — `after` est `null` pour une
  suppression. La forme exacte est celle de l'ADR 0038, et le manuel la tient.

**Appliquer, c'est relancer sans `--dry-run`.** Le calcul est une fonction du fichier et
des arguments, **déterministe** : ce que `--dry-run` a montré est ce que le lancement
suivant écrira, tant que le fichier n'a pas changé.

## Alternatives écartées

- **Un écrire-seulement-si-confirmé interactif** (`Apply? [y/N]`). Il casse tout appelant
  qui n'est pas un humain devant un terminal, et ce sont eux la raison d'être de la phase.
- **Un fichier de changements que `--apply` rejoue** (un correctif, un `.patch`, un état
  intermédiaire). C'est de l'état entre deux invocations : le fichier a pu changer, le
  correctif est périmé, et il faut décider ce qui arrive alors. Relancer le calcul, déterministe
  et bon marché — l'[ADR 0036](0036-icu-pour-les-motifs-de-correction.md) mesure l'ordre de
  grandeur d'un film entier : de quelques centièmes à un dixième de seconde —, n'a pas ce
  problème.
- **Un diff unifié sur la sortie standard.** Il ne porte pas le numéro de sous-titre, que
  rien dans un fichier ne rend avec certitude, il coupe un texte multiligne là où il le
  veut, et il lie la sortie au format du fichier. Le numéro, l'avant et l'après sont ce
  qu'un humain lit et ce qu'un script compare.
- **Inverser : proposer par défaut, écrire avec `--write`.** Cohérent avec la prudence, et
  contraire à la règle de la phase 3 que toutes les autres sous-commandes tiennent :
  `shift --output x` écrit. Une seule sous-commande qui s'y soustrait est celle dont un
  script oublie de savoir qu'elle est l'exception.
- **`--yes` / `--force`** comme une confirmation par défaut : il n'y a pas de question à
  laquelle répondre.
- **`--dry-run` pour `correct` seul.** Il tiendrait ce qu'on demande ici, et obligerait
  `replace`, qui a le même besoin — voir ce qu'on va remplacer avant de le faire —, à
  l'inventer à part, en deux grammaires.

## Conséquences

**Un seul mécanisme, dans `rewriteAll`** : le fichier est lu, l'opération calcule, et
l'écriture est la seule étape que `--dry-run` saute. Les sous-commandes n'en écrivent aucune
ligne propre ; celles qui changent des textes fournissent en plus la liste des changements.

**La liste de changements est un résultat de l'opération**, au même titre que ses comptes
(ADR 0038) : texte et JSON la lisent au même endroit. Elle n'est calculée que quand
`--dry-run` ou `--format json` la demande — un lot de milliers de sous-titres ne la
construit pas pour rien.

**Pas de mémoire entre les deux appels** : rien ne garantit qu'un fichier de motifs
déposé par l'utilisateur entre les deux n'a pas changé. C'est l'ordre normal des choses.

**Ce qui justifierait de rouvrir** : un appelant qui veut appliquer *une partie* des
changements proposés (le noyau sait composer un sous-ensemble : `applyCorrections` le prend
déjà), ce qui demanderait un format de changements qui se relit, donc un second schéma.

## Précisions de l'implémentation

Posées par [#557](https://github.com/Guyot-Bertrand/sub-edit/issues/557), qui construit le
mécanisme ; elles ne changent pas la décision.

- **La forme texte des changements** — le point ouvert 4 de la spec — est un bloc par sous-titre
  : `<chemin>: subtitle N`, puis chaque ligne du texte d'avant précédée de `- `, chaque ligne
  du texte d'après précédée de `+ `. Une suppression l'écrit sur la première ligne,
  `(removed)`, et n'a pas de lignes `+`. Le chemin en tête de bloc fait d'un lot une sortie qu'on
  filtre par fichier.
- **`destination` vaut `null` en JSON pour tout `--dry-run`, même quand une destination a été
  donnée** : elle a été vérifiée, elle n'a pas été écrite, et l'objet dit ce qui a eu lieu.
- **Le lancement à blanc fabrique les octets et ne les confie pas au système** : un caractère que
  l'encodage ne sait pas écrire fait échouer un `--dry-run` comme un vrai lancement, ce qui rend
  vrai « le code est celui d'un vrai lancement ». Seul un refus du disque lui échappe.
- **`changes` figure dans l'objet d'une sous-commande de texte aussi pour un vrai lancement en
  `--format json`**, vide quand rien ne change ; les autres n'ont pas la clé.
