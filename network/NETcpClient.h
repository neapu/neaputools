#pragma once
#include "NETcpBase.h"
#include "network_pub.h"
#include <thread>
#include <mutex>
#include <condition_variable>

namespace neapu {
	class NEAPU_NETWORK_EXPORT TcpClient : public TcpBase {
		friend class NetWorkThread;
		friend void cbClientRead(evutil_socket_t fd, short events, void* user_data);
	public:
		using RecvDataCallbackCli = std::function<void(const ByteArray&, uint64_t)>;
		using ConnectedCallback = std::function<void(uint64_t)>;
		int Connect(String _IPAddr, int _port, 
			const RecvDataCallbackCli& _recvCb = {}, 
			const ConnectedCallback& _connCb = {}, 
			uint64_t _userData = 0
		);
		void Close();
		void Send(const ByteArray& data);

	protected:
		virtual void OnRecvData(const ByteArray& data) {}
		virtual int OnFdReadReady(int _fd);
		virtual void OnConnected() {}
	private:
		void WorkThread();
	private:
		int m_fd;
		std::shared_ptr<NetChannel> m_channel;
		std::thread m_workThread;
		//std::shared_ptr<NetWorkThread> m_workThread;
		RecvDataCallbackCli m_recvCb;
		ConnectedCallback m_connectCb;
		std::mutex m_workThreadMutex;
		std::condition_variable m_workThreadCond;//通知主线程已进入事件循环
	};
}