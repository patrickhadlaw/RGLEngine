#pragma once

#include "rgle/Logger.h"

#include <iostream>
#include <exception>
#include <string>

namespace rgle {

	class Exception : public std::runtime_error {
	public:
		Exception();
		Exception(std::string& except, Logger::Detail& detail);
		Exception(const char* except, Logger::Detail& detail);
		virtual ~Exception();

		virtual std::string except();

		virtual std::string print();

	protected:

		virtual void _checkDoubleThrow();

		virtual std::string _type();

		std::string _exception;
		Logger::Detail _details;
	private:
		static int _thrown;
	};

	class NullPointerException : public Exception {
	public:
		NullPointerException();
		NullPointerException(Logger::Detail& detail);
		virtual ~NullPointerException();

	protected:

		virtual std::string _type();
	};

	class IOException : public Exception {
	public:
		IOException();
		IOException(std::string exception, Logger::Detail& detail);
		virtual ~IOException();

	protected:
		virtual std::string _type();
	};

	class BadCastException : public Exception {
	public:
		BadCastException();
		BadCastException(std::string exception, Logger::Detail& detail);
		virtual ~BadCastException();

	protected:

		virtual std::string _type();
	};

	class ApplicationException : public Exception {
	public:
		ApplicationException(std::string exception, Logger::Detail& detail);
		virtual ~ApplicationException();

	protected:
		virtual std::string _type();
	};
}