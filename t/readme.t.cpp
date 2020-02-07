#include <tappp.hpp>
#include <vector>
#include <bitset>
#include <stdexcept>
#include <cstdlib>

using namespace TAP;

int main(void) {
	plan(10);

	diag("let's start slowly");
	pass("the first one's free");

	ok(1 < 255, "integer comparison works");
	is("55", 55, "pluggable comparison",
		[&](std::string s, int i) {
			return s == std::to_string(i);
		}
	);

	std::vector a{5,10,12};
	std::vector b{5,10,15};

	is(a[0], 5, "first element is 5");
	isnt(a[2], b[2], "last elements differ");

	TODO("they do differ, let's see");
	is(a[2], b[2], "give me diagnostics");
	TODO("compiles, works but can't diagnose");
	is(a, b, "differing vectors");

	SUBTEST("exercising exceptions") {
		throws<std::out_of_range>([&] { a.at(3); },
			"index 3 is out of bounds");

		throws([&] {
			std::bitset<5> mybitset(std::string("01234"));
		}, "bitset takes only bits");

		TODO("research correct exception type!");
		throws<std::domain_error>([&] {
			b.resize(b.max_size() + 1);
		}, "resizing too much leaves domain");

		done_testing();
	}

	b[2] = a[2] = b[2] * 2;
	is(b[2], 30, "changed last element");
	is(a, b, "vectors match now");

	return EXIT_SUCCESS;
}
