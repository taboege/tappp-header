# tappp.hpp - Header-only C++ TAP producer

## SYNOPSIS

``` cpp
#include <tappp.hpp>
#include <vector>
#include <bitset>
#include <stdexcept>
#include <cstdlib>

using namespace TAP;

int main(void) {
    plan(8);

    diag("let's start slowly");
    pass("the first one's free");
    ok(1 < 255, "integer comparison works");

    std::vector a{5,10,12};
    std::vector b{5,10,15};

    is(a[0], 5, "first element is 5");
    isnt(a[2], b[2], "last elements differ");

    is("55", 55, "pluggable comparison",
        [&](std::string s, int i) {
            return s == std::to_string(i);
        }
    );

    SUBTEST("exercising exceptions") {
        throws<std::out_of_range>([&] { a.at(3); },
            "index 3 is out of bounds");

        throws([&] {
            std::bitset<5> mybitset(std::string("01234"));
        }, "bitset takes only bits");

        TODO("will fail, research correct exception type!");
        throws<std::domain_error>([&] {
            std::vector<int> myvector;
            myvector.resize(myvector.max_size() + 1);
        }, "resizing too much leaves domain");

        done_testing();
    }

    b[2] = a[2] = b[2] * 2;
    is(b[2], 30, "changed last element");
    is(a, b, "vectors match now");

    return EXIT_SUCCESS;
}
```

## DESCRIPTION

This library consists of a single header `tappp.hpp` which implements an
object-oriented [Test Anything Protocol](https://testanything.org/) producer
in C++17. For convenience, a procedural interface is written on top, which
becomes nice to use once you are `using namespace TAP`.

The library is short and completely documented in the source code.
Its interface is close to Perl's [Test::More](https://metacpan.org/pod/Test::More)
and Raku's [Test](https://docs.raku.org/type/Test) modules.

There are other C++ libraries of this kind. The author is aware of
[libperl++](https://github.com/Leont/libperl--) and
[libtap++](https://github.com/cbab/libtappp), the latter being a fork
of the former and this library taking inspiration from both.

I wouldn't have written this library if there weren't things to improve
in the existing ones, though. The advantages of `tappp.hpp` shared with
`libtap++` over `libperl++` are:

- The TAP producer is isolated into a single library.
- There is no compile-time boost dependency. Just pure C++17.

What I consider to be unique advantages over `libtap++`:

- `tappp.hpp` is just a single header, easy to include in projects.
- It has been (cleanly!) written from scratch to use modern C++ features.
- It supports subtests as a compatible TAP extension.

## TODO

The feature set of this library is already quite what I imagined, but other
TAP producers, such as [Raku's Test module](https://docs.raku.org/type/Test)
provide inspiration for more:

- `is_approx`: testing relative error for numerics
- `like` / `unlike`: compare string with regex
- `is_deeply`: recursive container unpacking

## SEE ALSO

- [Test Anything Protocol website](https://testanything.org/)
- [libperl++ includes the mother of all C++ TAP producers](https://github.com/Leont/libperl--)
- [libtap++ isolates the TAP part of libperl++](https://github.com/cbab/libtappp)

## AUTHOR

Tobias Boege <tobs@taboege.de>

## COPYRIGHT AND LICENSE

This software is copyright (C) 2020 by Tobias Boege.

This is free software; you can redistribute it and/or
modify it under the terms of the Artistic License 2.0.
