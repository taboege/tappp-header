#include <tappp.hpp>
#include <chrono>
#include <ctime>
#include <cstdlib>

using namespace TAP;
using namespace std::chrono;
using namespace std::chrono_literals;

static auto start_time = steady_clock::now();

static auto elapsed(void) {
	return duration<double>(steady_clock::now() - start_time);
}

int main(void) {
	plan(6);

	pass("the first one's free");

	TODO("not reliable yet");
	ok(time(0) % 2 == 0, "timestamp is even");

	SKIP(2, "failure is not an option");
	if (false) {
		fail("oops");
		fail("double oops");
	}

	ok(elapsed() < 1s, "executing fast enough");

	pass("we're done");

	done_testing();

	return EXIT_SUCCESS;
}
