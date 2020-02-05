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
#include <exception>

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
		Context(void) { }

		/**
		 * Create a new Context object and print a plan line.
		 */
		Context(unsigned int tests) {
			plan(tests);
		}

		/* TODO: We may also skip entire tests by planning 1..0 */
		/* TODO: Make output stream configurable */

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
			std::string of = "/" + std::to_string(how_many);
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

		bool ok( bool is_ok,  const std::string& message = "") { return TAPP.ok( is_ok,  message); }
		bool nok(bool is_nok, const std::string& message = "") { return TAPP.nok(is_nok, message); }

		bool pass(const std::string& message = "") { return TAPP.pass(message); }
		bool fail(const std::string& message = "") { return TAPP.fail(message); }

		void TODO(const std::string& reason = "-") { TAPP.TODO(reason); }
		void SKIP(const std::string& reason = "")  { TAPP.SKIP(reason); }
		void SKIP(unsigned int how_many, const std::string& reason = "") { TAPP.SKIP(how_many, reason); }

		void BAIL(const std::string& reason = "") { TAPP.BAIL(reason); }

		void diag(const std::string& message) { TAPP.diag(message); }
	}
}

#endif /* TAPPP_HPP */
