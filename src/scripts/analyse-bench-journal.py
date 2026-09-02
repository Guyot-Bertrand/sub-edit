#!/usr/bin/env python3
"""Pèse les critères d'admission dans l'enveloppe, sur les relevés du journal.

`docs/mesures/performances.md` garde deux choses : une table d'extrêmes, et les
relevés qui l'alimentent. Un relevé n'alimente l'enveloppe que s'il est jugé
propre — aujourd'hui sur la charge de la machine. Ce script existe pour que ce
jugement puisse être **mesuré plutôt que discuté**, à partir des relevés que le
dépôt porte déjà.

    ./src/scripts/analyse-bench-journal.py

Il n'entre dans aucune porte, et rien ne dépend de lui : c'est un instrument,
pas une vérification. Il est né de l'issue #168, qui affirmait que le seuil de
charge laissait entrer du bruit — et dont il a montré le contraire.

Ce qu'il calcule, et le vocabulaire qui va avec :

  pointe   une mesure au-delà de 1,5 fois la médiane de ses voisines dans le
           temps, ±FENETRE relevés. La fenêtre est centrée : une évolution
           réelle du code y est comprise, une pointe passagère non.
  allure   le rapport médian, mesure par mesure, entre un relevé et la médiane
           des FENETRE précédents. Ce que « ce relevé est-il lent ? » veut dire
           quand on ne veut pas s'en remettre à la charge.

Un critère se juge sur deux nombres à la fois : combien de mesures il admet, et
combien de pointes il laisse passer avec. Admettre peu est facile ; admettre
beaucoup sans bruit ne l'est pas.
"""

import re
import statistics
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
JOURNAL = REPO_ROOT / "docs" / "mesures" / "performances.md"

FENETRE = 3
POINTE = 1.5
UNITES = {"ns": 1.0, "µs": 1e3, "ms": 1e6, "s": 1e9}

EN_TETE = re.compile(
    r"^### (?P<version>\S+) — (?P<date>\d{4}-\d{2}-\d{2})"
    r" — (?P<mode>[^—\n]+?) — charge (?P<charge>\S+)"
)


def duree(cellule):
    """Relit une durée du journal, en nanosecondes."""
    trouve = re.match(r"^([0-9.]+)\s*(ns|µs|ms|s)$", cellule.strip())
    return float(trouve.group(1)) * UNITES[trouve.group(2)] if trouve else None


def releves(texte):
    """Les relevés du journal, du plus ancien au plus récent."""
    corps = texte.split("<!-- relevés -->", 1)[-1]
    trouves, courant = [], None
    for ligne in corps.splitlines():
        en_tete = EN_TETE.match(ligne)
        if en_tete:
            try:
                charge = float(en_tete.group("charge"))
            except ValueError:
                charge = None
            courant = {"version": en_tete.group("version"), "charge": charge,
                       "mesures": {}, "dispersions": {}}
            trouves.append(courant)
            continue
        if courant is None:
            continue
        cellules = [c.strip() for c in ligne.split("|")]
        if len(cellules) == 5 and cellules[1] not in ("Mesure", ":-----", ""):
            valeur, ecart = duree(cellules[2]), duree(cellules[3])
            if valeur:
                courant["mesures"][cellules[1]] = valeur
                if ecart is not None:
                    courant["dispersions"][cellules[1]] = ecart / valeur
    trouves.reverse()
    return trouves


def allures(trouves):
    """Pose sur chaque relevé son allure face aux FENETRE précédents."""
    for rang, releve in enumerate(trouves):
        precedents = trouves[max(0, rang - FENETRE):rang]
        rapports = []
        for nom, valeur in releve["mesures"].items():
            anciennes = [p["mesures"][nom] for p in precedents if nom in p["mesures"]]
            if len(anciennes) < 2:
                continue
            base = statistics.median(anciennes)
            if base > 0:
                rapports.append(valeur / base)
        releve["allure"] = statistics.median(rapports) if len(rapports) >= 5 else None


def mesures_comparables(trouves):
    """Chaque mesure, avec son rapport à ses voisines dans le temps."""
    suites = {}
    for rang, releve in enumerate(trouves):
        for nom, valeur in releve["mesures"].items():
            suites.setdefault(nom, []).append((rang, valeur))

    comparables = []
    for nom, points in suites.items():
        for index, (rang, valeur) in enumerate(points):
            debut, fin = max(0, index - FENETRE), min(len(points), index + FENETRE + 1)
            voisines = [v for j, (_, v) in enumerate(points[debut:fin], start=debut) if j != index]
            if len(voisines) < 3:
                continue
            base = statistics.median(voisines)
            if base <= 0:
                continue
            comparables.append({
                "nom": nom,
                "version": trouves[rang]["version"],
                "rapport": valeur / base,
                "charge": trouves[rang]["charge"],
                "allure": trouves[rang]["allure"],
            })
    return comparables


def main():
    if not JOURNAL.exists():
        print(f"journal introuvable : {JOURNAL}", file=sys.stderr)
        return 1

    trouves = releves(JOURNAL.read_text(encoding="utf-8"))
    if not trouves:
        print("aucun relevé lisible dans le journal", file=sys.stderr)
        return 1
    allures(trouves)
    comparables = mesures_comparables(trouves)
    pointes = [m for m in comparables if m["rapport"] > POINTE]

    print(f"{len(trouves)} relevés, {len(comparables)} mesures comparables à leurs voisines,")
    print(f"dont {len(pointes)} pointes au-delà de ×{POINTE}.")
    print()

    def peser(intitule, retient):
        gardees = [m for m in comparables if retient(m)]
        sales = [m for m in gardees if m["rapport"] > POINTE]
        part = f"{100 * len(sales) / len(gardees):.1f} %" if gardees else "—"
        print(f"  {intitule:<38} {len(gardees):4d} mesures, {len(sales):3d} pointes ({part})")
        return sales

    # La dispersion médiane de chaque mesure, sur tous les relevés conservés.
    # C'est à elle que se compare un relevé, et non à un pourcentage écrit
    # d'avance : la dispersion naturelle va de 3,8 % pour la mise à l'échelle
    # d'un rationnel à 43,6 % pour un décalage de quatre mille sous-titres.
    dispersions = {}
    for releve in trouves:
        for nom, part in releve["dispersions"].items():
            dispersions.setdefault(nom, []).append(part)
    mediane = {nom: statistics.median(parts) for nom, parts in dispersions.items()}
    for mesure in comparables:
        releve = next(r for r in trouves if r["version"] == mesure["version"])
        mesure["dispersion"] = releve["dispersions"].get(mesure["nom"])
        mesure["dispersion_usuelle"] = mediane.get(mesure["nom"])

    print("ce que chaque critère admettrait dans l'enveloppe :")
    peser("tout admettre", lambda m: True)
    peser("charge < 1,5 — la règle en place",
          lambda m: m["charge"] is not None and m["charge"] < 1.5)
    laissees = peser("allure ≤ 1,20", lambda m: m["allure"] is not None and m["allure"] <= 1.20)
    peser("les deux à la fois",
          lambda m: m["charge"] is not None and m["charge"] < 1.5
          and m["allure"] is not None and m["allure"] <= 1.20)

    def dispersee(mesure, facteur):
        if mesure["dispersion"] is None or not mesure["dispersion_usuelle"]:
            return True
        return mesure["dispersion"] <= facteur * mesure["dispersion_usuelle"]

    for facteur in (1.0, 1.25, 1.5, 2.0):
        peser(f"charge, et dispersion <= {facteur:g} x sa médiane",
              lambda m, f=facteur: m["charge"] is not None and m["charge"] < 1.5
              and dispersee(m, f))

    # ## Les maxima posés, et ceux que la suite dément — issue #202
    #
    # **La pointe n'est pas la bonne lunette pour cette question.** Elle voit ce
    # qui dépasse d'une moitié ses voisines ; un maximum se pose en dépassant le
    # précédent d'un cheveu, et il est définitif. Le relevé qui a ouvert #202
    # n'était pas une pointe — il valait ×1,15 — et il a fixé l'enveloppe pour
    # toujours.
    #
    # Ce qui se compte ici est donc : combien de maxima un critère laisse poser,
    # et combien d'entre eux les relevés suivants démentent — la médiane des cinq
    # suivants retombant sous quatre cinquièmes du maximum posé.
    print()
    print("maxima posés, et ceux que les relevés suivants démentent :")

    suites = {}
    for rang, releve in enumerate(trouves):
        for nom, valeur in releve["mesures"].items():
            suites.setdefault(nom, []).append((rang, valeur))

    def compter(intitule, retient):
        poses = dementis = 0
        for nom, points in suites.items():
            haut = None
            for index, (rang, valeur) in enumerate(points):
                releve = trouves[rang]
                part = releve["dispersions"].get(nom)
                if not retient(releve, nom, part):
                    continue
                if haut is None or valeur > haut:
                    haut = valeur
                    suivantes = [v for _, v in points[index + 1:index + 6]]
                    if len(suivantes) >= 3:
                        poses += 1
                        if statistics.median(suivantes) < 0.8 * valeur:
                            dementis += 1
        part = f"{100 * dementis / poses:.1f} %" if poses else "—"
        print(f"  {intitule:<38} {poses:4d} posés, {dementis:3d} démentis ({part})")

    calme = lambda r, n, p: r["charge"] is not None and r["charge"] < 1.5
    compter("charge < 1,5 — la règle d'avant #202", calme)
    for facteur in (1.0, 1.25, 1.5, 2.0):
        compter(f"charge, et dispersion <= {facteur:g} x sa médiane",
                lambda r, n, p, f=facteur: calme(r, n, p)
                and (p is None or not mediane.get(n) or p <= f * mediane[n]))

    if laissees:
        print()
        print("les pointes que l'allure laisserait entrer, et la charge non :")
        for mesure in sorted(laissees, key=lambda m: -m["rapport"])[:8]:
            if mesure["charge"] is not None and mesure["charge"] < 1.5:
                continue
            print(f"  ×{mesure['rapport']:5.2f}  {mesure['version']:<7}"
                  f"  allure ×{mesure['allure']:.2f}  charge {mesure['charge']:.2f}"
                  f"  {mesure['nom']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
