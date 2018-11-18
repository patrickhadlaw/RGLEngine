#pragma once

#include "Exception.h"

#include <vector>
#include <map>
#include <functional>

namespace cppogl {

	class EventException : public Exception {
	public:
		EventException();
		EventException(std::string& except, Detail& detail);
		EventException(const char* except, Detail& detail);
		virtual ~EventException();

	protected:
		virtual std::string _type();
	};

	template<typename Type>
	class Observable;

	template<typename Type>
	using sObservable = std::shared_ptr<Observable<Type>>;

	template<typename Type>
	class Subject {
	public:
		Subject() {
			_observers = {};
		}
		virtual ~Subject() {

		}

		void set(Type& state) {
			for (int i = 0; i < _observers.size(); i++) {
				if (_observers[i]._subscribe == nullptr) {

				}
				else {

				}
			}
		}

		sObservable<Type> observe(Observable<Type>* observer) {
			sObservable<Type> observable = std::make_shared<Observable<Type>>(observer);
			observable._subject = std::weak_ptr<Subject<Type>>(this);
			_observers.push_back(observable);
			return observable;
		}

	private:
		std::vector<sObservable<Type>> _observers;
	};

	template<typename Type>
	class Observable {
		friend class Subject<Type>;
	public:
		Observable();
		Observable(void(*_subscribe)(Type&));
		virtual ~Observable();

	private:
		void(*_subscribe)(Type&) = nullptr;
		std::weak_ptr<Subject<Type>> _subject = nullptr;
	};

	class EventMessage {
	public:
		EventMessage();
		virtual ~EventMessage();
	};

	class EventListener;

	class EventHost {
		friend class EventListener;
	public:
		EventHost();
		EventHost(const EventHost& other);
		EventHost(const EventHost&& rvalue);
		virtual ~EventHost();

		void operator=(const EventHost& other);
		void operator=(const EventHost&& rvalue);

		void broadcastEvent(std::string eventname, EventMessage* message);

		void registerListener(std::string eventname, EventListener* listener);
		void removeListener(std::string eventname, EventListener* listener);

	private:
		void _replaceListener(EventListener* previous, EventListener* replace);
		void _changePointer(EventHost* previous);

		EventHost* _self;
		std::map<std::string, std::vector<EventListener*>> _listeners;
	};

	class EventListener {
		friend class EventHost;
	public:
		EventListener();
		EventListener(const EventListener& other);
		EventListener(EventListener&& other);
		virtual ~EventListener();

		void operator=(const EventListener& other);
		void operator=(const EventListener&& rvalue);

		virtual void onMessage(std::string eventname, EventMessage* message);

	private:
		void _removeHost(EventHost* host);
		void _replaceHost(EventHost* previous, EventHost* replace);
		void _changePointer(EventListener* previous);

		EventListener* _self;
		std::map<EventHost*, std::vector<std::string>> _hosts;
	};
}