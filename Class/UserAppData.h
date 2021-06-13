#pragma once
#include<fstream>

class WinUserData
{
public:
	WinUserData() = delete;
	WinUserData(std::string name);
	~WinUserData();

public:
	/**
	* create a new directory in user appdata directory
	*/
	void makeNewDirectory(const char* name);

	/**
	* get a path
	*/
	std::string GetPath() {
		return this->directory;
	}
private:
	bool init();
private:
	std::string directory;
};

class UserAppData
{
public:
	UserAppData() = delete;
	UserAppData(const char* appName);
	~UserAppData();

private:
	/*
	These below need to override in child class
	 */

	/**
	* create a userdata directory structure in APPDATA path.called in child class's constructure function
	*/
	virtual void init() = 0;

public:
	/**
	* get User AppData Directory
	*/
	std::string GetPath(const char* name = nullptr);
protected:
	WinUserData* winUserData;
};


