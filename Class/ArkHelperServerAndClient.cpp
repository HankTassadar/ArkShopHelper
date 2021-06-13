#include"ArkHelperServerAndClient.h"
#include<fstream>
#include<openssl/md5.h>
using namespace ArkHelper;

std::map<MSGTYPE, std::function<ArkPack(const std::string&, ArkHelperData,const std::string&)>> ArkHelperPortocal::_callback;

ArkHelperPortocal::ArkHelperPortocal()
	:_packOver(true)
	, _packDataPtr(nullptr)
	, _lastPackLength(0)
{
	this->_callback[MSGTYPE::PACKNOTOVER] = [](const std::string&, ArkHelperData,const std::string&)->ArkPack {
		return { MSGTYPE::RSP_NULL,"" };
	};
	this->_callback[MSGTYPE::KEEPALIVE] = [](const std::string&, ArkHelperData, const std::string&)->ArkPack {
		return { MSGTYPE::RSP_NULL,"" };
	};
}

ArkHelper::ArkHelperPortocal::~ArkHelperPortocal()
{
	if (this->_packDataPtr != nullptr) {
		delete[]this->_packDataPtr;
		this->_packDataPtr = nullptr;
	}
}

std::pair<const char*, size_t> ArkHelperPortocal::pack(ArkPack pack) {
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

std::vector<ArkPack>* ArkHelperPortocal::unPack(const char* data, size_t size) {

	auto packvec = new std::vector<ArkPack>();
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
				ArkPack repack;
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

ArkPack ArkHelperPortocal::solveThePack(ArkPack& pack, ArkHelperData data, const std::string& name) {
	return this->_callback[pack.msgtype](pack.data, data, name);
}

void ArkHelperPortocal::setMsgTypeCallBack(MSGTYPE msgtype, std::function < ArkPack(const std::string&, ArkHelperData,const std::string&) > func) {
	ArkHelperPortocal::_callback[msgtype] = func;
}

uint32_t ArkHelperPortocal::byte32ToInt(const char* buffer)
{
	return static_cast<size_t>(
		static_cast<unsigned char>(buffer[0]) |
		static_cast<unsigned char>(buffer[1]) << 8 |
		static_cast<unsigned char>(buffer[2]) << 16 |
		static_cast<unsigned char>(buffer[3]) << 24);
}

MSGTYPE ArkHelperPortocal::byte32ToMsgType(const char* buffer) {
	return static_cast<MSGTYPE>(
		static_cast<unsigned char>(buffer[0]) |
		static_cast<unsigned char>(buffer[1]) << 8 |
		static_cast<unsigned char>(buffer[2]) << 16 |
		static_cast<unsigned char>(buffer[3]) << 24);
}

/*
switch (this->type)
{
case ArkHelper::ArkHelperType::SERVER: {

}
									 break;
case ArkHelper::ArkHelperType::CLINET: {

}
									 break;
default:
	break;
}
*/

ArkHelperServerAndClient* ArkHelperServerAndClient::instance = nullptr;

ArkHelperServerAndClient* ArkHelperServerAndClient::GetInstance(u_short Port, std::string IP)
{
	if (instance != nullptr)return instance;

	if (Port == 0) {
		instance = new ArkHelperServerAndClient();	//服务端
	}
	else {
		instance = new ArkHelperServerAndClient(IP, Port);	//客户端
	}

	instance->init();


	switch (instance->type)
	{
	case ArkHelper::ArkHelperType::SERVER: {
		instance->run();
	}
										 break;
	case ArkHelper::ArkHelperType::CLINET: {
		instance->run(true);
	}
										 break;
	default:
		break;
	}
	return instance;
}

ArkHelperServerAndClient::~ArkHelperServerAndClient()
{
	switch (this->type)
	{
	case ArkHelper::ArkHelperType::SERVER: {
		delete(data.serverData);
	}
										 break;
	case ArkHelper::ArkHelperType::CLINET: {
		delete(data.clientData);
	}
										 break;
	default:
		break;
	}
}

bool ArkHelper::ArkHelperServerAndClient::waitForConnected()
{
	bool re = false;
	bool flag = false;
	clock_t start, end;
	start = clock();
	while (true) {
		if (clock() - start > 5000)break;
		data.clientData->dataMutex.lock();
		re = data.clientData->connected;
		data.clientData->dataMutex.unlock();
		if (re)break;
	}
	return re;
}

void ArkHelper::ArkHelperServerAndClient::getPlayerData()
{
	doWithDataMutex([=]() {
		this->data.clientData->playerData.first = false;
		});
	ArkPack datapack({ MSGTYPE::GETPLAYERDATA, "" });
	auto send = data.clientData->portocol->pack(datapack);
	data.clientData->client->write(send.first, send.second);
}

void ArkHelper::ArkHelperServerAndClient::getShopData()
{
	doWithDataMutex([=]() {
		this->data.clientData->shopData.first = false;
		});
	ArkPack datapack({ MSGTYPE::GETSHOPDATA, "" });
	auto send = data.clientData->portocol->pack(datapack);
	data.clientData->client->write(send.first, send.second);
}

void ArkHelper::ArkHelperServerAndClient::getPlayerKitData()
{
	doWithDataMutex([=]() {
		this->data.clientData->kitData.first = false;
		});
	ArkPack datapack({ MSGTYPE::GETKITDATA, "" });
	auto send = data.clientData->portocol->pack(datapack);
	data.clientData->client->write(send.first, send.second);
}

void ArkHelper::ArkHelperServerAndClient::keepALive()
{
	auto pack = data.clientData->portocol->pack({ MSGTYPE::KEEPALIVE,"" });
	data.clientData->client->write(pack.first, pack.second);
}

BoolPair<std::string> ArkHelper::ArkHelperServerAndClient::login(std::string steam_id, std::string pass)
{
	doWithDataMutex([=]() {
		this->data.clientData->login.first = false;
		});
	std::string datastr = steam_id + "\n" + pass;
	data.clientData->playerData.second.steamid = steam_id;
	auto loginpack = data.clientData->portocol->pack({ MSGTYPE::LOGIN, datastr });
	data.clientData->client->write(loginpack.first, loginpack.second);
	int count = 0;
	bool ifloginResp = false;
	while (true) {
		count++;
		Sleep(20);
		data.clientData->dataMutex.lock();
		ifloginResp = data.clientData->login.first;
		data.clientData->dataMutex.unlock();
		if (ifloginResp)break;
		if (count == 5 * 50)break;
	}
	if (count == 250)return{ false,"TimeOut" };
	return { data.clientData->logined,data.clientData->login.second };
}

PlayerData& ArkHelper::ArkHelperServerAndClient::showPlayerData()
{
	waitForTrue(data.clientData->playerData);
	return data.clientData->playerData.second;
}

json& ArkHelper::ArkHelperServerAndClient::showShopData()
{
	waitForTrue(data.clientData->shopData);
	return data.clientData->shopData.second;
}

json& ArkHelper::ArkHelperServerAndClient::showKitData()
{
	waitForTrue(data.clientData->kitData);
	return data.clientData->kitData.second;
}

void ArkHelper::ArkHelperServerAndClient::doWithDataMutex(std::function<void()> fun)
{
	data.clientData->dataMutex.lock();
	fun();
	data.clientData->dataMutex.unlock();
}

//服务端构造
ArkHelperServerAndClient::ArkHelperServerAndClient()
	:type(ArkHelperType::SERVER)
	,timerCount(0)
	,data({})
{
	data.serverData = new ArkHelperServer();
	//读取服务端配置文件
	std::ifstream file("Config.json");
	file >> data.serverData->serverConfig;
	file.close();
	file.clear();
	auto dbpath = data.serverData->serverConfig["ArkShopDbPath"].get<std::string>();		//数据库路径
	auto shoppath = data.serverData->serverConfig["ShopConfigPath"].get<std::string>();	//商店数据路径	
	data.serverData->registerInterval = data.serverData->serverConfig["RegisterInterval"].get<int>();	//服务注册间隔
	data.serverData->serviceRegister = new ServiceRegister({
		data.serverData->serverConfig["ServiceRegisterServer"]["IP"].get<std::string>()	//服务注册服务端IP地址
		,data.serverData->serverConfig["ServiceRegisterServer"]["Port"].get<unsigned short>()	//服务注册服务端端口号
		,uv::SocketAddr::Ipv4
		}
	, 10005);
	data.serverData->serverPort = data.serverData->serverConfig["ServerPort"].get<unsigned short>();	//服务端使用的地址

	//构造数据库
	data.serverData->dataBase = new ServerDatabase(dbpath);

	//读取方舟商店文件
	file.open(shoppath);
	file >> data.serverData->shopData;
	file.close();
	file.clear();

	//构造服务器
	data.serverData->sockaddr = new uv::SocketAddr({ "0.0.0.0",data.serverData->serverPort,uv::SocketAddr::Ipv4 });
	data.serverData->loop = uv::EventLoop::DefaultLoop();
	data.serverData->server = new uv::TcpServer(data.serverData->loop);
	data.serverData->server->setTimeout(30);

	data.serverData->fileUpdateServer = new Update::FileUpdateServer(data.serverData->loop);
}

//客户端构造
ArkHelperServerAndClient::ArkHelperServerAndClient(std::string IP, u_short Port)
	: type(ArkHelperType::CLINET)
	, timerCount(0)
	,data({})
{
	data.clientData = new ArkHelperClient();
	data.clientData->loop = new uv::EventLoop();
	data.clientData->portocol = new ArkHelperPortocal();
	data.clientData->sockaddr = new uv::SocketAddr({ IP,Port,uv::SocketAddr::Ipv4 });
	data.clientData->client = std::make_shared<uv::TcpClient>(data.clientData->loop);
}

void ArkHelper::ArkHelperServerAndClient::init()
{
	this->setMsgTypeCallback();
	                       
	switch (this->type)
	{
	case ArkHelper::ArkHelperType::SERVER: {
		data.serverData->server->SetBufferMode(uv::GlobalConfig::BufferMode::NoBuffer);

		data.serverData->server->setNewConnectCallback(std::bind(&ArkHelperServerAndClient::serverNewConnect, this, std::placeholders::_1));

		data.serverData->server->setConnectCloseCallback(std::bind(&ArkHelperServerAndClient::serverCloseConnect, this, std::placeholders::_1));

		data.serverData->server->setMessageCallback([=](uv::TcpConnectionPtr ptr, const char* data, ssize_t size)
			{
				auto portocol = this->data.serverData->connections[ptr.get()->Name()].portocol;
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
		
		data.serverData->server->bindAndListen(*data.serverData->sockaddr);
		data.serverData->timer = new uv::Timer(data.serverData->loop, 1000, 1000
			, std::bind(&ArkHelperServerAndClient::TimerCallback, this, std::placeholders::_1));
		data.serverData->timer->start();
	}
		break;
	case ArkHelper::ArkHelperType::CLINET: {
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
				std::cout << "clien recv" << std::endl;
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

		this->data.clientData->client->connect(*(this->data.clientData->sockaddr));

		data.clientData->timer = new uv::Timer(data.clientData->loop, 1000, 1000
			, std::bind(&ArkHelperServerAndClient::TimerCallback, this, std::placeholders::_1));
		data.clientData->timer->start();
	}
		break;
	default:
		break;
	}
	
}

void ArkHelper::ArkHelperServerAndClient::run(bool onNewThread)
{
	uv::EventLoop* loop;

	switch (this->type)
	{
	case ArkHelper::ArkHelperType::SERVER: {
		loop = data.serverData->loop;
	}
										 break;
	case ArkHelper::ArkHelperType::CLINET: {
		loop = data.clientData->loop;
	}
										 break;
	default:
		break;
	}


	if (onNewThread) {
		data.clientData->loopThread = new std::thread([=]() {loop->run();});
		data.clientData->loopThread->detach();
	}
	else {
		loop->run();
	}
}

void ArkHelperServerAndClient::setMsgTypeCallback()
{
	switch (this->type)
	{
	case ArkHelper::ArkHelperType::SERVER: {
		this->setCallback<LoginPortocol>(MSGTYPE::LOGIN);
		this->setCallback<GetShopDataPortocol>(MSGTYPE::GETSHOPDATA);
		this->setCallback<GetPlayerDataPortocol>(MSGTYPE::GETPLAYERDATA);
		this->setCallback<GetKitDataPortocol>(MSGTYPE::GETKITDATA);
	}
										 break;
	case ArkHelper::ArkHelperType::CLINET: {
		this->setCallback<LoginPortocol>(MSGTYPE::RSP_LOGIN);
		this->setCallback<GetShopDataPortocol>(MSGTYPE::RSP_GETSHOPDATA);
		this->setCallback<GetPlayerDataPortocol>(MSGTYPE::RSP_GETPLAYERDATA);
		this->setCallback<GetKitDataPortocol>(MSGTYPE::RSP_GETKITDATA);
	}
										 break;
	default:
		break;
	}
}

void ArkHelper::ArkHelperServerAndClient::TimerCallback(uv::Timer* timer)
{
	switch (this->type)
	{
	case ArkHelper::ArkHelperType::SERVER: {
		if (this->timerCount % data.serverData->registerInterval == 0) {
#ifdef NDEBUG
			data.serverData->serviceRegister->regist("ArkHelperService", data.serverData->serverPort);
#endif // NDEBUG
		}
	}
										 break;
	case ArkHelper::ArkHelperType::CLINET: {
		if (this->timerCount % 10 == 0) {
			this->keepALive();
		}
	}
										 break;
	default:
		break;
	}

	if (++this->timerCount == 3601)
		this->timerCount = 0;
}

void ArkHelper::ArkHelperServerAndClient::serverNewConnect(std::weak_ptr<TcpConnection> newconnection)
{
	data.serverData->connections[newconnection.lock().get()->Name()];
	std::cout << "新链接名称：" << newconnection.lock().get() << std::endl;
	std::cout << "当前链接数量：" << data.serverData->connections.size() << std::endl;
}

void ArkHelper::ArkHelperServerAndClient::serverCloseConnect(std::weak_ptr<TcpConnection> closeconnection)
{
	data.serverData->connections.erase(closeconnection.lock().get()->Name());
	std::cout << "当前链接数量：" << data.serverData->connections.size() << std::endl;
}

void ArkHelper::BasePortocol::doWithClientDataMutex(ArkHelperData data, std::function<void()> fun)
{
	data.clientData->dataMutex.lock();
	fun();
	data.clientData->dataMutex.unlock();
}

ArkPack ArkHelper::LoginPortocol::ClientRecv(const std::string& str, ArkHelperData data, const std::string& name)
{
	doWithClientDataMutex(data,
		[=]() {
			data.clientData->login.first = true;

			if (str == "succeed") {
				data.clientData->logined = true;
			}
			else {
				data.clientData->login.second = str;
			}
		});
	return { MSGTYPE::RSP_NULL, "" };
}

ArkPack ArkHelper::LoginPortocol::ServerRecv(const std::string& str, ArkHelperData data, const std::string& name)
{
	ArkPack rsppack{ MSGTYPE::RSP_LOGIN, "" };

	std::cout << "处理包-login：" << str << std::endl;
	auto p1 = str.find('\n');
	std::string steam_id = str.substr(0, p1);
	std::string pass = str.substr(p1 + 1, str.size());
	if (data.serverData->dataBase->isSteamidExist(steam_id)) {
		auto repass = data.serverData->dataBase->getSteamidPass(steam_id);
		if (pass == repass) {
			rsppack.data = "succeed";
			data.serverData->connections[name].logined = true;
			data.serverData->connections[name].steamid = steam_id;
		}
		else if (repass == "") {
			rsppack.data = "no pass";
		}
		else {
			rsppack.data = "Login Pass Error";
		}
	}
	else {
		rsppack.data = "SteamId Not Exist!";
	}

	return rsppack;
}

ArkPack ArkHelper::GetShopDataPortocol::ClientRecv(const std::string& str, ArkHelperData data, const std::string& name)
{
	doWithClientDataMutex(data,
		[=]() {
			std::stringstream ss;
			ss << str;
			data.clientData->shopData.first = true;
			try
			{
				ss >> data.clientData->shopData.second;
			}
			catch (const std::exception& e)
			{
				printf_s("%s", e.what());
			}
		});
	return { MSGTYPE::RSP_NULL, "" };
}

ArkPack ArkHelper::GetShopDataPortocol::ServerRecv(const std::string& str, ArkHelperData data, const std::string& name)
{
	std::cout << "处理包-GetShopData：" << str << std::endl;
	std::stringstream ss;
	ss << data.serverData->shopData;
	std::string datastring;
	datastring = ss.str();
	std::cout << "Send ShopData Finished!" << std::endl;
	return { MSGTYPE::RSP_GETSHOPDATA,datastring };
}

ArkPack ArkHelper::GetPlayerDataPortocol::ClientRecv(const std::string& str, ArkHelperData data, const std::string& name)
{
	doWithClientDataMutex(data,
		[=]() {
			if (str != "Not Login") {
				auto p1 = str.find('\n');
				auto p2 = str.size();
				std::string group = str.substr(0, p1);
				std::string points = str.substr(p1 + 1, p2);
				data.clientData->playerData.second.group = group;
				data.clientData->playerData.second.points = std::stoi(points);
			}
			data.clientData->playerData.first = true;
		});
	return { MSGTYPE::RSP_NULL, "" };
}

ArkPack ArkHelper::GetPlayerDataPortocol::ServerRecv(const std::string& str, ArkHelperData data, const std::string& name)
{
	ArkPack rsppack{ MSGTYPE::RSP_GETPLAYERDATA, "" };
	if (!data.serverData->connections[name].logined) {
		rsppack.data = "Not Login";
		return rsppack;
	}

	auto steamid = data.serverData->connections[name].steamid;
	std::cout << "处理包-GetPlayerData:" << name << std::endl;
	auto groups = data.serverData->dataBase->getGroup(steamid);
	auto points = data.serverData->dataBase->getPlayerPoint(steamid);
	rsppack.data = groups + "\n" + std::to_string(points);
	return rsppack;
}

ArkPack ArkHelper::GetKitDataPortocol::ClientRecv(const std::string& str, ArkHelperData data, const std::string& name)
{
	doWithClientDataMutex(data,
		[=]() {
			std::stringstream ss;
			ss << str;
			data.clientData->kitData.first = true;
			data.clientData->kitData.second.clear();
			ss >> data.clientData->kitData.second;
		});
	return { MSGTYPE::RSP_NULL, "" };
}

ArkPack ArkHelper::GetKitDataPortocol::ServerRecv(const std::string& str, ArkHelperData data, const std::string& name)
{
	std::cout << "处理包-GetPlayerData:" << name << std::endl;
	ArkPack rsppack{ MSGTYPE::RSP_GETKITDATA, "" };
	if (!data.serverData->connections[name].logined) {
		rsppack.data = "{}";
		return rsppack;
	}

	auto steamid = data.serverData->connections[name].steamid;
	auto kits = data.serverData->dataBase->getPlayerKit(steamid);
	rsppack.data = kits;
	return rsppack;
}
