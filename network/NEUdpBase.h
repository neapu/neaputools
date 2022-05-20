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

	protected:
		virtual void OnRecvData(const ByteArray& _data, const IPAddress& _addr);
		virtual void OnWriteReady(int _fd);
	private:
		virtual void OnReadReady(int _fd);
		virtual void OnSignalReady(int _signal);
		virtual void Stoped() override;

	protected:
		using UdpBaseCallback = struct {
			std::function<void(const ByteArray&, const IPAddress&)> recvDataCallback;
			std::function<void()> writableCallback;
			uint64_t userData;
		};
		int m_udpFd = 0;
		UdpBaseCallback m_callback;
		IPAddress m_address;
	};
}