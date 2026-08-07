# Phase 1 — Noyau : modèle de données et formats

**État :** conception validée, prête à découper
**Milestone :** `1 — Noyau : modèle et formats`
**Issue de cadrage :** #1

## Objectif

Poser le modèle de données de `subedit_core` et la lecture-écriture des deux
formats prioritaires, avec l'architecture de commandes réversibles qui portera
toutes les opérations des phases suivantes.

À l'issue de cette phase, la bibliothèque sait ouvrir un `.srt` ou un `.vtt`,
en restituer le contenu fidèlement, et le réécrire — sans interface, sans
opération d'édition, qui relèvent de la phase 2.

## Portée

**Inclus** — types de position, sous-titre, projet, données spécifiques aux
formats, lecture et écriture SubRip et WebVTT, détection de format, décodage
UTF-8, diagnostics, historique de commandes, corpus de fichiers de test.

**Exclu, par phase** — détection automatique d'encodage et jeux d'encodages
(phase 8) ; autres formats et modèle de balises riche (phase 9) ; opérations
d'édition (phase 2) ; parseur conscient des balises (phase 4, où il devient
nécessaire).

Le contour vient des priorités de l'utilisateur, rappelées dans
[`../feuille-de-route.md`](../feuille-de-route.md) : ouvrir et enregistrer en
SubRip et WebVTT, en UTF-8, avec des fins de ligne Unix.

## Décisions applicables

| ADR | Décision |
| :-- | :------- |
| [0006](../adr/0006-positions-en-millisecondes.md) | positions en millisecondes entières, types forts, fréquence rationnelle |
| [0011](../adr/0011-numero-d-image-en-type-fort.md) | numéro d'image en type fort `Frame` |
| [0007](../adr/0007-cpp23-et-std-expected.md) | C++23 et `std::expected` |
| [0008](../adr/0008-lecture-au-mieux-avec-diagnostics.md) | ouvrir au mieux, rapporter des diagnostics |
| [0009](../adr/0009-texte-en-chaine-brute.md) | texte stocké en chaîne brute |
| [0010](../adr/0010-annulation-par-commandes.md) | commandes portant leur propre inverse |

## Organisation des modules

```
src/lib/subedit/core/
├── time/        Timestamp, Duration, Frame, FrameRate, conversions, formatage
├── model/       Subtitle, Project, extras spécifiques aux formats
├── format/      lecture et écriture, détection, diagnostics
└── command/     Command, CompositeCommand, History
```

Chaque module est testable seul. `time` ne dépend de rien, `model` de `time`,
`format` de `model`, `command` de `model`.

## Types de position

```cpp
class Duration;   // millisecondes signées
class Timestamp;  // millisecondes signées, position absolue
class Frame;      // numéro d'image signé
class FrameRate;  // rationnel exact
```

Non convertibles implicitement entre eux ni vers un entier. Arithmétique
autorisée :

```
Timestamp − Timestamp → Duration
Timestamp ± Duration  → Timestamp
Duration  ± Duration  → Duration
Duration  × entier    → Duration
Frame     ± Frame     → Frame
Timestamp + Timestamp → ne compile pas
Timestamp − Frame     → ne compile pas
```

Aucune de ces classes n'a de constructeur public : on passe par des fabriques
nommées — `Timestamp::fromMilliseconds`, `Frame::fromNumber`,
`FrameRate::create` — pour que l'unité soit écrite au site d'appel.

`Frame` est le type de la « vue en images » que
[0006](../adr/0006-positions-en-millisecondes.md) nomme sans la typer ;
[0011](../adr/0011-numero-d-image-en-type-fort.md) le décide et dit pourquoi.
L'image n'ayant pas de sous-unité, un seul type sert de position et de compte,
d'où `Frame + Frame` alors que `Timestamp + Timestamp` est refusé.

`FrameRate` est un rationnel `numérateur / dénominateur`. Les huit valeurs de
Gaupol sont exactes : `24000/1001`, `24/1`, `25/1`, `30000/1001`, `30/1`,
`50/1`, `60000/1001`, `60/1`. Les fréquences personnalisées restent
représentables.

### Conversions et arrondi

Arrondi **à l'entier le plus proche, la moitié s'éloignant de zéro** — symétrique
autour de l'origine, contrairement à l'arrondi au pair le plus proche dont
Gaupol hérite de Python.

Deux garanties, à couvrir par des tests :

- **`image → millisecondes → image` est l'identité**, pour toute fréquence
  jusqu'à 60 images par seconde. À 60 im/s une image dure 16,67 ms : la grille
  des millisecondes est plus fine que celle des images. C'est ce qui rend sûr
  d'unifier la représentation interne sur les millisecondes, y compris pour les
  formats nativement en images de la phase 9.
- **`millisecondes → image → millisecondes` ne l'est pas.** La précision
  infra-image est perdue. Comportement attendu, testé comme tel.

Les calculs intermédiaires se font en `std::int64_t` : la valeur maximale
plausible est de l'ordre de `10⁷ images × 1000 × 1001`, soit ~10¹³, largement
dans les bornes.

### Formatage

`HH:MM:SS,mmm` pour SubRip, `HH:MM:SS.mmm` ou `MM:SS.mmm` pour WebVTT. Les
positions négatives portent un signe en tête. **Saturation à `99:59:59,999`** à
l'écriture, comme Gaupol — contrainte de format, pas de représentation.

L'analyse d'un horodatage est délibérément permissive, à l'image de Gaupol :
champs de 1 ou 2 chiffres, millisecondes de 1 à 3 chiffres — ou absentes —,
heures facultatives en WebVTT, virgule ou point comme séparateur décimal, signe
négatif accepté, blancs alentour ignorés.

Deux limites à cette permissivité, décidées à l'écriture :

- **des minutes ou des secondes hors de leur plage sont refusées.** `00:60:00`
  n'est pas un horodatage de 60 minutes, c'est un fichier abîmé ; le refus
  permet au lecteur d'émettre un diagnostic sur la ligne plutôt que d'inventer
  une position. L'analyse rend `std::optional<Timestamp>` — une seule cause
  d'échec, aucune information qu'un `expected` porterait en plus.
- **les heures reviennent quand la position atteint une heure**, même si
  l'appelant les avait demandées omises : `61:01.500` ne se relit pas.

## Modèle

```cpp
enum class Document { Main, Translation };

struct Subtitle {
    Timestamp start;
    Timestamp end;
    std::string mainText;         // brut, balises du format comprises
    std::string translationText;
    FormatExtras extras;
};
```

`duration()` est dérivée : `end - start`. Elle n'est pas stockée. `text(Document)`
donne accès à l'un ou l'autre texte, en lecture et en écriture, pour que les
opérations de la phase 2 nomment leur cible au lieu de la coder en dur.

**Un sous-titre porte les deux textes et une seule paire de positions.** C'est le
modèle de Gaupol, et il exprime directement le fait que la traduction n'a pas de
temps propres. Le besoin d'un document de traduction n'étant pas confirmé, le
modèle l'accueille sans que rien d'autre ne le construise.

**`end ≥ start` n'est pas un invariant.** Exception assumée aux principes de
conception, justifiée dans [0008](../adr/0008-lecture-au-mieux-avec-diagnostics.md) :
un fichier réel peut contenir un sous-titre incohérent, et l'utilisateur doit
pouvoir le voir pour le corriger. L'anomalie produit un diagnostic.

### Données spécifiques aux formats

Les deux formats du MVP portent des données qui doivent survivre à
l'aller-retour — perdre les `settings` d'une cue WebVTT, c'est perdre son
positionnement à l'écran.

```cpp
struct Rectangle     { int x1, x2, y1, y2; };
struct SubRipExtras  { std::optional<Rectangle> coordinates; };
struct WebVttExtras  { std::string id, settings, style, comment; };

using FormatExtras = std::variant<std::monostate, SubRipExtras, WebVttExtras>;
```

Un `variant` plutôt que le sac d'attributs dynamiques de Gaupol : le compilateur
sait quels champs existent, et l'exhaustivité des traitements est vérifiable.
`std::monostate` couvre les formats sans données propres.

Les coordonnées sont une **option**, et non quatre entiers nuls : Gaupol ne les
écrit que si elles ne sont pas toutes à zéro, donc « absentes » et « toutes à
zéro » sont deux états distincts qui doivent tous deux survivre à
l'aller-retour.

### Projet

```cpp
enum class Newline { Lf, CrLf, Cr };

struct SourceFile {
    std::optional<std::filesystem::path> path;   // absent si jamais ouvert ni enregistré
    Newline     newline;
    bool        hadUtf8Bom;
    std::string header;                          // WebVTT
};

class Project {
    std::vector<Subtitle> m_subtitles;
    FrameRate  m_frameRate;         // par défaut 24000/1001, comme Gaupol
    SourceFile m_sourceFile;
};
```

Les sous-titres se joignent par `subtitleAt(SubtitleIndex)`, en lecture comme
en écriture — c'est ce dont une commande a besoin pour modifier un sous-titre
sans reconstruire le vecteur entier. Un indice hors bornes lève
`std::out_of_range` : c'est une erreur de programmation, et échouer bruyamment
vaut mieux que lire ce qui suit le vecteur.

**`SubtitleIndex` est un type fort**, comme l'annoncent les principes de
conception. Deux numérotations coexistent — la position à partir de zéro, côté
code, et le numéro à partir de un, celui que l'utilisateur lit et que SubRip
écrit. Les confondre est le décalage d'un classique d'un éditeur de
sous-titres ; la conversion est écrite une fois, dans le type.

`std::vector` plutôt qu'une structure plus savante : les accès sont
essentiellement séquentiels et par indice, les insertions et suppressions
groupées. Une structure différente devra être justifiée par une mesure.

`SourceFile` n'est pas de la décoration : réécrire sans BOM ni CRLF un fichier
qui en portait réécrirait chacune de ses lignes, et l'utilisateur trouverait un
diff là où il attendait un sous-titre corrigé. Le chemin y est une **valeur**,
pas une poignée — l'accès au disque passe par l'abstraction injectée de
l'issue #5.

`Project` ne décide de rien : il détient l'état et le rend. Les modifications
passent par les commandes réversibles, qui arrivent avec les opérations. Le
format du fichier n'y figure pas encore : il apparaîtra avec la détection, à
l'issue #8, plutôt que d'être introduit ici sans utilisateur.

## Lecture et écriture

```cpp
struct Diagnostic {
    Severity     severity;   // Warning | Recovered
    int          line;       // 1-fondé
    DiagnosticKind kind;     // énumération, traduisible, testable sans comparer du texte
    std::string  detail;     // contexte facultatif
};

struct ReadResult {
    std::vector<Subtitle>   subtitles;
    std::string             header;       // WebVTT
    Newline                 newline;      // détectée, conservée
    bool                    hadUtf8Bom;
    std::vector<Diagnostic> diagnostics;
};

class SubtitleReader {
public:
    virtual ~SubtitleReader() = default;
    [[nodiscard]] virtual std::expected<ReadResult, ReadError>
    read(std::string_view content) const = 0;
};
```

**Le lecteur reçoit du contenu déjà décodé, pas un chemin.** L'accès au système
de fichiers passe par une abstraction injectée : un test lit une chaîne, sans
disque ni fichier temporaire.

Distinction stricte, posée par [0008](../adr/0008-lecture-au-mieux-avec-diagnostics.md) :

| Cas | Traitement |
| :-- | :--------- |
| Fichier inaccessible, octets invalides en UTF-8, aucun sous-titre reconnaissable | `std::unexpected` |
| Ligne ignorée, horodatage incohérent, fin avant début, chevauchement, bloc inconnu | diagnostic, lecture poursuivie |

`DiagnosticKind` énumère les anomalies que le corpus doit couvrir : ligne
ignorée, horodatage illisible, champs courts, fin avant début, chevauchement,
numérotation absente ou incohérente, texte avant tout horodatage, balise non
fermée, bloc inconnu, fins de ligne mélangées. `Severity` distingue ce que le
lecteur a **laissé tel quel** faute de pouvoir décider — `Warning` — de ce sur
quoi il a **tranché**, comme une numérotation régénérée : `Recovered`.

Le numéro de ligne reste un `int`. C'est une exception délibérée à la règle des
types forts : il n'est jamais qu'affiché, jamais mêlé à un calcul avec un indice
de sous-titre, et un type de plus n'achèterait rien ici.

### Système de fichiers

```cpp
class FileSystem {                       // uniquement des primitives
    virtual bool exists(path) const = 0;
    virtual expected<string, FileError> readFile(path) const = 0;
    virtual expected<void, FileError>   writeFile(path, string_view) = 0;
    virtual expected<void, FileError>   rename(from, to) = 0;
    virtual expected<void, FileError>   remove(path) = 0;
};

expected<void, FileError> writeAtomically(FileSystem&, path, string_view);
```

L'interface est **délibérément primitive**. Écrire sans risque est une
*politique*, pas une primitive : `writeAtomically` la construit au-dessus de ces
opérations, de sorte qu'elle s'écrive une fois et se teste, au lieu d'être
réimplémentée — et éventuellement simulée — par chaque implémentation. Le même
raisonnement sort `fileErrorKindOf` de l'ombre : la correspondance
`std::error_code` → `FileErrorKind` est partagée, donc testable contre les codes
eux-mêmes plutôt que contre un périphérique qu'il faudrait faire échouer.

Trois valeurs seulement dans `FileErrorKind` — `NotFound`, `PermissionDenied`,
`Io` — parce que trois sont ce sur quoi un appelant peut agir : recommencer
ailleurs, demander des droits, ou renoncer.

Deux implémentations. `RealFileSystem` ouvre en binaire, un flux qui
traduirait les fins de ligne ruinant la détection. `InMemoryFileSystem` **vit
dans la bibliothèque**, et non dans les tests : tout module touchant aux
fichiers en a besoin, et sa capacité à échouer sur commande est la seule façon
honnête de tester une sauvegarde interrompue — attendre un vrai disque plein
n'est pas un test.

L'écriture est **atomique** — écriture dans un temporaire voisin puis renommage,
comme Gaupol. Une sauvegarde interrompue ne détruit pas le fichier existant, et
le temporaire est retiré si le renommage échoue, pour ne rien laisser traîner à
côté du fichier.

**Limite à connaître :** l'atomicité porte sur le *remplacement*, un renommage
dans un répertoire étant atomique. Elle ne garantit pas la *durabilité* face à
une coupure de courant, qui demanderait de vider le fichier et son répertoire
sur le périphérique. Gaupol ne le fait pas davantage, et la panne dont il s'agit
de protéger l'utilisateur est la fréquente : une sauvegarde qui échoue en cours
de route.

Deux lignes de `RealFileSystem` restent non couvertes — les vérifications qui
suivent une lecture et une fermeture de flux. Les atteindre demanderait un
périphérique défaillant en cours d'opération ; elles sont conservées comme
gardes, et signalées ici plutôt que retirées pour faire un chiffre.

### Découpage en lignes

```cpp
std::vector<std::string_view> splitLines(std::string_view content);
```

Partagé par les formats, qui sont tous en lignes. Les trois fins de ligne sont
reconnues, aucune n'entre dans le texte, et une fin de ligne finale n'invente
pas de ligne vide. Les vues pointent dans le contenu : un fichier de plusieurs
milliers de sous-titres se découpe sans copier une seule ligne.

### Écriture

```cpp
struct WriteRequest {
    std::span<const Subtitle> subtitles;
    Document document;   // lequel des deux textes part dans le fichier
    Newline  newline;    // LF par défaut
};

class SubtitleWriter {
    virtual std::string write(const WriteRequest&) const = 0;
};
```

**Un écrivain rend une chaîne, il ne touche pas au disque.** L'y porter est
l'affaire de `writeAtomically`, qui sait ne pas détruire la version
précédente — une préoccupation qu'aucun format n'a à répéter.

La fin de ligne est celle qu'on lui donne, LF par défaut, comme le demande la
feuille de route. Celle du fichier d'origine n'est pas perdue pour autant : elle
est conservée dans `SourceFile`, et c'est l'appelant qui décide de la remettre,
plutôt que l'écrivain qui le ferait dans son dos.

### SubRip

Numérotation régénérée à l'écriture. Séparateur décimal virgule. Coordonnées
étendues `X1/X2/Y1/Y2` écrites seulement si présentes et non nulles.

**Ce qui ouvre un sous-titre est la ligne d'horodatage** — ni le numéro
au-dessus, ni la ligne vide avant. Les fichiers réels ont des numéros absents,
des numéros qui ne se suivent pas, et séparent leurs blocs comme ils veulent ;
seul l'horodatage est fiable. Le numéro est donc repris *a posteriori* parmi les
lignes en attente, puisqu'il appartient au sous-titre qui commence et non à
celui qui finit.

Piège relevé chez Gaupol : sa lecture concatène toute ligne non horodatée au
sous-titre précédent, ce qui produit une exception non documentée quand le
premier bloc n'est pas un horodatage — une note de traducteur en tête de fichier
suffit. Notre lecteur émet un diagnostic par ligne et cherche le bloc suivant.

Deux règles de prudence, décidées à l'écriture du lecteur :

- **une ligne qui voulait être un horodatage sans y parvenir est signalée puis
  conservée comme texte.** Détruire une ligne qu'on n'a pas comprise serait pire
  que de la montrer telle quelle ;
- **des coordonnées à moitié écrites disqualifient la ligne entière.** Garder
  les horodatages et jeter le reste serait une décision qu'on n'a aucun titre à
  prendre.

À l'écriture, la disposition est celle de Gaupol, ligne vide close comprise, y
compris après le dernier bloc. D'où la forme exacte de la garantie
d'aller-retour :

- **fidèle octet pour octet** sur un fichier déjà dans cette disposition ;
- **idempotent** sinon — le premier enregistrement normalise, aucun ensuite ne
  touche plus rien.

La numérotation est la seule chose qui ne revient pas à l'identique, et c'est
voulu : c'est ce qui fait qu'un fichier dont les numéros ne se suivent pas se
soigne en étant ouvert puis enregistré.

### WebVTT

Analyse par blocs séparés par des lignes vides, avec les états `en-tête`,
`style`, `commentaire`, `identifiant`, `horodatage`, `texte`. Les blocs `STYLE`
et `NOTE` sont rattachés au sous-titre qui suit.

À l'écriture : en-tête `WEBVTT`, identifiant de cue s'il existe, réglages
accolés à la ligne d'horodatage, blocs `STYLE` et `NOTE` restitués. Les heures
sont omises si tous les sous-titres se terminent avant une heure, comme Gaupol.

### Détection de format

Par motif sur le contenu, à l'image de Gaupol. En cas d'ambiguïté, le format le
plus spécifique l'emporte ; en cas d'échec, une erreur de lecture explicite,
jamais un format supposé.

### Encodage

UTF-8 seul dans cette phase. Le **BOM UTF-8 est détecté**, retiré à la lecture
et restitué à l'écriture s'il était présent — il se rencontre couramment, même
si les autres encodages relèvent de la phase 8. Le type de fin de ligne est
détecté et conservé ; l'écriture utilise LF.

## Commandes réversibles

```cpp
class Command {
public:
    virtual ~Command() = default;
    virtual void apply(Project&) = 0;
    virtual void revert(Project&) = 0;
    [[nodiscard]] virtual std::vector<Change> describe() const = 0;
};

struct Change {
    ChangeKind                 kind;      // Positions | MainText | TranslationText
                                          // | Insertion | Removal
    std::vector<SubtitleIndex> indices;
};

bool affects(ChangeKind, Document);
```

`describe()` rend **une liste** et non un `Change` unique : une commande
composite rapporte ce que rapportent toutes celles qu'elle contient, et l'union
se fait sans que le socle ait à fusionner des indices.

`Change` porte les indices affectés et la nature du changement. L'interface de
la phase 5 s'en servira pour ne rafraîchir que ce qui bouge. Le noyau ne connaît
aucun mécanisme de signal : il retourne l'information, l'appelant en fait ce
qu'il veut.

Les documents concernés se **déduisent** de la nature du changement, plutôt
qu'être portés en double : un sous-titre n'a qu'une paire de positions pour ses
deux textes, donc un déplacement périme les deux documents, comme une insertion
ou une suppression. Seul un changement de texte ne concerne qu'un document.

`History` tient deux piles de `std::unique_ptr<Command>`, un compteur de
modification par document, et sait regrouper. Une commande composite contient une
suite de commandes, ne compte que pour une entrée, et **s'annule dans l'ordre
inverse** — défaire « insérer puis décaler » en commençant par l'insertion
décalerait des sous-titres qui n'existent plus.

Le **compteur de modification** vaut zéro quand le document correspond au fichier
sur disque, augmente à chaque action, diminue à chaque annulation. Annuler
jusqu'au point de sauvegarde y ramène, ce qu'un booléen ne saurait pas faire. Il
devient négatif si l'on annule au-delà d'une sauvegarde, ce qui est un écart
comme un autre.

**`History` n'est pas un membre de `Project`.** L'ADR 0010 dit que l'historique
appartient au projet ; il faut l'entendre comme « il relève du noyau, pas de
l'interface », ce que la comparaison avec `QUndoStack` établit. Deux raisons
concrètes s'opposent à l'inclusion : `model` ne dépendrait plus seulement de
`time` mais aussi de `command`, à rebours de l'organisation des modules ; et
surtout un `Project` cesserait d'être copiable, puisqu'une pile de
`unique_ptr` ne se copie pas. L'appelant tient les deux ensemble.

La **taille de l'historique est bornée**, à 1000 entrées par défaut. Le chiffre
est **provisoire** : l'ADR 0010 promet de mesurer l'empreinte réelle avant de le
fixer, et aucune commande concrète n'existe encore pour être mesurée. La borne
est là pour que la mémoire le soit ; le nombre se décidera en phase 2.

Cette phase ne fournit **aucune commande concrète** — elles arrivent avec les
opérations, en phase 2. Elle fournit le socle et des tests le prouvant sur des
commandes factices : l'une qui modifie réellement un projet, l'autre qui note
l'ordre dans lequel on l'a appelée.

## Tests

Le corpus vit dans `src/test/data/`, organisé en `valides/` et `malformes/`.

Cas malformés à couvrir au minimum : fichier vide ; BOM UTF-8 ; fins de ligne
mélangées ; horodatage à champs courts ; fin antérieure au début ; sous-titres
qui se chevauchent ; numérotation absente ou incohérente en SubRip ; ligne de
texte avant tout horodatage ; balise non fermée ; bloc WebVTT inconnu ; octets
invalides en UTF-8.

Des fichiers issus d'un usage réel valent mieux que des fichiers fabriqués.

Benchmarks à poser dès cette phase, car ils serviront de référence :
lecture d'un `.srt` de plusieurs milliers de sous-titres, écriture du même,
conversion de fréquence d'image sur l'ensemble.

## Découpage en issues

Dans l'ordre de traitement. Les numéros sont ceux des issues GitHub.

| Issue | Travail | Nature |
| :---- | :------ | :----- |
| #2 | Types `Timestamp`, `Duration`, `FrameRate`, conversions, formatage, analyse d'horodatage | **point de contrôle** |
| #3 | `Subtitle`, `Project`, extras typés | structurante |
| #4 | `Command`, `CompositeCommand`, `History`, compteur de modification | structurante |
| #5 | Diagnostics, abstraction du système de fichiers, écriture atomique | structurante |
| #6 | Lecture et écriture SubRip | mécanique |
| #7 | Lecture et écriture WebVTT | mécanique |
| #8 | Détection de format, BOM UTF-8, fins de ligne | mécanique |
| #9 | Corpus de test et benchmarks de référence | mécanique |

L'issue #1 est celle du cadrage, close par la présente spec.

**L'issue #2 est le point de contrôle convenu** : elle porte les décisions les
plus coûteuses à défaire, et sera relue avant que le reste s'appuie dessus. Les
issues 2 à 4 restent structurantes ; les suivantes sont à boucle courte, validées
par le corpus.

## Critères de fin

- [ ] Un `.srt` et un `.vtt` réels s'ouvrent, se relisent et se réécrivent à
      l'identique octet pour octet, hors renumérotation
- [ ] Les fichiers malformés du corpus produisent les diagnostics attendus, sans
      exception ni perte silencieuse
- [ ] `image → ms → image` est vérifié exact pour les huit fréquences
- [ ] L'historique annule et rétablit une commande factice, groupée et isolée,
      et le compteur de modification revient à zéro au point de sauvegarde
- [ ] `make check` passe, couverture des modules au-dessus du seuil
- [ ] Benchmarks de lecture, d'écriture et de conversion de fréquence en place
- [ ] Section de manuel et entrée de CHANGELOG à jour
