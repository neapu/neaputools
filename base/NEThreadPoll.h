#pragma once
#include <thread>
#include <vector>

namespace neapu{
    template <class T>
    class ThreadPoll {
    public:
        void Init(int _threadNum, T _proc)
        {
            for (int i = 0; i < _threadNum; i++) {
                m_threads.push_back(std::thread(_proc));
            }
        }
        void Join() {
            for (auto& thread : m_threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
        }
    private:
        std::vector<std::thread> m_threads;
    };
}