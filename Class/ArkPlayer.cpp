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
	if (_LoginState) {//�Ѿ���¼�������ظ���¼
		return  LoginReturnEnm::IS_LOGINED;
	}

	auto isExist = _DataBase->isSteamidExist(steamId);
	if (!isExist) {//���ݿ��в�����SteamId
		return LoginReturnEnm::NO_STEAMID;
	}

	auto pass = _DataBase->getSteamidPass(steamId);
	if ("" == pass) {//��һ�δ�趨����
		return LoginReturnEnm::NO_PASSWORD;
	}

	if (pass != passWord) {//�������
		return LoginReturnEnm::PASSWORD_ERR;
	}

	//��¼�ɹ�
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
