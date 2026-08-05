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
| [0007](../adr/0007-cpp23-et-std-expected.md) | C++23 et `std::expected` |
| [0008](../adr/0008-lecture-au-mieux-avec-diagnostics.md) | ouvrir au mieux, rapporter des diagnostics |
| [0009](../adr/0009-texte-en-chaine-brute.md) | texte stocké en chaîne brute |
| [0010](../adr/0010-annulation-par-commandes.md) | commandes portant leur propre inverse |

## Organisation des modules

```
src/lib/subedit/core/
├── time/        Timestamp, Duration, FrameRate, conversions, formatage
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
class FrameRate;  // rationnel exact
```

Non convertibles implicitement entre eux ni vers un entier. Arithmétique
autorisée :

```
Timestamp − Timestamp → Duration
Timestamp ± Duration  → Timestamp
Duration  ± Duration  → Duration
Duration  × entier    → Duration
Timestamp + Timestamp → ne compile pas
```

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
champs de 1 ou 2 chiffres, millisecondes de 1 à 3 chiffres, heures facultatives
en WebVTT, virgule ou point comme séparateur décimal, signe négatif accepté.

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

`duration()` est dérivée : `end - start`. Elle n'est pas stockée.

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
struct SubRipExtras  { std::optional<Rectangle> coordinates; };
struct WebVttExtras  { std::string id, settings, style, comment; };

using FormatExtras = std::variant<std::monostate, SubRipExtras, WebVttExtras>;
```

Un `variant` plutôt que le sac d'attributs dynamiques de Gaupol : le compilateur
sait quels champs existent, et l'exhaustivité des traitements est vérifiable.
`std::monostate` couvre les formats sans données propres.

### Projet

```cpp
class Project {
    std::vector<Subtitle> m_subtitles;
    FrameRate m_frameRate;          // par défaut 24000/1001, comme Gaupol
    History m_history;
    // fichier d'origine, encodage, fin de ligne, en-tête
};
```

`std::vector` plutôt qu'une structure plus savante : les accès sont
essentiellement séquentiels et par indice, les insertions et suppressions
groupées. Une structure différente devra être justifiée par une mesure.

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

L'écriture est **atomique** — écriture dans un temporaire puis renommage, comme
Gaupol. Une sauvegarde interrompue ne détruit pas le fichier existant.

### SubRip

Numérotation régénérée à l'écriture. Séparateur décimal virgule. Coordonnées
étendues `X1/X2/Y1/Y2` écrites seulement si présentes et non nulles.

Piège relevé chez Gaupol : sa lecture concatène toute ligne non horodatée au
sous-titre précédent, ce qui produit une exception non documentée quand le
premier bloc n'est pas un horodatage. Notre lecteur doit produire un diagnostic
puis chercher le bloc suivant.

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
    [[nodiscard]] virtual Change describe() const = 0;   // indices touchés, documents concernés
};
```

`History` tient deux piles de `std::unique_ptr<Command>`, un compteur de
modification par document, et sait regrouper. Une commande composite contient une
suite de commandes et ne compte que pour une entrée.

Le **compteur de modification** vaut zéro quand le document correspond au fichier
sur disque, augmente à chaque action, diminue à chaque annulation. Annuler
jusqu'au point de sauvegarde y ramène, ce qu'un booléen ne saurait pas faire.

`Change` porte les indices affectés et la nature du changement — positions,
texte principal, texte de traduction, insertion, suppression. L'interface de la
phase 5 s'en servira pour ne rafraîchir que ce qui bouge. Le noyau ne connaît
aucun mécanisme de signal : il retourne l'information, l'appelant en fait ce
qu'il veut.

Cette phase ne fournit **aucune commande concrète** — elles arrivent avec les
opérations, en phase 2. Elle fournit le socle et un test le prouvant sur une
commande factice.

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

| # | Travail | Nature |
| - | :------ | :----- |
| 1 | Types `Timestamp`, `Duration`, `FrameRate`, conversions, formatage, analyse d'horodatage | **point de contrôle** |
| 2 | `Subtitle`, `Project`, extras typés | structurante |
| 3 | `Command`, `CompositeCommand`, `History`, compteur de modification | structurante |
| 4 | Diagnostics, abstraction du système de fichiers, écriture atomique | structurante |
| 5 | Lecture et écriture SubRip | mécanique |
| 6 | Lecture et écriture WebVTT | mécanique |
| 7 | Détection de format, BOM UTF-8, fins de ligne | mécanique |
| 8 | Corpus de test et benchmarks de référence | mécanique |

**L'issue 1 est le point de contrôle convenu** : elle porte les décisions les
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
