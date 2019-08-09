#pragma once

#include "Console.h"

#include <iostream>
#include <exception>
#include <string>

#define EXCEPT_DETAIL_DEFAULT rgle::Exception::Detail{RGLE_FILENAME, __func__, __LINE__, ""}
#define EXCEPT_DETAIL_IDENTIFIER(id) rgle::Exception::Detail{RGLE_FILENAME, __func__, __LINE__, "", id}

namespace rgle {

	class Exception : public std::runtime_error {
	public:
		struct Detail {
			std::string file = "";
			std::string func = "";
			int line = 0;
			std::string timestamp = "";
			std::string id = "";
		};
		Exception();
		Exception(std::string& except, Detail& detail);
		Exception(const char* except, Detail& detail);
		virtual ~Exception();

		virtual std::string except();
		virtual std::string log();

		virtual std::string message();

	protected:

		virtual void _checkDoubleThrow();

		virtual std::string _type();

		std::string _exception;
		Detail _details;
	private:
		static int _thrown;
	};

	class NullPointerException : public Exception {
	public:
		NullPointerException();
		NullPointerException(Exception::Detail& detail);
		virtual ~NullPointerException();

	protected:

		virtual std::string _type();
	};

	class BadCastException : public Exception {
	public:
		BadCastException();
		BadCastException(std::string exception, Exception::Detail& detail);
		virtual ~BadCastException();

	protected:

		virtual std::string _type();
	};

	class ApplicationException : public Exception {
	public:
		ApplicationException(std::string exception, Exception::Detail& detail);
		virtual ~ApplicationException();

	protected:
		virtual std::string _type();
	};
}