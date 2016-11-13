#ifndef SYNC_QUEUE_HPP
#define SYNC_QUEUE_HPP

#include <mutex>
#include <boost/lockfree/spsc_queue.hpp>

namespace is {

using namespace boost;
using namespace boost::lockfree;

template <typename T, size_t Size> 
class SyncQueue {

public:
	SyncQueue() {
		mtx.lock();
	}

	bool push(const T& t) {
		bool ret =  queue.push(t);
		mtx.unlock();
		return ret;
	}

	void wait() {
        mtx.lock();
	}

	bool pop(T& t) {
        return queue.pop(t);
	}

	/* spsc_queue::reset not thread safe!
	void reset() {
		std::lock_guard<std::mutex> lg(mtx);
		queue.reset();
	}
	*/
private:
	bool ready;
	std::mutex mtx;
	boost::lockfree::spsc_queue<T, boost::lockfree::capacity<Size>> queue;
};

} // ::is

#endif // SYNC_QUEUE_HPP