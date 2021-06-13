#include"ArkHelperUserData.h"
#include<sstream>
ArkHelperUserData* ArkHelperUserData::instance = nullptr;

ArkHelperUserData::ArkHelperUserData()
	:UserAppData("ArkHelper")
{
	this->init();
}

void ArkHelperUserData::init()
{
	
}

ArkHelperUserData* ArkHelperUserData::GetInstance()
{
	if (instance)
		return instance;
	else
		instance = new ArkHelperUserData();
	return instance;
}

ArkHelperUserData::~ArkHelperUserData()
{
}

std::pair<std::string, std::string> ArkHelperUserData::ReadSteamidAndPass()
{
	auto file = this->ReadFile("PlayerData.txt");
	std::string steamid;
	std::string pass;
	if (file.is_open()) {
		std::stringstream ss;
		ss << file.rdbuf();
		file.close();
		ss >> steamid;
		ss >> pass;
	}
	else {
		return { "","" };
	}
	return { steamid,pass };
}

void ArkHelperUserData::WriteSteamidAndPass(std::string steamid, std::string pass)
{
	auto file = this->WriteFile("PlayerData.txt");
	file << steamid << std::endl << pass;
	file.close();
}

std::ifstream ArkHelperUserData::ReadFile(const char* filename)
{
	return std::ifstream(this->GetPath() + "\\" + filename);
}

std::ofstream ArkHelperUserData::WriteFile(const char* filename)
{
	return std::ofstream(this->GetPath() + "\\" + filename);
}