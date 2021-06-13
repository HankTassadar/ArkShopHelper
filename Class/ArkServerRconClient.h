#pragma once

#include<include/uv11.hpp>


#pragma comment(lib,"uv_cpp.lib")
#pragma comment(lib,"libuv.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment (lib,"Iphlpapi.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "userenv.lib")

namespace ArkRcon {

#define RCON_HEADER_SIZE 14

	enum class MSGTYPE
	{
		SERVERDATA_AUTH = 3,
		SERVERDATA_EXECCOMMAND = 2,
		SERVERDATA_AUTH_RESPONSE = 2,
		SERVERDATA_RESPONSE_VALUE = 0
	};
}