#pragma once
#include<include/uv11.hpp>
#include<json.hpp>
#include"ServiceRegister.hpp"
#include"ServiceQuery.hpp"
#include<io.h>

#pragma comment(lib,"uv_cpp.lib")
#pragma comment(lib,"libuv.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment (lib,"Iphlpapi.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "userenv.lib")

namespace Update {
	using json = nlohmann::json;
	class ServerData;
	class ClientData;
	enum class MSGTYPE {
		GETFILEMD5,
		RSP_GETFILEMD5,
		GETFILE,
		RSP_GETFILE,
		GETSUBFOLDER,
		RSP_GETSUBFOLDER,
		PACKNOTOVER,
		RSP_NULL
	};

	struct MsgPack {
		MSGTYPE msgtype;
		std::string data;
	};

	union FileUpdateData {
		ServerData* serverData;
		ClientData* clientData;
	};

	class FileUpdatePortocol {
	public:
		FileUpdatePortocol();

	public:
		/**
		* 是否已经处理了所有的包
		*/
		bool isPackOver() {
			return _packOver;
		}
	public:
		std::pair<const char*, size_t> pack(MsgPack pack);

		std::vector<MsgPack>* unPack(const char* data, size_t size);

		/*
		* last string is uv_cpp's connection name
		*/
		MsgPack solveThePack(MsgPack& pack, FileUpdateData, const std::string&);


		static void setMsgTypeCallBack(MSGTYPE msgtype, std::function < MsgPack(const std::string&, FileUpdateData, const std::string&)> func);

	public:
		static std::map<MSGTYPE, std::function<MsgPack(const std::string&, FileUpdateData, const std::string&)>> _callback;
	private:
		uint32_t byte32ToInt(const char* buffer);

		MSGTYPE byte32ToMsgType(const char* buffer);
	private:
		bool _packOver;
		int _lastPackLength;
		std::string _lastPackData;
		char* _packDataPtr;
	};

	struct ServerConnection {
		FileUpdatePortocol portocol;
	};

	struct ServerData {
		uv::SocketAddr* sockAddr;
		uv::EventLoop* loop;
		uv::TcpServer* server;
		uv::Timer* timer;

		u_short port;
		std::mutex dataMutex;
		int registerInterval;
		ServiceRegister* serviceRegister;
		std::string fileRootPath;	//the file path you want to update
		std::map<std::string, std::pair<std::string, std::string>> allFile;	//file name,file md5 and file data
		std::vector<std::string> subFolder;
		std::map<std::string, FileUpdatePortocol> connections;
	};

	struct ClientData {
		uv::SocketAddr* sockAddr;
		uv::EventLoop* loop;
		uv::TcpClientPtr client;

		std::mutex dataMutex;
		bool connected;
		FileUpdatePortocol* portocol;
		ServiceQuery* serviceQuery;
		bool recv;
		std::vector<std::string> subFolder;
		std::vector<std::pair<std::string, std::string>> allFile;	//file name,file md5
		std::string newFile;	//filefrom net
	};

	class FileUpdateServer {
	public:
		FileUpdateServer(uv::EventLoop* loop = nullptr);
		FileUpdateServer() = delete;

		/*
		* Only use for if this new a loop
		*/
		void run();

		void initAllUpdateFile();
	private:
		void init();

	private:
		void TimerCallback(uv::Timer* timer);

	private:
		/*
		* 服务器新连接建立的回调
		*/
		void serverNewConnect(std::weak_ptr<uv::TcpConnection> newconnection);

		/*
		* 服务器连接关闭的回调
		*/
		void serverCloseConnect(std::weak_ptr<uv::TcpConnection> closeconnection);
	private:
		FileUpdateData data;
		int timerCount;
	};

	class FileUpdateClient {
	public:
		FileUpdateClient(std::string ServiceName);
		FileUpdateClient(std::string IP, u_short Port);
		FileUpdateClient() = delete;

	public:
		void getFileMd5();

		std::string& getFile(const std::string&);

		void getSubFolder();

		std::vector<std::pair<std::string, std::string>>& showFileAndMd5() {
			return data.clientData->allFile;
		}

		std::vector<std::string>& showSubFolder() {
			return data.clientData->subFolder;
		}
	private:
		void init();

		void doWithClientDataMutex(std::function<void()>);

		void waitForRecv();

	private:
		FileUpdateData data;
	};

	class BasePortocol {
	public:
		static void doWithClientDataMutex(FileUpdateData, std::function<void()>);
	};

	class GetFileMd5Portocol:public BasePortocol {
	public:
		static MsgPack serverRecv(const std::string&, FileUpdateData, const std::string&);
		static MsgPack clientRecv(const std::string&, FileUpdateData, const std::string&);
	};

	class GetFilePortocol :public BasePortocol {
	public:
		static MsgPack serverRecv(const std::string&, FileUpdateData, const std::string&);
		static MsgPack clientRecv(const std::string&, FileUpdateData, const std::string&);
	};

	class GetSubFolder :public BasePortocol {
	public:
		static MsgPack serverRecv(const std::string&, FileUpdateData, const std::string&);
		static MsgPack clientRecv(const std::string&, FileUpdateData, const std::string&);
	};
	/*
	* get all the file in the path
	*/
	void getAllFile(std::string path, std::vector<std::pair<std::string, __finddata64_t>>& allfile);

	/*
	* get a md5 from a string
	*/
	std::string calculateMd5(std::string& data);

	template<typename _Ta>
	void setMsgTypeCallback(MSGTYPE type,bool isServer) {
		if (isServer)
			FileUpdatePortocol::setMsgTypeCallBack(type, &_Ta::serverRecv);
		else
			FileUpdatePortocol::setMsgTypeCallBack(type, &_Ta::clientRecv);
	}
}