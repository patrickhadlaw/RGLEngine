#include "Thread.h"

cppogl::ThreadException::ThreadException()
{
}

cppogl::ThreadPool::ThreadPool(size_t n) : _locked(false), _alive(true), _ready(false), _activeWorkers(0), _queueMutex()
{
	this->_workers.reserve(n);
	for (int i = 0; i < n; i++) {
		this->_workers.push_back(std::thread([this]() { this->_threadLoop(); }));
	}
}

cppogl::ThreadPool::ThreadPool(const ThreadPool & other) : ThreadPool(other._workers.size())
{
}

cppogl::ThreadPool::~ThreadPool()
{
	this->_alive = false;
	this->_ready = true;
	this->_workerCondition.notify_all();
	for (int i = 0; i < this->_workers.size(); i++) {
		this->_workers[i].join();
	}
}

void cppogl::ThreadPool::startJob(std::function<void()> job)
{
	while (this->_locked) {
		std::this_thread::yield();
	}
	{
		std::lock_guard<std::mutex> guard(this->_queueMutex);
		this->_jobQueue.push(job);
	}
	this->_ready = true;
	this->_workerCondition.notify_one();
}

bool cppogl::ThreadPool::standBy()
{
	return this->_jobQueue.empty() && this->_activeWorkers == 0;
}

void cppogl::ThreadPool::_threadLoop()
{
	while (this->_alive) {
		std::function<void()> job;
		{
			std::unique_lock<std::mutex> lock(this->_queueMutex);
			this->_workerCondition.wait(lock, [this]() { return this->_ready.load(); });
			if (!this->_alive) {
				break;
			}
			this->_ready = this->_jobQueue.size() > 1;
			this->_activeWorkers++;
			this->_locked = true;
			job = this->_jobQueue.front();
			this->_jobQueue.pop();
			this->_locked = false;
		}
		job();
		this->_activeWorkers--;
	}
}
