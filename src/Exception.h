#pragma once

#include <exception>
#include <string>

#define EXCEPT_DETAIL_DEFAULT cppogl::Exception::Detail{__FILE__, __func__, __LINE__, ""}

namespace cppogl {
	class Exception : public std::exception {
	public:
		struct Detail {
			std::string file = "";
			std::string func = "";
			int line = 0;
			std::string timestamp = "";
		};
		Exception();
		Exception(std::string& except, Detail& detail);
		Exception(const char* except, Detail& detail);
		virtual ~Exception();

		virtual std::string except();
		virtual std::string log();

	protected:

		virtual std::string _type();

		std::string _exception;
		Detail _details;
	};

	class NullPointerException : public Exception {
	public:
		NullPointerException();
		NullPointerException(Exception::Detail& detail);
		virtual ~NullPointerException();

	protected:

		virtual std::string _type();
	};
}