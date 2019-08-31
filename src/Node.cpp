#include "Node.h"


rgle::IdentifierException::IdentifierException(std::string exception, std::string identifier, Logger::Detail detail)
{
	detail.id = identifier;
	Exception(exception, detail);
}

rgle::IdentifierException::~IdentifierException()
{
}

std::string rgle::IdentifierException::_type()
{
	return std::string("rgle::IdentifierException");
}

rgle::Node::Node()
{
}

rgle::Node::~Node()
{
}

std::string & rgle::Node::typeName()
{
	return std::string("rgle::Node");
}

rgle::Resource::Resource()
{
}

rgle::Resource::~Resource()
{
}

std::string & rgle::Resource::typeName()
{
	return std::string("rgle::Resource");
}

rgle::ResourceManager::ResourceManager()
{
}

rgle::ResourceManager::ResourceManager(const ResourceManager & other)
{
	this->id = other.id;
	this->_resources = other._resources;
}

rgle::ResourceManager::~ResourceManager()
{
}

void rgle::ResourceManager::addResource(std::shared_ptr<Resource> resource)
{
	for (int i = 0; i < _resources.size(); i++) {
		if (resource->id == _resources[i]->id) {
			throw IdentifierException("resource already exists", resource->id, LOGGER_DETAIL_DEFAULT);
		}
	}
	this->_resources.push_back(resource);
}

std::string & rgle::ResourceManager::typeName()
{
	return std::string("rgle::ResourceManager");
}

rgle::LogicNode::LogicNode()
{
}

rgle::LogicNode::~LogicNode()
{
}

void rgle::LogicNode::update()
{
}

std::string & rgle::LogicNode::typeName()
{
	return std::string("rgle::LogicNode");
}

rgle::LogicException::LogicException(std::string exception, Logger::Detail detail) : Exception(exception, detail)
{
}

rgle::LogicException::~LogicException()
{
}

std::string rgle::LogicException::_type()
{
	return std::string("rgle::LogicException");
}
