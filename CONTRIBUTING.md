# Contribuer à subedit

## Contributions externes

**Le projet n'accepte pas de contributions externes pour l'instant.** Les pull
requests venant de forks ne seront pas fusionnées.

Ce n'est pas une prise de position sur le travail des autres : le projet est en
phase de fondation, ses conventions bougent encore, et il n'y a personne pour
relire. Signaler un bug par une issue reste utile et bienvenu.

## Mettre en place son poste

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
git clone https://github.com/otsaloma/gaupol.git reference/gaupol
./src/scripts/setup-toolchain.sh
```

Le clone de référence sert à consulter le comportement de Gaupol. Il est
maintenu **en lecture seule** par `src/scripts/reference.sh` : lecture et
`fetch` uniquement, jamais de modification.

## Le cycle de travail

Une phase du projet correspond à un milestone, et s'ouvre par une issue de
cadrage qui produit sa spec. Le travail se découpe ensuite en issues rattachées
à ce milestone. Voir la [feuille de route](docs/feuille-de-route.md).

Le travail se fait sur des branches `feat/…`, `fix/…`, `docs/…`. La branche
`main` est protégée contre la réécriture d'historique.

### Définition de « terminé »

Une issue n'est close que lorsque tout ceci est vrai :

- [ ] `make check` passe
- [ ] Le comportement ajouté ou corrigé est couvert par des tests
- [ ] Un benchmark existe si la performance est en jeu
- [ ] La section de manuel concernée est à jour
- [ ] `make changelog` a été régénéré
- [ ] Une ADR est écrite si une décision coûteuse à défaire a été prise

Rédiger le manuel fait partie du travail, pas d'une passe de rattrapage :
formuler en français le comportement attendu est un test de conception.

## Conventions

### Langue

Documentation, specs, ADR, manuels, messages de commit et issues en **français**.
Code C++, identifiants, commentaires et intitulés de tests en **anglais**.

### Commits

[Conventional Commits](https://www.conventionalcommits.org/fr/), vérifiés par un
hook local et par la CI.

```
<type>(<scope>): <description>
```

- **types** : `feat`, `fix`, `perf`, `refactor`, `test`, `docs`, `build`, `ci`,
  `chore`, `revert`
- **scopes** : `build`, `ci`, `cli`, `core`, `doc`, `format`, `gui`, `i18n`,
  `scripts`, `test`, `text`, `video` — les mêmes que les labels `area:` des
  issues
- en-tête de 72 caractères au plus, sans point final

Le `CHANGELOG.md` étant généré depuis ces messages, un message bâclé produit une
entrée bâclée.

### Code

Les règles permanentes sont dans
[docs/principes-de-conception.md](docs/principes-de-conception.md) : modèles de
données typés, propriété mémoire explicite, abstraction là où la variation est
connue, testabilité par construction, performance mesurée.

Le nommage et le format sont appliqués mécaniquement par `.clang-format` et
`.clang-tidy`, pas laissés à la vigilance.

**Toute exclusion clang-tidy porte sa justification en commentaire.** Une
exclusion non justifiée est un défaut, pas un réglage.

## Commandes

| Commande | Effet |
| :------- | :---- |
| `make build` | compile le preset `dev` |
| `make test` | compile et exécute les tests (hors bout en bout — voir `make asan`) |
| `make check` | la porte de qualité complète |
| `make format` | applique clang-format |
| `make tidy` | analyse statique |
| `make asan` | tests sous AddressSanitizer et UndefinedBehaviorSanitizer |
| `make coverage` | rapport de couverture |
| `make bench` | benchmarks en Release |
| `make changelog` | régénère `CHANGELOG.md` |
| `make help` | toutes les cibles |
