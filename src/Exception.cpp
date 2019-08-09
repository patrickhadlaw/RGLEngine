#include "Exception.h"

int rgle::Exception::_thrown = 0;

rgle::Exception::Exception() : std::runtime_error("undefined")
{
	this->_exception = "undefined";
	this->_checkDoubleThrow();
	this->_thrown++;
}

rgle::Exception::Exception(const char * except, Detail & detail) : std::runtime_error(except)
{
	this->_exception = std::string(except);
	this->_details = detail;
	this->_checkDoubleThrow();
	this->_thrown++;
	std::cerr << this->message() << std::endl;
}

rgle::Exception::Exception(std::string & except, Detail & detail) : std::runtime_error(except)
{
	this->_exception = except;
	this->_details = detail;
	this->_checkDoubleThrow();
	this->_thrown++;
	std::cerr << this->message() << std::endl;
}

rgle::Exception::~Exception()
{
	this->_thrown--;
}

std::string rgle::Exception::except()
{
	return _exception;
}

std::string rgle::Exception::log()
{
	std::string id = _details.id.empty() ? "" : std::string("[ID: ") + _details.id + "]";
	return _details.timestamp + "\t[" + this->_type() + "]" + "[file: " + _details.file + "][func: " + _details.func + "][line: " + std::to_string(_details.line) + "]" + id;
}

std::string rgle::Exception::message()
{
	return this->log() + "ERROR: " + _exception;
}

void rgle::Exception::_checkDoubleThrow()
{
	if (_thrown > 0) {
		std::cout << "WARNING: multiple exceptions thrown, exception possibly thrown in destructor..." << std::endl;
	}
}

std::string rgle::Exception::_type()
{
	return std::string("rgle::Exception");
}

rgle::NullPointerException::NullPointerException()
{
}

rgle::NullPointerException::NullPointerException(Exception::Detail & detail) : Exception("null pointer exception", detail)
{
}

rgle::NullPointerException::~NullPointerException()
{
}

std::string rgle::NullPointerException::_type()
{
	return std::string("rgle::NullPointerException");
}

rgle::BadCastException::BadCastException()
{
}

rgle::BadCastException::BadCastException(std::string exception, Exception::Detail & detail)
{
}

rgle::BadCastException::~BadCastException()
{
}

std::string rgle::BadCastException::_type()
{
	return std::string("rgle::BadCastException");
}

rgle::ApplicationException::ApplicationException(std::string exception, Exception::Detail & detail) : Exception(exception, detail)
{
}

rgle::ApplicationException::~ApplicationException()
{
}

std::string rgle::ApplicationException::_type()
{
	return std::string("rgle::ApplicationException");
}
