#include"ArkHelperFileUpdate.h"
#include<io.h>
#include<direct.h>

ArkHelperFileUpdate::ArkHelperFileUpdate(std::string path)
	:fileRootPath(path)
	, updateClient(new Update::FileUpdateClient("ArkHelperUpdate"))
	, updateFinished(false)
	//, updateClient(new Update::FileUpdateClient("127.0.0.1", 26313))
{
	//Update::getAllFile(this->fileRootPath, this->allFile);
}

bool ArkHelperFileUpdate::checkUpdate() {
	_mkdir((this->fileRootPath + "ArkHelper").c_str());
	updateClient->getSubFolder();
	auto subFolder = updateClient->showSubFolder();
	for (auto& i : subFolder) {
		_mkdir((this->fileRootPath + i).c_str());
	}
	updateClient->getFileMd5();
	auto data = updateClient->showFileAndMd5();
	for (auto& i : data) {
		auto isSame = this->checkIfSame(i);
		if (!isSame)this->needUpdate.push_back(i.first);
	}
	if (this->needUpdate.size() == 0)
		return false;
	return true;
}

void ArkHelperFileUpdate::startUpdateAll()
{
	std::thread t([=]() {
		this->updateAll();
		});
	t.detach();
}

bool ArkHelperFileUpdate::updateIsFinished()
{
	bool re = false;
	this->dataMutex.lock();
	re = this->updateFinished;
	this->dataMutex.unlock();
	return re;
}

std::string ArkHelperFileUpdate::getStatestring()
{
	this->dataMutex.lock();
	auto re = this->wStringState;
	this->dataMutex.unlock();
	return re;
}

void ArkHelperFileUpdate::updateAll()
{
	int num1 = 0;
	int num2 = this->needUpdate.size();
	for (auto& i : this->needUpdate) {
		num1++;
		this->setStringState("正在更新 " + std::to_string(num1) + "/" + std::to_string(num2));
		auto filedata = updateClient->getFile(i);
		std::ofstream file(this->fileRootPath + i, std::ios::binary);
		file << filedata;
		file.close();
	}
	this->dataMutex.lock();
	this->updateFinished = true;
	this->dataMutex.unlock();
}

bool ArkHelperFileUpdate::checkIfSame(std::pair<std::string, std::string>& data)
{
	std::ifstream file(this->fileRootPath + data.first, std::ios::binary);
	if (!file.is_open())return false;
	std::stringstream ss;
	ss << file.rdbuf();
	file.close();
	std::string str = ss.str();
	auto md5 = Update::calculateMd5(str);
	return md5 == data.second;
}

void ArkHelperFileUpdate::setStringState(std::string str)
{
	this->dataMutex.lock();
	this->wStringState = str;
	this->dataMutex.unlock();
}


