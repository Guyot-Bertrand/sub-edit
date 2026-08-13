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
milestones, rulesets — de façon idempotente. Le reste n'en a pas et figure plus
bas.

```bash
./src/scripts/setup-github.sh
```

Les rulesets exigent la permission **Administration: Read and write** du jeton ;
un jeton à granularité fine ne l'accorde pas par défaut. Sans elle, le script
signale l'échec, applique le reste et se termine normalement.

## Configuration à appliquer

### 1. Aucun collaborateur

*Settings → Collaborators and teams* — n'ajouter personne.

C'est la mesure qui fait l'essentiel du travail : sans droit d'écriture,
un tiers ne peut que forker et proposer une PR, qui se ferme d'un clic.

### 2. Deux rulesets sur `main`

*Settings → Rules → Rulesets*, ou `src/scripts/setup-github.sh`.

Deux plutôt qu'un, parce qu'un ruleset n'a **qu'une seule liste de dérogation
pour toutes ses règles**, alors que les deux besoins s'opposent.

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

#### « porte de qualité »

| Champ de l'interface | Valeur |
| :------------------- | :----- |
| Ruleset Name | `porte de qualité` |
| Enforcement status | **Active** |
| Bypass list → Add bypass | **Repository admin** — mode *Always* |
| Target branches → Add target | **Include default branch** |
| Rules | cocher **Require status checks to pass** |
| → Add checks | `porte de qualité` et `contrôles de pull request`, source **GitHub Actions** |
| → Require branches to be up to date | décoché |

Trois pièges dans cette seconde :

- **Le nom du contrôle est celui du *job*, pas du workflow.** C'est
  `porte de qualité`, valeur du champ `name:` du job `check` dans
  `.github/workflows/ci.yml` — et non `ci`. De même,
  `contrôles de pull request` est le `name:` du job `pull-request` de
  `.github/workflows/pull-request.yml`, et non `pull request`.
- **Le contrôle n'apparaît dans le sélecteur qu'après s'être exécuté au moins
  une fois.** Sur un dépôt dont la CI n'a jamais tourné, la liste est vide et
  il faut saisir le nom à la main.
- **`contrôles de pull request` ne rapporte rien lors d'un push direct sur
  `main`** : son workflow ne se déclenche que sur l'événement `pull_request`.
  C'est sans conséquence, la dérogation de l'administrateur couvrant déjà ce
  cas — mais un contrôle requis qui reste muet bloquerait une fusion si cette
  dérogation venait à disparaître.

Le job `messages de commit` de `ci.yml` n'est, lui, pas requis : il échoue en
rouge sans empêcher la fusion. C'est un écart connu, laissé en l'état.

La dérogation pour l'administrateur est indispensable : la CI ne s'exécute
qu'**après** le push, donc sans elle la règle rejetterait tout push direct sur
`main` et imposerait de fait le passage par une pull request — ce qui a été
écarté à ce stade.

#### Vérifier

```bash
gh api repos/Guyot-Bertrand/sub-edit/rulesets --jq '.[] | "\(.id) \(.name) [\(.enforcement)]"'
gh api repos/Guyot-Bertrand/sub-edit/rulesets/<id> \
  --jq '{bypass: [.bypass_actors[]? | "\(.actor_type)/\(.actor_id)"], rules: [.rules[].type]}'
```

Attendu : `protection de l'historique` sans dérogation avec les règles
`deletion` et `non_fast_forward` ; `porte de qualité` avec
`RepositoryRole/5` et la règle `required_status_checks`.

Les contextes exigés se lisent à part, la requête ci-dessus ne descendant pas
dans les paramètres des règles :

```bash
gh api repos/Guyot-Bertrand/sub-edit/rulesets/<id> \
  --jq '[.rules[] | select(.type == "required_status_checks")
         | .parameters.required_status_checks[].context]'
```

Attendu : `porte de qualité` et `contrôles de pull request`.

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

### Milestones

Un par phase, dans l'ordre du plan de route. Les huit premiers mènent au MVP, les suivants complètent
l'iso-fonctionnalité. `src/scripts/setup-github.sh` les crée et les tient à
jour ; la liste de référence est dans
[`feuille-de-route.md`](feuille-de-route.md).
