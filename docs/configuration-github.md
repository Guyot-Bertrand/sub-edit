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

**« protection de l'historique »** — aucune dérogation.

| Réglage | Valeur |
| :------ | :----- |
| Target | branche par défaut |
| Restrict deletions | activé |
| Block force pushes | activé |
| Bypass list | *vide* |

Le propriétaire étant seul à pouvoir écrire, le risque réel n'est pas un tiers
malveillant : c'est un `push --force` accidentel de sa part. Une règle dont il
serait exempté ne protégerait donc de rien.

**« porte de qualité »** — dérogation pour l'administrateur.

| Réglage | Valeur |
| :------ | :----- |
| Target | branche par défaut |
| Require status checks to pass | activé, contexte `porte de qualité` |
| Bypass list | rôle *Admin* |

Ici la dérogation est indispensable : la CI ne s'exécute qu'**après** le push,
donc sans elle la règle rejetterait tout push direct sur `main` et imposerait
de fait le passage par une pull request — ce qui a été écarté à ce stade.

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
| `area:` | `build`, `core`, `format`, `text`, `cli`, `gui`, `video`, `i18n`, `ci`, `doc` |
| `prio:` | `high`, `medium`, `low` |
| état | `blocked`, `needs-decision` |

Les valeurs de `area:` sont **les mêmes que les scopes des messages de commit**.
Une issue et les commits qui la traitent portent donc le même vocabulaire, ce qui
rend le CHANGELOG généré directement rapprochable des tickets.

### Milestones

Un par sous-projet, dans l'ordre du plan de route :

```
0 — Fondations
1 — Noyau : modèle et formats
2 — Opérations d'édition
3 — CLI
4 — Moteur de correction
5 — GUI : édition tabulaire
6 — Vidéo et timing
7 — Finitions
```
