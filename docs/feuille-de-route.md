# Feuille de route

Huit sous-projets, chacun associé à un milestone GitHub. Ce document tient le
cadrage amont : ce qu'il faut analyser, ce qu'il faut trancher, et ce qui sera
difficile. Il est révisé au fil du projet — une phase terminée voit son cadrage
remplacé par sa spec dans [`specs/`](specs/).

## Déroulé d'une phase

Chaque phase s'ouvre par une **issue de cadrage** (`type:task`), dont le travail
est :

1. **Analyse préalable** — lecture ciblée du code de Gaupol correspondant, dans
   `reference/gaupol`, pour comprendre ce qui est fait et pourquoi.
2. **Discussion des choix d'architecture** — les questions listées ci-dessous
   sont le point de départ, pas la liste complète ; l'analyse en fait
   généralement émerger d'autres.
3. **Production de la spec** dans `docs/specs/NN-<sujet>.md`, et des ADR pour
   les décisions coûteuses à revenir dessus.
4. **Découpage en issues d'implémentation**, rattachées au milestone.

Une phase n'est close que lorsque sa spec, ses tests, ses benchmarks, sa section
de manuel et son entrée de CHANGELOG existent.

**L'analyse se fait au démarrage de la phase concernée, pas maintenant.** Les
questions et points difficiles listés ci-dessous sont des repères relevés lors de
l'exploration initiale de Gaupol : ils servent à ne pas partir d'une page blanche
et à ne pas découvrir tard un obstacle connu. Ce ne sont ni des conclusions, ni
une liste close — le cadrage les confirme, les corrige et les complète.

Ordre prévu : 0 → 1 → 2 → 3, ce qui donne une bibliothèque et un outil en ligne
de commande mesurables. Ensuite 5 et 6 rendent l'application utilisable, 4
apporte la valeur métier. **L'arbitrage entre 4 et 5-6 dépend des priorités
réelles de l'utilisateur final** et reste ouvert.

---

## 1 — Noyau : modèle de données et formats

Le fondement de tout le reste : sous-titre, document, projet, positions, les neuf
formats, les balises, les encodages, et l'architecture de commandes réversibles.

**Analyse préalable** — `aeidon/` : `subtitle.py`, `position.py`, `calculator.py`,
`project.py`, `containers.py`, `revertable.py`, `file.py`, `files/*.py`,
`markup.py`, `markups/*.py`, `parser.py`, `encodings.py`.

**Questions d'architecture**

- **Représentation des positions.** Millisecondes entières en interne, frames
  dérivées du framerate ? Ou représentation duale ? MicroDVD stocke des frames,
  et le framerate est modifiable après ouverture — les allers-retours ne doivent
  pas dériver. Quelle politique d'arrondi, et quelle garantie sur
  `frames → ms → frames` ?
- **Modèle de balises.** Gaupol convertit d'un format à l'autre via une
  représentation intermédiaire. Mais ASS porte du positionnement, des styles
  nommés et des effets qui n'ont aucun équivalent en SubRip. La conversion est
  **structurellement à perte** : quelle politique de dégradation, et le
  signale-t-on à l'utilisateur ?
- **Gestion d'erreurs.** Exceptions, codes de retour, ou type résultat ?
  `std::expected` est C++23 et indisponible avec GCC 13 en C++20. Décision par
  ADR, elle imprègne toute l'API.
- **Architecture d'annulation.** Gaupol stocke dans chaque action une *fonction
  inverse* et ses arguments (`RevertableAction.revert_function`). Alternative :
  des commandes qui capturent l'état antérieur. La première est économe en
  mémoire mais impose que chaque opération sache s'inverser exactement ; la
  seconde est robuste mais coûteuse sur un remplacement global dans un fichier de
  plusieurs milliers de lignes. À trancher par mesure.
- **Détection d'encodage.** Gaupol s'appuie sur `charset-normalizer`, qui n'a pas
  d'équivalent direct en C++. Candidats : ICU, `uchardet`, `compact_enc_det`.
- **Tolérance au parsing.** Les fichiers réels sont malformés. Échec net, ou
  récupération avec rapport de diagnostics ? Le choix conditionne la signature de
  toutes les fonctions de lecture.

**Points difficiles**

- Le modèle *projet* porte **deux documents**, principal et traduction, qui
  **partagent les positions**. La traduction n'a pas de temps propres. Ce n'est ni
  deux projets, ni un projet à deux colonnes de texte indépendantes.
- SSA/ASS n'est pas un format de timing mais un format structuré avec sections,
  styles nommés et événements typés. Jusqu'où le modéliser sans contaminer le
  modèle commun ? C'est le test de la substitution de Liskov annoncé dans les
  principes de conception.
- Une interface de format unique doit accommoder : avec ou sans en-tête, temps ou
  frames, jeux de balises disjoints. Si une implémentation doit lever « non
  supporté », le découpage est mauvais.

---

## 2 — Opérations d'édition

Le catalogue des transformations, au-dessus du noyau.

**Analyse préalable** — `aeidon/agents/` : `edit.py`, `position.py`, `format.py`,
`search.py`, `clipboard.py`, `set.py`.

**Questions d'architecture**

- Toute opération est-elle une commande annulable de premier ordre, y compris la
  frappe dans une cellule ? Comment se fait le **regroupement** d'actions en une
  seule entrée d'annulation ?
- Modèle de **cible** : sélection, plage, projet courant, tous les projets
  ouverts. Gaupol le traite par un paramètre `target` répété partout — on peut
  faire mieux qu'un énuméré propagé dans chaque signature.
- Où passe la frontière entre opération du noyau et logique d'interface ?

**Points difficiles**

- **Ajustement des durées** : contraintes simultanées de durée minimale,
  maximale, écart minimal entre sous-titres et vitesse de lecture en
  caractères/seconde. Elles sont **potentiellement contradictoires** ; l'ordre de
  résolution et les arbitrages doivent être spécifiés, pas improvisés.
- **Recherche dans du texte balisé** : chercher dans le texte visible tout en
  remplaçant dans le texte source, sans casser les balises qui chevauchent la
  correspondance.
- Transformation affine des positions à partir de deux points de repère, avec les
  cumuls d'erreur d'arrondi que cela suppose.

---

## 3 — CLI

Premier exécutable. Gaupol n'a pas d'équivalent : c'est une conception neuve, et
un gain fonctionnel réel.

**Analyse préalable** — pas de source à reprendre. En revanche, `README.aeidon.md`
montre les usages programmatiques visés, et `bin/gaupol.in` les options
existantes.

**Questions d'architecture**

- Périmètre : conversion, décalage, transformation, ajustement des durées,
  correction, inspection. Jusqu'où va-t-on, sachant que ce n'est pas dans Gaupol
  et donc pas dans l'iso-fonctionnalité ?
- Bibliothèque d'analyse d'arguments : CLI11, cxxopts, ou implémentation propre.
  Choix par ADR — c'est une dépendance de plus.
- Sortie lisible par un humain et sortie exploitable par un script : deux modes,
  ou un format unique ? Codes de retour signifiants.
- Traitement par lot de plusieurs fichiers : comportement en cas d'échec partiel.

**Point notable** — cette phase fournit le **harnais de mesure** du noyau. Les
benchmarks des phases suivantes s'appuieront dessus, ce qui est une raison de la
placer tôt.

---

## 4 — Moteur de correction de texte

La partie la plus riche métier, et celle où l'objectif de performance se joue.

**Analyse préalable** — `aeidon/` : `pattern.py`, `patternman.py`, `liner.py`,
`finder.py`, `spell.py`, `data/patterns/*`, et `gaupol/assistants.py` pour
l'enchaînement des corrections.

**Questions d'architecture**

- **Moteur d'expressions régulières.** Les motifs sont écrits en syntaxe Python.
  PCRE2 est compatible mais peut exploser en temps sur certains motifs ; RE2
  garantit un temps linéaire mais **ne gère pas les références arrière**, que les
  motifs de Gaupol utilisent abondamment (`\1 \2`). Vérifier lesquels sont
  réellement incompatibles avant de trancher, et mesurer.
- **Mesure de longueur de texte.** Point d'attention majeur : Gaupol mesure les
  lignes en *ems*, et le fait en demandant à **un widget GTK de mesurer le rendu
  du texte** (`gaupol/ruler.py`). Un algorithme de découpage de lignes dépend donc
  du toolkit graphique. Chez nous, ce doit être une abstraction injectée, avec une
  implémentation triviale par caractères pour la CLI et les tests, et une
  implémentation Qt pour l'interface.
- Format des motifs : reprendre le format INI de Gaupol tel quel — ce qui permet
  de réutiliser les fichiers sans conversion et de bénéficier de leurs mises à
  jour — ou convertir vers un format propre ? Noter que chaque fichier de motifs
  est accompagné d'un `.conf` **XML** qui active ou désactive les motifs par nom,
  et qu'un `\0` en tête de valeur sert à protéger les espaces initiaux
  (`patternman.py`). Deux détails de compatibilité à ne pas découvrir tard.
- Correcteur orthographique : hunspell, nuspell, ou service système. Dictionnaire
  personnel et liste de remplacements à persister.

**Points difficiles**

- L'algorithme de découpage de lignes est une variante de Knuth–Plass avec boîtes,
  pénalités et démérites, où les pénalités proviennent des motifs `line-break` par
  langue. Coûteux, subjectif, et central dans la qualité perçue du résultat.
- Appliquer des dizaines de motifs à des milliers de sous-titres est le cas d'usage
  où la réécriture doit se voir. C'est **le** benchmark de référence du projet.

---

## 5 — GUI : édition tabulaire

**Analyse préalable** — `gaupol/` : `view.py`, `page.py`, `application.py`,
`agents/*.py`, `dialogs/*.py`, `renderers/*.py`, `config.py`.

**Questions d'architecture**

- `QAbstractTableModel` au-dessus du modèle du noyau : adaptateur mince, ou
  modèle propre synchronisé ? Le premier évite la duplication d'état, le second
  découple mais impose une synchronisation.
- **Ne pas dupliquer la pile d'annulation.** Qt propose `QUndoStack`. Le noyau a
  la sienne, et c'est elle qui fait autorité puisque la CLI en dépend aussi. Le
  GUI doit s'y brancher, pas en tenir une seconde.
- Configuration typée et persistée, en remplacement du dictionnaire imbriqué de
  Gaupol. Format de fichier et stratégie de migration entre versions.
- Édition en place de texte multiligne dans une cellule, avec affichage des
  longueurs par ligne et coloration des différences.

**Points difficiles**

- Rester fluide sur plusieurs milliers de lignes : le modèle ne doit jamais
  matérialiser ce qui n'est pas visible, et les signaux de modification doivent
  être fins plutôt que globaux.
- Multi-projets en onglets, avec états de modification, sauvegarde et fermeture
  groupées.

---

## 6 — Vidéo et timing

**Analyse préalable** — `gaupol/player.py`, `gaupol/agents/video.py`,
`gaupol/actions/{video,audio}.py`, `aeidon/agents/preview.py`.

**Questions d'architecture**

- **Backend vidéo** : libmpv, embarquable et très tolérant aux formats, contre
  QtMultimedia, intégré mais plus limité. Décision par ADR, en gardant à l'esprit
  la portabilité Windows.
- Incrustation des sous-titres et du timecode : rendu par le backend, ou
  superposition Qt au-dessus de la surface vidéo ?
- Prévisualisation par lecteur externe (mpv, VLC, MPlayer) : conservée en plus du
  lecteur intégré, comme dans Gaupol.

**Points difficiles**

- **Précision de positionnement.** Caler un sous-titre exige un `seek` exact à
  l'image près ; la plupart des backends ne le garantissent qu'au mot-clé le plus
  proche. C'est la fonctionnalité la plus exigeante de tout le projet, et celle
  qui décide de la qualité de l'outil pour le travail de timing.
- Latence entre la position vidéo et l'action de l'utilisateur : un décalage
  systématique ruine le calage.

---

## 7 — Finitions

**Analyse préalable** — `po/`, `data/`, `flatpak/`, `PACKAGING.md`.

**Questions d'architecture**

- **Internationalisation** : Qt Linguist, ou gettext comme Gaupol ? Gaupol
  dispose de 20 locales traduites, sous GPL donc réutilisables — mais la
  conversion `.po` vers `.ts` n'est fidèle que si les chaînes correspondent, ce
  qui ne sera pas le cas partout. Évaluer le gain réel avant de s'engager.
- Empaquetage : Flatpak, `.deb`, AppImage. Et à quel moment ouvrir la question
  Windows, qui rouvrira celle des dépendances.
- Thème clair et sombre, suivi du réglage système.
