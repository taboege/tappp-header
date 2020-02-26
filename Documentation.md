# tappp.hpp Documentation

This documentation assumes knowledge of the [Test Anything Protocol] (TAP).
The reader is advised to read its (very short) specification first.

This library has two layers: the first is an object-oriented layer where
producing TAP is encapsulated in a `TAP::Context` object. The second layer
is implemented on top and provides a global context object and free-standing
functions to manipulate it. This second layer is inteded to make writing
tests as convenient as [Test::More].

Almost every method of the object-oriented interface is mirrored exactly in
the convenience interface, so we document the former completely first and
only describe additions or changes to it in the latter.

[Test Anything Protocol]: https://testanything.org/
[Test::More]: https://metacpan.org/pod/Test::More

## Object-oriented interface

The one class implementing a TAP producer is `TAP::Context`. It contains
an `std::ostream` output device to which it prints TAP and various control
and statistical state required to produce accurate output. The methods of
`TAP::Context` are documented in the following sections.

Each assertion method produces exactly one TAP `ok` / `not ok` line on the
output device. This guarantee enables the user to maintain a test plan in
their tests. In addition, the methods make a best effort to produce useful
diagnostics when assertions fail. To learn more about this, see the section
[Diagnostics and stringifiability](#diagnostics-and-stringifiability).
Throughout, assertion methods take an optional `message` argument, which
is the description of the test.

### Constructor / Destructor

``` c++
Context(std::ostream& out = std::cout) { … }
Context(unsigned int tests, std::ostream& out = std::cout) { … }
Context(const skip_all& skip [[maybe_unused]], const std::string& reason = "", std::ostream& out = std::cout) { … }
```

The first variant creates a new empty Context object emitting to `out`.
The default output device is always `std::cout`. No plan line is printed.
You either have to call `plan` before any tests or `done_testing` after
the last one. The second variant uses `tests` to print a plan line.

The third variant is used to skip all tests. It receives as the first
argument the special value `SKIP_ALL` of `enum TAP::skip_all`. An optional
reason may be given. This results in a `1..0` plan line on the output
device. The Context is marked as finished.

``` c++
~Context(void) { … }
```

The destructor closes the TAP session by printing a final plan line if
none has been emitted yet.

### `plan`

``` c++
void plan(unsigned int tests) { … }
void plan(const skip_all& skip [[maybe_unused]], const std::string& reason = "") { … }
```

Print a plan line and remember the test plan in the context's state.

A plan line can only be printed before any assertion or after every
assertion. Calling `plan` after the first assertion throws the
`TAP::X::LatePlan` exception. Printing the plan after every assertion
is handled by the `done_testing` method and implicitly also by the
context destructor.

The test plan cannot be changed. Attempting to do so throws the
`TAP::X::Planned` exception.

### `done_testing`

``` c++
void done_testing(void) { … }
```

Close the TAP context, mark it as finished. If no plan line was emitted
before, this is done now according to how many assertions were done.

Attempting to print any more TAP lines (except `diag`) will result in a
`TAP::X::Finished` exception.

### `summary`

``` c++
bool summary(void) { … }
```

Whether all tests succeeded so far, taking into account that `TODO` tests
count as successful. If the context has a test plan, this will be false
until all tests have ran (and all were successful).

### `subtest`

``` c++
Context* subtest(const std::string& message = "") { … }
Context* subtest(unsigned int tests, const std::string& message = "") { … }
```

Derives a subtest from the invocant context. Subtests are a common
extension provided by TAP producers where a new, separate TAP stream
is printed with indentation (so that TAP parsers ignore it if they
don't understand it). The subtest can have its own plan. It always
uses the same output device as its parent.

When a subtest finishes, either by being destroyed or by `done_testing`
being called on it, it injects a single `ok` or `not ok` line into its
parent context, reflecting whether or not all assertions in the subtest
were successful (see `summary`). Consequently, a subtest must keep a
pointer to the context it was derived from. It is the user's responsibility
to keep the parent context alive for the subtest.

The arguments have the same meaning is in the `TAP::Context` constructor.

### `ok` / `nok`

``` c++
bool ok(bool is_ok, const std::string& message = "") { … }
bool nok(bool is_nok, const std::string& message = "") { … }
```

Write an "ok" or "not ok" line to the output device according to whether
`is_ok` is true or not or whether `is_nok` is false or not.

### `pass` / `fail`

``` c++
bool pass(const std::string& message = "") { … }
bool fail(const std::string& message = "") { … }
```

Pass or fail an assertion unconditionally.

### `diag`

``` c++
template<typename... Ts>
void diag(Ts... values) { … }
```

Print a diagnostic message on the TAP stream. The message is composed
by sending all the arguments to the output device in order. They must
be stringifiable.

### `is` / `isnt`

``` c++
template<typename T, typename U, typename Matcher = std::equal_to<T>>
bool is(const T& got, const U& expected, const std::string& message = "", Matcher m = Matcher()) { … }

template<typename T, typename U, typename Matcher = std::equal_to<T>>
bool isnt(const T& got, const U& unexpected, const std::string& message = "", Matcher m = Matcher()) { … }
```

Compare the two arguments according to a `Matcher` object that determines
if the values are "equal". The default matcher is `std::equal_to` which
imposes that the two values have the same type.

### `like` / `unlike`

``` c++
template<typename T>
bool like(const T& got, Predicate<T> p, const std::string& message = "") { … }

template<typename T>
bool unlike(const T& got, Predicate<T> p, const std::string& message = "") { … }
```

Check a value against a unary predicate. The `Predicate` type is just
an abbreviation for `std::function<bool(const T&)>` whose return `bool`
determines the ok-ness of the assertion.

``` c++
template<typename T>
bool like(const T& got, const std::string& pattern, const std::string& message = "") { … }

template<typename T>
bool unlike(const T& got, const std::string& pattern, const std::string& message = "") { … }
```

These variants compare the value against a regular expression using
default compilation flags (i.e. ECMAScript syntax). The `Predicate`
here is `std::regex_match` succeeding.

### `lives` / `throws` / `throws_like`

``` c++
bool lives(std::function<void(void)> f, const std::string& message = "") { … }
```

The given function `f` is executed in a `try` block. The assertion is ok
if no exception occurs.

``` c++
template<typename E = std::exception>
bool throws(std::function<void(void)> f, const std::string& message = "") { … }
```

The function `f` is executed in a `try` block. The assertion is ok if an
exception of type `E` is thrown. No exception or exceptions of incompatible
types fail the assertion.

``` c++
template<typename E = std::exception>
bool throws_like(std::function<void(void)> f, Predicate<E> p, const std::string& message = "") { … }

template<typename E = std::exception>
bool throws_like(std::function<void(void)> f, const std::string& pattern, const std::string& message = "") { … }
```

These variants are a mixture of `throws` and `like`. It runs the function
`f` under `try` and requires that an exception of type `E` happens which
additionally matches the predicate `p` or whose `what()` member matches a
regular expression.

### `TODO`

``` c++
void TODO(const std::string& reason = "-") { … }
```

Mark the next assertion as `TODO`. The TAP harness will disregard a failed
assertion marked TODO. If it succeeds, it is a bonus. The `reason` argument
must be non-empty to enable the TODO mark. Passing an empty string removes
the marking.

### `SKIP`

``` c++
void SKIP(const std::string& reason = "") { … }
void SKIP(unsigned int how_many, const std::string& reason = "") { … }
```

Skip one test by issuing a `pass` with the TAP `SKIP` directive. Note that
unlike `TODO` which adds a directive to the next regular assertion, the
`SKIP` method performs an assertion itself.

### `BAIL`

``` c++
void BAIL(const std::string& reason = "") { … }
```

Print a TAP `Bail out!` line which denotes an unexpected but orderly
termination of the test. The caller has to terminate the process.

## Exceptions

Exceptions thrown by tappp.hpp are all contained in a `TAP::X` namespace:

``` c++
struct TAP::X::Planned : std::runtime_error { … }
```

Thrown when a plan line has already been emitted but a change to it is
requested.

``` c++
struct TAP::X::Finished : std::runtime_error { … }
```

Thrown when `done_testing` or `BAIL` has been called already but more
state-changing TAP operations are requested.

``` c++
struct TAP::X::LatePlan : std::runtime_error { … }
```

Thrown when a plan line is requested through `plan` after the first test
line was printed. TAP only allows the plan line at the beginning or the
end. Printing it at the end is handled by `done_testing`.

## Diagnostics and stringifiability

In `is` and derived conversions, where one object is compared to another,
or `like`, where an object is tested for a predicate, diagnostics can be
printed about what the passed value and the expectation were in case they
don't match. tapp.hpp uses an `std::stringstream` to obtain a string
hopefully representing the values to the user --- but only if `operator<<`
can be called on an `std::ostream` with the respective value. If you want
to enable diagnostics for your types, provide such an overload.

## Convenience interface

The convenience interface is built around a global variable with internal
linkage called `TAP::TAPP` of type `std::shared_ptr<TAP::Context>`, which
is initialized to a default constructed object (emitting to stdout).

All methods on `TAP::Context` are available as free functions in the `TAP`
namespace which implicitly operate on `TAP::TAPP`.

Only the `subtest` function behaves a bit different from the `TAP::TAPP->subtest`
method. Instead of just constructing a derived `TAP::Context`, the free-standing
`subtest` function also wraps it in an `std::shared_ptr` and installs the new
subtest into the `TAP::TAPP` variable, thus making it the active `TAP::Context`,
and then returns a `TAP::Subtest::Guard` RAII object whose destructor
reinstates the previous context. The guard also keeps the parent context
alive by referencing its `std::shared_ptr`.

This allows you to switch the global context to a subtest temporarily (using
RAII semantics) and continue to use the same free-standing functions.
A `SUBTEST` macro is provided to hide the `TAP::Subtest::Guard`:

``` c++
#define SUBTEST(...)		\
    if constexpr (auto TAPP_SUBTEST = subtest(__VA_ARGS__); true)
```

It uses a phony `constexpr if` with an embedded variable declaration.
The `TAPP_SUBTEST` guard object's lifetime is bound to the block that
comes after the `SUBTEST` macro. Thanks to variable shadowing, nesting
subtests also works as expected:

``` c++
using namespace TAP;
SUBTEST("a first subtest") {
    plan(3);

    is(5 + 50, 55, "arithmetic is good");
    is("55", 55, "incompatible types but fitting matcher",
        [&](std::string s, int i) { return s == std::to_string(i); });
    SUBTEST("nested subtest") {
        pass("this works");
        diag("v-- without a plan(), it will be printed automatically");
    }
}
```

## Colophon

This document describes version v0.2.0 of tappp.hpp.

It was written by Tobias Boege and placed under the Creative Commons
[CC-BY-SA 4.0](http://creativecommons.org/licenses/by-sa/4.0/) license.
