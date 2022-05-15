#pragma once
#include "NENetBase.h"
#include "network_pub.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include "NENetChannel.h"
#include "NEIPAddress.h"

namespace neapu {
	class NEAPU_NETWORK_EXPORT TcpClient : public NetBase {
	public:
		int Connect(const IPAddress& _addr, bool _enableWriteCallback = false);
		void Send(const ByteArray& data);
		void Stop();

	protected:
		virtual void OnSignalReady(int _signal) override;
		virtual void OnWriteReady();

		virtual void OnRecvData(const ByteArray& data) {}
		virtual void OnChannelError(std::shared_ptr<neapu::NetChannel> _client) {}
	private:
		virtual void OnReadReady(int _fd) override;
		virtual void OnWriteReady(int _fd) override { OnWriteReady(); }

	private:
		int m_fd;
		std::shared_ptr<NetChannel> m_channel;
		IPAddress m_address;
	};
}