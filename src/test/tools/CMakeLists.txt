# Les programmes que les tests lancent, et que le projet possède.
#
# Ce ne sont pas des tests : ce sont les tiers qu'un test a besoin de faire
# exister. Ils vivent ici plutôt que dans src/exe/ parce que rien de ce qui est
# livré ne les contient — aucune cible d'installation ne les nomme.

add_executable(subedit_fake_player fake_player.cpp)
subedit_set_warnings(subedit_fake_player)
