#pragma once

// Concepts used to assert that an operation does *not* compile.
//
// A bare `requires(T a) { a + 1; }` written outside a template is a hard error
// in GCC rather than a false requirement: substitution only happens in a
// template context. Naming the concepts gives us that context, and lets a test
// state «this combination is rejected by the compiler» as an assertion.

namespace subedit::test {

template<typename Left, typename Right>
concept Addable = requires(Left left, Right right) { left + right; };

template<typename Left, typename Right>
concept Subtractable = requires(Left left, Right right) { left - right; };

template<typename Left, typename Right>
concept Multipliable = requires(Left left, Right right) { left* right; };

template<typename Left, typename Right>
concept Ordered = requires(Left left, Right right) { left < right; };

} // namespace subedit::test
