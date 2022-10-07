#pragma once

#include "rgle/Logger.h"

namespace rgle {

	class Exception : public std::runtime_error {
	public:
		Exception(std::string except, Logger::Detail detail);
		Exception(const char* except, Logger::Detail detail);
		virtual ~Exception();

		static Exception make(const std::exception& except);

		virtual std::string except();

		virtual std::string print();

		std::string type() const;

	protected:

		Exception(std::string& except, Logger::Detail detail, const char* type);
		Exception(const char* except, Logger::Detail detail, const char* type);

		virtual void _checkDoubleThrow();

		std::string _exception;
		std::string _typeReflected;
		Logger::Detail _details;
	private:
		static int _thrown;
	};

	class NullPointerException : public Exception {
	public:
		NullPointerException(Logger::Detail detail);
	};

	class OutOfBoundsException : public Exception {
	public:
		OutOfBoundsException(Logger::Detail detail);
	};

	class IOException : public Exception {
	public:
		IOException(std::string exception, Logger::Detail detail);
	};

	class BadCastException : public Exception {
	public:
		BadCastException(std::string exception, Logger::Detail detail);
	};

	class IllegalArgumentException : public Exception {
	public:
		IllegalArgumentException(std::string exception, Logger::Detail detail);
	};

	class InvalidStateException : public Exception {
	public:
		InvalidStateException(std::string exception, Logger::Detail detail);
	};

	class NotFoundException : public Exception {
	public:
		NotFoundException(std::string exception, Logger::Detail detail);
	};

	class NotImplementedException : public Exception {
	public:
		NotImplementedException(Logger::Detail detail);
	};

	class ApplicationException : public Exception {
	public:
		ApplicationException(std::string exception, Logger::Detail detail);
	};
}