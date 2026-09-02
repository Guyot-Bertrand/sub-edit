# Phase 8 — Encodages et fins de ligne

Cadrage, issue #288. Ce document dit ce que la phase livre, ce qu'elle ne livre
pas, et pourquoi.

## Ce que la feuille de route promet, et ce qui reste à faire

> Détection automatique de l'encodage, jeu complet d'encodages, fins de ligne
> Windows et Mac, forçage à l'enregistrement.

**La moitié « fins de ligne » est déjà livrée**, et le dire est le premier
travail de ce cadrage : `Newline` porte `Lf`, `CrLf` et `Cr` depuis la phase 1,
`--line-endings unix|windows|mac` existe depuis la phase 3, `scanNewlines`
rapporte la fin de ligne majoritaire, le mélange et la ligne où il commence, et
un fichier lu est réécrit avec les siennes.

**Ce qui reste de la promesse est l'encodage.** Et une chose que la feuille de
route ne dit pas : **la fenêtre n'offre rien de tout cela.** `Save As…` choisit
un chemin et un format ; ni encodage, ni fin de ligne, ni BOM. La ligne de
commande sait faire ce que la fenêtre ne propose pas, et cet écart-là est du
périmètre de cette phase.

## Analyse préalable — ce que Gaupol fait

`aeidon/encodings.py`, `aeidon/agents/open.py`, `aeidon/file.py`.

- **Quatre-vingt-dix-sept encodages**, chacun avec son code Python, son nom IANA
  et une description reprise de gedit.
- **La détection en deux temps** : le BOM d'abord — UTF-32 BE/LE, UTF-8,
  UTF-16 BE/LE —, `charset-normalizer` ensuite, et `None` si rien ne sort.
- **Le BOM l'emporte sur l'encodage demandé** : `open_main` recommence la
  lecture avec l'encodage du BOM s'il diffère de celui qu'on lui a passé.
- **La locale en dernier recours**, sans le dire à l'utilisateur.

Ce dernier point est le seul que ce cadrage écarte franchement : la règle du
projet est de dire plutôt que de taire — ADR 0008, lecture au mieux avec
diagnostics.

## D1 — ICU, pour convertir et pour détecter

Tranchée par la mesure, et elle a son ADR : [0027](../adr/0027-icu-pour-les-encodages.md).

Les trois voies ont été passées au même corpus avant qu'aucune soit choisie —
ICU 9/9, `uchardet` 9/9, un prototype de tables écrites 6/9. Ce qui décide n'est
pas la détection mais la **conversion** : ICU est le seul des trois à convertir,
et l'iso-fonctionnalité demande quatre-vingt-dix-sept encodages, qui ne s'écrivent
pas à la main.

Le prix est écrit dans l'ADR : `subedit-cli` y gagne sa première dépendance
externe.

## D2 — Le jeu d'encodages est celui d'ICU, pas une liste à nous

**Aucune table d'encodages n'est écrite dans ce dépôt.** ICU en connaît plus que
Gaupol ; en tenir une liste réduite reviendrait à écrire une seconde vérité à
maintenir, et à refuser un fichier qu'on sait lire.

Ce qui est écrit, en revanche, est **ce que l'interface propose** : une liste
courte, ordonnée, des encodages qu'un fichier de sous-titres porte en pratique,
plus « autre… » pour le reste. Un menu de quatre-vingt-dix-sept entrées n'est pas
une aide.

## D3 — Ce que le modèle enregistre

`SourceFile` porte aujourd'hui le format, la fin de ligne et `hadUtf8Bom`, **un
booléen propre à l'UTF-8**. Il ne suffit plus : un BOM UTF-16 ne dit pas la même
chose, il porte aussi l'ordre des octets.

`hadUtf8Bom` devient donc un encodage, et le BOM cesse d'être une propriété
séparée : il est **une variante de l'encodage** — `utf-8` et `utf-8-sig` sont
deux réponses différentes, comme Python les nomme. Un type fort plutôt qu'une
chaîne, comme `FrameRate` l'est déjà (ADR 0011) : un encodage inconnu doit être
irreprésentable, pas signalé à l'usage.

## D4 — Une détection est une proposition, jamais une certitude

**Latin-1 et CP1252 sont indistinguables** sur un texte qui n'use pas de la plage
`0x80–0x9f`. Les octets ne portent pas la réponse ; un détecteur qui tranche
devine. Le corpus étiqueté le montre — les trois voies répondent sur
`latin1.srt`, et aucune ne peut le prouver.

Conséquence, et elle suit l'ADR 0008 :

- **l'encodage retenu entre dans les diagnostics de lecture**, ceux que la
  fenêtre affiche sous la table et que `-vv` écrit ;
- **il se contredit** : `--encoding` à la lecture impose et ne propose pas ;
- **le BOM l'emporte sur tout**, y compris sur `--encoding`, parce qu'il est la
  seule marque explicite qu'un fichier porte. Un BOM contredit par une option
  est dit, pas arbitré en silence.

## D5 — La lecture ne refuse plus, elle signale

`ReadErrorKind::InvalidUtf8` rend « is not valid UTF-8 ». Avec plusieurs
encodages, ce message change de sens : ce n'est plus « ce fichier est cassé »
mais « je ne sais pas le lire dans l'encodage que j'ai choisi ». Les mots vivent
dans `core/wording.hpp`, seul endroit où ils s'écrivent.

Un fichier qu'aucun encodage ne décode reste une erreur ; un fichier décodé sous
un encodage deviné est une réussite avec un diagnostic.

## D6 — Les deux surfaces, et la fenêtre rattrape son retard

**Ligne de commande** — `--encoding` à la lecture et à l'écriture, dans la forme
que `--line-endings` a déjà. À l'écriture, l'encodage du fichier lu par défaut,
comme le BOM et les fins de ligne : un fichier réécrit sans consigne rend les
mêmes octets.

**Fenêtre** — `Save As…` gagne l'encodage, la fin de ligne et le BOM. Lui donner
l'encodage sans les deux autres la laisserait à moitié, et les deux autres
existent depuis la phase 3 sans que la fenêtre les propose.

## D7 — L'aller-retour d'octets est la propriété qui tient tout

Un fichier lu et réécrit sans consigne rend **les mêmes octets**. C'est déjà vrai
du BOM et des fins de ligne ; l'encodage le prolonge, et il ne se vérifie pas sur
le texte — deux encodages différents portent le même texte.

Le harnais existe : `corpus_test.cpp` énumère `valides/` depuis #289 et compare
au fichier d'origine. La phase y verse ses fixtures d'encodage à mesure qu'elle
sait les lire.

## Ce que la phase ne livre pas

- **UTF-32.** Gaupol le détecte par BOM et ne le liste pas dans ses encodages
  proposés. Aucun fichier de sous-titres n'en porte ; ICU le lira si on le lui
  demande, mais rien ne le proposera.
- **Une conversion d'encodage en lot depuis la ligne de commande.** `convert`
  saura écrire dans l'encodage demandé ; réencoder un répertoire est une
  fonctionnalité, pas une phase d'encodage.
- **La détection de la langue.** ICU la propose avec l'encodage ; rien ici n'en
  a l'usage.

## Exigences

| Identifiant | Ce qu'il promet |
| :---------- | :-------------- |
| `CLI-ENC-01` | un fichier non-UTF-8 s'ouvre, et l'encodage retenu est dit |
| `CLI-ENC-02` | `--encoding` impose l'encodage de lecture et l'emporte sur la détection |
| `CLI-ENC-03` | un BOM l'emporte sur `--encoding`, et l'écart est dit |
| `CLI-ENC-04` | `--encoding` à l'écriture choisit l'encodage produit |
| `CLI-ENC-05` | un fichier réécrit sans consigne rend les mêmes octets, encodage compris |
| `CLI-ENC-06` | un fichier qu'aucun encodage ne décode est refusé, et la raison le dit |
| `CLI-ENC-07` | `inspect` rapporte l'encodage lu et s'il a été deviné |
| `GUI-ENC-01` | la fenêtre ouvre un fichier non-UTF-8 et affiche l'encodage retenu |
| `GUI-ENC-02` | `Save As…` choisit l'encodage, la fin de ligne et le BOM |
| `GUI-ENC-03` | les réglages retiennent le dernier encodage d'écriture choisi |

## Découpage

L'ordre n'est pas négociable sur les deux premières : le modèle porte
l'encodage avant que quiconque le lise, et la lecture précède l'écriture.

| Ordre | Ce qu'elle fait |
| :---- | :-------------- |
| 1 | l'encodage entre dans le modèle, et le BOM devient sa variante |
| 2 | lire dans un encodage donné, et le refuser proprement |
| 3 | détecter, et dire ce qu'on a deviné |
| 4 | écrire dans un encodage donné, et l'aller-retour |
| 5 | la surface de la ligne de commande |
| 6 | la surface de la fenêtre |
