#include<sqlite_modern_cpp.h>

class ServerDatabase {
public:
	ServerDatabase(const std::string& path) 
		:_db(path.empty()
			? "ArkShop.db"
			: path)
	{
		
	}

public: 
	bool isSteamidExist(std::string steam_id) {
		int count = 0;

		try
		{
			_db << "SELECT count(1) FROM Players WHERE SteamId = ?;" << steam_id >> count;
		}
		catch (const sqlite::sqlite_exception& exception)
		{
			std::cout << "查询steamid是否存在错误：" << exception.what() << std::endl;
			return false;
		}

		return count != 0;
	}

	std::string getSteamidPass(std::string steam_id) {
		std::string pass = "";

		try
		{
			_db << "SELECT AppPass FROM Players WHERE SteamId = ?;" << steam_id >> pass;
		}
		catch (const sqlite::sqlite_exception& exception)
		{
			std::cout << "获取steamid的Pass是否存在错误：" << steam_id << exception.what() << std::endl;
		}

		return pass;
	}

	std::string getGroup(std::string steam_id) {
		std::string re = "";

		try
		{
			_db << "SELECT Groups FROM Players WHERE SteamId = ?;" << steam_id >> re;
		}
		catch (const sqlite::sqlite_exception& exception)
		{
			std::cout << "获取steamid的Groups是否存在错误：" << steam_id << exception.what() << std::endl;
		}

		return re;
	}

	int getPlayerPoint(std::string steam_id) {
		int point = 0;

		try
		{
			_db << "SELECT Points FROM Players WHERE SteamId = ?;" << steam_id >> point;
		}
		catch (const sqlite::sqlite_exception& exception)
		{
			std::cout << "获取steamid的Points是否存在错误：" << steam_id << exception.what() << std::endl;
		}

		return point;
	}

	int getPlayerOnlineTime(std::string steam_id) {
		int onlinetime = 0;

		try
		{
			_db << "SELECT OnlineTime FROM Players WHERE SteamId = ?;" << steam_id >> onlinetime;
		}
		catch (const sqlite::sqlite_exception& exception)
		{
			std::cout << "获取steamid的OnlineTime是否存在错误：" << steam_id << exception.what() << std::endl;
		}
		
		return onlinetime;
	}

	std::string getPlayerKit(std::string steam_id) {
		std::string re = "";

		try
		{
			_db << "SELECT Kits FROM Players WHERE SteamId = ?;" << steam_id >> re;
		}
		catch (const sqlite::sqlite_exception& exception)
		{
			std::cout << "获取steamid的Groups是否存在错误：" << steam_id << exception.what() << std::endl;
		}

		return re;
	}
private:
	sqlite::database _db;
};