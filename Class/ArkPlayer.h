#pragma once
#include<string>

class ServerDatabase;

enum class LoginReturnEnm {
	LOGIN_SUCCESS,
	NO_PASSWORD,
	PASSWORD_ERR,
	NO_STEAMID,
	IS_LOGINED
};

namespace ArkServer {

	class ArkPlayer {
	public:
		ArkPlayer() = delete;
		ArkPlayer(ServerDatabase* pDataBase);

	public:
		LoginReturnEnm Login(std::string& steamId, std::string& passWord);

		std::string GetVipLevel() {
			return _VipLevel;
		}

		uint32_t GetPoints() {
			return _Points;
		}

		void SetIPAndPort(std::string& ipAndPort) {
			_Addr = ipAndPort;
		}
	private:

	private:
		//玩家基础数据
		std::string _SteamID;
		std::string _PassWord;
		std::string _VipLevel;
		uint32_t _Points;

		//玩家状态数据
		bool _LoginState;

		//玩家的IP地址及链接端口
		std::string _Addr;

		//数据库指针
		ServerDatabase* _DataBase;
	};

};


namespace ArkClient {

	class ArkPlayer {
	public:

	public:
		LoginReturnEnm Login(std::string& steamId, std::string& passWord);

		bool UpdatePlayerData();

		void SetIPAndPort(std::string& ipAndPort) {
			_Addr = ipAndPort;
		}
	private:

	private:
		//玩家基础数据
		std::string _SteamID;
		std::string _PassWord;
		std::string _VipLevel;
		uint32_t _Points;

		//玩家状态数据
		bool _LoginState;
		
		//服务器的IP和地址
		std::string _Addr;
	};
};