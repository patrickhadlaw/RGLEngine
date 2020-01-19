#include "rgle/Event.h"



rgle::EventException::EventException()
{
}

rgle::EventException::EventException(std::string & except, Logger::Detail & detail) : Exception(except, detail)
{
}

rgle::EventException::EventException(const char * except, Logger::Detail & detail) : Exception(except, detail)
{
}

rgle::EventException::~EventException()
{
}

std::string rgle::EventException::_type()
{
	return std::string("rgle::EventException");
}

rgle::EventMessage::EventMessage()
{
}

rgle::EventMessage::~EventMessage()
{
}

rgle::EventHost::EventHost()
{
	_self = this;
}

rgle::EventHost::EventHost(const EventHost & other)
{
	_changePointer(other._self);
	this->_listeners = other._listeners;
}

rgle::EventHost::EventHost(const EventHost && rvalue)
{
	_changePointer(rvalue._self);
	this->_listeners = std::move(rvalue._listeners);
}

rgle::EventHost::~EventHost()
{
	for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
		std::vector<EventListener*>& list = it->second;
		for (int i = 0; i < list.size(); i++) {
			list[i]->_removeHost(this->_self);
		}
	}
}

void rgle::EventHost::operator=(const EventHost & other)
{
	_changePointer(other._self);
	this->_listeners = std::move(other._listeners);
}

void rgle::EventHost::operator=(const EventHost && rvalue)
{
	_changePointer(rvalue._self);
	this->_listeners = std::move(rvalue._listeners);
}

void rgle::EventHost::broadcastEvent(std::string eventname, EventMessage * message)
{
	if (_listeners.find(eventname) != _listeners.end()) {
		for (int i = 0; i < _listeners[eventname].size(); i++) {
			_listeners[eventname][i]->onMessage(eventname, message);
		}
		delete message;
	}
}

void rgle::EventHost::registerListener(std::string eventname, EventListener * listener)
{
	if (listener->_hosts.find(this) == listener->_hosts.end()) {
		listener->_hosts[this] = { eventname };
	}
	else {
		listener->_hosts[this].push_back(eventname);
	}
	_listeners[eventname].push_back(listener);
}

void rgle::EventHost::removeListener(std::string eventname, EventListener * listener)
{
	if (_listeners.find(eventname) == _listeners.end()) {
		throw EventException("failed to remove listener, reference not found", LOGGER_DETAIL_DEFAULT);
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
			std::cerr << "WARNING: event listener already removed" << std::endl;
		}
	}
}

void rgle::EventHost::_replaceListener(EventListener * previous, EventListener * replace)
{
	for (std::map<std::string, std::vector<EventListener*>>::iterator it = _listeners.begin(); it != _listeners.end(); ++it) {
		for (int i = 0; i < it->second.size(); i++) {
			if (it->second[i] == previous) {
				it->second[i] = replace;
			}
		}
	}
}

void rgle::EventHost::_changePointer(EventHost * previous)
{
	for (std::map<std::string, std::vector<EventListener*>>::iterator it = _listeners.begin(); it != _listeners.end(); ++it) {
		for (int i = 0; i < it->second.size(); i++) {
			it->second[i]->_replaceHost(previous, this);
		}
	}
	this->_self = previous;
}

rgle::EventListener::EventListener()
{
	_self = this;
	_hosts = {};
}

rgle::EventListener::EventListener(const EventListener & other)
{
	_changePointer(other._self);
	for (auto it = other._hosts.begin(); it != other._hosts.end(); ++it) {
		for (int i = 0; i < it->second.size(); i++) {
			it->first->registerListener(it->second[i], this);
		}
	}
}

rgle::EventListener::EventListener(EventListener && rvalue)
{
	_changePointer(rvalue._self);
	_hosts = std::move(rvalue._hosts);
}

rgle::EventListener::~EventListener()
{
	for (std::map<EventHost*, std::vector<std::string>>::iterator it = _hosts.begin(); it != _hosts.end(); ++it) {
		for (int i = 0; i < it->second.size(); i++) {
			it->first->removeListener(it->second[i], this);
		}
	}
}

void rgle::EventListener::operator=(const EventListener & other)
{
	_changePointer(other._self);
	for (auto it = other._hosts.begin(); it != other._hosts.end(); ++it) {
		for (int i = 0; i < it->second.size(); i++) {
			it->first->registerListener(it->second[i], this);
		}
	}
}


void rgle::EventListener::operator=(const EventListener && rvalue)
{
	_changePointer(rvalue._self);
	_hosts = std::move(rvalue._hosts);
}


void rgle::EventListener::onMessage(std::string eventname, EventMessage * message)
{
}

void rgle::EventListener::_removeHost(EventHost * host)
{
	bool found = false;
	for (std::map<EventHost*, std::vector<std::string>>::iterator it = _hosts.begin(); it != _hosts.end(); ++it) {
		if (it->first == host) {
			for (int i = 0; i < it->second.size(); i++) {
				it->first->removeListener(it->second[i], this);
			}
		}
	}
	if (!found) {
		std::cerr << "WARNING: event host already removed" << std::endl;
	}
}

void rgle::EventListener::_replaceHost(EventHost* previous, EventHost * replace)
{
	if (_hosts.find(previous) != _hosts.end()) {
		std::vector<std::string> value = _hosts[previous];
		_hosts.erase(_hosts.find(previous));
		_hosts[replace] = value;
	}
}

void rgle::EventListener::_changePointer(EventListener * previous)
{
	for (std::map<EventHost*, std::vector<std::string>>::iterator it = _hosts.begin(); it != _hosts.end(); ++it) {
		it->first->_replaceListener(previous, this);
	}
	this->_self = this;
}
