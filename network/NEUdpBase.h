#pragma once
#include <stdint.h>
#include <functional>
#include "NEIPAddress.h"
#include "NENetworkError.h"
#include <NEThreadPoll.h>
#include <NEString.h>
#include <NESafeQueue.h>
#include "network_pub.h"
#include "NENetBase.h"


namespace neapu {
	class ByteArray;
	class NEAPU_NETWORK_EXPORT UdpBase : public NetBase {
	public:
		UdpBase() {}
		virtual ~UdpBase() noexcept;
		UdpBase(const UdpBase&) = delete;
		UdpBase(UdpBase&& _ub) noexcept;
		int Init(int _threads, const IPAddress& _addr);
		int Send(const ByteArray& _data, const IPAddress& _addr);
		IPAddress Address() const { return m_address; }
		
		UdpBase& OnRecvData(std::function<void(const ByteArray&, const IPAddress&)> _cb);
		UdpBase& OnWriteReady(std::function<void()> _cb);

		void Stop();
		void Close();

	protected:
		virtual void OnRecvData(const ByteArray& _data, const IPAddress& _addr);
		virtual void OnWriteReady(evutil_socket_t _socket, EventHandle _handle) override;
	private:
		virtual void OnReadReady(evutil_socket_t _socket, EventHandle _handle) override;
		virtual void OnEventLoopStoped() override;

	protected:
		using UdpBaseCallback = struct {
			std::function<void(const ByteArray&, const IPAddress&)> recvDataCallback;
			std::function<void()> writableCallback;
		};
		int m_udpFd = 0;
		EventHandle m_eventHandle = EmptyHandle;
		UdpBaseCallback m_callback;
		IPAddress m_address;
	};
}