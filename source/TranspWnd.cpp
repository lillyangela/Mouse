#include <Windows.h>
#include "TranspWnd.h"
#include "BKBRepErr.h"
#include "WM_USER_messages.h"
#include "ToolWnd.h"
#include "AirMouse.h"   //(20150128)
#include "TobiiREX.h"   //(20150202)

static const TCHAR *wnd_class_name = L"BKBTransp";

extern HINSTANCE BKBInst;
extern int flag_using_airmouse;   //(20150128)
extern int has_created;   //(20150302)

void on_gaze_data_main_thread(); // определена в TobiiREX.cpp (defined in TobiiREX.cpp)   (20150202)

bool BKBTranspWnd::flag_show_transp_window=true;

int BKBTranspWnd::screen_x, BKBTranspWnd::screen_y;
HWND  BKBTranspWnd::Trhwnd=0;

int return_mouse_x;   //用來回傳給BKB001.cpp (20150202)

extern int has_create_toolwnd;   //(20150314) (20150324)
POINT last_time_point_ano;   //(20150314)
extern int xx, yy;   //(20150315)

int counter = 10;   //(20150317)
extern int prepare_flag;   //準備建立ToolWnd的flag (20150317)

extern int reset_counter_flag;   //(20150324)
int all_end_flag = 0;   //(20150324)

// Оконная процедура (The window procedure)
LRESULT CALLBACK BKBTranspWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	switch (message)
	{
		case WM_CREATE:
			// Содрано из интернета - так мы делаем окно прозрачным в белых его частях
			// Suppose the window background color is white (255,255,255).
            // Call the SetLayeredWindowAttributes when create the window.
            SetLayeredWindowAttributes(hwnd,RGB(255,255,255),NULL,LWA_COLORKEY);
			if (2 == flag_using_airmouse) BKBAirMouse::Init(hwnd);   //這個是一開始為甚麼小正方形位在(0,0)的關鍵 (原本沒有這個註解)//(20150128)
			break;

	 case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		hdc=BeginPaint(hwnd,&ps);
		//用來畫小正方形的框框(直線構成)//---------------------------------
		MoveToEx(hdc,100,100,NULL);
		LineTo(hdc,49,49);
		MoveToEx(hdc,49,55,NULL);
		LineTo(hdc,49,49);
		LineTo(hdc,55,49);
		
		EndPaint(hwnd,&ps);
		break;
	 
	 //(20150128)
	 //---------------------------------
	 //(20150302)
	 case WM_TIMER:
		 switch (wparam)
		 {
		 case 1 :   //(響應AirMouse.cpp的SetTimer 1號)
			 //每次呼叫BKBAirMouse::OnTimer() , 則counter每次-1 (單位為50ms) (20150317)
			 //if (prepare_flag==1)
			 //{
				 //counter--;   //如果prepare_flag=1 , 則開始倒數counter計時器(大約2秒) (20150317)
			 //}

			 //if (has_create_toolwnd==0 && reset_counter_flag==1)
			 //{
				 //counter = 10;
			 //}
			 
			 return_mouse_x = BKBAirMouse::OnTimer();
			 break;
		 case 2 :
			 
			 if (has_created==0)
			 {
				 KillTimer(hwnd, 2);
				 POINT p_next;
				 GetCursorPos(&p_next);
				// last_time_point_ano.x = xx;
				// last_time_point_ano.y = yy;
				 //BKBToolWnd::Init(hwnd, last_time_point_ano);

				 //POINT p_2;   //(20150309)
				 //Sleep(25);   //(20150309)
				 //GetCursorPos(&p_2);   //(20150309)

				 //if ((p_next.x - p_2.x > 10 && p_next.x - p_2.x < 100) || (p_2.x - p_next.x > 10 && p_2.x - p_next.x < 100))   //(20150309)
				 //{
				 //last_time_point_ano.x = 200;
				 //last_time_point_ano.y = 200;
				 //BKBToolWnd::Init(p_next);
					 has_created = 1;   //真的創造出來 (20150302)
				 //}
			 }
			 break;
			 
		 /*
		 case 5 :
			 if (has_create_toolwnd == 0)   //(20150314)
			 {   //(20150314)
				 KillTimer(hwnd, 5);
				 last_time_point_ano.x = xx;
				 last_time_point_ano.y = yy;
				 BKBToolWnd::Init(last_time_point_ano);   //(20150314)
				 has_create_toolwnd = 1;   //(20150314)
			 }   //(20150314)
		  */
		 }
		 
	 //---------------------------------

	 //(20150202)
	 //---------------------------------
	 case WM_USER_DATA_READY:
		 on_gaze_data_main_thread();
		 break;
	 //---------------------------------

	 case WM_USER_MOVEWINDOW:   //移動的關鍵
		 MoveWindow(hwnd,wparam,lparam,100,100,FALSE);
		 break;

	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}

	return 0; // Обработали, свалились сюда по break (Treated, fell here at break)
}

//================================================================
// Инициализация (initialization)
//================================================================
void BKBTranspWnd::Init()   //void BKBTranspWnd::Init(HWND master_hwnd)
{
	ATOM aresult; // Для всяких кодов возврата (For any return codes)
	

	// 0.  Для аэромыши вообще его не создаём! (0. For aeromyshi generally did not create!)
	if(!flag_show_transp_window)
	{
		Trhwnd=0;
		//return;
		//return 0;   //原本沒有這一行!(20150128)
	}

	// 1. Регистрация класса окна
	WNDCLASS wcl = { CS_HREDRAW | CS_VREDRAW, BKBTranspWndProc, 0,
		//sizeof(LONG_PTR), // Сюда пропишем ссылку на объект
		0,
		BKBInst,
		LoadIcon(NULL, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)GetStockObject(WHITE_BRUSH),
		NULL,
		wnd_class_name
	};

	aresult=::RegisterClass(&wcl); 


	if (aresult==0)
	{
		BKBReportError(__WIDEFILE__,L"RegisterClass (",__LINE__);
		//return;
		//return 0;   //原本沒有這一行!(20150128)
	}

	screen_x=GetSystemMetrics(SM_CXSCREEN);
	screen_y=GetSystemMetrics(SM_CYSCREEN);

	Trhwnd=CreateWindowEx(
	WS_EX_LAYERED|WS_EX_TOPMOST|WS_EX_CLIENTEDGE,
	//WS_EX_LAYERED|WS_EX_TOPMOST,
	wnd_class_name,
	NULL, //TEXT(KBWindowName),
    //WS_VISIBLE|WS_POPUP,
	WS_POPUP,
	//100,100, // Не здесь ли крылась мерзкая ошибка, когда окно с курсором рисовалось в стороне?? Нет, похоже это было из-за HighDPI
	0,0,
	100,100, 
    //0,
	//master_hwnd, // Чтобы в таскбаре и при альт-табе не появлялись лишние окна (That in the taskbar and alt-tabe not appear superfluous window)
	NULL,   //原本沒有這一行!(20150128)
	0, BKBInst, 0L );

	if(NULL==Trhwnd)
	{
		BKBReportError(__WIDEFILE__,L"CreateWindow",__LINE__);
	}

	Show();
	UpdateWindow(Trhwnd);

	//return Trhwnd;   //原本沒有這一行!(20150128)
}

void BKBTranspWnd::Move(int x, int y)   //在OnGazeData.cpp會用到 , x和y是分別傳入screen_cursor_point.x和screen_cursor_point.y
{
	// Это другой поток, а мы ждать не будем
	if(flag_show_transp_window)
		PostMessage(Trhwnd, WM_USER_MOVEWINDOW, x-50, y-50);
	//MoveWindow(Trhwnd,x-50,y-50,100,100,FALSE);
}

void BKBTranspWnd::Show()
{
	if(flag_show_transp_window)
		ShowWindow(Trhwnd,SW_SHOWNORMAL); 
}

void BKBTranspWnd::Hide()
{
	if(flag_show_transp_window)
		ShowWindow(Trhwnd,SW_HIDE); 
}

void BKBTranspWnd::ToTop()	
{ 
	if(flag_show_transp_window)
	{
		SetActiveWindow(Trhwnd);
		BringWindowToTop(Trhwnd); 
	}
	else
	{
		// Из-за отсутствия этого панель задач перекрывала панель инструментов
		//(Due to the absence of overlaps taskbar toolbar)
		SetActiveWindow(BKBToolWnd::GetHwnd());
		BringWindowToTop(BKBToolWnd::GetHwnd()); 
	}
}
/*
int BKBTranspWnd::for_BKB001()   //(20150202)
{
	return return_mouse_x;
}
*/