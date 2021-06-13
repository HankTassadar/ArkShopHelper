#ifndef ARKWINDOWOPERATE_H
#define ARKWINDOWOPERATE_H

#include<Windows.h>
#include<string>
#include<memory>

using namespace std;

/*
此类只处理向方舟窗口有粘贴的操作，如购买，礼包等
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
	*第一参数是消息类型，如购买，礼包，出售等枚举类型
	*第二参数是礼包或购买的id，从商店json中获取
	*/
	void sendMsgToArk(MsgType msg, string& data, int num = 0);

	/**
	*检测ARK窗口是否还存在
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
	*UTF8string转UTF16的wstring
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

