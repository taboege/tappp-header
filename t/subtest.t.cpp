#include <tappp.hpp>
#include <vector>
#include <bitset>
#include <cstdlib>
#include <cmath>

using namespace TAP;

int main(void) {
	plan(4);

	ok(1 < 255, "numbers are good");

	SUBTEST("a first subtest") {
		plan(3);

		diag("hello from a subtest!");
		is(5 + 50, 55, "arithmetic is good");
		is("55", 55, "incompatible types but fitting matcher",
				[&](std::string s, int i) { return s == std::to_string(i); });
		SKIP("can't think of anything");
	}

	pass("relaxing in between");

	SUBTEST("exercising exceptions") {
		throws([&] {
			std::bitset<5> mybitset(std::string("01234"));
		}, "bitset takes only bits");

		SUBTEST(2, "subtests are nestable") {
			lives([&] { std::sqrt( 2); }, "sqrt( 2) lives");
			lives([&] { std::sqrt(-2); }, "sqrt(-2) lives, too");
		}

		TODO("research correct exception type");
		throws<std::domain_error>([&] {
			std::vector<int> myvector;
			myvector.resize(myvector.max_size() + 1);
		}, "resizing too much leaves domain");

		done_testing();
	}

	return EXIT_SUCCESS;
}
