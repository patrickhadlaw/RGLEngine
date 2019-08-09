#pragma once

#include "cpp-opengl.h"

#include <iostream>
#include <string>
#include <vector>
#include <functional>

namespace cppogl {

	typedef std::function<bool()> ExpectationFunc;

	class TestBed {
	public:
		TestBed() : _total(0), _succeeded(0) {}

		static int run(std::function<void(TestBed& testBed)> test) {
			TestBed testBed = TestBed();
			try {
				test(testBed);
			}
			catch (cppogl::Exception& e) {
				std::cout << "TEST - ";
				cppogl::Console::coloredPrint(cppogl::Console::Color::RED, "FAILED");
				return -1;
			}
			catch (std::exception& e) {
				std::cout << "TEST - ";
				cppogl::Console::coloredPrint(cppogl::Console::Color::RED, "FAILED");
				std::cout << " - Unhandled exception thrown:\n" << e.what() << std::endl;
				return -1;
			}
			catch (...) {
				std::cout << "TEST - ";
				cppogl::Console::coloredPrint(cppogl::Console::Color::RED, "FAILED");
				std::cout << " - Unhandled exception thrown" << std::endl;
				return -1;
			}
			testBed.printStatus();
			return testBed._succeeded == testBed._total ? 0 : -1;
		}

		void expect(std::string description, ExpectationFunc expect) {
			this->_total++;
			if (expect()) {
				std::cout << '[' << this->_total << "] - ";
				cppogl::Console::coloredPrint(cppogl::Console::Color::GREEN, "PASSED");
				std::cout << " - " << description << std::endl;
				this->_succeeded++;
			}
			else {
				std::cout << '[' << this->_total << "] - ";
				cppogl::Console::coloredPrint(cppogl::Console::Color::RED, "FAILED");
				std::cout << " - " << description << std::endl;
			}
		}

		void printStatus() {
			std::cout << std::endl << "----------------------------------------------------" << std::endl;
			std::cout << this->_succeeded << '/' << this->_total << " - PASSED" << std::endl;
			if (this->_succeeded == this->_total) {
				std::cout << "TEST - ";
				cppogl::Console::coloredPrint(cppogl::Console::Color::GREEN, "SUCCEEDED");
			}
			else {
				std::cout << "TEST - ";
				cppogl::Console::coloredPrint(cppogl::Console::Color::RED, "FAILED");
			}
		}

	private:
		int _total;
		int _succeeded;
	};
}