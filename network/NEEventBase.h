/**************************************************************************
* 描述：对libevent进行封装
* 作者：neapu
* 日期：2022-06-07
***************************************************************************/

#pragma once

#include <map>
#include <memory>
#include <functional>
#include "network_pub.h"

#ifndef evutil_socket_t
#ifdef WIN32
#include <stdint.h>
#define evutil_socket_t intptr_t
#else
#define evutil_socket_t int
#endif // WIN32
#endif // !evutil_socket_t

struct event_base;
struct event;
struct timeval;

using EventHandle = void*;
constexpr auto EmptyHandle = nullptr;

namespace neapu {
    class NEAPU_NETWORK_EXPORT EventBase {
    private:
        static void FileDescriptorCallback(evutil_socket_t fd, short events, void* user_data);
        static void SignalCallback(evutil_socket_t fd, short events, void* user_data);
        static void TimerCallback(evutil_socket_t fd, short events, void* user_data);
    public:
        enum EventType: short
        {
            None = 0,
            Read = 0x02,
            Write = 0x04
        };
        /*
        * 初始化libevent，包括创建event_base，设置iocp线程池
        * 
        * @参数： _iocpThreadCount：线程数，用于初始化iocp线程池
        * @返回值： 成功返回0，失败返回错误码
        * @参见：NENetworkError.h
        * 
        * 注：在非WIN32环境下，_iocpThreadCount参数无效
        */
        int Init(int _iocpThreadCount = 1);

        /*
        * 创建一个event并添加到事件循环
        * @参数：_fd：文件描述符，一般是socket
        * @参数：_events：事件，读或写，默认边缘触发
        * @参数：_persist：是否持久事件
        * @参数：_cb：事件回调
        * @返回值：成功返回event句柄，失败返回EmptyHandle
        * @参见：EventType
        */
        EventHandle AddEvent(
            evutil_socket_t _fd, 
            EventType _events, 
            bool _persist = false,
            std::function<void(evutil_socket_t _fd, EventType _type, EventHandle _handle)> _cb = {}
        );

        /*
        * 将一个已存在的handle添加到事件循环中，
        * 一般是非持久事件触发完后，重新添加到事件循环
        * @参数：_handle：要添加的事件handle
        * @返回值：成功返回0，失败返回-1
        */
        int AddEvent(EventHandle _handle);

        /*
        * 创建一个信号event并添加到事件循环
        * @参数: _signal: 信号，例如SIGINT
        * @参数: _persist: 是否持久事件
        * @参数: _cb: 事件触发时的回调
        * @返回值: 成功返回event句柄，失败返回EmptyHandle
        */
        EventHandle AddSignal(
            int _signal, 
            bool _persist = false,
            std::function<void(int _signal, EventHandle _handle)> _cb = {}
        );

        EventHandle AddTimer(int _timeout, bool _persist = false, std::function<void(EventHandle _handle)> _cb = {});

        /*
        * 从事件循环中移除并释放事件
        * @参数：_handle：要释放的事件
        */
        void ReleaseEvent(EventHandle _handle);

        //设置事件触发时的回调
        void OnFileDescriptorCallback(std::function<void(evutil_socket_t _fd, EventType _type, EventHandle _handle)> _cb);
        //设置信号触发时的回调
        void OnSignalCallback(std::function<void(int _signal, EventHandle _handle)> _cb);
        //设置定时器触发时的回调
        void OnTimerCallback(std::function<void(EventHandle _handle)> _cb);

        /*
        * 启动事件循环
        * 事件循环结束后会自动释放event_base，
        * 但是不会清理event，注册的事件需要自己释放
        */
        int EventLoop();

        /*
        * 结束事件循环
        * 如果正在执行某个事件处理函数，会在函数处理完后结束
        * 如果是在事件处理函数中调用这个函数，请结束事件处理，
        * 不要在事件里等待循环结束
        */
        int EventLoopBreak();
    protected:
        virtual void OnFileDescriptorCallback(evutil_socket_t _fd, EventType _type, EventHandle _handle);
        virtual void OnSignalCallback(int _signal, EventHandle _handle);
        virtual void OnTimerCallback(EventHandle _handle);
        virtual void OnEventLoopStoped() {}
    
    private:
        event_base* m_eb = nullptr;
        struct Event {
            event* handle = nullptr;
            EventBase* eventBase = nullptr;
            std::function<void(evutil_socket_t _fd, EventType _type, EventHandle _handle)> fdcb = {};
            std::function<void(int _signal, EventHandle _handle)> sigcb = {};
            std::function<void(EventHandle _handle)> timercb = {};
        };
        std::map<EventHandle, std::shared_ptr<Event>> m_events;
        struct {
            std::function<void(evutil_socket_t _fd, EventType _type, EventHandle _handle)> onFileDescriptorCallback;
            std::function<void(int _signal, EventHandle _handle)> onSignalCallback;
            std::function<void(EventHandle _handle)> onTimerCallback;
        }m_callback;
    private:
        EventHandle AddEventImpl(
            evutil_socket_t _fd,
            short _events,
            timeval* _timeout,
            void(*_cb)(evutil_socket_t, short, void*),
            std::shared_ptr<Event> _ev
        );
    };

}