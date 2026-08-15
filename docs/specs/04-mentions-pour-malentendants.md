# Phase 4 — Suppression des mentions pour malentendants

**État :** conception validée, prête à découper
**Milestone :** `4 — Mentions pour malentendants`

## Objectif

Retirer d'un sous-titrage les mentions destinées aux malentendants — les bruits
décrits entre crochets ou entre parenthèses — sans rien laisser derrière elles :
ni espace en trop, ni ligne vide, ni sous-titre sans texte.

C'est **la première fois que le noyau calcule un texte**. Les trois phases
précédentes ont lu, écrit et déplacé ; `SetTextCommand` remplace un texte par
celui qu'on lui donne et ne décide rien de son contenu. Tout ce qui suit
découle de cette nouveauté : où le calcul vit, ce qui l'éprouve, et ce qu'il
coûte.

## Portée

**Inclus** — les deux motifs de la feuille de route, crochets et parenthèses ;
la règle de couture que `mentions.cas` a fixée cas par cas ; une commande
annulable ; la sous-commande `hearing-impaired` ; le manuel ; les exigences ;
et la première mesure de texte du journal des performances.

**Exclu, par phase**

| Ce qui n'est pas fait ici | Où |
| :------------------------ | :- |
| les trois autres motifs de Gaupol — paroles entre dièses, sur une ligne, nom du locuteur avant deux-points | phase 12 |
| motifs par langue, fichiers de motifs, `.conf` d'activation, correcteur orthographique, remise en majuscule | phase 12 |
| le choix d'un moteur d'expressions rationnelles, PCRE2 ou RE2 | phase 12 — voir [l'ADR 0017](../adr/0017-analyseur-de-mentions-ecrit-a-la-main.md) |
| appliquer l'opération à une sélection de sous-titres | phase 5, qui apporte une vraie sélection ; aucune sous-commande n'en a aujourd'hui |
| nettoyer le document de traduction depuis la ligne de commande | phase 11 — le noyau, lui, accepte déjà les deux documents |

## Décisions applicables

| ADR | Décision |
| :-- | :------- |
| [0009](../adr/0009-texte-en-chaine-brute.md) | le texte est une chaîne brute, balises du format comprises |
| [0010](../adr/0010-annulation-par-commandes.md) | toute modification passe par une commande qui porte son inverse |
| [0014](../adr/0014-registre-d-exigences.md) | registre d'exigences confronté aux tests |
| [0017](../adr/0017-analyseur-de-mentions-ecrit-a-la-main.md) | un analyseur écrit à la main, sans moteur d'expressions rationnelles |

## Ce que Gaupol fait, et où nous divergeons

Sa méthode `remove_hearing_impaired` applique une liste de motifs, puis
**sept passes de rattrapage** (`_remove_leftover_hi`) : bords de ligne, espaces
consécutifs, lignes sans caractère alphanumérique, lignes vides, espace après le
tiret, tiret orphelin, tiret d'un sous-titre à une seule ligne.

Trois divergences, assumées et décidées avant ce cadrage.

**La référence numérique est gardée.** `\[.*?\]` mange `[1]` ; `mentions.cas`
(#87) a décidé qu'un contenu purement numérique n'est pas un bruit et reste tel
quel, crochets compris.

**La couture est locale au retrait.** Une passe `" {2,}" → " "` est globale :
elle réécrit aussi les doubles espaces que le fichier avait déjà, et auxquels
personne n'a demandé de toucher. Notre règle ne parle que de ce qui bordait la
mention. C'est l'argument qui décide du moteur, développé dans l'ADR 0017.

**Rien n'est configurable.** Gaupol livre une liste de motifs activables ; la
phase 4 livre une transformation décidée. #88 a écarté de reprendre ses données
de motifs — la copie serait licite, les deux formats sont GPL-3, mais elle
engagerait sur un format de fichiers qu'on n'a pas choisi.

## La règle

`src/test/data/textes/mentions.cas` **est** la spécification, écrite cas par cas
et avant tout code. Ce qui suit énonce ce que ces cas font apparaître ; en cas de
désaccord entre les deux, le fichier de cas fait foi — c'est lui que le code doit
passer.

### Ce qu'est une mention

Un `[` ou un `(`, et **le premier fermant du même genre qui suit**. Le motif
traverse le saut de ligne : dans de vrais fichiers, une mention est souvent
coupée par lui, et sur quinze sous-titrages les douze crochets qui semblaient
orphelins étaient tous de cette forme.

### Ce qui n'en est pas une

| Cas | Ce qui se passe | Pourquoi |
| :-- | :-------------- | :------- |
| contenu **purement numérique** — `[1]`, `[ 156478 ]`, `(12)` | gardé tel quel, délimiteurs compris | une référence n'a jamais désigné un bruit |
| un blanc **entre** les chiffres, un signe, une lettre — `[1 2]`, `[-1]`, `[1a]` | c'est une mention | le critère est étroit à dessein |
| délimiteur **sans fermant** | laissé tel quel | aucun fichier réel n'en produit ; ce n'est pas une décision mais ce que fait un balayage qui ne trouve pas sa fin |

Le contenu vide, ou fait de blancs seuls — `[]`, `[ \t ]`, `()` — **est une
mention** : aucun chiffre, donc pas une référence ; rien à dire, donc rien à
garder.

**Les deux délimiteurs sont symétriques.** `mentions.cas` n'écrivait la
référence numérique qu'entre crochets ; le cadrage étend la règle aux
parenthèses. Un chiffre est un chiffre, et deux règles à retenir là où une
suffit se paient à chaque relecture. Les cas jumeaux sont ajoutés au corpus.

### Ce que le retrait laisse

**Exactement un espace entre ce qui l'entourait, et rien en bord de ligne.**

Ce n'est pas « on recolle les espaces qui traînent » : `Bonjour[il tousse]Marie`
n'en avait aucun et en gagne un. Ce qui compte est ce qui reste de part et
d'autre, pas ce qui était écrit autour.

| Entrée | Sortie |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `A [un] [deux] B` | `A B` — deux mentions voisines valent une couture |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `Bonjour [il tousse]` | `Bonjour` |

**Une balise ne borde pas une mention.** Cinquante-trois sous-titres du corpus
s'écrivent `<i>(Guild navigator) We have` : par la lettre de la règle, la balise
est ce qui entourait la mention, et le retrait laisserait `<i> The Bene` — une
espace en tête de ligne à l'écran, ce que « rien en bord de ligne » interdit
précisément. La balise ne compte donc pas, ni pour juger d'un bord, ni pour
juger d'une vacuité. C'est la même règle des deux côtés, et elle est écrite en
un seul endroit.

*Décidé à l'implémentation (#101), sur pièces.* La spec n'avait vu que la
vacuité ; le bord est apparu en cherchant les fichiers qui portent la première.

**Quand la mention enjambe le saut de ligne, c'est le saut qui subsiste**, pas
l'espace : les deux lignes restent deux lignes.

| Entrée | Sortie |
| :----- | :----- |
| `fall back. [ Hicks\n] Fall back, now!` | `fall back.\nFall back, now!` |
| `Go! [\nMurmurs Of Agreement ]` | `Go!` — la seconde ligne est vidée, donc elle disparaît |

### Ce que le retrait fait disparaître

**Une ligne que le retrait a vidée disparaît.** Vidée veut dire : il n'y reste
que des blancs, des balises de format, ou un tiret de dialogue seul.

Les balises sont dans la règle parce que le corpus les y a mises : neuf
sous-titres réels s'écrivent `<i>[PEOPLE SCREAMING]</i>`, et sans cette
précision il resterait `<i></i>` — un sous-titre vide à l'écran, qu'aucune règle
n'aurait retiré. L'ADR 0009 garde le texte brut, balises comprises ; c'est donc
au test de vacuité de les ignorer, pas au texte de les perdre.

**Le test est local**, comme la couture : une ligne déjà vide dans le fichier
n'est pas touchée. Le cas ne peut d'ailleurs pas se produire — une ligne vide
sépare deux blocs dans les deux formats du MVP — mais la règle est écrite ainsi
pour qu'elle reste vraie le jour où un format en produira une.

**Un texte entièrement vidé emporte son sous-titre.** C'est le cas coûteux, et
le plus fréquent : 58,9 % des sous-titres mordus du corpus sont dans ce
cas.

### Le dialogue

**Un dialogue réduit à une seule voix n'en est plus un**, et son tiret s'en va.
Tant qu'il en reste deux, les tirets restent.

| Entrée | Sortie |
| :----- | :----- |
| `- [ Hisses ]\n- Uhh-` | `Uhh-` |
| `- A\n- [rires]\n- B` | `- A\n- B` |

C'est une règle sur les tirets, pas sur les mentions : elle vaudrait aussi pour
une ligne supprimée à la main. Elle est ici parce que c'est ici qu'on la
rencontre, et la phase 12 en hérite.

**Le tiret est le seul trait d'union `-` (U+002D).** Gaupol reconnaît aussi le
tiret demi-cadratin et le cadratin ; sur les 97 059 lignes du corpus, 4 874
commencent par un tiret, et **toutes** sont U+002D — aucune n'est `–` ni `—`. Élargir sans fichier qui le réclame, c'est écrire du code qu'aucun
test ne peut motiver. Déclencheur pour y revenir : un fichier qui en montre un.

L'espace après le tiret est toléré absent — huit lignes du corpus l'omettent.

## L'architecture

### La fonction pure

```cpp
// src/lib/subedit/core/text/hearing_impaired.hpp
/// Rend le texte nettoyé, ou rien si le sous-titre est emporté.
[[nodiscard]] std::optional<std::string> withoutHearingImpaired(std::string_view text);
```

Sans état, sans `Project`, sans commande, sans fichier. C'est **la forme exacte
des cas de `mentions.cas`** — une entrée, une sortie, ou `supprimé` — donc ces
cas l'éprouvent directement, par le `checkTextCases` que #87 a déjà écrit.

`std::nullopt` dit « le sous-titre ne survit pas ». Une chaîne vide ne le dirait
pas : elle serait un texte, et un sous-titre au texte vide n'est pas la même
chose qu'un sous-titre absent. Le corpus de cas fait déjà cette distinction, et
c'est pour la porter qu'il l'a faite.

### La commande

```cpp
// src/lib/subedit/core/edit/hearing_impaired_removal.hpp
/// Rend la commande qui nettoie `document`, ou rien si aucun texte ne mord.
[[nodiscard]] std::unique_ptr<Command> removeHearingImpaired(const Project& project,
                                                             Document document);
```

Elle rend un `CompositeCommand` qui groupe **un `SetTextCommand` par texte
changé et un unique `RemoveCommand`** pour les sous-titres vidés. Une seule
entrée d'historique, donc une seule annulation — c'est précisément ce pour quoi
`CompositeCommand` existe.

**Aucune classe de commande nouvelle.** `CompositeCommand` *est* la commande ;
en écrire une qui lui délègue la doublerait. Ce qu'il faut en revanche, c'est un
`CommandKind::RemoveHearingImpaired`, pour que l'historique la nomme et que le
rapport de la CLI la distingue — un énumérateur avec un lecteur, donc, ce que la
phase 2 exigeait déjà de tout nouvel énumérateur.

**Rien du tout quand rien ne mord.** Un composite vide s'appliquerait sans rien
faire et pousserait une entrée d'historique que l'utilisateur ne comprendrait
pas. L'appelant reçoit un pointeur nul et rapporte qu'il n'y avait rien à faire.

**L'ordre à l'intérieur du groupe** : les textes d'abord, la suppression
ensuite. Les indices ne bougent pas pendant les réécritures, donc ceux que la
suppression vise restent valides. L'annulation reprend le groupe à l'envers —
`CompositeCommand` le fait déjà, et pour cette raison.

### Ce que le noyau ne gagne pas

Pas de fichier de motifs, pas de format à lire, pas d'interface de règle de
texte. Une interface à une seule implémentation ne se valide pas, et les champs
qui la feraient vivre — nom, activation, ordre, langue, remplacement — viennent
tous d'un format que la phase 12 choisira. Une fonction libre s'enveloppera dans
cette interface le jour où elle existera ; l'inverse coûte davantage.

## La surface de la ligne de commande

```
subedit-cli hearing-impaired (--output FICHIER | --output-dir DOSSIER | --in-place)
                             <fichier>...
```

Elle réutilise **la grammaire de destination, le rapporteur, les niveaux de
verbosité et les codes de retour de la phase 3**, sans rien inventer.

**Le fichier entier, le texte principal.** Aucune sous-commande ne prend de
sélection, et `Document::Translation` n'apparaît nulle part dans `src/lib/subedit/cli`
— une option `--document` serait de la grammaire morte. Le noyau, lui, accepte
les deux documents : c'est la fenêtre de la phase 5, puis la phase 11, qui s'en
serviront.

**Ce que le rapport compte** : les sous-titres changés et les sous-titres
supprimés, que `describe()` sait déjà dire. Pas les mentions : ce chiffre
demanderait une plomberie nouvelle à travers la fonction pure, la commande et le
rapport, pour un nombre que personne n'a réclamé.

**Un fichier où rien ne mord est écrit quand même**, comme chez `shift`, et le
rapport le dit. La règle du projet est qu'une sous-commande qui reçoit une
destination y écrit ; en faire l'exception ici obligerait un script à savoir
laquelle des sous-commandes produit parfois un fichier et parfois rien.

Une sous-commande **et non un verbe à drapeaux**. `strip --hearing-impaired`
laisserait la place aux quatre nettoyages de la phase 12 ; mais c'est la phase 12
qui saura s'ils se rassemblent, en les connaissant. La CLI d'aujourd'hui est un
harnais — la spec de la phase 3 l'écrit — et ses noms peuvent encore bouger.

## Tests

**Les cas de `mentions.cas` passent par la fonction pure** — vingt-quatre
aujourd'hui — en une ligne de test : `checkTextCases` existe depuis #87 et attend exactement cette
signature. Les cas jumeaux des parenthèses et ceux des balises y sont ajoutés.

**L'annulation se prouve sur le projet, pas sur la commande** : après `apply`
puis `revert`, le projet est celui d'avant, exactement — textes et sous-titres
supprimés remis à leur place, y compris quand la suppression est discontinue.

**De bout en bout, par le binaire réel**, sur un fichier qui porte les cas qui
comptent : une mention seule, une mention à cheval, un dialogue à une voix, une
référence numérique laissée tranquille.

**Aucun test ne compare deux implémentations.** Ce qui est attendu est écrit
dans le test ou dans le corpus de cas, jamais produit par Gaupol.

## Mesures

La phase apporte **la première mesure de texte du journal**. Elle se prend sur
la fixture de #89, dont c'était la raison d'être : quatre mille sous-titres dont
un sur cinq porte une mention, dans les proportions de vrais fichiers, et dont
11,6 % sont emportés par le retrait — le cas coûteux.

| Mesure | Ce qu'elle dit |
| :----- | :------------- |
| `suppression des mentions sur 4000 sous-titres` | ce que coûte l'opération entière, commande comprise |

Une seule, et à travers la commande : c'est ce qu'un utilisateur déclenche. La
fonction pure seule serait plus stable à mesurer, mais mesurerait moins.

## Exigences

Nouvelles entrées au registre, à l'état `prévue` jusqu'à ce qu'un test les cite :

| ID | Exigence |
| :- | :------- |
| `CLI-HEARING-01` | `hearing-impaired` retire les mentions entre crochets et entre parenthèses |
| `CLI-HEARING-02` | un sous-titre que le retrait vide est supprimé du fichier écrit |
| `CLI-HEARING-03` | une référence purement numérique est laissée telle quelle |
| `CLI-HEARING-04` | le rapport nomme le nombre de sous-titres changés et supprimés |
| `CLI-HEARING-05` | un fichier sans aucune mention est écrit inchangé, code `0` |
| `CLI-HEARING-06` | sans destination explicite, rien n'est écrit et le code est `1` |

## Découpage en issues

L'ADR 0017 n'y figure pas : elle est écrite **au cadrage**, avec cette spec,
pour la raison que l'ADR 0016 a déjà servie — une décision prise en même temps
que le code qui la consomme n'est plus une décision, c'est une justification
après coup.

| Issue | Sujet | Dépend de |
| :---- | :---- | :-------- |
| [#101](https://github.com/Guyot-Bertrand/sub-edit/issues/101) | le noyau : la règle de texte, et le corpus de cas élargi | — |
| [#102](https://github.com/Guyot-Bertrand/sub-edit/issues/102) | le noyau : la commande composée, son annulation, et la mesure au journal | #101 |
| [#103](https://github.com/Guyot-Bertrand/sub-edit/issues/103) | la CLI : la sous-commande, le rapport, le manuel, les exigences | #102 |
| — | relecture de fin de phase, puis publication de la `v0.4.0` | tout |

**La règle avant la commande, et seule.** C'est là qu'est toute la difficulté —
la couture, les vidés, les tirets, les balises — et elle s'éprouve sans rien
construire autour. Les mêler ferait porter à une seule issue le corpus de cas
et une composition de commandes, et l'échec de l'une masquerait l'autre.

## Points ouverts

| Point | Déclencheur |
| :---- | :---------- |
| élargir le tiret de dialogue à `–` et `—` | un fichier réel qui en montre un ; le corpus n'en a aucun sur 4 874 tirets |
| les trois autres motifs de Gaupol, et des motifs configurables | phase 12, qui apporte le moteur et le format de fichiers |
| nettoyer le document de traduction | phase 11 |
| appliquer l'opération à une sélection | phase 5, qui apporte une sélection à l'écran |

## Critères de fin

- [ ] La fonction pure passe tous les cas de `mentions.cas`, parenthèses et
      balises comprises
- [ ] L'annulation restitue le projet d'avant, exactement
- [ ] La sous-commande existe et est documentée au manuel
- [ ] Chaque exigence du tableau ci-dessus est `implémentée` et citée par un test
- [ ] La mesure de texte figure au journal des performances
- [ ] L'ADR 0017 est écrite
- [ ] La relecture de fin de phase est faite
