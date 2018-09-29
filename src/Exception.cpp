#include "Exception.h"

cppogl::Exception::Exception()
{
	this->_exception = "undefined";
}

cppogl::Exception::Exception(const char * except, Detail & detail)
{
	this->_exception = std::string(except);
	this->_details = detail;
}

cppogl::Exception::Exception(std::string & except, Detail & detail)
{
	this->_exception = except;
	this->_details = detail;
}

cppogl::Exception::~Exception()
{
}

std::string cppogl::Exception::except()
{
	return _exception;
}

std::string cppogl::Exception::log()
{
	return _details.timestamp + "\t\t[" + this->_type() + "]" + "[file: " + _details.file + "][func: " + _details.func + "][line: " + std::to_string(_details.line) + "] ERROR: " + _exception;
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
