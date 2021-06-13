#include"FileUpdate.h"
#include<fstream>
#include<iostream>
#include<openssl/md5.h>
using namespace Update;

std::map<MSGTYPE, std::function<MsgPack(const std::string&, FileUpdateData, const std::string&)>> FileUpdatePortocol::_callback;

Update::FileUpdatePortocol::FileUpdatePortocol()
	:_packOver(true)
	, _lastPackLength(0)
	, _packDataPtr(nullptr)
{
	this->_callback[MSGTYPE::PACKNOTOVER] = [](const std::string&, FileUpdateData, const std::string&)->MsgPack {
		return { MSGTYPE::RSP_NULL,"" };
	};
}

std::pair<const char*, size_t> Update::FileUpdatePortocol::pack(MsgPack pack)
{
	if (this->_packDataPtr != nullptr) {
		delete[]this->_packDataPtr;
		this->_packDataPtr = nullptr;
	}
	this->_packDataPtr = new char[10 + pack.data.size()];
	::memset(this->_packDataPtr, 0, 10 + pack.data.size());

	int datalength = 10 + pack.data.size();
	::memcpy((void*)this->_packDataPtr, &datalength, 4);
	::memcpy((void*)(this->_packDataPtr + 4), &pack.msgtype, 4);
	::memcpy((void*)(this->_packDataPtr + 8), pack.data.c_str(), pack.data.size());
	return { this->_packDataPtr,10 + pack.data.size() };
}

std::vector<MsgPack>* Update::FileUpdatePortocol::unPack(const char* data, size_t size)
{
	auto packvec = new std::vector<MsgPack>();
	char* ptr = (char*)data;
	long long length = size;

	while (length > 0) {

		if (this->_packOver) {	//这是新的包

			auto datalength = this->byte32ToInt(ptr);

			if (datalength > length) {	//未完整接收到一个包，后续还有数据
				this->_lastPackData.clear();
				this->_packOver = false;
				this->_lastPackLength = datalength;
				this->_lastPackData.append(ptr, length);
				packvec->push_back({ MSGTYPE::PACKNOTOVER,"" });

			}

			if (datalength <= length) {	//刚好一个包或者两个包粘包了

				auto msgtype = this->byte32ToMsgType(ptr + 4);
				packvec->push_back({ msgtype,std::string(ptr + 8,datalength - 10) });
				ptr += datalength;

			}

			length -= datalength;
		}
		else {	//上个包没接收完，还有数据

			if (length + this->_lastPackData.size() < this->_lastPackLength) {
				//还没有接收完这个包
				this->_lastPackData.append(ptr, length);
				length = 0;
			}
			else {
				//收到了包的最后一部分
				auto l = this->_lastPackLength - this->_lastPackData.size();
				this->_lastPackData.append(ptr, l);
				ptr += l;
				length -= l;
			}

			if (this->_lastPackData.size() == this->_lastPackLength) {

				this->_packOver = true;
				MsgPack repack;
				auto msgtype = this->byte32ToMsgType(this->_lastPackData.c_str() + 4);
				repack.msgtype = msgtype;
				repack.data = this->_lastPackData.substr(8, this->_lastPackData.size() - 10);
				packvec->push_back(repack);

			}
			else {

				packvec->push_back({ MSGTYPE::PACKNOTOVER,"" });

			}

		}

	}

	return packvec;
}

MsgPack Update::FileUpdatePortocol::solveThePack(MsgPack& pack, FileUpdateData data, const std::string& name)
{
	return this->_callback[pack.msgtype](pack.data, data, name);
}

void Update::FileUpdatePortocol::setMsgTypeCallBack(MSGTYPE msgtype, std::function<MsgPack(const std::string&, FileUpdateData, const std::string&)> func)
{
	FileUpdatePortocol::_callback[msgtype] = func;
}

uint32_t Update::FileUpdatePortocol::byte32ToInt(const char* buffer)
{
	return static_cast<size_t>(
		static_cast<unsigned char>(buffer[0]) |
		static_cast<unsigned char>(buffer[1]) << 8 |
		static_cast<unsigned char>(buffer[2]) << 16 |
		static_cast<unsigned char>(buffer[3]) << 24);
}

MSGTYPE Update::FileUpdatePortocol::byte32ToMsgType(const char* buffer)
{
	return static_cast<MSGTYPE>(
		static_cast<unsigned char>(buffer[0]) |
		static_cast<unsigned char>(buffer[1]) << 8 |
		static_cast<unsigned char>(buffer[2]) << 16 |
		static_cast<unsigned char>(buffer[3]) << 24);
}

Update::FileUpdateServer::FileUpdateServer(uv::EventLoop* loop)
	:data({})
	,timerCount(0)
{
	data.serverData = new ServerData();

	std::ifstream file("Config.json");
	json serverConfig;
	file >> serverConfig;
	file.close();
	file.clear();
	u_short Port = serverConfig["UpdateServerPort"].get<unsigned short>();
	std::cout << Port << std::endl;
	std::string path = serverConfig["FileRootPath"].get<std::string>();
	std::cout << path << std::endl;
	data.serverData->registerInterval = serverConfig["RegisterInterval"].get<int>();
	data.serverData->sockAddr = new uv::SocketAddr({ "0.0.0.0",Port,uv::SocketAddr::Ipv4 });
	if (loop == nullptr)
		data.serverData->loop = new uv::EventLoop();
	else
		data.serverData->loop = loop;
	data.serverData->server = new uv::TcpServer(data.serverData->loop);
	data.serverData->fileRootPath = path;
	data.serverData->port = Port;
	data.serverData->serviceRegister = new ServiceRegister({
		serverConfig["ServiceRegisterServer"]["IP"].get<std::string>()	//服务注册服务端IP地址
		,serverConfig["ServiceRegisterServer"]["Port"].get<unsigned short>()	//服务注册服务端端口号
		,uv::SocketAddr::Ipv4
		}
	, 10005);
	this->init();
}

void Update::FileUpdateServer::run()
{
	data.serverData->loop->run();
}

void Update::FileUpdateServer::initAllUpdateFile()
{
	data.serverData->allFile.clear();
	std::vector<std::pair<std::string, __finddata64_t>> fileList;
	getAllFile(data.serverData->fileRootPath, fileList);
	std::cout << "已找到的文件数：" << fileList.size() << std::endl;
	std::ifstream openFile;

	for (auto& file : fileList) {
		std::cout << "文件" << file.second.name << "/" << file.second.attrib << std::endl;
		if (file.second.attrib < _A_SUBDIR) {
			openFile.open(file.first, std::ios::binary);
			if (openFile.is_open())
				std::cout << "文件：" << file.first << " 打开成功" << std::endl;
			else
				std::cout << "文件：" << file.first << " 打开失败" << std::endl;
			std::stringstream ss;
			ss << openFile.rdbuf();
			openFile.close();
			openFile.clear();
			std::string filedata = ss.str();
			data.serverData->allFile[file.first] = std::make_pair(calculateMd5(filedata), filedata);
		}
		else if (file.second.attrib >= _A_SUBDIR && file.second.attrib < _A_ARCH) {
			data.serverData->subFolder.push_back(file.first);
		};
	}

	std::cout << "文件总数：" << data.serverData->allFile.size() << std::endl;
}

void Update::FileUpdateServer::init()
{
	this->initAllUpdateFile();

	setMsgTypeCallback<GetFilePortocol>(MSGTYPE::GETFILE, true);
	setMsgTypeCallback<GetFileMd5Portocol>(MSGTYPE::GETFILEMD5, true);
	setMsgTypeCallback<GetSubFolder>(MSGTYPE::GETSUBFOLDER, true);
	data.serverData->server->SetBufferMode(uv::GlobalConfig::BufferMode::NoBuffer);

	data.serverData->server->setNewConnectCallback(std::bind(&FileUpdateServer::serverNewConnect, this, std::placeholders::_1));

	data.serverData->server->setConnectCloseCallback(std::bind(&FileUpdateServer::serverCloseConnect, this, std::placeholders::_1));

	data.serverData->server->setMessageCallback([=](uv::TcpConnectionPtr ptr, const char* data, ssize_t size)
		{
			auto portocol = this->data.serverData->connections[ptr.get()->Name()];
			auto packvec = portocol.unPack(data, size);

			for (auto& pack : *packvec) {

				if (pack.msgtype == MSGTYPE::PACKNOTOVER)continue;
				auto rsppack = portocol.solveThePack(pack, this->data, ptr.get()->Name());

				if (rsppack.msgtype != MSGTYPE::RSP_NULL) {

					auto res = portocol.pack(rsppack);
					int i = 0;
					ptr->write(res.first, res.second, nullptr);

				}
			}

			delete(packvec);
		});

	data.serverData->server->bindAndListen(*data.serverData->sockAddr);
	data.serverData->timer = new uv::Timer(data.serverData->loop, 1000, 1000
		, std::bind(&FileUpdateServer::TimerCallback, this, std::placeholders::_1));
	data.serverData->timer->start();
}

void Update::FileUpdateServer::TimerCallback(uv::Timer* timer)
{
	if (this->timerCount % data.serverData->registerInterval == 0) {
		data.serverData->serviceRegister->regist("ArkHelperUpdate", data.serverData->port);
	}

	if (++this->timerCount == 3601)this->timerCount = 0;
}

void Update::FileUpdateServer::serverNewConnect(std::weak_ptr<uv::TcpConnection> newconnection)
{
	data.serverData->dataMutex.lock();
	data.serverData->connections[newconnection.lock().get()->Name()];
	data.serverData->dataMutex.unlock();
}

void Update::FileUpdateServer::serverCloseConnect(std::weak_ptr<uv::TcpConnection> closeconnection)
{
	data.serverData->dataMutex.lock();
	data.serverData->connections.erase(closeconnection.lock().get()->Name());
	data.serverData->dataMutex.unlock();
}

Update::FileUpdateClient::FileUpdateClient(std::string ServiceName)
	:data{ {} }
{
	data.clientData = new ClientData();
	data.clientData->serviceQuery = new ServiceQuery({ "49.232.218.70",35996,uv::SocketAddr::Ipv4 }, 10005);
	auto ipPort = data.clientData->serviceQuery->query("ArkHelperUpdate");
	if (ipPort.second == 0) {
		throw("Can not connect to Service Query Server or Service ArkHelperUpdate Not Exist!");
	}
	data.clientData->sockAddr = new uv::SocketAddr({ ipPort.first,ipPort.second,uv::SocketAddr::Ipv4 });
	data.clientData->loop = new uv::EventLoop();
	data.clientData->client = std::make_shared<uv::TcpClient>(data.clientData->loop);
	data.clientData->portocol = new FileUpdatePortocol();

	this->init();
}

Update::FileUpdateClient::FileUpdateClient(std::string IP, u_short Port)
	:data{ {} }
{
	data.clientData = new ClientData();
	data.clientData->sockAddr = new uv::SocketAddr({ IP,Port,uv::SocketAddr::Ipv4 });
	data.clientData->loop = new uv::EventLoop();
	data.clientData->client = std::make_shared<uv::TcpClient>(data.clientData->loop);
	data.clientData->portocol = new FileUpdatePortocol();

	this->init();
}

void Update::FileUpdateClient::getFileMd5()
{
	data.clientData->recv = false;
	auto pack = data.clientData->portocol->pack({ MSGTYPE::GETFILEMD5,"" });
	data.clientData->client->write(pack.first, pack.second);

	this->waitForRecv();
}

std::string& Update::FileUpdateClient::getFile(const std::string& filename)
{
	data.clientData->recv = false;
	auto pack = data.clientData->portocol->pack({ MSGTYPE::GETFILE,filename });
	data.clientData->client->write(pack.first, pack.second);

	this->waitForRecv();
	return data.clientData->newFile;
}

void Update::FileUpdateClient::getSubFolder()
{
	data.clientData->recv = false;
	auto pack = data.clientData->portocol->pack({ MSGTYPE::GETSUBFOLDER,"" });
	data.clientData->client->write(pack.first, pack.second);

	this->waitForRecv();
}

void Update::FileUpdateClient::init()
{
	setMsgTypeCallback<GetFilePortocol>(MSGTYPE::RSP_GETFILE, false);
	setMsgTypeCallback<GetFileMd5Portocol>(MSGTYPE::RSP_GETFILEMD5, false);
	setMsgTypeCallback<GetSubFolder>(MSGTYPE::RSP_GETSUBFOLDER, false);
	this->data.clientData->client->setConnectStatusCallback([=](uv::TcpClient::ConnectStatus status)
		{
			this->data.clientData->dataMutex.lock();
			if (status == uv::TcpClient::ConnectStatus::OnConnectSuccess) {
				this->data.clientData->connected = true;
			}
			else if (status == uv::TcpClient::ConnectStatus::OnConnnectClose) {
				this->data.clientData->connected = false;
			}
			else if (status == uv::TcpClient::ConnectStatus::OnConnnectFail) {
				this->data.clientData->connected = false;
			}
			this->data.clientData->dataMutex.unlock();
		});

	this->data.clientData->client->setMessageCallback([=](const char* data, ssize_t size)
		{
			auto portocol = this->data.clientData->portocol;
			auto packvec = portocol->unPack(data, size);

			for (auto& pack : *packvec) {
				if (pack.msgtype == MSGTYPE::PACKNOTOVER)continue;
				std::string name("");
				auto rsppack = portocol->solveThePack(pack, this->data, name);
				if (rsppack.msgtype != MSGTYPE::RSP_NULL) {
					auto res = portocol->pack(rsppack);
					this->data.clientData->client->write(res.first, res.second, nullptr);
				}
			}

			delete(packvec);
		});

	this->data.clientData->client->connect(*(this->data.clientData->sockAddr));

	std::thread loopThread([=]() {
		this->data.clientData->loop->run();
		});
	loopThread.detach();

	//wait for connected succeed
	bool Flag = false;
	clock_t start = clock();
	while (!Flag) {
		doWithClientDataMutex([&]() {
			Flag = data.clientData->connected;
			});
		if (clock() - start > 5000)break;
	};
	if (!data.clientData->connected) {
		throw("Server Connected TimeOut!");
	}
}

void Update::FileUpdateClient::doWithClientDataMutex(std::function<void()> fun)
{
	data.clientData->dataMutex.lock();
	fun();
	data.clientData->dataMutex.unlock();
}

void Update::FileUpdateClient::waitForRecv()
{
	bool Flag = false;
	while (!Flag) {
		Sleep(20);
		this->doWithClientDataMutex(
			[&]() {
				Flag = data.clientData->recv;
			});
	}
}

void Update::BasePortocol::doWithClientDataMutex(FileUpdateData data, std::function<void()> fun)
{
	data.clientData->dataMutex.lock();
	fun();
	data.clientData->dataMutex.unlock();
}

MsgPack Update::GetFileMd5Portocol::serverRecv(const std::string& str, FileUpdateData data, const std::string&)
{
	MsgPack rsppack({ MSGTYPE::RSP_GETFILEMD5, "" });
	std::string dataStr;
	for (auto& file : data.serverData->allFile) {
		dataStr += file.first;
		dataStr += "\n" + file.second.first + "\n";
	}
	rsppack.data = dataStr;
	return rsppack;
}

MsgPack Update::GetFileMd5Portocol::clientRecv(const std::string& str, FileUpdateData data, const std::string&)
{
	doWithClientDataMutex(data,
		[&]() {
			auto end = str.size();
			char* ptr = (char*)str.c_str();
			size_t p1 = 0;
			size_t p2 = 0;
			size_t maxsize = str.size();
			while (p1 != maxsize) {
				p2 = str.find('\n', p1 + 1);
				data.clientData->allFile.push_back(std::make_pair(std::string(ptr + p1, ptr + p2), std::string(ptr + p2 + 1, 16)));
				p1 = p2 + 18;
			};
			data.clientData->recv = true;
		});
	return { MSGTYPE::RSP_NULL, "" };
}

MsgPack Update::GetFilePortocol::serverRecv(const std::string& str, FileUpdateData data, const std::string& name)
{
	MsgPack rsppack({ MSGTYPE::RSP_GETFILE, "" });
	rsppack.data = data.serverData->allFile[str].second;
	return rsppack;
}

MsgPack Update::GetFilePortocol::clientRecv(const std::string& str, FileUpdateData data, const std::string&)
{
	doWithClientDataMutex(data,
		[&]() {
			data.clientData->newFile = str;
			data.clientData->recv = true;
		});
	return { MSGTYPE::RSP_NULL, "" };
}

MsgPack Update::GetSubFolder::serverRecv(const std::string&, FileUpdateData data, const std::string&)
{
	MsgPack rsppack({ MSGTYPE::RSP_GETSUBFOLDER, "" });
	std::string dataStr;
	for (auto& file : data.serverData->subFolder) {
		dataStr += file + "\n";
	}
	rsppack.data = dataStr;
	return rsppack;
}

MsgPack Update::GetSubFolder::clientRecv(const std::string& str, FileUpdateData data, const std::string&)
{
	doWithClientDataMutex(data,
		[&]() {
			auto end = str.size();
			char* ptr = (char*)str.c_str();
			size_t p1 = 0;
			size_t p2 = 0;
			size_t maxsize = str.size();
			while (p1 != maxsize) {
				p2 = str.find('\n', p1 + 1);
				data.clientData->subFolder.push_back(str.substr(p1, p2 - p1));
				p1 = p2 + 1;
			};
			data.clientData->recv = true;
		});
	return { MSGTYPE::RSP_NULL, "" };
}

void Update::getAllFile(std::string path, std::vector<std::pair<std::string, __finddata64_t>>& allfile)
{
	__finddata64_t fileinfo;
	intptr_t handle;
	handle = _findfirst64((path + "\\*").c_str(), &fileinfo);

	if (handle == -1) {
		return;
	}
	do {
		if (0 == strcmp(fileinfo.name, ".") || 0 == strcmp(fileinfo.name, ".."))
			continue;
		if (fileinfo.attrib >= _A_SUBDIR && fileinfo.attrib < _A_ARCH) {
			std::cout << "进入目录" << fileinfo.name << std::endl;
			getAllFile(path + "\\" + fileinfo.name, allfile);
		}
		allfile.push_back(std::make_pair(path + "\\" + fileinfo.name, fileinfo));
		std::cout << path + "\\" + fileinfo.name << "已找到" << std::endl;

	} while (!_findnext64(handle, &fileinfo));

	_findclose(handle);
}

std::string Update::calculateMd5(std::string& data) {
#define MD5LENTH 16
	unsigned char MD5result[MD5LENTH] = { 0 };
	MD5_CTX md5_ctx;
	MD5_Init(&md5_ctx);
	MD5_Update(&md5_ctx, data.c_str(), data.size());
	MD5_Final(MD5result, &md5_ctx);
	return std::string((const char*)MD5result, 16);
}


