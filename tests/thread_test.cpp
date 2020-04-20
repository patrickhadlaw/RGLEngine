#include "TestBed.h"

#include <iostream>
#include <cassert>

int main() {
	return rgle::TestBed::run([](rgle::TestBed& testBed) {
		rgle::ThreadPool pool = rgle::ThreadPool();
		std::atomic_int count = 0;
		for (int i = 0; i < 100; i++) {
			pool.startJob([&count]() {
				count++;
			});
		}
		while (!pool.standBy()) std::this_thread::yield();
		testBed.expect("thread pool should execute 100 jobs", [&count]() {
			return count == 100;
		});
	});
}