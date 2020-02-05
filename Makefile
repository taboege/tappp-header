.PHONY: all
all: test.t

%.t: %.t.cpp tappp.hpp
	g++ -std=c++17 -Wall -Wno-unused-function -I. -o $@ $<

.PHONY: test
test: test.t
	prove -e '' $(abspath $<)

.PHONY: test-valgrind
test-valgrind: test.t
	prove -e 'valgrind --quiet --error-exitcode=111 --exit-on-first-error=yes --leak-resolution=low --leak-check=full --errors-for-leak-kinds=all' $(abspath $<)

.PHONY: clean
clean:
	rm -f test.t
