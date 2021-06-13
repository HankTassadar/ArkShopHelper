#ifndef ARKWINDOWOPERATE_H
#define ARKWINDOWOPERATE_H

#include<Windows.h>
#include<string>
#include<memory>

using namespace std;

/*
����ֻ�������۴�����ճ���Ĳ������繺�������
*/

class ArkWindow {

public:
	static shared_ptr<ArkWindow> getInstance();
	~ArkWindow();

private:
	ArkWindow();
public:
	enum  class MsgType
	{
		NO_MSG,
		SHOP_BUY,
		SHOP_KIT,
		SHOP_SELL,
		SETQQ,
		SETPASS,
		KILL
	};

public:
	/**
	*��һ��������Ϣ���ͣ��繺����������۵�ö������
	*�ڶ���������������id�����̵�json�л�ȡ
	*/
	void sendMsgToArk(MsgType msg, string& data, int num = 0);

	/**
	*���ARK�����Ƿ񻹴���
	*/
	bool ifArkWindowExist();

	void setCmdString(string buy, string kit, string sell, string qq, string pass, string kill) {
		this->_shopBuyCmd = buy;
		this->_shopKitCmd = kit;
		this->_shopSellCmd = sell;
		this->_bdqq = qq;
		this->_setpass = pass;
		this->_kill = kill;
	}

	std::string getCmd(MsgType);

private:
	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
	int writeInClipboard();
	void paste(HWND hwnd);
	bool init();


	/**
	*UTF8stringתUTF16��wstring
	*/
	wstring string2wstring(string&);


private:
	static shared_ptr<ArkWindow> _instance;
	string _windowName;
	string _pasteBoard;
	HWND _arkWindow;
	string _shopBuyCmd;
	string _shopKitCmd;
	string _shopSellCmd;
	string _bdqq;
	string _setpass;
	string _kill;
};


#endif // !ARKWINDOWOPERATE_H

