#include <subedit/core/io/file_system.hpp>

#include <catch2/catch_test_macros.hpp>

#include <system_error>

namespace {

using subedit::core::FileErrorKind;
using subedit::core::fileErrorKindOf;

std::error_code codeOf(std::errc value) {
    return std::make_error_code(value);
}

} // namespace

TEST_CASE("a missing file is recognised as such", "[format][filesystem]") {
    CHECK(fileErrorKindOf(codeOf(std::errc::no_such_file_or_directory)) == FileErrorKind::NotFound);
}

TEST_CASE("a refused permission is recognised as such", "[format][filesystem]") {
    // Testing the mapping against the error codes themselves, rather than
    // against a device that has to be made to refuse: a test that needs a
    // read-only mount or a chmod would pass or fail depending on who runs it.
    CHECK(fileErrorKindOf(codeOf(std::errc::permission_denied)) == FileErrorKind::PermissionDenied);
}

TEST_CASE("anything else the system refuses lands in the same bucket", "[format][filesystem]") {
    // Three cases, because three are what a caller can act upon: retry
    // elsewhere, ask for rights, or give up. Splitting further would give the
    // interface distinctions it has nothing to do with.
    CHECK(fileErrorKindOf(codeOf(std::errc::directory_not_empty)) == FileErrorKind::Io);
    CHECK(fileErrorKindOf(codeOf(std::errc::no_space_on_device)) == FileErrorKind::Io);
    CHECK(fileErrorKindOf(codeOf(std::errc::cross_device_link)) == FileErrorKind::Io);
}
