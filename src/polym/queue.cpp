#include "queue.hpp"

#include <chrono>
#include <utility>

namespace PolyM {

Queue::Queue()
{
}

Queue::~Queue()
{
}

void Queue::put(Msg&& msg)
{
	{
		std::lock_guard<std::mutex> lock(queueMutex_);
		queue_.push(msg.move());
	}

	queueCond_.notify_one();
}

std::unique_ptr<Msg> Queue::get(int timeoutMillis)
{
	std::unique_lock<std::mutex> lock(queueMutex_);

	if (timeoutMillis < 0) {
		queueCond_.wait(lock, [this]{return !queue_.empty();});
	} else if(timeoutMillis == 0) {
		queue_.emplace(new Msg(MSG_TIMEOUT));
	} else {
		// wait_for returns false if the return is due to timeout
		auto timeoutOccured = !queueCond_.wait_for(
			lock,
			std::chrono::milliseconds(timeoutMillis),
			[this]{return !queue_.empty();});

		if (timeoutOccured)
			queue_.emplace(new Msg(MSG_TIMEOUT));
	}

	auto msg = queue_.front()->move();
	queue_.pop();
	return msg;
}

std::unique_ptr<Msg> Queue::request(Msg&& msg)
{
	// Construct an ad hoc Queue to handle response Msg
	std::unique_lock<std::mutex> lock(responseMapMutex_);
	auto it = responseMap_.emplace(
		std::make_pair(msg.getUniqueId(), std::unique_ptr<Queue>(new Queue))).first;
	lock.unlock();

	put(std::move(msg));
	auto response = it->second->get(-1); // Block until response is put to the response Queue

	lock.lock();
	responseMap_.erase(it); // Delete the response Queue
	lock.unlock();

	return response;
}

void Queue::respondTo(MsgUID reqUid, Msg&& responseMsg)
{
	std::lock_guard<std::mutex> lock(responseMapMutex_);
	if (responseMap_.count(reqUid) > 0)
		responseMap_[reqUid]->put(std::move(responseMsg));
}

}
