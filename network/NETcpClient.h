#pragma once
#ifdef USE_LIBEVENT
#include "network/NENetBase.h"
#include "network/network_pub.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include "network/NENetChannel.h"
#include "network/NEIPAddress.h"
#include "network/NENetworkError.h"
#include "base/NEByteArray.h"

namespace neapu {
	class NEAPU_NETWORK_EXPORT TcpClient : public NetBase {
	public:
		int Connect(const IPAddress& _addr);
		int Send(const ByteArray& data);
		int Send(const char* _data, size_t _len);
		bool IsConnected() { return m_channel != nullptr; }

		TcpClient& OnWrite(std::function<void()> _cb);
		TcpClient& OnRecvData(std::function<void(std::shared_ptr<NetChannel> _channel)> _cb);
		TcpClient& OnError(std::function<void(const NetworkError& _err)> _cb);
		TcpClient& OnClosed(std::function<void()> _cb);

		IPAddress GetAddress() const { return m_address; }
	protected:
		virtual void OnWrite();
		virtual void OnRecvData(std::shared_ptr<NetChannel> _channel);
		virtual void OnError(const NetworkError& _err);
		virtual void OnClosed();

		virtual void OnEventLoopStoped() override;
		virtual void OnReadReady(evutil_socket_t _socket, EventHandle _handle) override;
		virtual void OnWriteReady(evutil_socket_t _socket, EventHandle _handle) override { OnWrite(); }

	private:
		struct {
			std::function<void()> onWrite;
			std::function<void(std::shared_ptr<NetChannel> _channel)> onRecvData;
			std::function<void(const NetworkError& _err)> onError;
			std::function<void()> onClosed;
		} m_callback;
		int m_fd;
		std::shared_ptr<NetChannel> m_channel;
		IPAddress m_address;
	};

	class NEAPU_NETWORK_EXPORT TcpClientSync {
	public:
		int Connect(const IPAddress& _addr);
		NetworkError GetError() { return m_err; }
		int Send(const ByteArray& _data);
		int Send(const void* _data, size_t _len);
		ByteArray Recv(size_t _len = (size_t)(-1), int _timeout = 3000);
		void Close();
		bool IsConnected() { return m_fd != 0; }
	protected:
		void SetLastError(int _err, String _errstr);
	private:
		std::mutex m_recvLock;
		std::mutex m_writeLock;
		int m_fd = 0;
		IPAddress m_address;
		NetworkError m_err;
	};
}
#endif