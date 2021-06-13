#include "ArkWindowOperate.h"


shared_ptr<ArkWindow> ArkWindow::_instance = nullptr;

ArkWindow::ArkWindow()
	:_windowName("ARK: Survival Evolved")
	,_arkWindow(0)
{
}

shared_ptr<ArkWindow> ArkWindow::getInstance()
{
	if (!_instance) {
		_instance = (shared_ptr<ArkWindow>)(new ArkWindow());
		if (_instance)
			return _instance;
	}
	return _instance;
}

ArkWindow::~ArkWindow()
{
}

inline void PressKey(HWND hwnd, WPARAM param) {
	SendNotifyMessage(hwnd, WM_KEYDOWN, param, 0);
	Sleep(50);
	SendNotifyMessage(hwnd, WM_KEYUP, param, 0);
}

void ArkWindow::sendMsgToArk(MsgType msg, string& data, int num)
{
	//检测方舟窗口是否存在
	if (!this->init())
		return;

	//组装方舟商店消息
	string cmd = "";

	switch (msg)
	{
	case MsgType::NO_MSG:
		break;
	case MsgType::SHOP_BUY:
		cmd = this->_shopBuyCmd;
		break;
	case MsgType::SHOP_KIT:
		cmd = this->_shopKitCmd;
		break;
	case MsgType::SHOP_SELL:
		cmd = this->_shopSellCmd;
		break;
	case ArkWindow::MsgType::SETQQ:
		cmd = this->_bdqq;
		break;
	case ArkWindow::MsgType::SETPASS:
		cmd = this->_setpass;
		break;
	case ArkWindow::MsgType::KILL:
		cmd = this->_kill;
		break;
	default:
		break;
	}
	if(data != "")
		this->_pasteBoard = cmd + " " + data;
	if (num != 0)
		this->_pasteBoard += (" " + to_string(num));

	//向方舟窗口发送
	HWND hwnd = this->_arkWindow;
	SwitchToThisWindow(hwnd, 0);
	PressKey(hwnd, VK_RETURN);
	SetFocus(hwnd);
	Sleep(100);
	this->paste(hwnd);
	Sleep(100);
	PressKey(hwnd, VK_RETURN);

	//清空方舟商店消息
	this->_pasteBoard.empty();
}

bool ArkWindow::ifArkWindowExist()
{
	this->init();
	LPSTR  a=(LPSTR)new char[50];
	memset(a, '\0', 50);
	GetWindowTextA(this->_arkWindow, a, 50);
	if ((const char*)a==this->_windowName)
		return true;
	return false;
}

std::string ArkWindow::getCmd(MsgType type)
{
	std::string re;
	switch (type)
	{
	case ArkWindow::MsgType::NO_MSG:
		re = "";
		break;
	case ArkWindow::MsgType::SHOP_BUY:
		re = this->_shopBuyCmd;
		break;
	case ArkWindow::MsgType::SHOP_KIT:
		re = this->_shopKitCmd;
		break;
	case ArkWindow::MsgType::SHOP_SELL:
		re = this->_shopSellCmd;
		break;
	case ArkWindow::MsgType::SETQQ:
		re = this->_bdqq;
		break;
	case ArkWindow::MsgType::SETPASS:
		re = this->_setpass;
		break;
	case ArkWindow::MsgType::KILL:
		re = this->_kill;
		break;
	default:
		break;
	}
	return re;
}

BOOL ArkWindow::EnumWindowsProc(HWND hwnd, LPARAM lParam){
	ArkWindow* win = (ArkWindow*)lParam;
	LPSTR a = (LPSTR)new char[100];
	memset(a, 0, sizeof(a));
	GetWindowTextA(hwnd, a, 100);
	if (strstr(a, win->_windowName.c_str()) != NULL) {
		win->_arkWindow = hwnd;
	}
	delete[](a);
	return TRUE;
}

wstring ArkWindow::string2wstring(string &str)
{
	wstring result;
	//获取缓冲区大小，并申请空间，缓冲区大小按字符计算  
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), NULL, 0);
	TCHAR* buffer = new TCHAR[len + 1];
	//多字节编码转换成宽字节编码  
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), buffer, len);
	buffer[len] = '\0';             //添加字符串结尾  
	//删除缓冲区并返回值  
	result.append(buffer);
	delete[] buffer;
	return result;
}

int ArkWindow::writeInClipboard() {
	HGLOBAL hMemory;
	LPTSTR lpMemory;
	char* content = (char*)this->_pasteBoard.c_str();   // 待写入数据
	int contentSize = this->_pasteBoard.size() + 1;

	if (!OpenClipboard(NULL))    // 打开剪切板，打开后，其他进程无法访问
	{
		puts("剪切板打开失败");
		return 1;
	}

	if (!EmptyClipboard())       // 清空剪切板，写入之前，必须先清空剪切板
	{
		puts("清空剪切板失败");
		CloseClipboard();
		return 1;
	}

	if ((hMemory = GlobalAlloc(GMEM_MOVEABLE, contentSize)) == NULL)    // 对剪切板分配内存
	{
		puts("内存赋值错误!!!");
		CloseClipboard();
		return 1;
	}

	if ((lpMemory = (LPTSTR)GlobalLock(hMemory)) == NULL)             // 将内存区域锁定
	{
		puts("锁定内存错误!!!");
		CloseClipboard();
		return 1;
	}

	memcpy_s(lpMemory, contentSize, content, contentSize);   // 将数据复制进入内存区域

	GlobalUnlock(hMemory);                   // 解除内存锁定

	if (SetClipboardData(CF_TEXT, hMemory) == NULL)
	{
		puts("设置剪切板数据失败!!!");
		CloseClipboard();
		return 1;
	}
	return 0;
}

void ArkWindow::paste(HWND hwnd)
{
	wstring str = this->string2wstring(this->_pasteBoard);
	for (size_t i = 0; i <str.length(); i++) {
		Sleep(50);
		WPARAM c = str[i];
		//char* q = (char*)&a;
		SendMessage(this->_arkWindow, WM_IME_CHAR, c, 0);
	}
	this->_pasteBoard = "";
}

bool ArkWindow::init()
{
	this->_arkWindow = 0;
	if (!this->_arkWindow)
		this->_arkWindow = FindWindowA(NULL, this->_windowName.c_str());
	if (!this->_arkWindow)
		return false;
	return true;
}


