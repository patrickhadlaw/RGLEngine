#include "rgle.h"

int main() {
	return rgle::Tester::run([](rgle::Tester& tester) {
		rgle::ThreadPool pool = rgle::ThreadPool();
		std::atomic_int count = 0;
		for (int i = 0; i < 100; i++) {
			pool.startJob([&count]() {
				count++;
			});
		}
		while (!pool.standBy()) std::this_thread::yield();
		tester.expectAndPrint("thread pool should execute 100 jobs", [&count]() {
			return count == 100;
		}, [&count]() -> std::string {
			std::stringstream ss;
			ss << "expected " << count << " to equal 100";
			return ss.str();
		});
	});
}