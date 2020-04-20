#pragma once

#include "rgle/Exception.h"

#define RGLE_DEBUG_ASSERT(x) RGLE_DEBUG_ONLY(rgle::debug_assert(x, LOGGER_DETAIL_DEFAULT);)

namespace rgle {
	const int UID_LENGTH = 25;

	class DebugException : public Exception {
	public:
		DebugException(std::string except, Logger::Detail detail);
	};

	void debug_assert(bool assert, Logger::Detail detail);

	unsigned char random_byte();

	// Generates unique identifiers
	// @unsecure
	// @returns a unique identifier string
	std::string uid();

	template<typename Type>
	Type unity() {
		Type result = Type{};
		result++;
		return result;
	}

	template<typename Type>
	struct Range {
		Type lower;
		Type upper;

		bool in(const Type& value) const {
			return value >= this->lower && value < this->upper;
		}

		Type length() const {
			return this->upper - this->lower;
		}
	};

	template<typename Type>
	class CollectingQueue {
	public:
		CollectingQueue() {}
		CollectingQueue(const Range<Type>& range) : _queue{ range } {}

		void push(const Type& value) {
			if (this->_queue.empty()) {
				this->_queue.push_back(Range<Type>{ value, value + unity<Type>() });
			}
			else {
				size_t idx = this->_queue.size() / 2;
				size_t half = static_cast<size_t>(std::ceil(this->_queue.size() / 4.0f));
				while (idx < this->size() && !this->_queue[idx].in(value) && std::ceil(half / 2.0f) != half) {
					if (value > this->_queue[idx].lower) {
						idx += half;
					}
					else {
						idx -= half;
					}
					half = static_cast<size_t>(std::ceil(half / 2.0f));
				}
				this->_insert(value, std::min(this->size() - 1, idx));
			}
		}

		Range<Type> pop() {
			Range<Type> result = this->_queue.back();
			this->_queue.pop_back();
			return result;
		}

		size_t size() const {
			return this->_queue.size();
		}

		bool empty() const {
			return this->_queue.empty();
		}

		void clear() {
			this->_queue.clear();
		}

	private:
		void _insert(const Type& value, const size_t& where) {
			Range<Type>& at = this->_queue[where];
			if (!at.in(value)) {
				if (where > 0 && value == this->_queue[where - 1].upper) {
					Range<Type>& before = this->_queue[where - 1];
					before.upper++;
					if (at.lower == before.upper) {
						Range<Type> merge = { before.lower, at.upper };
						this->_queue[where - 1] = merge;
						this->_queue.erase(this->_queue.begin() + where);
					}
				}
				else if (value == at.lower - unity<Type>()) {
					at.lower--;
				}
				else if (where < this->_queue.size() - 1 && value == this->_queue[where + 1].lower - 1) {
					Range<Type>& after = this->_queue[where + 1];
					after.lower--;
					if (at.upper == after.lower) {
						Range<Type> merge = { at.lower, after.upper };
						after = merge;
						this->_queue.erase(this->_queue.begin() + where);
					}
				}
				else if (value == at.upper) {
					at.upper++;
				}
				else {
					this->_queue.insert(this->_queue.begin() + where, Range<Type>{ value, value + unity<Type>() });
				}
			}
		}

		std::vector<Range<Type>> _queue;
	};
}