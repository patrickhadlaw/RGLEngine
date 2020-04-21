#include "rgle.h"

int main() {
	return rgle::Tester::run([](rgle::Tester& tester) {

		tester.expect("uid should generate changing strings", []() {
			return rgle::uid() != rgle::uid();
		});

		rgle::CollectingQueue<size_t> collectingQueue;

		tester.expectAndPrint("collecting queue should have size of 1", [&collectingQueue]() {
			collectingQueue.push(0);
			collectingQueue.push(1);
			collectingQueue.push(2);
			return collectingQueue.size() == 1;
		}, [&collectingQueue]() -> std::string {
			std::stringstream ss;
			ss << "expected " << collectingQueue.size() << " to equal 1";
			return ss.str();
		});

		tester.expectAndPrint("collecting queue clear should clear queue", [&collectingQueue]() {
			collectingQueue.clear();
			return collectingQueue.size() == 0;
		}, [&collectingQueue]() -> std::string {
			std::stringstream ss;
			ss << "expected " << collectingQueue.size() << " to equal 0";
			return ss.str();
		});

		tester.expectAndPrint("collecting queue should have size of 3", [&collectingQueue]() {
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
		}, [&collectingQueue]() -> std::string {
			std::stringstream ss;
			ss << "expected " << collectingQueue.size() << " to equal 3";
			return ss.str();
		});

		rgle::Range<size_t> range;

		tester.expectAndPrint("collecting queue pop should return top of queue", [&collectingQueue, &range]() {
			collectingQueue.clear();
			collectingQueue.push(0);
			collectingQueue.push(1);
			collectingQueue.push(2);
			range = collectingQueue.pop();
			return range.lower == 0 && range.upper == 3 && collectingQueue.size() == 0;
		}, [&collectingQueue, &range]() -> std::string {
			std::stringstream ss;
			if (collectingQueue.size() == 0)
				ss << "expected " << collectingQueue.size() << " to equal 0";
			else
				ss << "expected {" << range.lower << ", " << range.upper << "} to equal {0, 3}";
			return ss.str();
		});

		rgle::Range<size_t> first;
		rgle::Range<size_t> second;
		rgle::Range<size_t> third;

		tester.expectAndPrint("collecting queue should have correct ordering", [&collectingQueue, &first, &second, &third]() {
			collectingQueue.clear();
			collectingQueue.push(9);
			collectingQueue.push(0);
			collectingQueue.push(10);
			collectingQueue.push(2);
			collectingQueue.push(1);
			collectingQueue.push(6);
			collectingQueue.push(8);
			collectingQueue.push(5);
			collectingQueue.push(4);
			first = collectingQueue.pop();
			second = collectingQueue.pop();
			third = collectingQueue.pop();
			return first.lower == 8 &&
				first.upper == 11 &&
				second.lower == 4 &&
				second.upper == 7 &&
				third.lower == 0 &&
				third.upper == 3 &&
				collectingQueue.size() == 0;
		}, [&collectingQueue, &first, &second, &third]() -> std::string {
			std::stringstream ss;
			if (first.lower != 8 || first.upper != 11)
				ss << "expected {" << first.lower << ", " << first.upper << "} to equal {8, 11}";
			else if (second.lower != 4 || second.upper != 7)
				ss << "expected {" << second.lower << ", " << second.upper << "} to equal {4, 7}";
			else if (third.lower != 0 || third.upper != 3)
				ss << "expected {" << second.lower << ", " << second.upper << "} to equal {0, 3}";
			else
				ss << "expected " << collectingQueue.size() << " to equal 0";
			return ss.str();
		});
	});
}