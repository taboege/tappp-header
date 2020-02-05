TESTS = $(patsubst %.t.cpp,%.t,$(wildcard *.t.cpp))

.PHONY: all
all: $(TESTS)

%.t: %.t.cpp tappp.hpp
	g++ -std=c++17 -Wall -Wno-unused-function -I. -o $@ $<

.PHONY: test
test: $(TESTS)
	for f in $(foreach f,$(TESTS),$(abspath $(f))); \
	do prove -e '' "$$f"; \
	done

.PHONY: test-valgrind
test-valgrind: $(TESTS)
	for f in $(foreach f,$(TESTS),$(abspath $(f))); \
	do prove -e 'valgrind --quiet --error-exitcode=111 --exit-on-first-error=yes --leak-resolution=low --leak-check=full --errors-for-leak-kinds=all' "$$f"; \
	done

.PHONY: clean
clean:
	rm -f $(TESTS)
