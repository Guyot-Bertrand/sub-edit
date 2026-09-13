# Configuration du dépôt GitHub

Le dépôt `subedit` est **public**. Ce document liste la configuration à appliquer
dans l'interface GitHub — elle ne vit pas dans le code et se perdrait autrement.

## Ce qu'un dépôt public ne permet pas

À poser d'emblée pour éviter les fausses attentes : sur un dépôt public, GitHub
**n'offre aucun moyen d'empêcher les forks ni la création d'issues**. Ce qui se
contrôle, c'est ce qui atterrit dans le dépôt, et là le verrouillage est complet.

Si un jour l'exigence devient « aucune interaction extérieure possible », la
seule réponse complète est un dépôt privé. La GPL-3.0 n'impose rien tant que le
binaire n'est pas distribué.

## Application

`src/scripts/setup-github.sh` applique tout ce qui a une API — labels,
milestones, ruleset — de façon idempotente. Le reste n'en a pas et figure plus
bas.

```bash
./src/scripts/setup-github.sh
```

Le ruleset exige la permission **Administration: Read and write** du jeton ;
un jeton à granularité fine ne l'accorde pas par défaut. Sans elle, le script
signale l'échec, applique le reste et se termine normalement.

## Configuration à appliquer

### 1. Aucun collaborateur

*Settings → Collaborators and teams* — n'ajouter personne.

C'est la mesure qui fait l'essentiel du travail : sans droit d'écriture,
un tiers ne peut que forker et proposer une PR, qui se ferme d'un clic.

### 2. Un ruleset sur `main`

*Settings → Rules → Rulesets*, ou `src/scripts/setup-github.sh`.

Les libellés ci-dessous sont ceux de l'interface GitHub, et non ceux de l'API.

#### « protection de l'historique »

| Champ de l'interface | Valeur |
| :------------------- | :----- |
| Ruleset Name | `protection de l'historique` |
| Enforcement status | **Active** |
| Bypass list | *laisser vide* |
| Target branches → Add target | **Include default branch** |
| Rules | cocher **Restrict deletions** et **Block force pushes** |

Le propriétaire étant seul à pouvoir écrire, le risque réel n'est pas un tiers
malveillant : c'est un `push --force` accidentel de sa part. Une règle dont il
serait exempté ne protégerait donc de rien — d'où la liste de dérogation vide.

#### Il n'y a plus de « porte de qualité »

**Supprimé à la #232.** Ce second ruleset exigeait le check `porte de qualité`
de `.github/workflows/ci.yml`. Les minutes d'Actions et le stockage du cache
partaient trop vite pour une exécution par pull request : `ci.yml` et
`pull-request.yml` ne tournent plus qu'à la demande, et une règle qui attend un
contrôle que rien ne produira ne protège de rien — elle apprend seulement à
passer outre par la dérogation admin.

Il avait été mis en `disabled` le 2026-08-27, à l'épuisement du quota d'août.
`setup-github.sh` ne le recrée plus. S'il existe encore sur le dépôt, il se
supprime par *Settings → Rules → Rulesets*, ou :

```console
$ gh api -X DELETE repos/Guyot-Bertrand/sub-edit/rulesets/20469201
```

**Ce qui garde le dépôt est `make check`, en local**, et la pull request dit ce
qui a été franchi. Ce qui tourne sur GitHub à chaque version est la publication
des paquets — voir la section 10.

#### Vérifier

```bash
gh api repos/Guyot-Bertrand/sub-edit/rulesets --jq '.[] | "\(.id) \(.name) [\(.enforcement)]"'
gh api repos/Guyot-Bertrand/sub-edit/rulesets/<id> \
  --jq '{bypass: [.bypass_actors[]? | "\(.actor_type)/\(.actor_id)"], rules: [.rules[].type]}'
```

Attendu : `protection de l'historique` seul, sans dérogation, avec les règles
`deletion` et `non_fast_forward`.

### 3. Approbation des workflows de fork

*Settings → Actions → General → Fork pull request workflows from outside collaborators*
→ **Require approval for all external contributors**

Empêche qu'une PR d'un inconnu déclenche la CI, et donc consomme des minutes ou
exécute du code non relu.

### 4. Limites d'interaction

*Settings → Moderation → Interaction limits* → **Limit to prior contributors**

Renouvelable par tranches de six mois. C'est ce qui se rapproche le plus de
« personne d'autre n'intervient » sur un dépôt public. À renouveler
explicitement : GitHub laisse la limite expirer sans prévenir.

### 5. Réduire la surface publique

*Settings → General → Features* — désactiver **Wiki**, **Discussions** et
**Projects** tant qu'ils ne servent pas.

### 6. Propriété du code et contribution

- `.github/CODEOWNERS` désignant le propriétaire sur `*`.
- `CONTRIBUTING.md` indiquant que le projet n'accepte pas de contributions
  externes pour l'instant. C'est une politesse autant qu'une protection : ça
  évite à un tiers de travailler pour rien.

### 7. Rétention des journaux et des artefacts

*Settings → Actions → General → Artifact and log retention* → **7 jours**
(réglable de 1 à 90 ; 90 par défaut, mais ce dépôt était **déjà à 30** — mesuré,
et non supposé, sur l'écart entre `created_at` et `expires_at` des artefacts).

C'est le seul poste de stockage facturé du dépôt avec le cache. Un unique
artefact est téléversé — `couverture`, le rapport HTML produit par `ci.yml` à
chaque exécution de la porte, 321 Kio invariablement. **Depuis #232, `ci.yml` ne
tourne qu'à la demande**, donc le calcul qui suit décrit l'ancien régime ; il
reste la raison du réglage. À une douzaine d'exécutions par
jour, le plateau est d'environ 103 Mio à 30 jours et de **~24 Mio à 7 jours**,
sur les 500 Mio d'un compte gratuit. Les 105 artefacts présents ne pesaient que
23,4 Mio parce que dix jours d'accumulation seulement les séparaient du départ :
le plateau n'était pas atteint.

Le réglage n'est pas scriptable : l'API des Actions répond
`403 Resource not accessible by personal access token`. Il se pose donc à la
main, comme les six précédents.

**Il ne fait pas le travail de l'élagueur, et réciproquement.** La durée efface
les journaux et les artefacts ; elle laisse la ligne d'exécution dans la liste,
avec ses étapes vides. C'est pourquoi il faut les deux — l'un borne le
stockage, l'autre le nombre.

#### Vérifier

Le réglage ne se lit pas par l'API sans jeton d'administration. Ce qui se lit,
et qui suffit, c'est l'écart que portent les artefacts eux-mêmes :

```bash
gh api repos/Guyot-Bertrand/sub-edit/actions/artifacts --paginate \
  --jq '.artifacts[] | "\(.created_at)\t\(.expires_at)"' |
  while IFS=$'\t' read -r c e; do
      echo $(( ( $(date -u -d "$e" +%s) - $(date -u -d "$c" +%s) ) / 86400 ))
  done | sort -u
```

Attendu : `6`, l'expiration étant calculée depuis le début de l'exécution et non
depuis le téléversement. Avant l'application du réglage, la même commande
répondait `29`.

Le réglage ne vaut que pour les artefacts créés après lui : ceux d'avant gardent
la date d'expiration qu'ils portaient déjà. C'est l'élagueur qui les emporte,
en supprimant leur exécution.

### 8. Élagage des exécutions

`.github/workflows/elagage.yml`, tous les lundis à 6 h, et à la main par
*Actions → élagage → Run workflow*.

**La règle :** de chaque workflow, garder les trente exécutions les plus
récentes, plus la plus récente dont la branche est `main` si elle n'y est pas
déjà ; supprimer le reste ; ne jamais toucher une exécution non terminée.

Contrairement aux sept réglages précédents, celui-ci vit dans le dépôt et non
dans l'interface — et il n'avait pas le choix. Un jeton personnel ne peut pas
supprimer une exécution ; seul le `GITHUB_TOKEN` d'un workflow le peut, avec
`permissions: actions: write`. La règle elle-même est dans
`src/scripts/prune-runs.sh`, dont `verify-gates.sh` prouve la sélection.

L'exception sur `main` protège l'analyse complète du lundi, dont la période est
de sept jours : sans elle, elle sortirait des trente en trois jours. Elle est au
singulier, ce qui l'empêche de ressusciter les quarante-sept exécutions du
déclencheur `push` supprimé à la #106.

#### Ce que ça donne

| | |
| :--- | ---: |
| avant | 146 |
| à supprimer | 86 |
| gardées | 60 |

Relevé par `--dry-run` le 2026-08-17, avant que le workflow n'ait tourné : c'est
donc la sélection qui est mesurée, pas encore la suppression. Le premier passage
réel se déclenche à la main une fois ce fichier sur `main`, et son résumé porte
le compte effectif.

Les 60 se répartissent en 30 `ci` et 30 `pull request`. L'hebdomadaire du lundi
était déjà dans les trente : l'exception n'a rien eu à épargner ce jour-là, ce
qui est le cas normal — elle ne sert que les jours où l'analyse complète a
vieilli de plus de trente exécutions. Le régime permanent se stabilise à 60, au
lieu de croître d'une quinzaine par jour.

#### Vérifier

```bash
./src/scripts/prune-runs.sh --dry-run
```

Lecture seule, et donc lançable avec un jeton personnel : il écrit sur la sortie
d'erreur le compte total, le nombre à supprimer et le nombre gardé, et sur la
sortie standard les identifiants condamnés. Après un passage de l'élagueur, le
nombre à supprimer doit être nul ou proche de zéro.

Chaque exécution du workflow écrit d'elle-même son avant/après dans le résumé de
son travail — un chiffre figé dans ce document vieillirait, celui-là non.

### 9. Le `.rpm` sur Fedora

`.github/workflows/fedora.yml`, tous les mardis à 4 h, et à la main par
*Actions → fedora → Run workflow* — issue #266.

**Ce qu'il fait :** produit le `.rpm`, l'installe sur une `fedora:42` en
conteneur, et lance ce qui en sort. C'est le seul contrôle du dépôt qui joue une
vraie transaction de gestionnaire de paquets ; tout le reste inspecte le paquet
sans l'installer, et un paquet peut être irréprochable à l'inspection et refusé
à l'installation — c'est exactement ce qu'il a trouvé au premier passage.

**Pourquoi hebdomadaire et non par pull request.** Il télécharge Qt et ses
dépendances, près de trois cents mégaoctets, et le quota d'Actions a déjà été
épuisé une fois. Ce qu'il éprouve ne bouge qu'avec l'empaquetage, qui bouge
rarement. Ce qui garde les pull requests entre-temps est local et gratuit :
`check-installation.sh` refuse un `.rpm` qui possède un répertoire hors de
`share/subedit`, ce qui était la cause du défaut trouvé.

**Il n'est requis par aucun ruleset**, et ne doit pas l'être : il ne tourne pas
sur une pull request, donc l'exiger ferait attendre indéfiniment.

**Le mardi et non le lundi**, pour ne pas se mettre en travers de l'analyse
complète de `ci.yml` à 4 h et de l'élagueur à 6 h.

#### Vérifier

```bash
make rpm-check
```

Le même geste que le workflow, sur la machine de développement. Il lui faut
podman ou docker, et il dit lequel il a pris.

### 10. Les paquets à chaque tag

`.github/workflows/release.yml`, quand un tag `vX.Y.Z` est poussé, et à la main
par *Actions → release → Run workflow* pour republier un tag existant — issue
#232.

**Ce qu'il fait, dans l'ordre :**

1. refuse un nom qui n'est pas `vX.Y.Z`, avant même le checkout ;
2. `src/scripts/check-release-tag.sh` — le `project(VERSION)` du commit tagué
   porte le même numéro, et ce commit est sur `main` ;
3. installe la chaîne d'outils par `setup-toolchain.sh` et lance
   `make packages` ;
4. écrit les notes par `git-cliff --current`, la section du journal de cette
   version ;
5. crée la release avec le `.deb` et le `.rpm` — ou, pour un tag déjà publié,
   remplace ses fichiers et ses notes ;
6. `src/scripts/prune-releases.sh` — élague les releases de patch des milestones
   passées.

**Pas de cache et pas d'artefact.** Le cache est le poste de stockage qui a
coûté ; chaque exécution compile donc à froid, une fois par version plutôt
qu'une fois par poussée. Les paquets vont directement dans la release, dont les
fichiers ne comptent pas dans le stockage d'Actions.

**Le tag se pose sur le commit de fusion dans `main`**, une fois la pull request
du bump fusionnée :

```console
$ git -C /home/beber/Projects/subedit tag -a v0.10.10 -m "v0.10.10 — …" <commit de fusion>
$ git -C /home/beber/Projects/subedit push origin v0.10.10
```

Un tag posé ailleurs — sur une branche de travail, ou sur un commit dont le
`CMakeLists.txt` porte un autre numéro — est refusé à l'étape 2, et rien n'est
construit.

#### La rétention

| Version | Release et paquets | Tag |
| :------ | :----------------- | :-- |
| `vX.Y.0`, mineure ou majeure | gardée toujours | gardé |
| `vX.Y.Z`, `Z > 0`, milestone en cours | gardée | gardé |
| `vX.Y.Z`, `Z > 0`, milestone passée | **supprimée** | **gardé** |

« Milestone en cours » est le couple `X.Y` le plus haut parmi les releases. À la
publication de `v0.11.0`, les releases `0.10.1` à `0.10.N` partent avec leurs
paquets ; `v0.10.0` reste, et tous les tags aussi.

**Un correctif sur une milestone passée est élagué dans la foulée** : un `v0.9.1`
poussé après `v0.10.0` est construit, publié, puis supprimé par la même
exécution. La règle ne fait pas d'exception pour le plus récent.

`verify-gates.sh` prouve la sélection sur une liste écrite à la main, et deux
refus du garde de tag.

#### Vérifier

```bash
./src/scripts/prune-releases.sh --dry-run
./src/scripts/check-release-tag.sh v0.10.9
gh release list --repo Guyot-Bertrand/sub-edit
```

Le premier est en lecture seule et passe avec un jeton personnel : il écrit les
tags dont la release partirait, et sur la sortie d'erreur le compte. Chaque
exécution du workflow écrit d'elle-même son avant/après dans le résumé de son
travail.

## Taxonomie des issues

### Labels

| Famille | Valeurs |
| :------ | :------ |
| `type:` | `bug`, `story`, `enhancement`, `task`, `doc` |
| `area:` | `build`, `ci`, `cli`, `core`, `doc`, `format`, `gui`, `i18n`, `scripts`, `test`, `text`, `video` |
| `prio:` | `high`, `medium`, `low` |
| état | `blocked`, `needs-decision` |

Les valeurs de `area:` sont **les mêmes que les scopes des messages de commit**.
Une issue et les commits qui la traitent portent donc le même vocabulaire, ce qui
rend le CHANGELOG généré directement rapprochable des tickets.

### Gabarits d'issue

`.github/ISSUE_TEMPLATE/` porte des **formulaires**, pas des fichiers Markdown :
GitHub en tire des champs, et un champ requis vide empêche d'ouvrir l'issue. Les
issues vierges sont désactivées, ce qui force le passage par l'un d'eux.

| Gabarit | Pour quoi | Labels posés |
| :------ | :-------- | :----------- |
| `story.yml` | une fonctionnalité vue de l'utilisateur | `type:story` |
| `task.yml` | un travail technique sans effet direct | `type:task` |
| `bug.yml` | un défaut | `type:bug` |
| `enhancement.yml` | une amélioration de l'existant | `type:enhancement` |
| `phase-review.yml` | la relecture qui clôt une phase | `type:task`, `area:doc`, `prio:high` |

**Le dernier est le seul à arriver pré-rempli**, et c'est ce qui le distingue :
la relecture de fin de phase a des axes qui valent pour toutes les phases —
confronter la spec au réalisé, répercuter les renvois, relire le manuel et les
ADR, confronter les mesures — et les redécouvrir à chaque fois, c'est en oublier
un. Ce qui est pré-rempli est un point de départ à modifier, pas une formule à
laisser telle quelle ; deux champs le disent explicitement, parce que la partie
qui compte est celle qui change d'une phase à l'autre.

Ce que le gabarit **n'essaie pas** de pré-remplir : ce que la phase a de
particulier. Le champ « points à regarder » est requis et sa description
demande ce qui a surpris pendant la phase — une relecture sans point propre est
une relecture que personne n'a faite.

### Milestones

Un par phase, dans l'ordre du plan de route. Les huit premiers mènent au MVP, les suivants complètent
l'iso-fonctionnalité. `src/scripts/setup-github.sh` les crée et les tient à
jour ; la liste de référence est dans
[`feuille-de-route.md`](feuille-de-route.md).
