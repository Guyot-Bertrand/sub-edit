# Principes de conception

Règles permanentes, applicables à tout le code de `subedit`. Elles priment sur
les habitudes personnelles et sur la commodité du moment. Les specs de chaque
sous-projet s'y réfèrent plutôt que de les répéter.

## 1. Le compilateur est le premier test

**Un type flou est un bug différé.** Toute donnée porteuse de sens a son type ;
la validation par le compilateur ne se contourne pas au profit d'un conteneur
générique.

Interdit, quelle qu'en soit la commodité :

- une `map<string, any>` ou une `map<string, variant<…>>` comme modèle de
  données ou comme configuration ;
- un `int` nu pour représenter une durée, une position, un numéro de sous-titre
  ou un indice de piste ;
- une `string` employée comme énumération ;
- une structure dont la moitié des champs est valide selon un champ « type ».

Le contre-exemple est dans le projet d'origine : la configuration de Gaupol est
un dictionnaire imbriqué d'une trentaine de sections dont les valeurs sont
arbitraires, et chaque lecture est un pari sur le type. Chez nous, c'est une
structure typée, validée au chargement, fausse impossible à compiler.

Corollaires :

- **types forts** plutôt qu'alias : `Timestamp`, `Duration`, `FrameRate`,
  `SubtitleIndex` sont des types distincts, non convertibles implicitement entre
  eux ni vers `int64_t`. Confondre une durée et une position devient une erreur
  de compilation, pas un bug de timing découvert par l'utilisateur ;
- **`enum class`** pour tout ensemble fermé de valeurs : format, mode, fin de
  ligne, cible d'opération ;
- **rendre les états invalides non représentables** : préférer `std::optional`,
  `std::variant` et des constructeurs qui valident, plutôt qu'un objet
  construisible dans un état incohérent puis « initialisé » ensuite.

## 2. Propriété mémoire explicite

La règle tient en une phrase : **à tout instant, il doit être évident qui possède
quoi.**

- **Sémantique de valeur par défaut.** Une allocation dynamique doit se
  justifier ; l'absence d'allocation ne se justifie pas.
- **Aucun `new` ni `delete` nu.** `std::unique_ptr` pour la propriété
  polymorphique, `std::make_unique` pour la création.
- **Pointeurs et références nus = observateurs non propriétaires**, dont la durée
  de vie est garantie par la conception, jamais par la chance.
- **`std::shared_ptr` seulement si la propriété partagée est démontrée**, pas
  parce que la durée de vie est confuse. Une propriété confuse se corrige, elle
  ne s'enrobe pas.
- **Règle de zéro.** Une classe qui ne gère pas de ressource ne déclare aucun
  destructeur, constructeur de copie ou opérateur d'affectation. Si elle en
  déclare un, elle les déclare tous.
- **Exception documentée : Qt.** La relation parent-enfant de `QObject` est un
  mécanisme de propriété légitime, limité au code Qt et signalé en commentaire.

Ces règles ne reposent pas sur la vigilance : LeakSanitizer, embarqué dans le
preset `asan`, fait échouer les tests sur toute fuite, et clang-tidy rejette les
allocations propriétaires nues.

## 3. Abstraction là où la variation est connue

SOLID mal appliqué produit une interface par classe, des fabriques partout et un
code plus rigide que celui qu'il remplace. La règle retenue : **on introduit une
abstraction quand la variation est réelle et identifiée, pas par principe.**

Dans `subedit`, elle l'est à cinq endroits, connus dès maintenant :

| Point de variation | Pourquoi |
| :----------------- | :------- |
| Lecture et écriture de formats | neuf implémentations dès le sous-projet 1 |
| Moteur d'expressions régulières | PCRE2 contre RE2, non tranché |
| Lecteur vidéo | libmpv contre QtMultimedia, non tranché |
| Correcteur orthographique | hunspell, nuspell, ou service système |
| Accès au système de fichiers | remplaçable en test, sans écrire sur disque |

Partout ailleurs, une classe concrète suffit jusqu'à preuve du contraire. Extraire
une interface plus tard coûte peu ; se traîner une indirection inutile coûte à
chaque lecture du code.

Les autres principes, appliqués sans dogmatisme :

- **Responsabilité unique** — un fichier long est un signal, pas une faute en
  soi. La question n'est pas le nombre de lignes mais : peut-on décrire ce que
  fait cette unité en une phrase sans « et » ?
- **Substitution de Liskov** — une implémentation qui doit lever « non
  supporté » signale une interface mal découpée. Le cas se présentera avec les
  formats : tous ne portent pas d'en-tête, ne gèrent pas les mêmes balises, et ne
  travaillent pas tous en temps. C'est le modèle qui doit l'exprimer, pas une
  exception à l'exécution.
- **Inversion de dépendance** — le cœur ne connaît ni Qt, ni le disque, ni le
  réseau. Cette contrainte est déjà vérifiée par la CI.
- **DRY porte sur la connaissance, pas sur les caractères.** Deux fragments
  identiques par coïncidence restent séparés ; factoriser une règle métier
  dupliquée est en revanche impératif.

## 4. Testabilité par construction

- Tout le code utile vit en bibliothèque ; les exécutables ne contiennent que du
  câblage. Ce qui n'est pas testable est signe d'une frontière mal placée.
- Les dépendances externes — horloge, système de fichiers, processus, lecteur
  vidéo — passent par une abstraction injectée, afin qu'un test n'ait besoin ni
  de disque, ni de réseau, ni d'attente.
- Une fonction pure est préférable à une méthode qui mute un état partagé, y
  compris quand la seconde est plus courte à écrire.

## 5. Performance : mesurer, pas supposer

La performance est un objectif transversal, ce qui n'autorise pas
l'optimisation spéculative.

- Choisir d'emblée la structure de données adaptée relève de la conception, pas
  de l'optimisation : c'est ce qui est attendu.
- Toute optimisation qui **complique le code** doit être appuyée par un
  benchmark, versionné dans `src/test/bench/`, montrant le gain.
- Éviter les copies gratuites relève de l'hygiène : `string_view` et `span` en
  paramètre, `move` quand le transfert est réel, allocations hors des boucles.
- Aucun chiffre de performance n'est annoncé sans mesure à l'appui.
