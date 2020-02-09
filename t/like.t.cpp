#include <tappp.hpp>
#include <cstdlib>

using namespace TAP;

template<typename T>
bool truthy(const T& x) {
	return !!x;
}

int main(void) {
	plan(7);

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

	return EXIT_SUCCESS;
}
