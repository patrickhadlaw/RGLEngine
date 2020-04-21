#pragma once

#include "rgle/Exception.h"

namespace rgle {
	class ThreadException : public Exception {
	public:
		ThreadException(std::string except, Logger::Detail detail);
	};

	class ThreadPool {
	public:
		ThreadPool(size_t n = std::thread::hardware_concurrency());
		ThreadPool(const ThreadPool& other);
		~ThreadPool();

		void startJob(std::function<void()> job);

		bool standBy();

	private:
		void _threadLoop();

		std::vector<std::thread> _workers;

		std::atomic_int _activeWorkers;

		std::queue<std::function<void()>> _jobQueue;
		std::mutex _queueMutex;
		std::atomic_bool _locked;
		std::atomic_bool _alive;
		std::atomic_bool _ready;
		std::condition_variable _workerCondition;
	};
}