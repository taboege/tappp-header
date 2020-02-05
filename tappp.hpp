/*
 * tappp.hpp - Header-only C++ TAP producer
 *
 * Copyright (C) 2020 Tobias Boege
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the Artistic License 2.0
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Artistic License 2.0 for more details.
 */

#ifndef TAPPP_HPP
#define TAPPP_HPP

#include <iostream>
#include <sstream>
#include <exception>
#include <functional>
#include <type_traits>

namespace TAP {
	/**
	 * Exceptions that a TAP producer may throw.
	 */
	namespace X {
		/**
		 * Thrown when a plan line has already been emitted but a
		 * change to it is requested.
		 */
		struct Planned : std::runtime_error {
			Planned(void) : std::runtime_error("Plan line emitted already") { }
		};

		/**
		 * Thrown when `done_testing` or `BAIL` has been called already
		 * but more state-changing TAP operations are requested.
		 */
		struct Finished : std::runtime_error {
			Finished(void) : std::runtime_error("TAP session closed already") { }
		};

		/**
		 * Thrown when a plan line is requested through `plan` after
		 * the first test line was printed. TAP only allows the plan
		 * line at the beginning or the end. Printing it at the end
		 * is handled by `done_testing`.
		 */
		struct LatePlan : std::runtime_error {
			LatePlan(void) : std::runtime_error("Too late to plan tests now") { }
		};
	}

	/* Misc tools */
	namespace {
		/**
		 * Stringify something in a pluggable way, using stringstream.
		 */
		template<typename T>
		static std::string to_string(const T& x) {
			std::stringstream ss;
			ss << x;
			return ss.str();
		}

		/**
		 * Determine at compile-time whether the given expression is
		 * stringifiable using operator<< on a stringstream.
		 */
		namespace Occult {
			/* Many thanks to https://stackoverflow.com/a/39348287
			 * and https://stackoverflow.com/a/49004530 */
			template<typename Op, typename R, typename ... Args>
			std::is_convertible<std::invoke_result_t<Op, Args...>, R> is_invokable_test(int);

			template<typename Op, typename R, typename ... Args>
			std::false_type is_invokable_test(...);

			template<typename Op, typename R, typename ... Args>
			using is_invokable = decltype(is_invokable_test<Op, R, Args...>(0));

			struct left_shift {
				template <typename L, typename R>
				constexpr auto operator()(L&& l, R&& r) const
						-> decltype(std::forward<L>(l) << std::forward<R>(r)) {
					return std::forward<L>(l) << std::forward<R>(r);
				}
			};

			template<typename T>
			using Stringifiable = is_invokable<left_shift, std::ostream&, std::ostream&, T>;
		}
	}

	/**
	 * A sentinel type accepted by the Context constructor to indicate
	 * that all tests should be skipped.
	 */
	enum skip_all { SKIP_ALL };

	/**
	 * A Context holds a TAP producer's state, including the test
	 * plan, the test numbering, output stream and 'TODO' directives.
	 * Its methods update the state and print TAP directly to the
	 * output device.
	 */
	class Context {
		std::ostream& out = std::cout;  /**< Output device     */
		unsigned int planned = 0; /**< Number of planned tests */
		unsigned int run     = 0; /**< Number of run tests     */
		unsigned int good    = 0; /**< Number of "ok" tests    */
		std::string  todo   = ""; /**< Next test's 'TODO'      */

		bool have_plan = false; /**< Whether a plan line was printed */
		bool finished  = false; /**< Whether done_testing was called */

	public:

		/**
		 * Create a new empty Context object. The default output device
		 * is std::cout. No plan line is printed. You either have to call
		 * `plan` before any tests or `done_testing` after the last one.
		 */
		Context(std::ostream& out = std::cout) : out(out) { }

		/**
		 * Create a new Context object and print a plan line.
		 */
		Context(unsigned int tests, std::ostream& out = std::cout) : out(out) {
			plan(tests);
		}

		/**
		 * Create a new Context and skip it entirely. The `1..0` plan
		 * line is printed and the context is marked as finished.
		 */
		Context(const skip_all& skip, const std::string& reason = "", std::ostream& out = std::cout) : out(out) {
			plan(skip, reason);
		}

		/**
		 * Set up a test plan and emit the plan line.
		 */
		void plan(unsigned int tests) {
			if (have_plan)
				throw TAP::X::Planned();
			if (finished)
				throw TAP::X::Finished();

			if (run > 0)
				throw TAP::X::LatePlan();

			out << "1.." << tests << std::endl;
			planned = tests;
			have_plan = true;
		}

		/**
		 * Skip the entire test. Print the `1..0` plan line and then
		 * mark the context as finished.
		 */
		void plan(const skip_all& skip, const std::string& reason = "") {
			out << "1..0";
			if (!reason.empty())
				out << " # SKIP " << reason;
			out << std::endl;
			finished = true;
		}

		/**
		 * Return whether the whole session is good or not, taking into
		 * account the test plan (if any) and the number of successful
		 * vs. all run tests.
		 */
		bool summary(void) {
			return good == (have_plan ? planned : run);
		}

		/**
		 * Close this TAP context from emitting further test lines.
		 * If no test plan was printed in the beginning, it is done now.
		 */
		void done_testing(void) {
			if (finished)
				throw TAP::X::Finished();

			if (!have_plan) {
				out << "1.." << run << std::endl;
			}
			else {
				if (planned != run) {
					diag("Looks like you planned " + std::to_string(planned) +
					    " tests but ran " + std::to_string(run));
				}
			}

			finished = true;
		}

		/**
		 * Write an "ok" or "not ok" line depending on the `is_ok`
		 * argument.
		 */
		bool ok(bool is_ok, const std::string& message = "") {
			if (finished)
				throw TAP::X::Finished();

			out << (is_ok ? "ok " : "not ok ")
			    << ++run << " - "
				<< message;
			if (!todo.empty()) {
				out << (message.empty() ? "" : " ");
				out << "# TODO " << todo;
				todo.clear();
			}
			out << std::endl;

			if (is_ok)
				++good;

			return is_ok;
		}

		/**
		 * Like `ok` but negates the bool first.
		 */
		bool nok(bool is_nok, const std::string& message = "") {
			return ok(not is_nok, message);
		}

		/**
		 * Pass a test unconditionally.
		 */
		bool pass(const std::string& message = "") {
			return ok(true, message);
		}

		/**
		 * Fail a test unconditionally.
		 */
		bool fail(const std::string& message = "") {
			return ok(false, message);
		}

		/**
		 * Mark the next test as "to-do". The next "ok" / "not ok"
		 * line will be printed with the TODO directive, but only
		 * if the reason string is non-empty.
		 */
		void TODO(const std::string& reason = "-") {
			if (finished)
				throw TAP::X::Finished();
			todo = reason;
		}

		/**
		 * Skip a test by emitting a `pass` with the SKIP directive.
		 */
		void SKIP(const std::string& reason = "") {
			pass("# SKIP" + std::string(reason.empty() ? "" : " ") + reason);
		}

		/**
		 * Skip the given number of tests by emitting `pass`es with
		 * the SKIP directive. The reason is repeated for every `pass`
		 * but a counter is added.
		 */
		void SKIP(unsigned int how_many, const std::string& reason = "") {
			auto current_of = [&] (unsigned int cur) {
				std::string ret = reason.empty() ? "" : " ";
				ret += std::to_string(1 + cur) + "/" + std::to_string(how_many);
				return ret;
			};
			for (unsigned int i = 0; i < how_many; ++i)
				SKIP(reason + current_of(i));
		}

		/**
		 * Print a "Bail out!" message but does not exit.
		 * Clients should do that after calling this function
		 * and performing appropriate cleanup.
		 */
		void BAIL(const std::string& reason = "") {
			if (finished)
				throw TAP::X::Finished();

			out << "Bail out!";
			if (!reason.empty())
				out << " " << reason;
			out << std::endl;

			finished = true;
		}

		/**
		 * Print a diagnostic message.
		 */
		void diag(const std::string& message) {
			out << "# " << message << std::endl;
		}

		/**
		 * Check if the first argument equals the second. The meaning of
		 * "equality" is dictated by the last argument, which defaults
		 * to `std::equal_to`. If the test fails and the two values can
		 * be stringified by operator<<'ing them to a stringstream, then
		 * the differing values are printed as diagnostics.
		 */
		template<typename T, typename U, typename Matcher = std::equal_to<T>>
		bool is(const T& got, const U& expected, const std::string& message = "", Matcher m = Matcher()) {
			bool is_ok = ok(m(got, expected), message);
			if (!is_ok) {
				if (!message.empty())
					diag("Test '" + message + "' failed:");
				if constexpr (Occult::Stringifiable<T>::value)
					diag("       Got: " + to_string(got));
				if constexpr (Occult::Stringifiable<U>::value)
					diag("  Expected: " + to_string(expected));
			}
			return is_ok;
		}

		/**
		 * Like `is` but negates the comparison.
		 */
		template<typename T, typename U, typename Matcher = std::equal_to<T>>
		bool isnt(const T& got, const U& unexpected, const std::string& message = "", Matcher m = Matcher()) {
			bool is_ok = nok(m(got, unexpected), message);
			if (!is_ok) {
				if (!message.empty())
					diag("Test '" + message + "' failed:");
				if constexpr (Occult::Stringifiable<T>::value)
					diag("         Got: " + to_string(got));
				if constexpr (Occult::Stringifiable<U>::value)
					diag("  Unexpected: " + to_string(unexpected));
			}
			return is_ok;
		}
	};

	/**
	 * Procedural interface. We keep a global Context object that is
	 * default-constructed and expose its methods as free-standing
	 * functions.
	 */
	namespace {
		Context TAPP;

		void plan(unsigned int tests) { TAPP.plan(tests);      }
		bool summary(void)            { return TAPP.summary(); }
		void done_testing(void)       { TAPP.done_testing();   }
		void plan(const skip_all& skip, const std::string& reason = "") {
			return TAPP.plan(skip, reason);
		}

		bool ok( bool is_ok,  const std::string& message = "") { return TAPP.ok( is_ok,  message); }
		bool nok(bool is_nok, const std::string& message = "") { return TAPP.nok(is_nok, message); }

		bool pass(const std::string& message = "") { return TAPP.pass(message); }
		bool fail(const std::string& message = "") { return TAPP.fail(message); }

		void TODO(const std::string& reason = "-") { TAPP.TODO(reason); }
		void SKIP(const std::string& reason = "")  { TAPP.SKIP(reason); }
		void SKIP(unsigned int how_many, const std::string& reason = "") { TAPP.SKIP(how_many, reason); }

		void BAIL(const std::string& reason = "") { TAPP.BAIL(reason); }

		void diag(const std::string& message) { TAPP.diag(message); }

		template<typename T, typename U, typename Matcher = std::equal_to<T>>
		bool is(const T& got, const U& expected, const std::string& message = "", Matcher m = Matcher()) {
			return TAPP.is(got, expected, message, m);
		}

		template<typename T, typename U, typename Matcher = std::equal_to<T>>
		bool isnt(const T& got, const U& expected, const std::string& message = "", Matcher m = Matcher()) {
			return TAPP.isnt(got, expected, message, m);
		}
	}
}

#endif /* TAPPP_HPP */
