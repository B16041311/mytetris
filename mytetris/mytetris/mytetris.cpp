//mytetris.cpp: 定义应用程序的入口点。
//

#include "stdafx.h"
#include "mytetris.h"
#include "tchar.h"
#include <windows.h>
#include <mmsystem.h>//包含windows中与多媒体有关的大多数接口
#pragma comment(lib, "WINMM.LIB")//导入winmm库：WINMM.LIB是Windows多媒体相关应用程序接口
#define MAX_LOADSTRING 100 
#define MAX_LOADSTRING 100

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);//窗口注册函数
BOOL                InitInstance(HINSTANCE, int);//在启动时初始化应用程序
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);//窗口过程处理函数
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HMENU diff;//难度菜单句柄的定义
HMENU lay;//布局菜单句柄的定义
HMENU endp;//结束游戏菜单句柄定义

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	init_game();//初始化游戏
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MYTETRIS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYTETRIS));

    MSG msg;

    // 主消息循环: 
	while (1)
	{
		
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))//消息检查线程消息队列
		{
			TranslateMessage(&msg);//将消息的虚拟键转化字符消息
			DispatchMessage(&msg);//将消息传送到指定窗口函数
			if (msg.message == WM_QUIT)//若消息是退出，则终止
			{
				break;
			}
		}
		else
		{
			
			if ((GAME_STATE & 2) != 0)
			{
				tCur = GetTickCount();//返回（retrieve）从操作系统启动所经过（elapsed）的毫秒数
				if (tCur - tPre>g_speed)//超过反应时间时
				{
					int flag = CheckValide(curPosX, curPosY + 1, bCurTeris);//判断下一行的情况，返回给flag
					if (flag == 1)//正常下降一行
					{
						curPosY++;//方块坐标y加1
						tPre = tCur;//重新计算反应时间
						HWND hWnd = GetActiveWindow();
						//这里两次调用实现了双缓冲的作用，改善了视觉效果
						InvalidateRect(hWnd, &rc_left, FALSE);//向指定的窗体更新区域添加一个矩形，然后窗口客户区域的这一部分将被重新绘制
						InvalidateRect(hWnd, &rc_right_top, FALSE);//系统不会像窗口发送WM_PAINT消息，来重绘
					}
					else if (flag == -2)//方块到底时
					{
						g_speed = t_speed;//重置速度
						fillBlock();//将方块填充给矩形
						checkComplite(); //查看能否消去这行
						setRandomT();//随机产生新的方块
						curPosX = (NUM_X - 4) >> 1;//重新设置方块的坐标x
						curPosY = 0;//重新设置方块的坐标y
						HWND hWnd = GetActiveWindow();//获得与调用线程的消息队列相关的活动窗口的窗口句柄
						InvalidateRect(hWnd, &rc_main, FALSE);//刷新矩形区域
					}
					else if (flag == -3)//放不下下一个方块（游戏输了）时
					{
						HWND hWnd = GetActiveWindow();//获得与调用线程的消息队列相关的活动窗口的窗口句柄
						if (MessageBox(hWnd, L"You lose ! Wanna try again?", L"You lose!", MB_YESNO) == IDYES)//原创内容：更改失败提示
						{
							init_game();//重新开始游戏
						}
						else
						{
							break;//退出了消息循环，结束程序
						}
					}
				}
			}
		}
	}


    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYTETRIS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MYTETRIS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

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
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	int nWinx, nWiny;
	HDC hdc;//标识设备环境句柄
	HMENU hSysmenu;//菜单窗口的句柄
	PAINTSTRUCT ps;
	int nClientX, nClientY;
	int posX, posY;
	RECT rect;

    switch (message)
    {
	case WM_CREATE:
		GetWindowRect(hWnd, &rect);
		nWinx = 530;//窗口宽度
		nWiny = 680;//窗口高度
		posX = GetSystemMetrics(SM_CXSCREEN);
		posY = GetSystemMetrics(SM_CYSCREEN);
		posX = (posX - nWinx) >> 1;
		posY = (posY - nWiny) >> 1;
		GetClientRect(hWnd, &rect);
		nClientX = rect.right - rect.left;
		nClientY = rect.bottom - rect.top;
		MoveWindow(hWnd, posX, posY, 530, 680, TRUE); //设置窗口

		hSysmenu = GetMenu(hWnd);//获取菜单句柄
		AppendMenu(hSysmenu, MF_SEPARATOR, 0, NULL);//添加一条水平分割线
		diff = CreatePopupMenu();//创建一个难度菜单句柄
		endp = CreatePopupMenu();
		AppendMenu(hSysmenu, MF_POPUP, (UINT_PTR)diff, L"难度选择");//添加菜单diff
		AppendMenu(hSysmenu, MF_POPUP, (UINT_PTR)diff, L"开始游戏");//添加菜单开始游戏diff
		AppendMenu(hSysmenu, MF_POPUP, (UINT_PTR)endp, L"结束游戏");//添加菜单结束游戏diff
		AppendMenu(endp, MF_STRING, IDM_EXIT, L"结束游戏");//在新添加的菜单下创建菜单项结束游戏
		AppendMenu(diff, MF_STRING, ID_dif1, L"难度1");//在新添加的菜单下创建菜单项难度1
		AppendMenu(diff, MF_STRING, ID_dif2, L"难度2");//在新添加的菜单下创建菜单项难度1
		AppendMenu(diff, MF_STRING, ID_dif3, L"难度3");//在新添加的菜单下创建菜单项难度1
		lay = CreatePopupMenu();//创建一个布局菜单句柄
		AppendMenu(hSysmenu, MF_POPUP, (UINT_PTR)lay, L"布局选择");//添加菜单diff
		AppendMenu(lay, MF_STRING, ID_LAYOUT1, L"布局1");//在新添加的菜单下创建菜单项布局1
		AppendMenu(lay, MF_STRING, ID_LAYOUT2, L"布局2");//在新添加的菜单下创建菜单项布局1
		SetMenu(hWnd, hSysmenu);//设置系统自定义菜单
		SetMenu(hWnd, diff);//设置新添的难度菜单
		SetMenu(hWnd, lay);//设置新添的布局菜单
		break;
		
    case WM_COMMAND:
        {
			LPCTSTR str;
            int wmId = LOWORD(wParam);
            // 分析菜单选择: 
            switch (wmId)
            {
            case IDM_ABOUT:
				MessageBox(NULL, _T("Please Enjoy Mytetris!\nB16041311马睿\n版 本 号 1.09"), _T("关于"), MB_OK);//原创内容：重写关于对话框中的内容
                //DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);//弹出“关于”消息框
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case ID_dif1:
				selectDiffculty(hWnd, 1);//难度1
				MessageBox(NULL, _T("难度1"), _T("当前难度"), MB_OK);
				break;
			case ID_dif2:
				selectDiffculty(hWnd, 2);//难度2
				MessageBox(NULL, _T("难度2"), _T("当前难度"), MB_OK);
				break;
			case ID_dif3:
				selectDiffculty(hWnd, 3);//难度3
				MessageBox(NULL, _T("难度3"), _T("当前难度"), MB_OK);
				break;
			case ID_LAYOUT1:
				selectLayOut(hWnd, 1);//布局1
				MessageBox(NULL, _T("布局1"), _T("当前布局"), MB_OK);
				break;
			case ID_LAYOUT2:
				selectLayOut(hWnd, 2);//布局2
				MessageBox(NULL, _T("布局2"), _T("当前布局"), MB_OK);
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

	case WM_KEYDOWN://键盘消息
		hdc = GetDC(hWnd);
		InvalidateRect(hWnd, NULL, false);
		switch (wParam)
		{
		case VK_LEFT:
			curPosX--;
			if (CheckValide(curPosX, curPosY, bCurTeris) != 1)
			{
				curPosX++;//左移
			}
			break;
		case VK_RIGHT:
			curPosX++;
			if (CheckValide(curPosX, curPosY, bCurTeris) != 1)
			{
				curPosX--;//右移
			}
			break;
		case VK_UP:
			RotateTeris(bCurTeris);//改变形状
			break;
		case VK_DOWN://加速到底
			if (g_speed == t_speed)
				g_speed = 10;
			else
				g_speed = t_speed;
			break;
		case 'W'://同 VK_UP
			RotateTeris(bCurTeris);
			break;
		case 'A'://同 VK_LEFT
			curPosX--;
			if (CheckValide(curPosX, curPosY, bCurTeris) != 1)
			{
				curPosX++;
			}
			break;
		case 'D'://同 VK_RIGHT
			curPosX++;
			if (CheckValide(curPosX, curPosY, bCurTeris) != 1)
			{
				curPosX--;
			}
			break;

		case 'S'://同 VK_DOWN
			if (g_speed == t_speed)
				g_speed = 10;
			else
				g_speed = t_speed;
			break;
		default:
			break;
		}

    case WM_PAINT:
        {
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
			
			DrawBackGround(hdc);
			DrawTeris(hdc);
			drawNext(hdc);
			drawScore(hdc);
            EndPaint(hWnd, &ps);
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

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
		
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
			
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void checkComplite()
{
	int i, j, k, count = 0;
	for (i = 0; i<NUM_Y; i++)
	{
		bool flag = true;
		for (j = 0; j<NUM_X; j++)
		{
			if (!g_hasBlocked[i][j])
			{
				flag = false;//有一个为空时，不能消去
			}
		}
		if (flag)
		{
			count++;
			for (j = i; j >= 1; j--)//从上往下把每行下移
			{
				for (k = 0; k<NUM_X; k++)
				{
					g_hasBlocked[j][k] = g_hasBlocked[j - 1][k];
				}

			}
			drawCompleteParticle(i);//在消去的那行画一行空白做出消去的效果
			Sleep(300);//暂停300ms

			PlaySound(_T("coin.wav"), NULL, SND_FILENAME | SND_ASYNC);//消去一行时播放系统自带提示音
		}
	}
	GAME_SCORE += int(count*1.5);//单行+1分，两行+3分，三行+4分，四行6分，以此类推
}

void drawBlocked(HDC mdc)
{
	int i, j;
	//在应用程序调用CreateSolidBrush创建刷子以后，可以通过调用SelectObject函数把该刷子选入设备环境
	HBRUSH hBrush = (HBRUSH)CreateSolidBrush(RGB(255, 192, 203));//原创内容：下落方块颜色：天依蓝

	SelectObject(mdc, hBrush);//函数选择一对象到指定的设备上下文环境中，新对象替换先前的相同类型的对象

	for (i = 0; i<NUM_Y; i++)
	{
		for (j = 0; j<NUM_X; j++)
		{
			if (g_hasBlocked[i][j])
			{
				Rectangle(mdc, BORDER_X + j * BLOCK_SIZE, BORDER_Y + i * BLOCK_SIZE,
					BORDER_X + (j + 1)*BLOCK_SIZE, BORDER_Y + (i + 1)*BLOCK_SIZE
				);//绘画矩形的函数，并用当前颜色的画刷进行颜色的填充
			}
		}
	}
	DeleteObject(hBrush);
}

void DrawBackGround(HDC hdc)
{
	HBRUSH hBrush = (HBRUSH)GetStockObject(GRAY_BRUSH);
	//适用于支持光栅操作的设备，应用程序可以通过调用GetDeviceCaps函数来确定一个设备是否支持这些操作
	HDC mdc = CreateCompatibleDC(hdc);
	HBITMAP hBitmap = CreateCompatibleBitmap(hdc, SCREEN_X, SCREEN_Y);

	SelectObject(mdc, hBrush);
	SelectObject(mdc, hBitmap);

	HBRUSH hBrush2 = (HBRUSH)GetStockObject(WHITE_BRUSH);
	FillRect(mdc, &rc_main, hBrush2);
	Rectangle(mdc, rc_left.left + BORDER_X, rc_left.top + BORDER_Y, rc_left.right, rc_left.bottom);
	Rectangle(mdc, rc_right.left + BORDER_X, rc_right.top + BORDER_Y, rc_right.right, rc_right.bottom);
	
	::BitBlt(hdc, 0, 0, SCREEN_X, SCREEN_Y, mdc, 0, 0, SRCCOPY);
	DeleteObject(hBrush);
	DeleteDC(mdc);
	DeleteObject(hBitmap);
	DeleteObject(hBrush2);
}


void setRandomT()
{
	int rd_start = RandomInt(0, sizeof(state_teris) / sizeof(state_teris[0]));
	int rd_next = RandomInt(0, sizeof(state_teris) / sizeof(state_teris[0]));

	if (GAME_STATE == 0)
	{
		GAME_STATE = GAME_STATE | 0x0001;

		memcpy(bCurTeris, state_teris[rd_start], sizeof(state_teris[rd_start]));
		memcpy(bNextCurTeris, state_teris[rd_next], sizeof(state_teris[rd_next]));
	}
	else
	{
		memcpy(bCurTeris, bNextCurTeris, sizeof(bNextCurTeris));
		memcpy(bNextCurTeris, state_teris[rd_next], sizeof(state_teris[rd_next]));
	}
}

void init_game()
{
	GAME_SCORE = 0;
	setRandomT();//随机生成一个方块用作下一次掉落
	curPosX = (NUM_X - 4) >> 1;//设置初始坐标
	curPosY = 0;
	memset(g_hasBlocked, 0, sizeof(g_hasBlocked));
	rc_left.left = 0;
	rc_left.right = SCREEN_LEFT_X;
	rc_left.top = 0;
	rc_left.bottom = SCREEN_Y;

	rc_right.left = rc_left.right + BORDER_X;
	rc_right.right = 180 + rc_right.left;
	rc_right.top = 0;
	rc_right.bottom = SCREEN_Y;

	rc_main.left = 0;
	rc_main.right = SCREEN_X;
	rc_main.top = 0;
	rc_main.bottom = SCREEN_Y;

	rc_right_top.left = rc_right.left;
	rc_right_top.top = rc_right.top;
	rc_right_top.right = rc_right.right;
	rc_right_top.bottom = (rc_right.bottom) / 2;

	rc_right_bottom.left = rc_right.left;
	rc_right_bottom.top = rc_right_top.bottom + BORDER_Y;
	rc_right_bottom.right = rc_right.right;
	rc_right_bottom.bottom = rc_right.bottom;

	g_speed = t_speed = 1000 - GAME_DIFF * 280;
}

void fillBlock()
{
	int i, j;
	for (i = 0; i<4; i++)
	{
		for (j = 0; j<4; j++)
		{
			if (bCurTeris[i][j])
			{
				g_hasBlocked[curPosY + i][curPosX + j] = 1;
			}
		}
	}
}

void RotateTeris(BOOL bTeris[4][4])
{
	BOOL bNewTeris[4][4];
	int x, y;
	for (x = 0; x<4; x++)
	{
		for (y = 0; y<4; y++)
		{//旋转角度
			bNewTeris[x][y] = bTeris[3 - y][x];
			//逆时针：
			//bNewTeris[x][y] = bTeris[y][3-x];
		}
	}
	if (CheckValide(curPosX, curPosY, bNewTeris) == 1)
	{
		memcpy(bTeris, bNewTeris, sizeof(bNewTeris));//成功则将变换后的形状保存
	}
}

void DrawTeris(HDC mdc)
{
	int i, j;
	HPEN hPen = (HPEN)GetStockObject(BLACK_PEN);//画笔句柄
	HBRUSH hBrush = (HBRUSH)CreateSolidBrush(RGB(102, 204, 255));//GetStockObject(WHITE_BRUSH);
	SelectObject(mdc, hPen);//electObject是计算机编程语言函数，该函数选择一对象到指定的设备上下文环境中，新对象替换先前的相同类型的对象
	SelectObject(mdc, hBrush);
	for (i = 0; i<4; i++)
	{
		for (j = 0; j<4; j++)
		{
			if (bCurTeris[i][j])
			{
				Rectangle(mdc, (j + curPosX)*BLOCK_SIZE + BORDER_X, (i + curPosY)*BLOCK_SIZE + BORDER_Y, (j + 1 + curPosX)*BLOCK_SIZE + BORDER_X, (i + 1 + curPosY)*BLOCK_SIZE + BORDER_Y);
			}
		}
	}
	drawBlocked(mdc);
	DeleteObject(hPen);
	DeleteObject(hBrush);
}

void drawNext(HDC hdc)
{
	int i, j;

	HBRUSH hBrush = (HBRUSH)CreateSolidBrush(RGB(57, 197, 187));//原创内容 下一个方块颜色：初音绿
	SelectObject(hdc, hBrush);
	for (i = 0; i<4; i++)
	{
		for (j = 0; j<4; j++)
		{
			if (bNextCurTeris[i][j])
			{
				Rectangle(hdc, rc_right_top.left + BLOCK_SIZE * (j + 1), rc_right_top.top + BLOCK_SIZE * (i + 1), rc_right_top.left + BLOCK_SIZE * (j + 2), rc_right_top.top + BLOCK_SIZE * (i + 2));
			}
		}
	}
	HFONT hFont = CreateFont(30, 0, 0, 0, FW_THIN, 0, 0, 0, UNICODE, 0, 0, 0, 0, L"微软雅黑");
	SelectObject(hdc, hFont);
	SetBkMode(hdc, TRANSPARENT);
	SetBkColor(hdc, RGB(57, 197, 187));
	RECT rect;
	rect.left = rc_right_top.left + 40;
	rect.top = rc_right_top.bottom - 150;
	rect.right = rc_right_top.right;
	rect.bottom = rc_right_top.bottom;
	DrawTextW(hdc, TEXT("下一个"), _tcslen(TEXT("下一个")), &rect, 0);
	DeleteObject(hFont);
	DeleteObject(hBrush);
}

void drawScore(HDC hdc)
{
	HFONT hFont = CreateFont(30, 0, 0, 0, FW_THIN, 0, 0, 0, UNICODE, 0, 0, 0, 0, L"微软雅黑");
	SelectObject(hdc, hFont);
	SetBkMode(hdc, TRANSPARENT);
	SetBkColor(hdc, RGB(255, 255, 0));
	RECT rect;
	rect.left = rc_right_bottom.left;
	rect.top = rc_right_bottom.top;
	rect.right = rc_right_bottom.right;
	rect.bottom = rc_right_bottom.bottom;
	TCHAR szBuf[30];
	LPCTSTR cstr = TEXT("难度：%d");
	wsprintf(szBuf, cstr, GAME_DIFF);
	DrawTextW(hdc, szBuf, _tcslen(szBuf), &rect, DT_CENTER | DT_VCENTER);

	RECT rect2;
	rect2.left = rc_right_bottom.left;
	rect2.top = rc_right_bottom.bottom / 2 + 100;
	rect2.right = rc_right_bottom.right;
	rect2.bottom = rc_right_bottom.bottom;
	TCHAR szBuf2[30];
	LPCTSTR cstr2 = TEXT("Score：%d");
	wsprintf(szBuf2, cstr2, GAME_SCORE);

	DrawTextW(hdc, szBuf2, _tcslen(szBuf2), &rect2, DT_CENTER | DT_VCENTER);

	DeleteObject(hFont);
}

void drawCompleteParticle(int line)
{
	HWND hWnd = GetActiveWindow();
	HDC hdc = GetDC(hWnd);
	HBRUSH hBrush = (HBRUSH)GetStockObject(GRAY_BRUSH);
	HPEN hPen = (HPEN)CreatePen(PS_DOT, 1, RGB(102, 204, 255));//原创内容：完成方块：LUKA粉
	SelectObject(hdc, hBrush);
	SelectObject(hdc, hPen);
	Rectangle(hdc, BORDER_X,
		BORDER_Y + line * BLOCK_SIZE,
		BORDER_X + NUM_X * BLOCK_SIZE,
		BORDER_Y + BLOCK_SIZE * (1 + line));
	DeleteObject(hBrush);
	DeleteObject(hPen);
}

int RandomInt(int _min, int _max)
{
	srand((rd_seed++) % 65532 + GetTickCount());
	return _min + rand() % (_max - _min);
}

int CheckValide(int startX, int startY, BOOL bCurTeris[4][4])
{
	int i, j;
	for (i = 3; i >= 0; i--)
	{
		for (j = 3; j >= 0; j--)
		{
			if (bCurTeris[i][j])
			{
				if (j + startX<0 || j + startX >= NUM_X)
				{
					return -1;
				}
				if (i + startY >= NUM_Y)
				{
					return -2;
				}
				if (g_hasBlocked[i + startY][j + startX])
				{

					if (curPosY == 0)
					{
						return -3;
					}
					return -2;
				}
			}
		}
	}


	return 1;
}

int selectDiffculty(HWND hWnd, int diff)
{
	TCHAR szBuf2[30];
	LPCTSTR cstr2 = TEXT("是否选择难度 %d ？");
	wsprintf(szBuf2, cstr2, diff);
	if (MessageBox(hWnd, szBuf2, L"难度选择", MB_YESNO) == IDYES)
	{
		GAME_DIFF = diff;
		InvalidateRect(hWnd, &rc_right_bottom, false);
		GAME_STATE |= 2;
		init_game();
		return GAME_DIFF;
	}
	return -1;
}

int selectLayOut(HWND hWnd, int layout)
{
	NUM_X = 10 * layout;
	NUM_Y = 20 * layout;
	BLOCK_SIZE = 30 / layout;
	GAME_STATE |= 2;
	InvalidateRect(hWnd, &rc_right_bottom, false);
	init_game();
	return layout;
}
