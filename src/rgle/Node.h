#pragma once

#include "rgle/Exception.h"

namespace rgle {

	class IdentifierException : public Exception {
	public:
		IdentifierException(std::string exception, std::string identifier, Logger::Detail detail);
		virtual ~IdentifierException();

	protected:
		virtual std::string _type();

		std::string _identifier;
	};

	namespace UUID {
		std::string generate();
	}

	class Node {
	public:
		Node();
		virtual ~Node();

		virtual std::string& typeName();

		std::string id;
	};

	class LogicException : public Exception {
	public:
		LogicException(std::string exception, Logger::Detail detail);
		virtual ~LogicException();

	protected:
		virtual std::string _type();
	};

	class LogicNode : public Node {
	public:
		LogicNode();
		virtual ~LogicNode();

		virtual void update();

		virtual std::string& typeName();
	};

	typedef std::shared_ptr<LogicNode> sLogicNode;

	class Resource : public Node {
	public:
		Resource();
		virtual ~Resource();

		virtual std::string& typeName();
	};

	class ResourceManager : public Node {
	public:
		ResourceManager();
		ResourceManager(const ResourceManager& other);
		virtual ~ResourceManager();

		void addResource(std::shared_ptr<Resource> resource);

		template<typename Type>
		std::shared_ptr<Type> getResource(std::string id) {
			for (int i = 0; i < _resources.size(); i++) {
				if (_resources[i]->id == id) {
					std::shared_ptr<Type> cast = std::dynamic_pointer_cast<Type>(_resources[i]);
					if (cast) {
						return cast;
					}
					else {
						throw BadCastException(std::string("failed to cast resource"), LOGGER_DETAIL_IDENTIFIER(id));
					}
				}
			}
			throw IdentifierException("failed to look up resource", id, LOGGER_DETAIL_DEFAULT);
		}

		virtual std::string& typeName();

	private:
		std::vector<std::shared_ptr<Resource>> _resources;
	};

	typedef std::shared_ptr<ResourceManager> sResourceManager;
}