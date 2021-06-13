#include"UserAppData.h"
#include<direct.h>
WinUserData::WinUserData(std::string name)
	:directory(name)
{
	this->init();
}

WinUserData::~WinUserData()
{
}

void WinUserData::makeNewDirectory(const char* name)
{
	std::string fullpath = this->directory + "\\" + name;
	auto flag = _mkdir(fullpath.c_str());
}

bool WinUserData::init()
{
	size_t bufferCount = 256;
	char* p = new(std::nothrow) char[bufferCount];
	int flag = 0;
	if (p) {
		_dupenv_s(&p, &bufferCount, "APPDATA");
		this->directory = std::string(p) + "\\" + this->directory;
		flag = _mkdir(this->directory.c_str());
	}
	delete[](p);
	return true;
}

UserAppData::UserAppData(const char* appName)
	:winUserData(new WinUserData(appName))
{
}

UserAppData::~UserAppData()
{
	delete(this->winUserData);
}

std::string UserAppData::GetPath(const char* name)
{
	if (name == nullptr)
		return this->winUserData->GetPath();
	return this->winUserData->GetPath() + "\\" + name;
}
