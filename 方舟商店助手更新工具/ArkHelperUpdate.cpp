// 方舟商店助手更新工具.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "ArkHelperUpdate.h"
#include<ArkHelperFileUpdate.h>
#include<ArkHelperUserData.h>
#include<string>

#define ARKHELPER_VERSION 100

#define MAX_LOADSTRING 100

// 全局变量:A
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void PaintText(HWND hWnd, std::string text);
HWND thisHwnd;



auto userdata = ArkHelperUserData::GetInstance();
std::string path = userdata->GetPath("\\ArkHelper\\ArkHelper.exe");
std::string rootpath = userdata->GetPath("\\");
ArkHelperFileUpdate update(rootpath);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY));

    MSG msg;
    
    PaintText(thisHwnd, "正在检查更新");
    auto needupdate = update.checkUpdate();
    
    clock_t t1, t2;
    t1 = clock();
    if (needupdate) {
        // 主消息循环:
        update.startUpdateAll();
        while (TRUE)
        {
            while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                    break;
                if (msg.message == WM_PAINT)
                    break;
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (update.updateIsFinished())
                break;
            if ((clock() - t1 > 1000)) {
                PaintText(thisHwnd, update.getStatestring()+",请勿关闭");
                t1 = clock();
            }
            if (msg.message == WM_QUIT)
                break;
            Sleep(20);
        }
    }
    if(!needupdate||update.updateIsFinished())
        WinExec(path.c_str(), SW_SHOWDEFAULT);
    //system("start ./ArkHelper/ArkHelper.exe");
    return 0;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MY);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_SYSMENU,
      200, 100, 200, 100, nullptr, nullptr, hInstance, nullptr);
   thisHwnd = hWnd;
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
        //PaintText(hWnd, update.getStatestring());
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

HFONT ChooseMyFont()
{
    auto a = CreateFont(20,8,0,0,FW_REGULAR,FALSE,FALSE,FALSE,GB2312_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,FIXED_PITCH | FF_MODERN,L"隶书");
    return (a);
}

void clearWithWhite(HWND hWnd,HDC hdc) {
    RECT rc;
    GetClientRect(hWnd, &rc); // 实际上就是获取左上和右下的两个坐标

    //auto hpen = CreatePen(PS_SOLID, 10, RGB(255,255,255));	// 创建红色实心10px粗的画笔
    auto hbrush = CreateSolidBrush(RGB(255, 255, 255)); // 创建蓝色画刷
    //auto hbrush = CreateSolidBrush(RGB(0,0,0));
    //SelectObject(hdc, hpen);	// 指定画笔
    SelectObject(hdc, hbrush);	// 指定画刷

    // 绘制矩形
    //Rectangle(hdc, 80, 50, 300, 200);
    FillRect(hdc, &rc, hbrush);

    // 清理资源
// DeleteObject(hpen);
    DeleteObject(hbrush);
}

void PaintText(HWND hWnd, std::string text) {
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rc;

    hdc = BeginPaint(hWnd, &ps);
    
    SetBkMode(hdc, TRANSPARENT);
    //清空窗口为白色
    clearWithWhite(hWnd, hdc);

    // 获取当前窗口的矩形区域
    GetClientRect(hWnd, &rc);

    
    // 获得字体对象句柄
    auto hFontSetting = ChooseMyFont();
    // DC 加载字体
    SelectObject(hdc, hFontSetting);
    //SetTextColor(hdc, RGB(255, 0, 255));
    int k = text.size();
    
    //TextOutA(hdc, 0,0, text.c_str(),text.size());
    DrawTextA(
        hdc,
        text.c_str(),	// 文字内容
        text.size(), // 文字长度
        &rc,		// 输出区域
        DT_SINGLELINE|DT_CENTER|DT_VCENTER	// 排版格式 居中
    );

    //DeleteObject(hFontSetting);
    EndPaint(hWnd, &ps);

    InvalidateRect(thisHwnd, &rc, true);
}
