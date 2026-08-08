#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/order_policy.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {

using subedit::core::CommandKind;
using subedit::core::mayBreakOrder;

} // namespace

TEST_CASE("only what moves a start can break the order", "[edit][order]") {
    // The strict policy reads this and nothing else. Getting it wrong in one
    // direction appends a pointless sort; in the other, it leaves a file
    // disordered in the mode meant to prevent exactly that.
    CHECK(mayBreakOrder(CommandKind::SetStart));
    CHECK(mayBreakOrder(CommandKind::Insert));
    CHECK(mayBreakOrder(CommandKind::Shift));
    CHECK(mayBreakOrder(CommandKind::Transform));
    CHECK(mayBreakOrder(CommandKind::ConvertFrameRate));
}

TEST_CASE("what leaves the starts alone cannot break the order", "[edit][order]") {
    // A removal deserves its place here: taking lines out of an ordered
    // sequence leaves it ordered, so the strict mode has nothing to add.
    CHECK_FALSE(mayBreakOrder(CommandKind::SetText));
    CHECK_FALSE(mayBreakOrder(CommandKind::SetEnd));
    CHECK_FALSE(mayBreakOrder(CommandKind::Remove));
    CHECK_FALSE(mayBreakOrder(CommandKind::Sort));
}

TEST_CASE("the answer is known at compile time", "[edit][order]") {
    static_assert(mayBreakOrder(CommandKind::Shift));
    static_assert(!mayBreakOrder(CommandKind::SetText));
}
