# Vérifie que le compilateur C++ sait produire un exécutable C++.
#
# Ce test paraît absurde jusqu'à ce qu'on rencontre le cas : lorsque
# /etc/alternatives/c++ pointe sur gcc au lieu de g++, le pilote compile bien
# le C++ mais n'édite pas les liens avec libstdc++. L'erreur qui en résulte est
# une avalanche de « référence indéfinie vers std::cout » dans du code
# parfaitement valide, sur laquelle on peut perdre une heure.
#
# Mieux vaut échouer ici, avec la cause et le remède.

include(CheckCXXSourceCompiles)

check_cxx_source_compiles(
    "#include <string>
     int main() { return std::string(\"x\").empty() ? 1 : 0; }"
    SUBEDIT_CXX_TOOLCHAIN_LINKS)

if(NOT SUBEDIT_CXX_TOOLCHAIN_LINKS)
    message(
        FATAL_ERROR
        "Le compilateur C++ (${CMAKE_CXX_COMPILER}) compile mais n'édite pas les liens "
        "avec la bibliothèque standard C++.\n"
        "Cause habituelle : l'alternative « c++ » du système pointe sur gcc au lieu de g++.\n"
        "  vérifier : ls -l /etc/alternatives/c++\n"
        "  corriger : sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100\n"
        "  contourner sans droits administrateur : CXX=g++ make build")
endif()
