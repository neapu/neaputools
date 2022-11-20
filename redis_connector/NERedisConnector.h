#pragma once
#include <base/NEString.h>
#include <mutex>
#include "redis_connector/NERedisPublic.h"
#include <network/NETcpClient.h>

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

		bool IsError() { return _errorCode != 0; }
		bool IsString() { return _type == Type::String; }
		bool IsInteger() { return _type == Type::Integer; }
	};

	class NEAPU_READ_CONNECTOR_EXPORT RedisConnector
	{
	public:
		int Connect(const IPAddress& _addr);
		RedisResponse SyncRunCommand(String _cmd);
		int Auth(String _password);
		int SyncSet(String _key, String _value);
		RedisResponse SyncGet(String _key);

		String GetReidsError() 
		{
			return m_redisError;
		}

	private:
		std::mutex m_connectorMutex;
		String m_responseData;
		String m_redisError;
		std::unique_ptr<TcpClientSync> m_tcpClientSync;
	};
}