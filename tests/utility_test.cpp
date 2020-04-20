#include "TestBed.h"

int main() {
	return rgle::TestBed::run([](rgle::TestBed& testBed) {

		testBed.expect("uid should generate changing strings", []() {
			return rgle::uid() != rgle::uid();
		});

		rgle::CollectingQueue<size_t> collectingQueue;

		testBed.expect("collecting queue should have size of 1", [&collectingQueue]() {
			collectingQueue.push(0);
			collectingQueue.push(1);
			collectingQueue.push(2);
			return collectingQueue.size() == 1;
		});

		testBed.expect("collecting queue clear should clear queue", [&collectingQueue]() {
			collectingQueue.clear();
			return collectingQueue.size() == 0;
		});

		testBed.expect("collecting queue should have size of 3", [&collectingQueue]() {
			collectingQueue.push(0);
			collectingQueue.push(1);
			collectingQueue.push(2);
			collectingQueue.push(4);
			collectingQueue.push(5);
			collectingQueue.push(6);
			collectingQueue.push(8);
			collectingQueue.push(9);
			collectingQueue.push(10);
			return collectingQueue.size() == 3;
		});

		collectingQueue.clear();

		testBed.expect("collecting queue pop should return top of queue", [&collectingQueue]() {
			collectingQueue.push(0);
			collectingQueue.push(1);
			collectingQueue.push(2);
			auto range = collectingQueue.pop();
			return range.lower == 0 && range.upper == 3 && collectingQueue.size() == 0;
		});

		collectingQueue.clear();

		testBed.expect("collecting queue should have correct ordering", [&collectingQueue]() {
			collectingQueue.push(9);
			collectingQueue.push(0);
			collectingQueue.push(10);
			collectingQueue.push(2);
			collectingQueue.push(1);
			collectingQueue.push(6);
			collectingQueue.push(8);
			collectingQueue.push(5);
			collectingQueue.push(4);
			auto first = collectingQueue.pop();
			auto second = collectingQueue.pop();
			auto third = collectingQueue.pop();
			return first.lower == 8 &&
				first.upper == 11 &&
				second.lower == 4 &&
				second.upper == 7 &&
				third.lower == 0 &&
				third.upper == 3 &&
				collectingQueue.size() == 0;
		});
	});
}