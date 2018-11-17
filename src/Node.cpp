#include "Node.h"


cppogl::IdentifierException::IdentifierException(std::string exception, std::string identifier, Exception::Detail detail)
{
	detail.id = identifier;
	Exception(exception, detail);
}

cppogl::IdentifierException::~IdentifierException()
{
}

std::string cppogl::IdentifierException::_type()
{
	return std::string("cppogl::IdentifierException");
}

cppogl::Node::Node()
{
}

cppogl::Node::~Node()
{
}

std::string & cppogl::Node::typeName()
{
	return std::string("cppogl::Node");
}

cppogl::Resource::Resource()
{
}

cppogl::Resource::~Resource()
{
}

std::string & cppogl::Resource::typeName()
{
	return std::string("cppogl::Resource");
}

cppogl::ResourceManager::ResourceManager()
{
}

cppogl::ResourceManager::ResourceManager(const ResourceManager & other)
{
	this->id = other.id;
	this->_resources = other._resources;
}

cppogl::ResourceManager::~ResourceManager()
{
}

void cppogl::ResourceManager::addResource(sResource resource)
{
	for (int i = 0; i < _resources.size(); i++) {
		if (resource->id == _resources[i]->id) {
			throw IdentifierException("resource already exists", resource->id, EXCEPT_DETAIL_DEFAULT);
		}
	}
	this->_resources.push_back(resource);
}

std::string & cppogl::ResourceManager::typeName()
{
	return std::string("cppogl::ResourceManager");
}

cppogl::LogicNode::LogicNode()
{
}

cppogl::LogicNode::~LogicNode()
{
}

void cppogl::LogicNode::update()
{
}

std::string & cppogl::LogicNode::typeName()
{
	return std::string("cppogl::LogicNode");
}

cppogl::LogicException::LogicException(std::string exception, Exception::Detail detail) : Exception(exception, detail)
{
}

cppogl::LogicException::~LogicException()
{
}

std::string cppogl::LogicException::_type()
{
	return std::string("cppogl::LogicException");
}
