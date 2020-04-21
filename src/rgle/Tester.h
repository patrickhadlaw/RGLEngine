#pragma once

#include "rgle/Exception.h"

namespace rgle {

	typedef std::function<bool()> ExpectationFunc;
	typedef std::function<std::string()> PrintFunc;

	class Tester {
	public:
		Tester() : _total(0), _succeeded(0) {}

		static int run(std::function<void(Tester&)> test);

		void expect(std::string description, ExpectationFunc expect);

		void expectAndPrint(std::string description, ExpectationFunc expect, PrintFunc print);

		void expectToThrow(std::string description, std::string type, std::function<void()> run);

		void printStatus();

	private:
		void _addTestResult(std::string description, bool result, std::string errorString = "");

		int _total;
		int _succeeded;
	};
}