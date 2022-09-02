#pragma once

#include "rgle/util/Utility.h"

namespace rgle {

	class IdentifierException : public Exception {
	public:
		IdentifierException(std::string exception, std::string identifier, Logger::Detail detail);
	protected:
		static Logger::Detail _makeDetail(Logger::Detail&& detail, const std::string& identifier);
	};

	class Node {
	public:
		Node();
		virtual ~Node();

		virtual const char* typeName() const;

		std::string id;
	};

	class LogicException : public Exception {
	public:
		LogicException(std::string exception, Logger::Detail detail);
	};

	class LogicNode : public Node {
	public:
		LogicNode();
		virtual ~LogicNode();

		virtual void update();

		virtual const char* typeName() const;
	};

	typedef std::shared_ptr<LogicNode> sLogicNode;

	class Resource : public Node {
	public:
		Resource();
		virtual ~Resource();

		virtual const char* typeName() const;
	};

	class ResourceManager : public Node {
	public:
		ResourceManager();
		ResourceManager(const ResourceManager& other);
		virtual ~ResourceManager();

		void addResource(std::shared_ptr<Resource> resource);

		template<typename Type>
		std::shared_ptr<Type> getResource(std::string id) {
			for (size_t i = 0; i < this->_resources.size(); i++) {
				if (this->_resources[i]->id == id) {
					std::shared_ptr<Type> cast = std::dynamic_pointer_cast<Type>(this->_resources[i]);
					if (cast) {
						return cast;
					}
					else {
						throw BadCastException(std::string("failed to cast resource"), LOGGER_DETAIL_IDENTIFIER(this->id));
					}
				}
			}
			throw IdentifierException("failed to look up resource", id, LOGGER_DETAIL_DEFAULT);
		}

		virtual const char* typeName() const;

	private:
		std::vector<std::shared_ptr<Resource>> _resources;
	};

	typedef std::shared_ptr<ResourceManager> sResourceManager;
}