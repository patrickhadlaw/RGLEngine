#include "rgle/Exception.h"

int rgle::Exception::_thrown = 0;

rgle::Exception::Exception() : std::runtime_error("undefined")
{
	this->_exception = "undefined";
	this->_checkDoubleThrow();
	this->_thrown++;
}

rgle::Exception::Exception(const char * except, Logger::Detail & detail) : std::runtime_error(except)
{
	this->_exception = std::string(except);
	this->_details = detail;
	this->_checkDoubleThrow();
	this->_thrown++;
	Logger::except(this);
}

rgle::Exception::Exception(std::string & except, Logger::Detail & detail) : std::runtime_error(except)
{
	this->_exception = except;
	this->_details = detail;
	this->_checkDoubleThrow();
	this->_thrown++;
	Logger::except(this);
}

rgle::Exception::~Exception()
{
	this->_thrown--;
}

std::string rgle::Exception::except()
{
	return _exception;
}

std::string rgle::Exception::print()
{
	std::string time = Logger::timeString(_details.timestamp);
	std::cout << time << '|';
	Console::coloredPrint(Console::Color::RED, "EXCEPTION");
	std::string result = std::string("|") + this->_type() + '|' + _details.file + '|' + _details.func + '|' + std::to_string(_details.line);
	result += _details.id.empty() ? "" : std::string("|ID: ") + _details.id;
	result += std::string(": ") + _exception;
	std::cout << result << std::endl;
	result = time + "|EXCEPTION|" + result;
	return result;
}

void rgle::Exception::_checkDoubleThrow()
{
	if (_thrown > 0) {
		Logger::warn("multiple exceptions thrown, exception possibly thrown in a destructor...", LOGGER_DETAIL_DEFAULT);
	}
}

std::string rgle::Exception::_type()
{
	return std::string("rgle::Exception");
}

rgle::NullPointerException::NullPointerException()
{
}

rgle::NullPointerException::NullPointerException(Logger::Detail & detail) : Exception("null pointer exception", detail)
{
}

rgle::NullPointerException::~NullPointerException()
{
}

std::string rgle::NullPointerException::_type()
{
	return std::string("rgle::NullPointerException");
}

rgle::IOException::IOException()
{
}

rgle::IOException::IOException(std::string exception, Logger::Detail & detail) : Exception(exception, detail)
{
}

rgle::IOException::~IOException()
{
}

std::string rgle::IOException::_type()
{
	return std::string("rgle::IOException");
}

rgle::BadCastException::BadCastException()
{
}

rgle::BadCastException::BadCastException(std::string exception, Logger::Detail & detail)
{
}

rgle::BadCastException::~BadCastException()
{
}

std::string rgle::BadCastException::_type()
{
	return std::string("rgle::BadCastException");
}

rgle::ApplicationException::ApplicationException(std::string exception, Logger::Detail & detail) : Exception(exception, detail)
{
}

rgle::ApplicationException::~ApplicationException()
{
}

std::string rgle::ApplicationException::_type()
{
	return std::string("rgle::ApplicationException");
}
