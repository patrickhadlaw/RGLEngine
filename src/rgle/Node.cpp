#include "rgle/Node.h"


rgle::IdentifierException::IdentifierException(std::string exception, std::string identifier, Logger::Detail detail) :
	Exception(exception, IdentifierException::_makeDetail(std::move(detail), identifier), "rgle::IdentifierException")
{
}

rgle::Logger::Detail rgle::IdentifierException::_makeDetail(Logger::Detail && detail, const std::string & identifier)
{
	Logger::Detail det = std::move(detail);
	det.id = identifier;
	return det;
}

rgle::Node::Node() : id(uid())
{
}

rgle::Node::~Node()
{
}

const char * rgle::Node::typeName() const
{
	return "rgle::Node";
}

rgle::Resource::Resource()
{
}

rgle::Resource::~Resource()
{
}

const char * rgle::Resource::typeName() const
{
	return "rgle::Resource";
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

const char * rgle::ResourceManager::typeName() const
{
	return "rgle::ResourceManager";
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

const char * rgle::LogicNode::typeName() const
{
	return "rgle::LogicNode";
}

rgle::LogicException::LogicException(std::string exception, Logger::Detail detail) : Exception(exception, detail, "rgle::LogicException")
{
}
