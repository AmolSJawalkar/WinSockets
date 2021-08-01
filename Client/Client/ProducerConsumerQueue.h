#pragma once
#include <deque>
#include <mutex>
#include <condition_variable>

template<class T>
class ProducerConsumerQueue
{
	std::deque<T> _queue;
	std::mutex _mu;
	std::condition_variable _cv;
public:
	void Enqueue(const T&);
	T Dequeue();
};

template<class T>
inline void  ProducerConsumerQueue<T>::Enqueue(const T& val)
{
	std::unique_lock<std::mutex> ul(_mu);
	_queue.push_back(val);
	ul.unlock();
	_cv.notify_one();
}

template<class T>
inline T ProducerConsumerQueue<T>::Dequeue()
{
	std::unique_lock<std::mutex> ul(_mu);
	_cv.wait(ul, [&]() { return _queue.size() > 0; });
	auto val = _queue.front();
	_queue.pop_front();
	ul.unlock();
	return val;
}
