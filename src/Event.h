#pragma once

#include "Exception.h"

#include <vector>
#include <map>
#include <functional>

namespace cppogl {

	class EventException : Exception {
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

		std::string eventName;

	};

	class EventListener;

	class EventHost {
	public:
		EventHost();
		virtual ~EventHost();

		void broadcastEvent(std::string eventname, EventMessage* message);

		void registerListener(std::string eventname, EventListener* listener);
		void removeListener(std::string eventname, EventListener* listener);

	private:
		std::map<std::string, std::vector<EventListener*>> _listeners;
	};

	typedef std::function<void(std::string, EventMessage*) > EventCallback;

	class EventListener {
		friend class EventHost;
	public:
		EventListener();
		virtual ~EventListener();

		virtual void onMessage(std::string eventname, EventMessage* message);

	private:
		std::vector<std::string> _events;
		EventHost* _host = nullptr;
	};
}