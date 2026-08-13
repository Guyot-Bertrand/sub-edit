# Phase 2 — Opérations d'édition

**État :** conception validée, prête à découper
**Milestone :** `2 — Opérations d'édition`

## Objectif

Donner au noyau les opérations que l'édition dans une table suppose, et les
trois opérations de positions listées en priorité. Toutes annulables, toutes
écrites comme des commandes sur le socle de la phase 1.

À l'issue de cette phase, on peut modifier un sous-titre, en insérer, en
supprimer, décaler, transformer et convertir la fréquence d'image — sans
interface, qui relève de la phase 5.

## Portée

**Inclus** — modifier un texte, un début, une fin ; insérer, supprimer ;
décaler, transformer par deux points de repère, convertir la fréquence
d'image ; trier ; la cible d'une opération ; le type qui réunit un projet et
son historique.

**Exclu, par phase** — ajustement des durées, casse, italiques, tirets de
dialogue, fusion, scission, recherche et remplacement, presse-papiers
(phase 10) ; suppression des mentions pour malentendants (phase 4) ; toute
interface (phases 3 et 5).

## Décisions applicables

| ADR | Décision |
| :-- | :------- |
| [0006](../adr/0006-positions-en-millisecondes.md) | positions en millisecondes entières, types forts |
| [0010](../adr/0010-annulation-par-commandes.md) | commandes portant leur propre inverse |
| [0011](../adr/0011-numero-d-image-en-type-fort.md) | numéro d'image en type fort |
| [0012](../adr/0012-ordre-des-sous-titres-par-composition.md) | ne pas trier de soi-même, mode strict par composition |
| [0013](../adr/0013-mise-a-l-echelle-exacte-des-positions.md) | mise à l'échelle par un rationnel exact, arrondi une fois |

Les deux dernières sont propres à cette phase et ont été écrites avant elle.

## Ce qui manque au noyau

L'état des lieux, fait avant d'écrire cette spec, a relevé quatre manques et
une incohérence. Ils commandent le découpage.

| Manque | Conséquence |
| :----- | :---------- |
| aucune mise à l'échelle d'une position | transformation et conversion de fréquence inexprimables |
| `Project` ne sait ni insérer ni supprimer | deux opérations sur sept sans support |
| rien ne réunit un projet et son historique | toute opération a besoin des deux |
| une commande n'a pas de nom | la phase 5 n'aurait rien à afficher |

L'incohérence : l'ADR 0010 pose que modifier un projet hors d'une commande est
un défaut, alors que `setSubtitles`, `setFrameRate` et l'accès mutable à un
sous-titre sont publics et sans garde-fou. Traitée plus bas.

## Mise à l'échelle des positions

```cpp
class Ratio;   // rationnel exact, signé, réduit

Timestamp Timestamp::scaledBy(Ratio) const;
Duration  Duration::scaledBy(Ratio) const;
```

Une méthode nommée plutôt qu'un opérateur : mettre une position à l'échelle est
une **décision d'arrondi**, et l'ADR 0006 veut qu'aucune ne se prenne
implicitement.

**L'aller-retour par les images ne convient pas.** On pourrait croire que
convertir une fréquence s'écrit `fromFrame(toFrame(t, entrée), sortie)`. C'est
faux : ce chemin quantifie chaque position sur la grille d'images d'entrée,
perte que la spec de la phase 1 documente déjà — `ms → image → ms` n'est pas
l'identité. Mesuré de 25 vers 24000/1001, l'écart avec le calcul exact atteint
**21 ms**, soit une demi-image :

| position | par les images | exact | écart |
| -------: | -------------: | ----: | ----: |
| 1010 ms | 1043 | 1053 | 10 ms |
| 1020 ms | 1084 | 1064 | 20 ms |

La règle est donc : **un seul arrondi, sur un rationnel exact.** Les calculs
intermédiaires tiennent dans un `std::int64_t` — une position plausible ne
dépasse pas `3,6 × 10⁸` ms, et un rationnel réduit garde un numérateur du même
ordre.

## Cible d'une opération

```cpp
class Selection;   // indices triés, sans doublon

Selection::all(const Project&);
Selection::of(std::span<const SubtitleIndex>);
Selection::range(SubtitleIndex first, SubtitleIndex last);
```

Gaupol répète dans chaque signature un paramètre `indices` qui vaut `None` pour
« tout le projet ». Une valeur absente qui signifie « tous » est exactement le
genre de convention que le compilateur devrait porter : `Selection::all` le dit.

Trié et sans doublon **par construction**, ce dont l'insertion et la suppression
ont besoin pour être justes.

## Session

```cpp
enum class OrderPolicy { Lenient, Strict };

class Session {
    Project     m_project;
    History     m_history;
    OrderPolicy m_policy = OrderPolicy::Lenient;
};
```

Le type qui manquait. L'issue #4 a écarté de mettre `History` dans `Project` —
`model` dépendrait de `command`, et surtout un `Project` cesserait d'être
copiable. `Session` les réunit sans les mêler : c'est ce qu'une application
tient par fichier ouvert, et ce que la ligne de commande de la phase 3 comme la
fenêtre de la phase 5 manipuleront.

Elle vit dans un cinquième module, `core/edit/`, qui héberge aussi les
opérations.

**`Session::project()` rend une référence constante.** C'est la réponse à
l'incohérence relevée plus haut : à l'intérieur d'une session, le seul chemin
vers une modification est une commande, et le compilateur le tient. Un `Project`
nu reste modifiable — le lecteur doit bien le remplir — mais qui en tient un est
en train de le construire, pas de l'éditer. L'ADR 0010 s'entend ainsi, et la
frontière est vérifiable au lieu d'être promise.

## Politique d'ordre

**`Lenient` par défaut.** Le noyau ne trie jamais de lui-même : il rapporte et
ne répare pas, comme l'ADR 0008 le pose pour la lecture. Un fichier ouvert garde
l'ordre de son fichier, et une édition ne déplace aucune ligne.

**`Strict` est une politique de composition, pas un invariant de modèle.** Après
une opération susceptible de rompre l'ordre, la session enchaîne une commande de
tri dans la même composite : une seule entrée d'historique, une seule annulation.

Ce choix, plutôt qu'un invariant tenu par `Project` :

- **les indices restent stables à l'intérieur d'une commande.** `Change`
  transporte des `SubtitleIndex`, l'historique rejoue des commandes, la table se
  rafraîchira par indice. Si une édition pouvait déplacer une ligne en silence,
  *toute* commande deviendrait « et peut-être un déplacement » ;
- **un seul modèle de données.** Deux invariants selon un mode, ce ne serait pas
  un réglage mais deux modèles, et chaque commande devrait demander lequel ;
- **le tri devient annulable**, ce qu'il n'est pas chez Gaupol, où il précède
  l'existence de l'historique ;
- **le compteur de modification dit la vérité.** Ouvrir un fichier désordonné en
  mode strict marque le document modifié — ce qu'il est, puisqu'il diffère du
  disque.

Prix assumé : quand le tri se déclenche, la composite déplace des lignes et son
`Change` doit l'exprimer. La différence est que ce cas vient d'une commande
identifiée, pas de n'importe laquelle.

### Ce que Gaupol fait, et pourquoi nous nous en écartons

Gaupol maintient l'ordre à deux endroits seulement : à l'ouverture, où il trie
et avertit par une boîte de dialogue ; et sur `set_start`. `replace_positions` —
par où passent le décalage, la transformation et la conversion de fréquence — ne
trie pas. Décaler une partie d'une sélection peut donc y désordonner le fichier
sans que rien ne le remarque : **l'invariant n'en est pas un.**

Son comptage a de plus un trou. Il compte en images ce qu'il trie en secondes :
deux sous-titres inversés de moins d'une image ne sont pas comptés, aucune boîte
de dialogue ne s'ouvre, et le fichier est pourtant réordonné.

Notre modèle de diagnostics répond mieux à ce besoin qu'un dialogue : un
`Diagnostic` est un **état** qui persiste dans le `ReadResult`, non un événement
qui disparaît une fois la fenêtre fermée.

### Ce que la lecture doit apprendre à dire

`DiagnosticKind` gagne `OutOfOrder` — un sous-titre dont le début précède celui
du sous-titre précédent. Nous signalons déjà le chevauchement, qui est plus
faible. Comme toute valeur de cette énumération, elle est émise par un lecteur :
une valeur que personne ne produit est une promesse non tenue.

`Project` gagne une requête pure, `outOfOrder()`, qui rend les indices concernés.
L'interface s'en sert pour montrer le désordre sans que la règle vive dans
l'interface.

## Les opérations

Toutes sont des commandes. Chacune capture à la construction ce qu'il faut pour
se défaire, et rien de plus.

| Opération | Ce que l'inverse retient |
| :-------- | :----------------------- |
| modifier un texte | l'ancien texte, l'indice, le document |
| modifier un début, une fin | l'ancienne position |
| insérer | les indices insérés |
| supprimer | les sous-titres retirés et leurs indices |
| décaler | le décalage, à appliquer en sens inverse |
| transformer | les positions antérieures de la sélection |
| convertir la fréquence | les positions antérieures, et l'ancienne fréquence |
| trier | l'ordre antérieur |

Le décalage retient un `Duration` et non les positions antérieures : son inverse
est exact, et c'est l'économie que l'ADR 0010 réclame. La transformation et la
conversion, elles, arrondissent — leur inverse ne peut pas être un second calcul
et doit retenir les positions.

### Insertion

Insérer sans contenu crée des sous-titres vides, à la manière de Gaupol : ils se
partagent la fenêtre libre avant le sous-titre suivant, ou durent trois secondes
s'ils sont ajoutés à la fin.

### Transformation par deux points de repère

L'utilisateur désigne deux sous-titres et la position que leur début doit
prendre. La correction est affine :

```
t′ = arrondi( (t − x₁) × r ) + y₁        avec r = (y₂ − y₁) / (x₂ − x₁)
```

Écrite ainsi plutôt que `arrondi(r × t) + constante` — la forme de Gaupol —
pour deux raisons : **un seul arrondi**, et surtout **les deux repères tombent
exactement où l'utilisateur les a demandés**, puisque `(x₂ − x₁) × r` vaut
`y₂ − y₁` exactement. La forme de Gaupol ne le garantit pas.

`r` est un `Ratio`, donc exact. Un dénominateur nul — deux repères sur le même
sous-titre — est refusé à la construction.

### Conversion de fréquence d'image

```
t′ = arrondi( t × (entrée / sortie) )
```

C'est la sémantique de Gaupol : un film masterisé à 23,976 et lu à 25 images par
seconde défile plus vite, donc les positions avancent. La commande change aussi
la fréquence du projet, et son inverse restitue les deux.

## Nom d'une commande

```cpp
enum class CommandKind { SetText, SetStart, SetEnd, Insert, Remove,
                         Shift, Transform, ConvertFrameRate, Sort };

CommandKind Command::kind() const;
```

Une énumération et non une chaîne, pour la raison qui vaut déjà pour
`DiagnosticKind` : elle se traduit, et un test s'y accroche sans comparer une
prose qui sera reformulée. C'est ce que la phase 5 affichera dans « Annuler :
décalage des positions ».

## Ce que `Project` gagne

```cpp
void insert(SubtitleIndex at, std::span<const Subtitle>);
std::vector<Subtitle> remove(const Selection&);   // rend ce qui a été retiré
std::vector<SubtitleIndex> outOfOrder() const;
```

`remove` rend les sous-titres retirés : c'est exactement l'état que la commande
inverse doit retenir, et le rendre évite de le relire avant de l'effacer.

## Tests

Pour chacune des huit opérations : appliquer puis annuler restitue l'état
**exact**, et rétablir le reproduit. C'est la propriété que l'issue #4 a prouvée
sur des commandes factices ; elle se vérifie ici sur les vraies.

Cas qui ne se devinent pas et doivent être couverts :

- les **deux repères d'une transformation tombent exactement** sur les positions
  demandées ;
- une conversion de fréquence donne le **même résultat que le calcul rationnel
  exact**, et non celui de l'aller-retour par les images ;
- un **décalage partiel** qui rompt l'ordre : signalé en `Lenient`, trié en
  `Strict`, et annulable dans les deux cas ;
- une suppression suivie d'une annulation restitue les sous-titres **à leurs
  indices d'origine**, y compris pour une sélection discontinue ;
- le **compteur de modification** revient à zéro en annulant jusqu'au point de
  sauvegarde, à travers une composite.

Le benchmark de conversion de fréquence posé à l'issue #9 mesure l'aller-retour
par les images, c'est-à-dire une opération que cette phase n'implémentera pas.
Il est à refaire.

## Découpage en issues

Dans l'ordre de traitement.

| Travail | Nature |
| :------ | :----- |
| `Ratio`, mise à l'échelle de `Timestamp` et `Duration` | structurante |
| `Selection`, `Project::insert` / `remove` / `outOfOrder`, diagnostic `OutOfOrder` | structurante |
| `Session`, `CommandKind`, politique d'ordre, commande de tri | **point de contrôle** |
| modifier un texte, un début, une fin | mécanique |
| insérer, supprimer | mécanique |
| décaler | mécanique |
| transformer par deux repères | mécanique |
| convertir la fréquence d'image | mécanique |
| benchmarks refaits, section de manuel | mécanique |

La troisième est le point de contrôle : elle fixe la frontière entre le modèle
et la politique, et tout ce qui suit s'y appuie.

## Critères de fin

- [ ] Les huit opérations s'annulent et se rétablissent à l'identique
- [ ] Les deux repères d'une transformation tombent exactement sur leur cible
- [ ] La conversion de fréquence égale le calcul rationnel exact
- [ ] Un décalage partiel rompant l'ordre est signalé en souple, trié en strict
- [ ] `Session::project()` est constante : aucune modification hors commande
- [ ] `make check` passe, cliquet de couverture tenu
- [ ] Benchmarks refaits, section de manuel et entrée de CHANGELOG à jour
