// The one test that touches a real disk, because it is the one place where
// the disk is the subject. Everything else goes through InMemoryFileSystem.

#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/find_executable.hpp>
#include <subedit/core/io/find_video.hpp>
#include <subedit/core/io/real_file_system.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace {

using subedit::core::FileError;
using subedit::core::FileErrorKind;
using subedit::core::findExecutable;
using subedit::core::findVideoBeside;
using subedit::core::RealFileSystem;
using subedit::core::writeAtomically;

/// A directory that removes itself, so that a failing assertion cannot leave
/// anything behind in the temporary directory of the machine.
class ScratchDirectory {

public:
    ScratchDirectory() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path = std::filesystem::temp_directory_path() /
                 ("subedit-test-" + std::to_string(stamp) + "-" + std::to_string(nextSerial()));
        std::filesystem::create_directories(m_path);
    }

    ScratchDirectory(const ScratchDirectory&) = delete;
    ScratchDirectory(ScratchDirectory&&) = delete;
    ScratchDirectory& operator=(const ScratchDirectory&) = delete;
    ScratchDirectory& operator=(ScratchDirectory&&) = delete;

    ~ScratchDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(m_path, ignored);
    }

    [[nodiscard]] std::filesystem::path file(const std::string& name) const {
        return m_path / name;
    }

private:
    /// The clock alone could hand out the same value twice; the serial makes
    /// two directories created in the same instant distinct anyway.
    [[nodiscard]] static int nextSerial() {
        static int serial = 0;
        return ++serial;
    }

    std::filesystem::path m_path;
};

} // namespace

TEST_CASE("a file written to disk reads back byte for byte", "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;
    const std::filesystem::path path = scratch.file("dialogue.srt");
    // A null byte and a CR, to prove the stream is not translating anything.
    const std::string content{"ligne\r\nsuite\0fin", 16};

    REQUIRE(files.writeFile(path, content).has_value());

    const std::expected<std::string, FileError> read = files.readFile(path);
    REQUIRE(read.has_value());
    CHECK(*read == content);
}

TEST_CASE("reading a file that is not on disk fails", "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    const RealFileSystem files;

    const std::expected<std::string, FileError> read = files.readFile(scratch.file("absent.srt"));

    REQUIRE_FALSE(read.has_value());
    CHECK(read.error().kind == FileErrorKind::NotFound);
}

TEST_CASE("existence, renaming and removal follow the disk", "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;
    const std::filesystem::path path = scratch.file("un.srt");
    const std::filesystem::path other = scratch.file("deux.srt");

    CHECK_FALSE(files.exists(path));
    REQUIRE(files.writeFile(path, "contenu").has_value());
    CHECK(files.exists(path));

    REQUIRE(files.rename(path, other).has_value());
    CHECK_FALSE(files.exists(path));
    CHECK(files.exists(other));

    REQUIRE(files.remove(other).has_value());
    CHECK_FALSE(files.exists(other));
}

TEST_CASE("renaming a file that is not on disk fails", "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;

    const std::expected<void, FileError> renamed =
        files.rename(scratch.file("absent.srt"), scratch.file("autre.srt"));

    REQUIRE_FALSE(renamed.has_value());
    CHECK(renamed.error().kind == FileErrorKind::NotFound);
}

TEST_CASE("removing a directory that still holds something fails", "[format][filesystem][disk]") {
    // The path where the system answers with an error code rather than with a
    // plain « nothing removed ».
    const ScratchDirectory scratch;
    RealFileSystem files;
    const std::filesystem::path directory = scratch.file("plein");
    std::filesystem::create_directories(directory);
    REQUIRE(files.writeFile(directory / "dedans.srt", "contenu").has_value());

    const std::expected<void, FileError> removed = files.remove(directory);

    REQUIRE_FALSE(removed.has_value());
    CHECK(removed.error().kind == FileErrorKind::Io);
}

TEST_CASE("removing what is not on disk fails", "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;

    const std::expected<void, FileError> removed = files.remove(scratch.file("absent.srt"));

    REQUIRE_FALSE(removed.has_value());
    CHECK(removed.error().kind == FileErrorKind::NotFound);
}

TEST_CASE("an atomic write on disk replaces the file and leaves nothing beside it",
          "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;
    const std::filesystem::path path = scratch.file("dialogue.srt");
    REQUIRE(files.writeFile(path, "ancien").has_value());

    REQUIRE(writeAtomically(files, path, "nouveau").has_value());

    const std::expected<std::string, FileError> read = files.readFile(path);
    REQUIRE(read.has_value());
    CHECK(*read == "nouveau");
    CHECK_FALSE(files.exists(scratch.file("dialogue.srt.subedit-tmp")));
}

TEST_CASE("writing into a directory that does not exist fails without a crash",
          "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;

    const std::expected<void, FileError> written =
        files.writeFile(scratch.file("absent/dialogue.srt"), "contenu");

    REQUIRE_FALSE(written.has_value());
    CHECK(written.error().kind == FileErrorKind::Io);
}

// **La seule chose que `InMemoryFileSystem` ne peut pas éprouver** : il n'a pas
// de répertoires, un chemin y est une clé, et son `createDirectories` est donc
// un succès qui ne crée rien. Ce qui compte — que l'arborescence manquante
// apparaisse, et qu'un répertoire déjà là ne soit pas une erreur — ne se voit
// que sur un vrai disque.
TEST_CASE("on disk, the missing directories above a file are made", "[filesystem][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;
    const std::filesystem::path nested = scratch.file("config/subedit/settings.conf");

    REQUIRE(files.createDirectories(nested.parent_path()).has_value());

    CHECK(std::filesystem::is_directory(nested.parent_path()));
    // Et l'écriture qui suit, qui est la seule raison de les avoir faits.
    CHECK(writeAtomically(files, nested, "window.maximised = true\n").has_value());
    CHECK(files.readFile(nested).value_or("") == "window.maximised = true\n");
}

TEST_CASE("on disk, a directory that is already there is not a failure", "[filesystem][disk]") {
    // Le cas courant : on demande avant d'écrire, et il existe presque
    // toujours. Un refus ici ferait écrire à chaque appelant le « sauf s'il
    // existe » que cette fonction lui épargne.
    const ScratchDirectory scratch;
    RealFileSystem files;

    REQUIRE(files.createDirectories(scratch.file("deux/niveaux")).has_value());

    CHECK(files.createDirectories(scratch.file("deux/niveaux")).has_value());
}

TEST_CASE("on disk, a directory that cannot be made says so", "[filesystem][disk]") {
    // **Un échec provoqué sans droits ni bricolage** : un fichier ne peut pas
    // devenir un répertoire, et rien ne peut vivre dessous. Le système répond
    // ENOTDIR, ce qui est exactement le chemin d'erreur qu'on veut voir passer
    // — et il ne laisse rien derrière lui.
    const ScratchDirectory scratch;
    RealFileSystem files;
    const std::filesystem::path blocker = scratch.file("un-fichier");
    REQUIRE(files.writeFile(blocker, "").has_value());

    CHECK_FALSE(files.createDirectories(blocker / "dessous").has_value());
}

TEST_CASE("on disk, a program is told apart from an ordinary file and from a directory",
          "[filesystem][executable][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;

    const std::filesystem::path program = scratch.file("faux-lecteur");
    REQUIRE(files.writeFile(program, "#!/bin/sh\nexit 0\n").has_value());
    std::filesystem::permissions(
        program, std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);

    const std::filesystem::path plain = scratch.file("dialogue.srt");
    REQUIRE(files.writeFile(plain, "contenu").has_value());

    const std::filesystem::path directory = scratch.file("bin");
    std::filesystem::create_directories(directory);

    CHECK(files.isExecutable(program));
    // A directory is searchable, which is a different thing from runnable, and
    // the difference is exactly what a search on a `PATH` must not confuse.
    CHECK_FALSE(files.isExecutable(directory));
    CHECK_FALSE(files.isExecutable(plain));
    CHECK_FALSE(files.isExecutable(scratch.file("absent")));
}

TEST_CASE("on disk, the real ffprobe is found through the search path or is absent",
          "[filesystem][executable][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;

    const std::filesystem::path program = scratch.file("ffprobe");
    REQUIRE(files.writeFile(program, "#!/bin/sh\nexit 0\n").has_value());
    std::filesystem::permissions(
        program, std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);

    const std::string searchPath = program.parent_path().string();
    CHECK(findExecutable(files, "ffprobe", searchPath) == program);
    // The same machine, told to look somewhere else: this is how a test walks
    // the branch where the program is not installed, on a real disk.
    CHECK_FALSE(findExecutable(files, "ffprobe", "/nowhere/at/all").has_value());
}

TEST_CASE("on disk, a directory hands back its files, sorted, without its directories",
          "[filesystem][video][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;

    // Written in an order that is not the sorted one, so that a listing left
    // to the device would have to be lucky to pass.
    REQUIRE(files.writeFile(scratch.file("film.mkv"), "").has_value());
    REQUIRE(files.writeFile(scratch.file("dialogue.srt"), "").has_value());
    std::filesystem::create_directories(scratch.file("archives"));

    const std::filesystem::path directory = scratch.file("film.mkv").parent_path();
    const auto listed = files.filesIn(directory);

    REQUIRE(listed.has_value());
    CHECK(*listed == std::vector<std::filesystem::path>{scratch.file("dialogue.srt"),
                                                        scratch.file("film.mkv")});
}

TEST_CASE("on disk, a directory that is not there refuses", "[filesystem][video][disk]") {
    const ScratchDirectory scratch;
    const RealFileSystem files;

    const auto listed = files.filesIn(scratch.file("nulle-part"));

    REQUIRE_FALSE(listed.has_value());
    CHECK(listed.error().kind == FileErrorKind::NotFound);
}

// The convention of D5 against a real disk, on files the repository carries:
// everything else about it is proved in memory, and this is what proves that
// the rule and the device agree.
TEST_CASE("on disk, the film beside a subtitle file is found", "[filesystem][video][disk]") {
    const RealFileSystem files;
    const std::filesystem::path neighbourhood =
        std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "voisinage";

    CHECK(findVideoBeside(files, neighbourhood / "film.fr.srt") == neighbourhood / "film.mkv");
    CHECK_FALSE(findVideoBeside(files, neighbourhood / "orphelin.srt").has_value());
}
