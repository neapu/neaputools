#pragma once
#include <NEString.h>
#include "NETcpClient.h"
#include <mutex>
#include <condition_variable>
#include "NERedisPublic.h"

namespace neapu {
	class NEAPU_READ_CONNECTOR_EXPORT RedisResponse {
	public:
		enum class Type
		{
			Empty,
			Error,
			Success,
			String,
			Integer
		};
		Type _type = Type::Empty;
		int _errorCode = 0;
		String _errorString;
		String _string;
		int64_t _integer = 0;
	};

	class NEAPU_READ_CONNECTOR_EXPORT RedisConnector : public TcpClient
	{
	public:
		RedisResponse SyncRunCommand(String _cmd);
		String GetReidsError() 
		{
			return m_redisError;
		}

	private:
		virtual void OnRecvData(std::shared_ptr<NetChannel> _channel) override;
		virtual void OnError(const NetworkError& _err) override;
		virtual void OnClosed() override;

	private:
		std::mutex m_connectorMutex;
		std::mutex m_syncMutex;
		std::condition_variable m_syncCond;
		bool m_rsped = false;
		String m_responseData;
		String m_redisError;
	};
}