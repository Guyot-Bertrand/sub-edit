# Les fichiers attendus

Ce que telle sous-commande doit écrire pour telle entrée, octet pour octet. Un
test de bout en bout les compare à la sortie du programme avec `MatchesFile`
(`src/test/e2e/cli/cli_run.hpp`), qui dit la première ligne qui diffère. Issue
[#543](https://github.com/Guyot-Bertrand/sub-edit/issues/543).

**Un attendu ne s'engendre jamais par le programme testé.** Un fichier que l'outil
a écrit puis qu'un test relit ne prouve que l'accord de l'outil avec lui-même
(leçon de [#338](https://github.com/Guyot-Bertrand/sub-edit/issues/338)). Chaque
fichier ci-dessous dit d'où il vient.

| Fichier | Entrée | Provenance |
| :------ | :----- | :--------- |
| `mentions.hearing-impaired.srt` | `valides/mentions.srt`, sous-commande `hearing-impaired` | **écrit à la main**, d'après l'entrée et la règle du retrait : les mentions entre crochets et parenthèses partent, un sous-titre qu'elles vident disparaît et les suivants sont renumérotés, une référence numérique `[1]` reste, le tiret de dialogue resté seul s'en va, la mention coupée par le retour à la ligne emporte l'espace qui l'entourait et laisse deux lignes. La sortie réelle n'a servi qu'à vérifier cet attendu, jamais à le copier. |
