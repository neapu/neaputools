#pragma once
#include "base_pub.h"
#include <queue>
#include <mutex>
namespace neapu{
template <typename T>
class NEAPU_BASE_EXPORT SafeQueue
{
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
public:
    SafeQueue() { }
    SafeQueue(const SafeQueue&) = delete;
    SafeQueue(SafeQueue&& sq)
    {
        std::unique_lock<std::mutex> lock(sq.m_mutex);
        m_queue=sq.m_queue;
    }
    ~SafeQueue() {}

    bool empty() const
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    int size() const
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void enqueue(T &t){
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(t);
    }

    bool dequeue(T &t){
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_queue.empty()){
            return false;
        }
        t=std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
};

}