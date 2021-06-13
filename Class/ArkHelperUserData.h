#pragma once
#include"UserAppData.h"
#include<fstream>
class ArkHelperUserData:public UserAppData
{
public:
	static ArkHelperUserData* GetInstance();
	~ArkHelperUserData();
	
	std::pair<std::string, std::string> ReadSteamidAndPass();

	void WriteSteamidAndPass(std::string steamid, std::string pass);

private:
	ArkHelperUserData();
	virtual void init() override;

	std::ifstream ReadFile(const char* filename);

	std::ofstream WriteFile(const char* filename);
private:
	static ArkHelperUserData* instance;
};

