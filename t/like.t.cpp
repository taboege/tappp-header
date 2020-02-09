#include <tappp.hpp>
#include <vector>
#include <cstdlib>

using namespace TAP;

template<typename T>
bool truthy(const T& x) {
	return !!x;
}

int main(void) {
	plan(9);

	Predicate<int> le5 = [&] (int x) -> bool { return x <= 5; };
	like(-4, le5, "-4 <= 5");
	like(5, le5,  " 5 <= 5");

	like("a 55 ", "\\D \\d+\\s+", "regex match");
	TODO("see diagnostics");
	like("a 55 ", "\\d+\\s+", "regex non-match");

	unlike(0, truthy<int>, "0 is falsy");
	unlike(0.0, truthy<double>, "0.0 is falsy");
	TODO("0.1 is actually truthy");
	unlike(0.1, truthy<double>, "test diags again");

	std::vector a{5,10,12};
	throws_like<std::out_of_range>([&] { a.at(3); },
		"vector::_M_range_check.*", "index 3 is out of bounds");
	TODO();
	throws_like([&] {
		a.resize(a.max_size() + 1);
	}, "\\?", "show me the what()");

	return EXIT_SUCCESS;
}
