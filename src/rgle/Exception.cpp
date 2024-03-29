#include "rgle/Exception.h"


int rgle::Exception::_thrown = 0;

rgle::Exception::Exception(const char * except, Logger::Detail detail) :
	_exception(except),
	_details(detail),
	_typeReflected("rgle::Exception"),
	std::runtime_error(except)
{
	this->_checkDoubleThrow();
	this->_thrown++;
	Logger::except(this);
}

rgle::Exception::Exception(std::string except, Logger::Detail detail) :
	_exception(except),
	_details(detail),
	_typeReflected("rgle::Exception"),
	std::runtime_error(except)
{
	this->_checkDoubleThrow();
	this->_thrown++;
	Logger::except(this);
}

rgle::Exception rgle::Exception::make(const std::exception & except)
{
	return Exception(except.what(), LOGGER_DETAIL_DEFAULT, "std::exception");
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
	std::string time = Logger::timeString(this->_details.timestamp);
	std::cout << time << '|';
	util::Console::coloredPrint(util::Console::Color::RED, "EXCEPTION");
	std::string result = std::string("|") + this->_typeReflected + Logger::detailString(this->_details);
	result += std::string("|: ") + _exception;
	std::cout << result << std::endl;
	result = time + "|EXCEPTION" + result;
	return result;
}

std::string rgle::Exception::type() const
{
	return this->_typeReflected;
}

rgle::Exception::Exception(std::string & except, Logger::Detail detail, const char * type) :
	_exception(except),
	_details(detail),
	_typeReflected(type),
	std::runtime_error(except)
{
	this->_checkDoubleThrow();
	this->_thrown++;
	Logger::except(this);
}

rgle::Exception::Exception(const char * except, Logger::Detail detail, const char * type) :
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

rgle::NullPointerException::NullPointerException(Logger::Detail detail) : Exception("null pointer exception", detail, "rgle::NullPointerException")
{
}

rgle::IOException::IOException(std::string exception, Logger::Detail detail) : Exception(exception, detail, "rgle::IOException")
{
}

rgle::BadCastException::BadCastException(std::string exception, Logger::Detail detail) : Exception(exception, detail, "rgle::BadCastException")
{
}

rgle::ApplicationException::ApplicationException(std::string exception, Logger::Detail detail) : Exception(exception, detail, "rgle::ApplicationException")
{
}

rgle::IllegalArgumentException::IllegalArgumentException(std::string exception, Logger::Detail detail) : Exception(exception, detail, "rgle::IllegalArgumentException")
{
}

rgle::OutOfBoundsException::OutOfBoundsException(Logger::Detail detail) : Exception("out of bounds exception", detail, "rgle::OutOfBoundsException")
{
}

rgle::NotFoundException::NotFoundException(std::string exception, Logger::Detail detail) : Exception(exception, detail, "rgle::NotFoundException")
{
}

rgle::InvalidStateException::InvalidStateException(std::string exception, Logger::Detail detail) : Exception(exception, detail, "rgle::InvalidStateException")
{
}
