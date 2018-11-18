#include "Exception.h"

int cppogl::Exception::_thrown = 0;

cppogl::Exception::Exception() : std::runtime_error("undefined")
{
	this->_exception = "undefined";
	this->_checkDoubleThrow();
	this->_thrown++;
}

cppogl::Exception::Exception(const char * except, Detail & detail) : std::runtime_error(except)
{
	this->_exception = std::string(except);
	this->_details = detail;
	this->_checkDoubleThrow();
	this->_thrown++;
	std::cerr << this->message() << std::endl;
}

cppogl::Exception::Exception(std::string & except, Detail & detail) : std::runtime_error(except)
{
	this->_exception = except;
	this->_details = detail;
	this->_checkDoubleThrow();
	this->_thrown++;
	std::cerr << this->message() << std::endl;
}

cppogl::Exception::~Exception()
{
	this->_thrown--;
}

std::string cppogl::Exception::except()
{
	return _exception;
}

std::string cppogl::Exception::log()
{
	std::string id = _details.id.empty() ? "" : std::string("[ID: ") + _details.id + "]";
	return _details.timestamp + "\t[" + this->_type() + "]" + "[file: " + _details.file + "][func: " + _details.func + "][line: " + std::to_string(_details.line) + "]" + id;
}

std::string cppogl::Exception::message()
{
	return this->log() + "ERROR: " + _exception;
}

void cppogl::Exception::_checkDoubleThrow()
{
	if (_thrown > 0) {
		std::cout << "WARNING: multiple exceptions thrown, exception possibly thrown in destructor..." << std::endl;
	}
}

std::string cppogl::Exception::_type()
{
	return std::string("cppogl::Exception");
}

cppogl::NullPointerException::NullPointerException()
{
}

cppogl::NullPointerException::NullPointerException(Exception::Detail & detail) : Exception("null pointer exception", detail)
{
}

cppogl::NullPointerException::~NullPointerException()
{
}

std::string cppogl::NullPointerException::_type()
{
	return std::string("cppogl::NullPointerException");
}

cppogl::BadCastException::BadCastException()
{
}

cppogl::BadCastException::BadCastException(std::string exception, Exception::Detail & detail)
{
}

cppogl::BadCastException::~BadCastException()
{
}

std::string cppogl::BadCastException::_type()
{
	return std::string("cppogl::BadCastException");
}

cppogl::ApplicationException::ApplicationException(std::string exception, Exception::Detail & detail) : Exception(exception, detail)
{
}

cppogl::ApplicationException::~ApplicationException()
{
}

std::string cppogl::ApplicationException::_type()
{
	return std::string("cppogl::ApplicationException");
}
