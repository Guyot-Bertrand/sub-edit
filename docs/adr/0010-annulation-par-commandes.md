# 0010 — Annuler par des commandes portant leur propre inverse

**Date :** 2026-08-05
**Statut :** acceptée

## Contexte

L'annulation n'apparaît pas dans les priorités de l'utilisateur, mais éditer
dans une table l'implique, et c'est typiquement ce qu'on ne peut pas ajouter
après coup : la moindre opération écrite sans elle devra être reprise.

Gaupol la traite ainsi : chaque action réversible porte une **fonction inverse**
et ses arguments (`RevertableAction.revert_function`, `revert_args`). Annuler
consiste à appeler cette fonction avec `register=UNDO`, ce qui la fait s'empiler
sur la pile de rétablissement. Défaire et refaire empruntent donc la même
machinerie. L'état antérieur n'est pas capturé en bloc mais **dans les arguments
de l'opération inverse**, donc réduit au strict nécessaire : supprimer des
sous-titres enregistre comme inverse « insérer ces sous-titres à ces indices ».

## Décision

Une **commande** est un objet portant `apply` et `revert`, ainsi que ce qu'il
faut pour se défaire — capturé à la construction, et limité au nécessaire.
L'historique tient deux piles de commandes.

Trois mécanismes repris de Gaupol, parce qu'ils répondent à des besoins réels :

- **Regroupement.** Une commande composite contient une suite de commandes et ne
  compte que pour une entrée dans l'historique. Une opération qui en enchaîne
  d'autres ne doit pas obliger l'utilisateur à annuler sept fois.
- **Compteur de modification par document.** Un entier décalé de ±1 à chaque
  action ; zéro signifie « identique au fichier sur disque ». L'état revient
  donc à « non modifié » quand on annule jusqu'au point de sauvegarde, ce qu'un
  simple booléen ne sait pas faire.
- **Indices des éléments touchés.** Chaque commande rapporte les indices
  qu'elle affecte, et à quel titre — positions, texte principal, texte de
  traduction, insertion, suppression. C'est ce qui permettra à l'interface de la
  phase 5 de ne rafraîchir que ce qui change au lieu de tout reconstruire.

## Alternatives écartées

- **Capturer l'état complet avant et après** — robuste, et impossible à écrire
  de travers. Écarté sur le coût : un remplacement global sur un fichier de
  plusieurs milliers de sous-titres dupliquerait l'ensemble à chaque opération,
  et l'historique le conserverait. C'est précisément le genre de gaspillage que
  la performance comme objectif transversal proscrit.
- **Journal de différences** — économe, mais impose de définir un format de
  différence pour chaque type de donnée, et de le maintenir à chaque évolution
  du modèle. Complexité sans contrepartie à cette échelle.
- **`QUndoStack` de Qt** — existe déjà et fait le travail. Écarté sans hésiter :
  il placerait l'historique dans le code Qt, alors que la ligne de commande de
  la phase 3 en a besoin et que le cœur ne connaît pas Qt. L'interface s'y
  branchera, elle n'en tiendra pas une seconde.
- **Fonction inverse et arguments, à l'identique de Gaupol** — c'est la même
  idée, exprimée en Python. En C++, un objet commande la porte plus clairement
  qu'un pointeur de fonction accompagné d'un sac d'arguments, et le compilateur
  vérifie que l'inverse reçoit ce qu'il attend.

## Conséquences

Toute opération du noyau s'écrit comme une commande, dès la phase 2. Une
fonction qui modifie un projet sans passer par là est un défaut, et cette règle
doit être posée avant qu'une seule opération existe — c'est la raison d'être de
cette décision en phase 1.

Chaque commande doit savoir se défaire **exactement**. Une opération non
inversible — s'il en apparaît une — devra vider la pile plutôt que de mentir sur
sa réversibilité.

La taille de l'historique est bornée. Gaupol fixe une limite de 100 000, soit en
pratique aucune ; nous mesurerons l'empreinte réelle avant de choisir un chiffre
plutôt que de recopier le sien.

L'historique appartient au projet et vit dans le cœur, sans dépendance à
l'interface. Il est donc testable sans fenêtre, et utilisable par la ligne de
commande.
