# Checks that the C++ compiler can produce a C++ executable.
#
# This test looks absurd until one meets the case: when /etc/alternatives/c++
# points to gcc instead of g++, the driver compiles C++ fine but does not link
# against libstdc++. The resulting error is an avalanche of "undefined
# reference to std::cout" in perfectly valid code, on which one can lose an
# hour.
#
# Better to fail here, with the cause and the remedy.

include(CheckCXXSourceCompiles)

check_cxx_source_compiles(
    "#include <string>
     int main() { return std::string(\"x\").empty() ? 1 : 0; }"
    SUBEDIT_CXX_TOOLCHAIN_LINKS)

if(NOT SUBEDIT_CXX_TOOLCHAIN_LINKS)
    message(
        FATAL_ERROR
        "The C++ compiler (${CMAKE_CXX_COMPILER}) compiles but does not link "
        "against the C++ standard library.\n"
        "Usual cause: the system's \"c++\" alternative points to gcc instead of g++.\n"
        "  check: ls -l /etc/alternatives/c++\n"
        "  fix: sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100\n"
        "  work around without administrator rights: CXX=g++ make build")
endif()
