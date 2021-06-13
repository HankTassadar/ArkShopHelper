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
		PACKNOTOVER	//�ص���Э����ʵ��
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
	���Ҫ����һ����������Ϣ���ͣ�������һ���̳�������ӿڵ��࣬
	����ArkHelperServerAndClient::setMsgTypeCallback����Ӹ���
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
		* �Ƿ��Ѿ����������еİ�
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
		BoolPair<std::string> login;	//�Ƿ���ܵ���¼���ذ��Լ������ݰ�����
		BoolPair<PlayerData> playerData;	//�Ƿ���յ�������ݷ��ذ��Լ����ݰ�����
		BoolPair<json> shopData;	//�Ƿ���յ��̵����ݷ��ذ��Լ����ݰ�����
		BoolPair<json> kitData;	//�Ƿ���յ����������ݷ��ذ��Լ���������
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
	* ���ͻ��˺ͷ����д��һ�����з�������޸�
	*/
	class ArkHelperServerAndClient {
	public:
		static ArkHelperServerAndClient* GetInstance(u_short Port = 0, std::string IP = "");
		~ArkHelperServerAndClient();
		
	public://�˲��ֺ������ǿͻ���ʹ�õĺ���
		/*
		* �ȴ����ӳɹ�
		*/
		bool waitForConnected();

		//��������������
		void getPlayerData();

		void getShopData();

		void getPlayerKitData();

		void keepALive();

		/*
		* ��¼(Ϊ��������)
		*/
		BoolPair<std::string> login(std::string steam_id, std::string pass);

		//�������еõ����ݵ�����
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
		* ִ�д�������
		*/
		void doWithDataMutex(std::function<void()> fun);

		/*
		* �ȴ����ݽ�����ɸı�BoolPair��firstΪtrue
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
		* ����˹��캯��
		*/
		ArkHelperServerAndClient();

		/*
		* �ͻ��˹��캯��
		*/
		ArkHelperServerAndClient(std::string IP, u_short Port);

	private:
		void init();

		void run(bool onNewThread = false);

		/*
		* ���ø�����Ϣ���͵Ļص�����
		*/
		void setMsgTypeCallback();

		/*
		* ������Ϣ�Ļص�
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

	private://�������ͻ��˹��ú���
		void TimerCallback(uv::Timer* timer);

	private://�˲��ֵĺ������Ƿ���˵ĺ���
		/*
		* �����������ӽ����Ļص�
		*/
		void serverNewConnect(std::weak_ptr<TcpConnection> newconnection);

		/*
		* ���������ӹرյĻص�
		*/
		void serverCloseConnect(std::weak_ptr<TcpConnection> closeconnection);

	private:
		static ArkHelperServerAndClient* instance;

	private:
		ArkHelperType type;	//��ʵ�������ͣ�����˻��ǿͻ���
		ArkHelperData data;	//����

	private:
		int timerCount;	//��ʱ���ļ�����
	};

}