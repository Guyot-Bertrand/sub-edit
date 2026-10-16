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
- [ ] Le patch de `project(VERSION)` a été incrémenté dans `CMakeLists.txt`
- [ ] `make changelog` a été régénéré
- [ ] Le corps de la pull request porte une ligne `Closes #N`
- [ ] Une ADR est écrite si une décision coûteuse à défaire a été prise

Les trois points mécaniquement décidables de cette liste — le bump, le journal
et la ligne `Closes #N` — sont vérifiés par le job `contrôles de pull request`,
et n'ont donc plus à être relus. **`Closes` et non « Ferme » :** GitHub ne
ferme une issue que sur ses propres mots-clés, tous anglais. La règle de langue
du projet ne s'applique pas à cette ligne, qui est une instruction à GitHub et
non de la prose.

Rédiger le manuel fait partie du travail, pas d'une passe de rattrapage :
formuler en français le comportement attendu est un test de conception.

**Ses exemples d'appel, en revanche, ne s'écrivent pas : ils se déclarent.** Un
bloc `console` du manuel est précédé d'un marqueur qui nomme la commande à
illustrer, et `make manual` réécrit le bloc avec ce que cette commande produit
réellement.

````markdown
<!-- example: subedit-cli -->
```console
… réécrit par make manual …
```
````

Le marqueur porte une ligne de shell entière, ce qui permet d'illustrer un
enchaînement — `subedit-cli fichier.srt; echo $?` — comme un utilisateur
l'écrirait.

Une sortie recopiée à la main est de la documentation périmée en sursis : le
seul numéro de version que le manuel contenait a été corrigé douze fois pendant
la phase 2, sans que rien vérifie qu'il était vrai. `make check-local` refuse
désormais un manuel dont les exemples ne correspondent plus.

**Le manuel a deux lecteurs, et ils ne lisent pas le même Markdown.** GitHub le
rend dans le navigateur ; `QTextDocument::setMarkdown` le rend dans la fenêtre
qu'ouvre `Help ▸ Manual`. Le second est plus strict que le premier sur un point
au moins, mesuré en traitant l'issue #268 : **une ligne de séparation de tableau
demande au moins trois caractères par colonne** — `| :--- |` passe partout,
`| :- |` fait un tableau sur GitHub et du texte à barres dans la fenêtre. Trois
tableaux du manuel étaient dans ce cas, et rien ne le disait.

Il n'y a rien à retenir de plus : `src/test/gui/real_manual_test.cpp` ouvre
chaque page réelle dans une `ManualWindow` et confronte les tableaux, les
images et les renvois de la source à ce que le rendu en fait. Une page qui rend
mal fait échouer la porte, sur le nom de la page.

## Conventions

### Language

**Everything is written in English; French is a locale only** —
[ADR 0043](docs/adr/0043-english-is-the-language-of-the-repository.md). Code,
comments, scripts, build files, documentation, specs, ADRs, commit messages,
issues, pull requests and test titles are English. French lives in the
catalogue `src/po/fr.po` and the translated manual `docs/i18n/fr/manual/`.

The repository is mixed until the existing French is translated. Two checks of
`check-architecture.sh` hold the rule:

- `check-comment-language.py` refuses French in every tracked file outside the
  two locale trees, except the files named in `.french-allowlist`. The list only
  shrinks: translating a file means deleting its line, and a line that its
  version on `origin/main` does not have is refused.
- test case titles, which Catch2 prints, are held to a stricter rule: a
  non-ASCII character or a French function word fails the gate. A title that
  quotes a French datum — the fixture format has some — goes on the check's
  exemption list, with its reason.

A string the program shows is English in the source and goes through the
translation API — `translate`, `translateIn`, `translatePlural` or Qt's `tr` —
so that a catalogue can translate it. The `untranslated` step of `make check`
counts the user-visible literals that do not, file by file, against
`.untranslated-ratchet`: a new one fails the gate, and the count only goes down.
A literal that is not for a reader — a settings key, an option name, a JSON key,
a format's syntax — is exempted in `.translation-exemptions`, with its reason.

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
| `make ratchet` | enregistre la couverture mesurée comme nouveau cliquet |
| `make bench` | benchmarks en Release, versés dans `docs/mesures/performances.md` |
| `make changelog` | régénère `CHANGELOG.md` |
| `make manual` | régénère les exemples d'appel du manuel |
| `make rpm-check` | installe le `.rpm` sur une Fedora en conteneur — hebdomadaire, hors porte |
| `make help` | toutes les cibles |
