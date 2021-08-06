#include "ArkPlayer.h"
#include "ServerDatabase.hpp"


//testo

ArkServer::ArkPlayer::ArkPlayer(ServerDatabase* pDataBase)
	: _Points(0)
	, _LoginState(false)
	, _DataBase(pDataBase)
{
}

LoginReturnEnm ArkServer::ArkPlayer::Login(std::string& steamId, std::string& passWord)
{
	if (_LoginState) {//已经登录过无需重复登录
		return  LoginReturnEnm::IS_LOGINED;
	}

	auto isExist = _DataBase->isSteamidExist(steamId);
	if (!isExist) {//数据库中不存在SteamId
		return LoginReturnEnm::NO_STEAMID;
	}

	auto pass = _DataBase->getSteamidPass(steamId);
	if ("" == pass) {//玩家还未设定密码
		return LoginReturnEnm::NO_PASSWORD;
	}

	if (pass != passWord) {//密码错误
		return LoginReturnEnm::PASSWORD_ERR;
	}

	//登录成功
	_SteamID = steamId;
	_PassWord = passWord;
	_VipLevel = _DataBase->getGroup(_SteamID);
	_Points = _DataBase->getPlayerPoint(_SteamID);

	return LoginReturnEnm::LOGIN_SUCCESS;
}

bool ArkClient::ArkPlayer::UpdatePlayerData()
{
	return false;
}
