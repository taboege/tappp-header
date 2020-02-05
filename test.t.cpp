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

// make `is` and `isnt` work with durations below,
// until C++20 where this operator exists
template<typename T>
std::ostream& operator<<(std::ostream& os, const duration<T>& d) {
	return os << duration_cast<nanoseconds>(d).count() << "ns";
}

int main(void) {
	plan(11);

	pass("the first one's free");

	TODO("not reliable yet");
	ok(time(0) % 2 == 0, "timestamp is even");

	SKIP(2, "failure is not an option");
	if (false) {
		fail("oops");
		fail("double oops");
	}

	ok(elapsed() < 1s, "executing fast enough");

	auto e = elapsed();
	auto f = e;
	is(e, f, "different objects but equal");

	TODO("we're probably too fast");
	is(elapsed(), elapsed(), "executing slow enough");
	//is("55", 55); // <- incompatible types: friendly error message

	std::string s = "dlrow olleh";
	std::reverse(s.begin(), s.end());
	is(s, "hello world", "reverse works");

	is("55", 55, "incompatible types but fitting matcher",
			[&](std::string s, int i) { return s == std::to_string(i); });

	TODO("demonstration of error");
	is(steady_clock::now(), steady_clock::now(), "not ostream'able, no diagnostics");

	pass("we're done");

	done_testing();

	return EXIT_SUCCESS;
}
