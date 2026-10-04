# Fixtures de durée

Neuf petits fichiers (huit SubRip, un WebVTT), construits pour que **chaque paire
de contraintes de durée décide quelque part**, et chaque recouvrement. Ils servent
au recoupement de `subedit-cli adjust` avec
[`measure-duration-constraints.py`](../../../scripts/measure-duration-constraints.py)
— issue [#560](https://github.com/Guyot-Bertrand/sub-edit/issues/560), qui tient le renvoi de #407.

**Les comptes attendus sont écrits à la main**, à partir des définitions, dans
[`attendus/json/recoupement-defaut.jsonl`](../attendus/json/recoupement-defaut.jsonl)
(vitesse 15 caractères par seconde qui allonge, minimum 1,5 s, maximum 6 s, écart 0) et
[`recoupement-ecart.jsonl`](../attendus/json/recoupement-ecart.jsonl) (le même avec un écart de
0,5 s, sans quoi une place négative n'existe pas dans un fichier chronologique). Les
deux outils y sont confrontés :

```console
$ ./src/scripts/measure-duration-constraints.py --check-fixtures    # le script (make check-local)
$ ./build/dev/bin/subedit_e2e_test "[CLI-ADJUST-06]"               # le noyau
```

Ne pas les éditer sans refaire le calcul à la main : **un attendu qu'on remplace par ce
que l'outil écrit ne prouve plus que son accord avec lui-même.**

La place d'un sous-titre est `suivant.début − début − écart`, le besoin
`longueur × 1000 / vitesse` arrondi à la milliseconde ; `S1` est l'écart 0, `S2` l'écart 0,5 s.
Chaque fichier a deux sous-titres, sauf `ecart.srt` qui en a trois.

| Fichier | Ce qu'il isole | S1 (vitesse, minimum, écart) | S2 |
| :------ | :------------- | :--------------------------- | :-- |
| `minimum-contre-ecart.srt` | une place de 1,0 s pour un minimum de 1,5 s | 0, 1, 0 | 0, 1, 0 |
| `vitesse-contre-ecart.srt` | 30 caractères (2,0 s) pour une place de 1,8 s | 1, 0, 0 | 1, 1, 0 |
| `vitesse-contre-maximum.srt` | 100 caractères (6,667 s) au-dessus du maximum de 6 s | 1, 0, 0 | 1, 0, 0 |
| `union.srt` | les deux à la fois : **compte pour un, non pour deux** | 1, 0, 0 | 1, 0, 0 |
| `ecart.srt` | deux sous-titres qui commencent à 0,3 s l'un de l'autre | 0, 1, 0 | 1, 1, 1 |
| `balises.srt` | `vitesse-contre-ecart` avec des balises, qui ne comptent pas | 1, 0, 0 | 1, 1, 0 |
| `deux-lignes.srt` | un texte de deux lignes : le saut de ligne compte (31 caractères) | 1, 0, 0 | 1, 0, 0 |
| `arrondi.srt` | « Hi » : 133,33 ms arrondis à 133, pour une place de 133 ms | 0, 1, 0 | 1, 1, 1 |
| `lecture.vtt` | `minimum-contre-ecart`, en WebVTT | 0, 1, 0 | 0, 1, 0 |

`arrondi.srt` est celui qui a trouvé un défaut : le script comparait en flottants, et comptait
une vitesse sacrifiée que le noyau, qui arrondit, ne compte pas.
