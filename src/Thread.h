#pragma once

#include "Exception.h"

#include <thread>
#include <atomic>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace cppogl {
	class ThreadException : public Exception {
	public:
		ThreadException();
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