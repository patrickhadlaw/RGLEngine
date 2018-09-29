#include "Event.h"



cppogl::EventException::EventException()
{
}

cppogl::EventException::EventException(std::string & except, Detail & detail) : Exception(except, detail)
{
}

cppogl::EventException::EventException(const char * except, Detail & detail) : Exception(except, detail)
{
}

cppogl::EventException::~EventException()
{
}

std::string cppogl::EventException::_type()
{
	return std::string("cppogl::EventException");
}

cppogl::EventMessage::EventMessage()
{
}

cppogl::EventMessage::~EventMessage()
{
}

cppogl::EventHost::EventHost()
{
}

cppogl::EventHost::~EventHost()
{
	for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
		std::vector<EventListener*>& list = it->second;
		for (int i = 0; i < list.size(); i++) {
			list[i]->_host == nullptr;
		}
	}
}

void cppogl::EventHost::broadcastEvent(std::string eventname, EventMessage * message)
{
	if (_listeners.find(eventname) != _listeners.end()) {
		for (int i = 0; i < _listeners[eventname].size(); i++) {
			_listeners[eventname][i]->onMessage(eventname, message);
		}
	}
}

void cppogl::EventHost::registerListener(std::string eventname, EventListener * listener)
{
	listener->_host = this;
	listener->_events.push_back(eventname);
	_listeners[eventname].push_back(listener);
}

void cppogl::EventHost::removeListener(std::string eventname, EventListener * listener)
{
	if (_listeners.find(eventname) == _listeners.end()) {
		throw EventException("failed to remove listener, reference not found", EXCEPT_DETAIL_DEFAULT);
	}
	else {
		std::vector<EventListener*>& list = _listeners[eventname];
		bool found = false;
		for (int i = 0; i < list.size(); i++) {
			if (list[i] == listener) {
				list.erase(list.begin() + i);
				found = true;
				break;
			}
		}
		if (!found) {
			throw EventException("failed to remove listener, reference not found", EXCEPT_DETAIL_DEFAULT);
		}
	}
}

cppogl::EventListener::EventListener()
{
	_host = nullptr;
}

cppogl::EventListener::~EventListener()
{
	if (_host != nullptr) {
		for (int i = 0; i < _events.size(); i++) {
			_host->removeListener(_events[i], this);
		}
	}
	_host = nullptr;
	_events.clear();
}

void cppogl::EventListener::onMessage(std::string eventname, EventMessage * message)
{
}
