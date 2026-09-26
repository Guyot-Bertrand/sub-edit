# 0037 — Lire les fichiers de motifs de Gaupol tels quels

**Date :** 2026-09-26
**Statut :** acceptée

Décidée en cadrant la phase 12, issue
[#493](https://github.com/Guyot-Bertrand/sub-edit/issues/493).

## Contexte

Les motifs de correction de Gaupol sont vingt-quatre fichiers : douze fichiers
de motifs, nommés `<Code>.<type>`, et leurs douze `.conf` d'activation. Le code
est `Écriture[-langue[-PAYS]]`, `Zyyy` valant « toute écriture ». Un fichier de
motifs est une suite d'enregistrements à la manière de GKeyFile — un en-tête
`[Common Error Pattern]`, puis `Name`, `Description`, `Classes`, `Pattern`,
`Flags`, `Replacement`, `Repeat`, `Policy`, `SkipIn`, `Capitalize`, `Group`,
`Penalty` —, **avec des en-têtes répétés** qu'aucun lecteur INI standard
n'accepte. Un `.conf` est un XML d'une ligne par nom : `<pattern name="…"
enabled="…"/>`.

La question avait été posée une première fois à la phase 4 et écartée (#88) :
« la copie serait licite, les deux projets sont sous GPL-3, mais elle
engagerait sur un format de fichiers qu'on n'a pas choisi ». Depuis, deux faits
ont changé la donne :

- **#494 a versionné ces fichiers** dans `src/test/data/motifs/gaupol/`, avec
  leur attribution, et les attendus que Gaupol en tire : ce que le moteur doit
  reproduire est écrit, **à partir de ces fichiers-là** ;
- **la phase 12 doit lire des fichiers de motifs de toute façon** — ceux de
  l'utilisateur. Gaupol lit ceux qu'on dépose dans son répertoire de données
  (`~/.local/share/gaupol/patterns`), et l'iso-fonctionnalité demande la même
  porte.

## Décision

**Lire le format de Gaupol tel quel**, par un lecteur écrit au noyau, et en
faire au chargement un modèle typé — un enregistrement est un
`CorrectionPattern` à champs nommés, pas un dictionnaire de chaînes. Les
fichiers livrés sont **ceux de Gaupol, sans retouche**, installés avec le
programme. Ce qu'on garde de l'utilisateur — l'activation de chaque motif — va
dans **nos réglages**, et non dans des `.conf` écrits à la manière de Gaupol.

## Ce qui décide

**Une seule copie, lue par tout le monde.** L'oracle Python de #494 lit ces
fichiers ; le moteur C++ lira les mêmes ; le paquet installe les mêmes. Un
format à nous demanderait un convertisseur, et le jour où Gaupol modifie un
motif, il faudrait convertir avant de pouvoir relancer l'oracle — deux vérités
au lieu d'une.

**La mise à jour depuis l'amont reste un geste de copie.** `LISEZMOI.md` de #494
le décrit déjà : recopier l'ensemble depuis une révision plus récente, relancer
l'oracle, lire le diff des attendus. Aucun format intermédiaire ne s'intercale.

**Un fichier de l'utilisateur de Gaupol se lit chez nous.** Quelqu'un qui a
écrit ses motifs pour Gaupol les dépose dans notre répertoire et ils
s'appliquent — sous réserve de la traduction de syntaxe de
l'[ADR 0036](0036-icu-pour-les-motifs-de-correction.md), qui dit ce qu'elle
refuse.

**Les objections de #88 ont une réponse.** Le format n'est pas le nôtre, mais le
lecteur tient en une centaine de lignes, et ses règles sont déjà écrites deux
fois — dans `aeidon/patternman.py` et dans `pattern-oracle.py`, qui en nomme
chaque étape : commentaire de ligne entière, `\0` non suivi d'un chiffre
retiré, clés préfixées d'un souligné acceptées. Les en-têtes répétés, qu'aucun
lecteur INI n'accepte, ne sont un problème que pour qui cherche un lecteur
INI ; on n'en cherche pas.

## Ce qui ne vient pas de Gaupol

**L'activation se garde dans nos réglages.** Gaupol écrit un `.conf` XML par code
dans son répertoire de configuration. Chez nous, les `.conf` livrés donnent
**les valeurs par défaut** ; ce que l'utilisateur coche ou décoche se retient
dans le fichier de réglages de l'[ADR 0022](0022-configuration-au-noyau-et-tolerance-par-option.md),
par type, code et nom — l'endroit unique de la configuration, tolérant option
par option. Écrire un second mécanisme de persistance pour une liste de cases à
cocher serait l'inverse de 0022.

**La clé est le nom anglais.** Gaupol traduit `Name` et `Description` par
gettext, mais active et retrouve un motif par son nom non traduit ; plusieurs
enregistrements partagent un nom et une seule case. On garde la clé ; la
traduction attend la phase 15.

**Les fichiers livrés quittent le répertoire des tests.** Ils sont aujourd'hui
dans `src/test/data/motifs/gaupol/`, parce que #494 n'avait qu'un oracle à
nourrir. Le programme installé ne peut pas lire un répertoire de tests : ils
passent dans `packaging/patterns/`, à côté des autres fichiers que le paquet
installe, et l'oracle comme les tests les y lisent. **Une copie, pas deux.**

## Alternatives écartées

- **Convertir une fois vers un format à nous** — TOML, JSON, ou un format ligne
  à ligne. Plus lisible, et l'on choisirait ses champs. Mais la conversion
  devient une étape de plus à chaque mise à jour amont, l'oracle devrait lire
  l'un ou l'autre, et les fichiers d'un utilisateur de Gaupol ne se liraient
  plus sans outil.
- **Compiler les motifs dans le binaire** — des tables C++ engendrées au
  moment de la construction. Aucun fichier à trouver à l'exécution, mais plus
  aucun moyen d'ajouter un motif sans recompiler, et la porte des motifs de
  l'utilisateur, que Gaupol ouvre, se refermerait.
- **Écrire des `.conf` à la manière de Gaupol** pour l'activation — la
  compatibilité jusqu'au bout. Mais la compatibilité qui compte est celle des
  motifs, pas celle de leurs cases à cocher, et le prix serait un second
  mécanisme de configuration à côté de celui de l'ADR 0022.

## Conséquences

**Un lecteur à tenir au noyau**, avec ses diagnostics : un enregistrement sans
`Pattern`, une clé inconnue, un fichier illisible se disent — fichier, ligne,
raison — et n'arrêtent pas la lecture des autres, comme l'ouverture d'un
sous-titre ([ADR 0008](0008-lecture-au-mieux-avec-diagnostics.md)).

**Un répertoire de données à l'exécution.** Le programme doit trouver les
fichiers livrés, et il le fait comme il trouve son manuel,
`installedManualPath()` : **à partir de l'exécutable**, `../share/subedit/patterns`,
jamais par un chemin figé à la construction — le préfixe de configuration et
celui d'installation ne sont pas le même. Les motifs de l'utilisateur ont leur
emplacement, `$XDG_DATA_HOME/subedit/patterns`, résolu par une seule fonction
comme `userSettingsPath()` ; et, comme pour lui, **un test ne résout jamais ni
l'un ni l'autre** : il reçoit ses répertoires. Le premier emplacement étant
propre à l'exécutable, et la ligne de commande devant corriger en phase 13, sa
résolution vit hors de `gui`.

**Ce qui justifierait de rouvrir** : Gaupol qui change de format, ou un besoin
que son format ne sait pas porter — un motif propre à `subedit`, par exemple.
Ce jour-là, un fichier à nous s'ajouterait à côté des siens plutôt que de les
remplacer.
