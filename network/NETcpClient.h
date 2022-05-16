#pragma once
#include "NENetBase.h"
#include "network_pub.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include "NENetChannel.h"
#include "NEIPAddress.h"
#include "NENetworkError.h"

namespace neapu {
	class NEAPU_NETWORK_EXPORT TcpClient : public NetBase {
	public:
		int Connect(const IPAddress& _addr, bool _enableWriteCallback = false);
		int Send(const ByteArray& data);

		TcpClient& OnWrite(std::function<void()> _cb);
		TcpClient& OnRecvData(std::function<void(const ByteArray& data)> _cb);
		TcpClient& OnError(std::function<void(const NetworkError& _err)> _cb);
		TcpClient& OnClosed(std::function<void()> _cb);

		IPAddress GetAddress() const { return m_address; }
	protected:
		virtual void OnSignalReady(int _signal) override;
		virtual void OnWrite();
		virtual void OnRecvData(const ByteArray& data);
		virtual void OnError(const NetworkError& _err);
		virtual void OnClosed();

		virtual void Stoped() override;
	private:
		virtual void OnReadReady(int _fd) override;
		virtual void OnWriteReady(int _fd) override { OnWrite(); }

	private:
		struct {
			std::function<void()> onWrite;
			std::function<void(const ByteArray& data)> onRecvData;
			std::function<void(const NetworkError& _err)> onError;
			std::function<void()> onClosed;
		} m_callback;
		int m_fd;
		std::shared_ptr<NetChannel> m_channel;
		IPAddress m_address;
	};
}