#include "rgle/util/Tester.h"


int rgle::Tester::run(std::function<void(Tester&)> test)
{
	Platform::initialize();
	Settings::setLoggerWrite(false);
	Tester tester = Tester();
	int ret = 0;
	try {
		test(tester);
	}

	catch (rgle::Exception&) {
		std::cout << "TEST - ";
		Console::coloredPrint(Console::Color::RED, "FAILED");
		ret = -1;
	}
	catch (std::exception& e) {
		std::cout << "TEST - ";
		Console::coloredPrint(Console::Color::RED, "FAILED");
		std::cout << " - Unhandled exception thrown: " << e.what() << std::endl;
		ret = -1;
	}
	catch (...) {
		std::cout << "TEST - ";
		Console::coloredPrint(Console::Color::RED, "FAILED");
		std::cout << " - Unhandled exception thrown" << std::endl;
		ret = -1;
	}
	tester.printStatus();
	Settings::setLoggerWrite(true);
	return tester._succeeded == tester._total ? ret : -1;
}

void rgle::Tester::expect(std::string description, ExpectationFunc expect)
{
	try {
		this->_addTestResult(description, expect());
	}
	catch (Exception&) {
		this->_addTestResult(description, false, "exception thrown");
	}
	catch (std::exception& e) {
		Exception::make(e);
		this->_addTestResult(description, false, "exception thrown");
	}
	catch (...) {
		this->_addTestResult(description, false, "exception thrown");
	}
}

void rgle::Tester::expectAndPrint(std::string description, ExpectationFunc expect, PrintFunc print)
{
	try {
		if (expect()) {
			this->_addTestResult(description, true);
		}
		else {
			this->_addTestResult(description, false, print());
		}
	}
	catch (Exception&) {
		this->_addTestResult(description, false, "exception thrown");
	}
	catch (std::exception& e) {
		Exception::make(e);
		this->_addTestResult(description, false, "exception thrown");
	}
	catch (...) {
		this->_addTestResult(description, false, "exception thrown");
	}
}

void rgle::Tester::expectToThrow(std::string description, std::string type, std::function<void()> run)
{
	try {
		run();
	}
	catch (Exception& e) {
		if (e.type() == type) {
			this->_addTestResult(description, true);
		}
		else {
			this->_addTestResult(description, false, "expected exception of type: " + type + " to be thrown");
		}
		return;
	}
	catch (std::exception& e) {
		Exception::make(e);
		this->_addTestResult(description, false, "expected exception of type: " + type + " to be thrown");
		return;
	}
	catch (...) {
		this->_addTestResult(description, false, "expected exception of type: " + type + " to be thrown");
		return;
	}
	this->_addTestResult(description, false, "expected exception to be thrown");
}

void rgle::Tester::printStatus()
{
	std::cout << std::endl << "----------------------------------------------------" << std::endl;
	std::cout << this->_succeeded << '/' << this->_total << " - PASSED" << std::endl;
	if (this->_succeeded == this->_total) {
		std::cout << "TEST - ";
		Console::coloredPrint(Console::Color::GREEN, "SUCCEEDED");
	}
	else {
		std::cout << "TEST - ";
		Console::coloredPrint(Console::Color::RED, "FAILED");
	}
	std::cout << std::endl;
}

void rgle::Tester::_addTestResult(std::string description, bool result, std::string errorString)
{
	this->_total++;
	if (result) {
		std::cout << '[' << this->_total << "] - ";
		Console::coloredPrint(Console::Color::GREEN, "PASSED");
		std::cout << " - " << description << std::endl;
		this->_succeeded++;
	}
	else {
		std::cout << '[' << this->_total << "] - ";
		Console::coloredPrint(Console::Color::RED, "FAILED");
		std::cout << " - " << description << (!errorString.empty() ? " - " + errorString : "") << std::endl;
	}
}
