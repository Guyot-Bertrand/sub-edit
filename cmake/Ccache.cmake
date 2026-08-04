# Utilise ccache s'il est installé, l'ignore sinon.
#
# Aucune dépendance : ccache accélère les reconstructions, il n'en conditionne
# aucune.

find_program(CCACHE_EXECUTABLE ccache)

if(CCACHE_EXECUTABLE)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
    message(STATUS "ccache : ${CCACHE_EXECUTABLE}")
endif()
