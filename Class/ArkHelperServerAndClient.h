#pragma once
#include<include/uv11.hpp>
#include<json.hpp>
#include"ServerDatabase.hpp"
#include"ServiceRegister.hpp"
#include"FileUpdate.h"

#pragma comment(lib,"uv_cpp.lib")
#pragma comment(lib,"libuv.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment (lib,"Iphlpapi.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "userenv.lib")

namespace ArkHelper {
	using namespace uv;
	using json = nlohmann::json;
	using json = nlohmann::json;

	class ArkHelperClient;
	class ArkHelperServer;

	template<typename _Ta>
	using BoolPair = std::pair<bool, _Ta>;

	enum class MSGTYPE {
		LOGIN = 1859,	//1
		RSP_LOGIN,
		GETPLAYERDATA,	//2
		RSP_GETPLAYERDATA,
		GETSHOPDATA,	//3
		RSP_GETSHOPDATA,
		GETKITDATA,	//4
		RSP_GETKITDATA,
		LOGOUT,
		RSP_NULL,
		KEEPALIVE,
		PACKNOTOVER	//回调在协议中实现
	};

	struct ArkPack {
		MSGTYPE msgtype;
		std::string data;
	};

	union ArkHelperData {
		ArkHelper::ArkHelperClient* clientData;
		ArkHelper::ArkHelperServer* serverData;
	};

	/*
	如果要新增一个往返的消息类型，就新增一个继承了这个接口的类，
	并在ArkHelperServerAndClient::setMsgTypeCallback中添加改类
	*/
	class BasePortocol {
	public:
		static void doWithClientDataMutex(ArkHelperData,std::function<void()>);
	};

	class LoginPortocol :BasePortocol {
	public:
		static ArkPack ClientRecv(const std::string&, ArkHelperData, const std::string&);
		static ArkPack ServerRecv(const std::string&, ArkHelperData, const std::string&);
	};

	class GetShopDataPortocol :BasePortocol {
	public:
		static ArkPack ClientRecv(const std::string&, ArkHelperData, const std::string&);
		static ArkPack ServerRecv(const std::string&, ArkHelperData, const std::string&);
	};

	class GetPlayerDataPortocol :BasePortocol {
	public:
		static ArkPack ClientRecv(const std::string&, ArkHelperData, const std::string&);
		static ArkPack ServerRecv(const std::string&, ArkHelperData, const std::string&);
	};

	class GetKitDataPortocol :BasePortocol {
	public:
		static ArkPack ClientRecv(const std::string&, ArkHelperData, const std::string&);
		static ArkPack ServerRecv(const std::string&, ArkHelperData, const std::string&);
	};

	class ArkHelperPortocal {
	public:
		ArkHelperPortocal();
		~ArkHelperPortocal();
	public:
		/**
		* 是否已经处理了所有的包
		*/
		bool isPackOver() {
			return _packOver;
		}
	public:
		std::pair<const char*, size_t> pack(ArkPack pack);

		std::vector<ArkPack>* unPack(const char* data, size_t size);

		ArkPack solveThePack(ArkPack& pack, ArkHelperData,const std::string&);

		static void setMsgTypeCallBack(MSGTYPE msgtype, std::function < ArkPack(const std::string&, ArkHelper::ArkHelperData, const std::string&)> func);

	public:
		static std::map<MSGTYPE, std::function<ArkPack(const std::string&, ArkHelperData, const std::string&)>> _callback;
	private:
		uint32_t byte32ToInt(const char* buffer);

		MSGTYPE byte32ToMsgType(const char* buffer);
	private:
		bool _packOver;
		int _lastPackLength;
		std::string _lastPackData;
		char* _packDataPtr;

	};

	struct PlayerData {
		std::string steamid;
		std::string pass;
		int points;
		std::string group;
	};

	struct Player {
		std::string steamid;
		bool logined;
		ArkHelperPortocal portocol;
	};

	struct ArkHelperClient {
		uv::SocketAddr* sockaddr;
		uv::EventLoop* loop;
		uv::TcpClientPtr client;
		uv::Timer* timer;

		ArkHelperPortocal* portocol;
		bool logined;
		bool connected;
		std::mutex dataMutex;
		std::thread* loopThread;
		BoolPair<std::string> login;	//是否接受到登录返回包以及包数据包内容
		BoolPair<PlayerData> playerData;	//是否接收到玩家数据返回包以及数据包内容
		BoolPair<json> shopData;	//是否接收到商店数据返回包以及数据包内容
		BoolPair<json> kitData;	//是否接收到玩家礼包数据返回包以及数据内容
	};
	
	struct ArkHelperServer {
		uv::SocketAddr* sockaddr;
		uv::EventLoop* loop;
		uv::TcpServer* server;
		uv::Timer* timer;

		u_short serverPort;
		json serverConfig;
		json shopData;
		ServerDatabase* dataBase;
		int registerInterval;
		ServiceRegister* serviceRegister;
		std::map<std::string, Player> connections;
		Update::FileUpdateServer* fileUpdateServer;
	};

	enum class ArkHelperType {
		SERVER,
		CLINET
	};

	/*
	* 将客户端和服务端写到一个类中方便进行修改
	*/
	class ArkHelperServerAndClient {
	public:
		static ArkHelperServerAndClient* GetInstance(u_short Port = 0, std::string IP = "");
		~ArkHelperServerAndClient();
		
	public://此部分函数都是客户端使用的函数
		/*
		* 等待连接成功
		*/
		bool waitForConnected();

		//向服务端请求数据
		void getPlayerData();

		void getShopData();

		void getPlayerKitData();

		void keepALive();

		/*
		* 登录(为阻塞函数)
		*/
		BoolPair<std::string> login(std::string steam_id, std::string pass);

		//从数据中得到数据的引用
		PlayerData& showPlayerData();

		json& showShopData();

		json& showKitData();

		bool showLoginState() {
			return data.clientData->logined;
		}

		bool showConnectState() {
			return data.clientData->connected;
		}
		/*
		* 执行带数据锁
		*/
		void doWithDataMutex(std::function<void()> fun);

		/*
		* 等待数据接受完成改边BoolPair的first为true
		*/
		template<typename _Ta>
		void waitForTrue(_Ta& boolPair) {
			clock_t start, end;
			start = clock();
			bool flag = false;
			while (!flag) {
				Sleep(10);
				doWithDataMutex([&]() {
					flag = boolPair.first;
					});
				if (clock() - start > 2000)flag = true;
			};
		}
	private:
		/*
		* 服务端构造函数
		*/
		ArkHelperServerAndClient();

		/*
		* 客户端构造函数
		*/
		ArkHelperServerAndClient(std::string IP, u_short Port);

	private:
		void init();

		void run(bool onNewThread = false);

		/*
		* 设置各种消息类型的回调函数
		*/
		void setMsgTypeCallback();

		/*
		* 设置消息的回调
		*/
		template<typename _Ta>
		void setCallback(MSGTYPE type) {
			switch (this->type)
			{
			case ArkHelper::ArkHelperType::SERVER: {
				ArkHelperPortocal::setMsgTypeCallBack(type
					, &_Ta::ServerRecv);
			}
												 break;
			case ArkHelper::ArkHelperType::CLINET: {
				ArkHelperPortocal::setMsgTypeCallBack(type
					, &_Ta::ClientRecv);
			}
												 break;
			default:
				break;
			}
		}

	private://服务端与客户端公用函数
		void TimerCallback(uv::Timer* timer);

	private://此部分的函数都是服务端的函数
		/*
		* 服务器新连接建立的回调
		*/
		void serverNewConnect(std::weak_ptr<TcpConnection> newconnection);

		/*
		* 服务器连接关闭的回调
		*/
		void serverCloseConnect(std::weak_ptr<TcpConnection> closeconnection);

	private:
		static ArkHelperServerAndClient* instance;

	private:
		ArkHelperType type;	//此实例的类型，服务端还是客户端
		ArkHelperData data;	//数据

	private:
		int timerCount;	//计时器的计数器
	};

}