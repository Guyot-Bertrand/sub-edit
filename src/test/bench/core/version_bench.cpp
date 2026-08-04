// This benchmark measures nothing meaningful. It exists to prove the
// measurement harness works end to end, so that the benchmarks that do matter
// — pattern correction and line breaking — have somewhere to land.

#include <subedit/core/version.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("version formatting", "[benchmark]") {
    BENCHMARK("versionString") {
        return subedit::core::versionString();
    };
}
