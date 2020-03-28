#include "rgle/Exception.h"


int rgle::Exception::_thrown = 0;

rgle::Exception::Exception(const char * except, Logger::Detail & detail) :
	_exception(except),
	_details(detail),
	_typeReflected("rgle::Exception"),
	std::runtime_error(except)
{
	this->_checkDoubleThrow();
	this->_thrown++;
	Logger::except(this);
}

rgle::Exception::Exception(std::string & except, Logger::Detail & detail) :
	_exception(except),
	_details(detail),
	_typeReflected("rgle::Exception"),
	std::runtime_error(except)
{
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
	std::string result = std::string("|") + this->_typeReflected + '|' + _details.file + '|' + _details.func + '|' + std::to_string(_details.line);
	result += _details.id.empty() ? "" : std::string("|ID: ") + _details.id;
	result += std::string(": ") + _exception;
	std::cout << result << std::endl;
	result = time + "|EXCEPTION" + result;
	return result;
}

rgle::Exception::Exception(std::string & except, Logger::Detail & detail, const char * type) :
	_exception(except),
	_details(detail),
	_typeReflected(type),
	std::runtime_error(except)
{
	this->_checkDoubleThrow();
	this->_thrown++;
	Logger::except(this);
}

rgle::Exception::Exception(const char * except, Logger::Detail & detail, const char * type) :
	_exception(except),
	_details(detail),
	_typeReflected(type),
	std::runtime_error(except)
{
	this->_checkDoubleThrow();
	this->_thrown++;
	Logger::except(this);
}

void rgle::Exception::_checkDoubleThrow()
{
	if (_thrown > 0) {
		Logger::warn("multiple exceptions thrown, exception possibly thrown in a destructor...", LOGGER_DETAIL_DEFAULT);
	}
}

rgle::NullPointerException::NullPointerException(Logger::Detail & detail) : Exception("null pointer exception", detail, "rgle::NullPointerException")
{
}

rgle::IOException::IOException(std::string exception, Logger::Detail & detail) : Exception(exception, detail, "rgle::IOException")
{
}

rgle::BadCastException::BadCastException(std::string exception, Logger::Detail & detail) : Exception(exception, detail, "rgle::BadCastException")
{
}

rgle::ApplicationException::ApplicationException(std::string exception, Logger::Detail & detail) : Exception(exception, detail, "rgle::ApplicationException")
{
}

rgle::IllegalArgumentException::IllegalArgumentException(std::string exception, Logger::Detail & detail) : Exception(exception, detail, "rgle::IllegalArgumentException")
{
}

rgle::OutOfBoundsException::OutOfBoundsException(Logger::Detail & detail) : Exception("out of bounds exception", detail, "rgle::OutOfBoundsException")
{
}

rgle::NotFoundException::NotFoundException(std::string exception, Logger::Detail & detail) : Exception(exception, detail, "rgle::NotFoundException")
{
}

rgle::InvalidStateException::InvalidStateException(std::string exception, Logger::Detail & detail) : Exception(exception, detail, "rgle::InvalidStateException")
{
}
