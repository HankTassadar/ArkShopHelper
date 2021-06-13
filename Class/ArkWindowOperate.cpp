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
	//��ⷽ�۴����Ƿ����
	if (!this->init())
		return;

	//��װ�����̵���Ϣ
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

	//���۴��ڷ���
	HWND hwnd = this->_arkWindow;
	SwitchToThisWindow(hwnd, 0);
	PressKey(hwnd, VK_RETURN);
	SetFocus(hwnd);
	Sleep(100);
	this->paste(hwnd);
	Sleep(100);
	PressKey(hwnd, VK_RETURN);

	//��շ����̵���Ϣ
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
	//��ȡ��������С��������ռ䣬��������С���ַ�����  
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), NULL, 0);
	TCHAR* buffer = new TCHAR[len + 1];
	//���ֽڱ���ת���ɿ��ֽڱ���  
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), buffer, len);
	buffer[len] = '\0';             //����ַ�����β  
	//ɾ��������������ֵ  
	result.append(buffer);
	delete[] buffer;
	return result;
}

int ArkWindow::writeInClipboard() {
	HGLOBAL hMemory;
	LPTSTR lpMemory;
	char* content = (char*)this->_pasteBoard.c_str();   // ��д������
	int contentSize = this->_pasteBoard.size() + 1;

	if (!OpenClipboard(NULL))    // �򿪼��а壬�򿪺����������޷�����
	{
		puts("���а��ʧ��");
		return 1;
	}

	if (!EmptyClipboard())       // ��ռ��а壬д��֮ǰ����������ռ��а�
	{
		puts("��ռ��а�ʧ��");
		CloseClipboard();
		return 1;
	}

	if ((hMemory = GlobalAlloc(GMEM_MOVEABLE, contentSize)) == NULL)    // �Լ��а�����ڴ�
	{
		puts("�ڴ渳ֵ����!!!");
		CloseClipboard();
		return 1;
	}

	if ((lpMemory = (LPTSTR)GlobalLock(hMemory)) == NULL)             // ���ڴ���������
	{
		puts("�����ڴ����!!!");
		CloseClipboard();
		return 1;
	}

	memcpy_s(lpMemory, contentSize, content, contentSize);   // �����ݸ��ƽ����ڴ�����

	GlobalUnlock(hMemory);                   // ����ڴ�����

	if (SetClipboardData(CF_TEXT, hMemory) == NULL)
	{
		puts("���ü��а�����ʧ��!!!");
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


