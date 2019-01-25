#pragma once

#include <vector>
#include <memory>

#include "Exception.h"

namespace cppogl {

	class IdentifierException : public Exception {
	public:
		IdentifierException(std::string exception, std::string identifier, Exception::Detail detail);
		virtual ~IdentifierException();

	protected:
		virtual std::string _type();

		std::string _identifier;
	};

	class Node {
	public:
		Node();
		virtual ~Node();

		virtual std::string& typeName();

		std::string id;
	};

	class LogicException : public Exception {
	public:
		LogicException(std::string exception, Exception::Detail detail);
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

	typedef std::shared_ptr<Resource> sResource;

	class ResourceManager : public Node {
	public:
		ResourceManager();
		ResourceManager(const ResourceManager& other);
		virtual ~ResourceManager();

		void addResource(sResource resource);

		template<typename Type>
		std::shared_ptr<Type> getResource(std::string id) {
			for (int i = 0; i < _resources.size(); i++) {
				if (_resources[i]->id == id) {
					std::shared_ptr<Type> cast = std::dynamic_pointer_cast<Type>(_resources[i]);
					if (cast) {
						return cast;
					}
					else {
						throw BadCastException(std::string("failed to cast resource"), EXCEPT_DETAIL_IDENTIFIER(id));
					}
				}
			}
			throw IdentifierException("failed to look up resource", id, EXCEPT_DETAIL_DEFAULT);
		}

		virtual std::string& typeName();

	private:
		std::vector<sResource> _resources;
	};

	typedef std::shared_ptr<ResourceManager> sResourceManager;
}