#include <tappp.hpp>
#include <stdexcept>
#include <cstdlib>

using namespace TAP;

int main(void) {
	plan(5);

	std::vector a{5,10,12};
	std::vector b{5,10,15};

	is(a[0], 5, "first element is 5");
	isnt(a[2], b[2], "last elements differ");

	throws<std::out_of_range>([&] { a.at(3); }, "3 out of bounds");

	b[2] = a[2] = b[2] * 2;
	is(b[2], 30, "changed last element");
	is(a, b, "vectors match now");

	return EXIT_SUCCESS;
}
