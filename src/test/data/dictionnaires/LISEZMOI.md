# Un dictionnaire d'essai, pour la jonction et la scission de mots

`zz.dic` et `zz.aff` : **six mots et rien d'autre**, au format de Hunspell, pour une langue
(`zz`) que nulle machine ne possède. Les cas de bout en bout de `correct --tasks join-words,split-words`
les déposent dans `hunspell/` sous le `ENCHANT_CONFIG_DIR` que le harnais a déplacé : Enchant les
trouve par son moteur Hunspell, et **aucun test ne dépend d'un dictionnaire installé sur la machine
qui le lance**. Issue [#569](https://github.com/Guyot-Bertrand/sub-edit/issues/569).

| Fichier | Ce que c'est |
| :------ | :----------- |
| `zz.dic` | le nombre de mots, puis `hello`, `there`, `world`, `how`, `are`, `you` — un par ligne |
| `zz.aff` | l'encodage (UTF-8) et l'ordre des lettres que Hunspell essaie pour proposer une correction |

**Ce que ces cas prouvent, et ce qu'ils ne prouvent pas.** Ils prouvent que la ligne de commande
ouvre un dictionnaire par Enchant, le donne à la jonction et à la scission, et que ce qu'elles
rendent arrive dans le fichier — avec des attendus écrits à la main, d'après la règle de Gaupol :
`hel lo` est `hello` seul quand une seule direction orthographie ; `hellothere` est `hello there`
seul quand une seule proposition est le mot avec une espace. **Ils ne prouvent rien d'un dictionnaire
réel** — la qualité de `fr_FR` n'est pas notre affaire — et **ils supposent le moteur Hunspell
d'Enchant**, qui est celui de la plupart des installations : sans lui, ils échouent en le disant.
