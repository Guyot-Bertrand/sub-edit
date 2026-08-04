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

## Configuration à appliquer

### 1. Aucun collaborateur

*Settings → Collaborators and teams* — n'ajouter personne.

C'est la mesure qui fait l'essentiel du travail : sans droit d'écriture,
un tiers ne peut que forker et proposer une PR, qui se ferme d'un clic.

### 2. Ruleset sur `main`

*Settings → Rules → Rulesets → New branch ruleset*

| Réglage | Valeur |
| :------ | :----- |
| Target | `main` |
| Restrict deletions | activé |
| Block force pushes | activé |
| Require status checks to pass | activé, cocher le job `check` |
| Bypass list | le propriétaire du dépôt |

Le *bypass* est ce qui distingue un ruleset de l'ancienne protection de branche :
il autorise le push direct sur `main` sans ouvrir la porte à qui que ce soit
d'autre. Pas de PR obligatoire, donc, mais un historique impossible à réécrire
accidentellement.

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
