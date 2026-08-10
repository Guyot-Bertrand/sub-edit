# 0014 — Nommer les exigences dans un registre plat, les citer par un tag de test

**Date :** 2026-08-10
**Statut :** acceptée

## Contexte

La phase 3 ouvre la ligne de commande. C'est la première fois que le projet aura
quelque chose qu'un utilisateur invoque, et une commande est un tout :
arguments analysés, fichier lu, opération appliquée, fichier écrit, code de
retour rendu. Les tests unitaires disent que chaque morceau fonctionne ; aucun
ne dit que l'assemblage fonctionne, et c'est l'assemblage que l'utilisateur
invoque.

La couverture de lignes ne comble pas ce trou. Elle dit **quel code a été
exécuté**, jamais **quelle fonctionnalité annoncée est démontrée**. Les deux
questions se ressemblent et n'ont pas la même réponse : un fichier couvert à
100 % peut ne rien promettre, et une promesse du manuel peut n'être vérifiée
nulle part sans qu'aucun chiffre ne baisse.

Le manuel décrit déjà exhaustivement ce que l'utilisateur voit — c'est une règle
du projet. Ce qu'il ne fait pas, et ne peut pas faire, c'est se lier à ce qui le
prouve : une phrase de manuel et le test qui la démontre n'ont aujourd'hui aucun
point commun qu'une machine sache lire.

Ce qu'on sait au moment de décider : le projet n'a qu'un exécutable, qui affiche
sa version, et le manuel le spécifie exhaustivement. Ce qu'on ignore : combien
de sous-commandes la phase 3 produira, et si les phases d'interface graphique
sauront se prêter au même mécanisme.

## Décision

Les exigences vivent dans **un registre plat et citable**, `docs/exigences.md` :
une table, une ligne par exigence, quatre colonnes — identifiant, énoncé, phase,
état.

Une exigence est **ce que le binaire montre** : sortie standard, sortie
d'erreur, code de retour, fichier produit. Les garanties internes du noyau n'en
sont pas ; leurs tests unitaires et la couverture de lignes tiennent ce rôle.

Un identifiant a la forme `SURFACE-SUJET-NN`, en capitales — `CLI-VERSION-01`.
La forme n'est pas décorative : c'est elle qui permet de reconnaître un
identifiant parmi les tags d'un test sans tenir de liste. Un tag ordinaire du
projet — `[e2e]`, `[format]`, `[framerate]` — n'a ni capitales ni segment
numérique final, et ne peut donc pas être pris pour un identifiant. La
réciproque est ce qui compte : un tag qui a cette forme **doit** désigner une
exigence, sans quoi le script échoue.

**Un identifiant n'est jamais réutilisé.** Une ligne remplacée reste, barrée,
avec sa raison. Un identifiant qui disparaît sans trace laisse un trou que
personne ne sait interpréter.

Quatre états, et chacun contraint la présence d'un test dans les deux sens :

| État | Ce que le registre affirme |
| :--- | :------------------------- |
| `implémentée` | au moins un test la cite |
| `prévue` | aucun test ne la cite |
| `abandonnée` | aucun test ne la cite |
| `remplacée` | aucun test ne la cite |

Les tests de bout en bout de `src/test/e2e/` citent une exigence par un **tag
Catch2**, `[CLI-VERSION-01]`, et non par un préfixe de titre.
`src/scripts/check-requirements.sh` interroge le binaire de test — `--list-tags`
sort l'ensemble exact des tags — et confronte les deux listes. Il entre dans
`make check`.

## Alternatives écartées

- **Chercher les tags en analysant le C++.** Un `grep` sur `TEST_CASE` aurait
  évité de construire le binaire. Écarté parce que la réponse serait
  *approchée* : elle raterait un tag produit par une macro ou par une
  compilation conditionnelle, et inventerait ceux d'un fichier commenté. Catch2
  connaît ses tags exactement ; les lui demander rend la vérification exacte au
  lieu de plausible.

- **Étiqueter par un préfixe dans le titre du test**, `TEST_CASE("[CLI-VERSION-01]
  l'invocation…")`. Écarté pour la raison inverse et symétrique : le binaire ne
  saurait plus lister les identifiants lui-même, et on retomberait sur l'analyse
  de texte qu'on vient d'écarter.

- **Deux registres, l'un pour les exigences, l'autre pour les
  fonctionnalités.** Écarté : le rôle du second est déjà tenu par le manuel, que
  la règle du projet oblige à être exhaustif sur tout ce que l'utilisateur voit.
  Trois endroits pour dire la même chose feraient deux occasions de diverger.

- **Donner un identifiant aussi aux garanties du noyau** — annulation exacte,
  aller-retour de format, politique d'ordre. Écarté : la frontière de « ce qui
  mérite un identifiant » redeviendrait une question à trancher à chaque test,
  et le registre doublerait la suite unitaire sans rien affirmer de plus. La
  portée « ce que le binaire montre » a l'avantage d'être décidable sans
  discussion.

- **Autoriser un test unitaire à couvrir une exigence.** Écarté : une exigence
  de bout en bout « couverte » par un test qui n'invoque jamais le binaire est
  exactement la confusion que le harnais existe pour dissiper.

- **Rapporter plutôt qu'échouer**, ou n'échouer que sur `implémentée` en
  laissant les autres états en avertissement. Écarté : un avertissement noyé
  dans dix minutes de sortie est un avertissement que personne ne lit. Une
  contrainte qui ne referme pas la porte n'en est pas une.

- **Ne contraindre que l'état `implémentée`**, les trois autres restant
  documentaires. Écarté de peu — c'est la lecture littérale du besoin. Mais un
  test qui cite une exigence `prévue` prouve qu'elle est implémentée et que
  l'état n'a pas été tourné ; un test qui cite une exigence `abandonnée` est un
  test resté derrière. Ces deux dérives sont mécaniquement décidables, donc il
  n'y a pas de raison de les laisser à la relecture.

## Conséquences

Ce que la décision rend facile : lire en un seul endroit ce que l'outil promet
et ce qui le prouve. Une exigence implémentée sans test ne franchit plus la
porte, et un tag qui ne désigne rien non plus.

Ce qu'elle rend contraignant : **toute issue qui ajoute un comportement visible
alimente le registre en début de travail**, avant le code. C'est le but, et
c'est une discipline qui ne tient que parce qu'un script la vérifie — la même
règle écrite dans `CLAUDE.md` aurait dérivé, comme y a dérivé la règle du
`Closes #N`.

Le harnais lance un vrai processus, ce qui a un effet à surveiller sur la
mesure de couverture : sous le preset `coverage`, `subedit-cli` est lui aussi
instrumenté et chaque invocation gonflerait la couverture de `src/lib` sans
qu'aucun test unitaire ait été écrit pour ce code. Les tests de bout en bout ne
sont donc **pas enregistrés dans CTest sous ce preset**. Ils le restent sous
`asan`, où une fuite du binaire réel se voit.

Le registre ne dit rien du noyau, et c'est délibéré. Si une garantie interne
mérite un jour d'être nommée, ce sera par une décision distincte, pas par un
glissement de portée.

Défaire la décision coûterait peu : retirer le script de la porte suffit, les
tags redeviennent des tags inertes et le registre un document. Le déclencheur
d'un réexamen est identifié — le jour où une exigence ne sera démontrable que
par l'interface graphique, le harnais devra couvrir une seconde surface, et
« un binaire, un `--list-tags` » devra devenir « plusieurs ».
