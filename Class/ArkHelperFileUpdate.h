#include<fstream>
#include"ServiceQuery.hpp"
#include<openssl/md5.h>
#include<json.hpp>
#include"FileUpdate.h"
using json = nlohmann::json;

class ArkHelperFileUpdate {
public:
	ArkHelperFileUpdate(std::string path);
	bool checkUpdate();

	void startUpdateAll();

	bool updateIsFinished();

	std::string getStatestring();
private:
	void updateAll();
	bool checkIfSame(std::pair<std::string,std::string>& data);

	void setStringState(std::string);
private:
	Update::FileUpdateClient* updateClient;
	std::string fileRootPath;
	std::vector<std::pair<std::string, __finddata64_t>> allFile;
	std::vector<std::string> needUpdate;
	std::mutex dataMutex;
	bool updateFinished;

	std::string wStringState;
};