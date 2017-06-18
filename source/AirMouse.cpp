#include <Windows.h>
#include "AirMouse.h"
#include "TranspWnd.h"
#include "ToolWnd.h"
#include "Fixation.h"
#include <time.h>

int g_BKB_MOUSE_X_MULTIPLIER=20, g_BKB_MOUSE_Y_MULTIPLIER=30; // усилитель мыши для тех, кому трудно поворачивать голову

// Заголовочные файлы из Tobii Gaze SDK
#include "tobiigaze_error_codes.h"
#include "tobiigaze.h"
//#include "tobiigaze_config.h" - такой файл был в Gaze SDK 2.0

// Прототип callback-функции из TobiiREX.cpp (The prototype of the callback-function TobiiREX.cpp)
void on_gaze_data(const tobiigaze_gaze_data* gazedata, void *user_data);

extern int screenX, screenY, mouscreenX, mouscreenY;

// Прототип перехватчика мыши (описан ниже)
LRESULT  CALLBACK HookProc(int disabled,WPARAM wParam,LPARAM lParam) ;
static HHOOK handle=0;
static bool hook_initialized=false;
bool skip_mouse_hook=false;
static LONG last_x,last_y;

//HWND  BKBAirMouse::Airhwnd = 0;   //(20150202)
HWND  Airhwnd = 0;   //(20150204)
extern HWND BKBhwnd;

static bool m_track = true;   //(20150204)

int has_created = 0;   //用來辦別是否有跑出ToolWnd視窗 , 0代表沒有 (20150302)
int iHoldOn = 0;       //enter hold mode due to no any action, add by sflin (20160307)
int iHoldTime = 300;   //add to define hold time, add by sflin (20160307)

//-----------------------------------------------------------------------------------------------------------------------------------------
int has_create_toolbox = 0;   //(20150317)
//extern int BKBTranspWnd::counter;   //以50 ms為單位 (20150203) (20150317)
POINT last_point;   //此point是用來儲存上一個0.05秒的鼠標位置<對應OnTimer()的p> , 每0.05秒更新一次 (20150317)
int counter_of_distance;    //要讓這個counter_of_distance加1的條件是 : 在0.05秒的間隔內 , last_point和p的x座標差值在100以內 (20150317)
                                //*****上限為20(大約一秒) , 當加到20時 , 則啟動counter計時2秒 , 來真正啟動ToolBox***** (20150317)
extern int counter;
int prepare_flag;   //準備建立ToolWnd的flag (20150317)
int mouse_to_mouse = 50;   //設定每1000毫秒取一次滑鼠位置 , 不然都是同樣的位置 (20150317)

int reset_counter_flag = 0;   //(20150324)
extern int all_end_flag;   //(20150324)
extern int has_create_toolwnd;   //(20150324)

extern POINT pAction;

int counter2 = 0;   //(20150324)
//-----------------------------------------------------------------------------------------------------------------------------------------

//============================================================================
// Запускает таймер, по которому якобы приходят координаты курсора от Tobii
// 10 раз в секунду
//============================================================================
int BKBAirMouse::Init(HWND hwnd)
{
	//screenX=GetSystemMetrics(SM_CXSCREEN);
	//screenY=GetSystemMetrics(SM_CYSCREEN);

	SetTimer(hwnd,1,30,NULL);   //該計時器每0.02秒就會發出WM_TIMER訊號 , 即WM_TIMER事件發生(TranspWnd.cpp的WM_TIMER) (20150318)
	                            //改成0.05秒 , Magnify window 比較沒那麼快出現 (20150310)

	last_point.x = 0;   //初始化last_point的x值 (20150317)
	last_point.y = 0;   //初始化last_point的y值 (20150317)
	counter_of_distance = 0;   //初始化 (20150317)
	prepare_flag = 0;   //初始化 (20150317)

	// Не показывать прозрачное окно, ибо курсор сам движется
	// BKBTranspWnd::flag_show_transp_window=false;   //先暫時打開小正方形(2015/01/28)

	// Для тех, кому трудно поворачивать голову
	// Теперь включаем всегда, а контролируем параметры внутри хука HookProc
	//if((g_BKB_MOUSE_X_MULTIPLIER!=10)||(g_BKB_MOUSE_Y_MULTIPLIER!=10))
	{
		if(!handle)
		{
			handle = SetWindowsHookEx(WH_MOUSE_LL, 
				HookProc, 
				GetModuleHandle(NULL), 
				NULL);
		}
	}

	Airhwnd = hwnd;   //(20150202)

	return 0;
}

//============================================================================
// Убивает таймер, по которому якобы приходят координаты курсора от Tobii
// 10 раз в секунду
//============================================================================
int BKBAirMouse::Halt(HWND hwnd)
{
	KillTimer(hwnd,1);

	// Для тех, кому трудно поворачивать голову
	// Теперь выключаем всегда, а контролируем параметры внутри хука HookProc
	// if((g_BKB_MOUSE_X_MULTIPLIER!=10)||(g_BKB_MOUSE_Y_MULTIPLIER!=10))
	{
		if(handle)
		{
			UnhookWindowsHookEx(handle);
			handle = 0;
		}
	}

	return 0;
}

//============================================================================
// При срабатывании таймера имитирует сигнал от Tobii REX
// (When the timer simulates the signal from Tobii REX)
//============================================================================
//void BKBAirMouse::OnTimer()   //(原來的)
int BKBAirMouse::OnTimer()   //(20150202)
{
	tobiigaze_gaze_data gd;
	//這邊可以真正獲取目前滑鼠的位置
	POINT p;
	GetCursorPos(&p);

	//***TRICK***
	if (mouse_to_mouse == 50)
	{
		last_point.x = p.x;   //把p的xy位置傳給last_point (20150317)
		last_point.y = p.y;

		mouse_to_mouse = 0;   //歸零 (20150317)
	}
	else
	{
		mouse_to_mouse++;
	}
	//if(0==GetCursorPos(&p))
	//{
		//OutputDebugString("Bad cursor position\n");
		//return -1;   //(20150202)
	//}
	//else
	//{
#ifdef BELYAKOV
		BKBToolWnd::SleepCheck(&p);
#endif

		//(20150202)
		//-----------------------------------------------------------------------------------------------------
		//透過p和p_next中間間隔0.05秒來獲取差值 (20150311)
		/*
		Sleep(50);
		POINT p_next;
		GetCursorPos(&p_next);
		*/
		//SetTimer(Airhwnd, 2, 2000, NULL);

		//if (SetTimer(Airhwnd, 2, 10, NULL) && p_next.x - p.x > 0 && p_next.x - p.x < 50)
		//if ((p_next.x - p.x > 0 && p_next.x - p.x < 100) || (p.x - p_next.x > 0 && p.x - p_next.x < 100))   //如果滑鼠的x範圍在0~50之間的話 , 則啟動計時器
		//if (p_next.x - p.x > 0 || p.x - p_next.x > 0)


		//if ( (p.x - last_point.x < 100 || p.x - last_point.x > -100) && p.x - last_point.x != 0)   //(20150317)
		//if ( p.x - last_point.x < 100 && p.x - last_point.x > -100 && p.x - last_point.x != 0 )   //(20150317)
		
		//if (mouse_to_mouse == 500)
		//{
		
		//parameter set by Tlhwnd not hardcode define, add by sflin (20160307)
		if (BKBToolWnd::Tlhwnd == 0)
		{
			has_create_toolwnd = 0;
		}
		else has_create_toolwnd = 1;
		
		if (p.x - last_point.x < 100 && p.x - last_point.x > -100)   //-100 < distance < 100 (20150317)
		    {
				//if (p.x - last_point.x != 0)
				{
					counter_of_distance++;   //(20150317)
				}
				//mouse_to_mouse = 0;   //歸零 (20150317)
			}
			else
			{
				counter_of_distance = 0;   //如果其中有一度超過100的界線 , 則歸0 (20150317)
				iHoldOn = 0;
			}
		//}
		
		//if (has_create_toolbox==0)   //(20150317)
		if ((counter_of_distance == 50) && (iHoldOn == 0))
		{
				//ShowWindow(Airhwnd, SW_NORMAL);   //目的是要用了一次功能之後,把工具箱藏起來,不過目前只能隱藏外表(20150303)	
				//SetTimer(Airhwnd, 2, 2000, NULL);   //設定計時器3秒 , 超過3秒則發出WM_TIMER訊號
				//BKBToolWnd::Init(p);   //(20150317)
				prepare_flag = 1;
				//has_create_toolbox = 1;   //(20150317)
				//BKBToolWnd::Init(Airhwnd, p_next);
				//KillTimer(Airhwnd, 2);
		}
		// hold while no move change during the time period, add by sflin (20160307)
		else if ((counter_of_distance == 300) && (has_create_toolwnd = 1) && (iHoldOn == 0) && (Fixation::BKB_Mode != BKB_MODE_KEYBOARD) && (Fixation::BKB_Mode != BKB_MODE_SCROLL))
		{
			BKBToolWnd::Reset(&Fixation::BKB_Mode);
			iHoldOn = 1;
		}
		/*
		if (counter == 0 && prepare_flag == 1 && has_create_toolwnd==0)
		{
			BKBToolWnd::Init(p);   //(20150317)
			has_create_toolwnd = 1;
			prepare_flag = 0;   //(20150317)
			//counter = 10;   //(20150317)
			reset_counter_flag = 1;

			BKBToolWnd::end_flag = 0;   //(20150324)
			//last_point.x = 0;   //(20150324)
			//last_point.y = 0;   //(20150324)
			//ShowWindow(BKBToolWnd::Tlhwnd, SW_HIDE);   //當counter=0時 , 則把ToolWnd隱藏起來 (Test) (20150317)
		}
		*/
		if (prepare_flag == 1 && has_create_toolwnd == 0 && iHoldOn == 0)
		{
			if (counter2==50)
			{
				pAction = p;
				BKBToolWnd::Init(p);   //(20150324)
				//has_create_toolwnd = 1; //parameter set by Tlhwnd not hardcode define, mark by sflin (20160307)
				counter2 = 0;
				prepare_flag = 0;
			}
			else
			{
				counter2++;
			}
		}
		//-----------------------------------------------------------------------------------------------------
		//(20150202)
		

		gd.tracking_status = TOBIIGAZE_TRACKING_STATUS_BOTH_EYES_TRACKED;
		//gd.left.gaze_point_on_display_normalized.x=p.x/(double)screenX;
		//gd.left.gaze_point_on_display_normalized.y=p.y/(double)screenY;
		gd.left.gaze_point_on_display_normalized.x=p.x/(double)screenX;
		gd.left.gaze_point_on_display_normalized.y=p.y/(double)screenY;

		gd.right.gaze_point_on_display_normalized.x=gd.left.gaze_point_on_display_normalized.x;
		gd.right.gaze_point_on_display_normalized.y=gd.left.gaze_point_on_display_normalized.y;

		gd.timestamp=1000UL*timeGetTime(); // Используется скроллом

		on_gaze_data(&gd, NULL);
		return p.x;   //(20150202)
	//}
}

//====================================================================================
// Собственно, хук 
//====================================================================================
LRESULT  CALLBACK HookProc(int disabled,WPARAM wParam,LPARAM lParam) 
{
	LRESULT l;
	INPUT input={0};
	
    if ((!disabled)&&((g_BKB_MOUSE_X_MULTIPLIER!=10)||(g_BKB_MOUSE_Y_MULTIPLIER!=10)))
	{
		MOUSEHOOKSTRUCT * pMouseStruct = (MOUSEHOOKSTRUCT *)lParam;
		if (pMouseStruct != NULL)
		{
			switch(wParam)
			{
				case WM_MOUSEMOVE:
					if(hook_initialized)
					{
						// Пропускаем всё, что создали сами
						if(skip_mouse_hook)
						{
							//swprintf_s(debug_buf,_countof(debug_buf),L"%ld %ld\r\n",pMouseStruct->pt.x,pMouseStruct->pt.y);
							//OutputDebugString(debug_buf);
						}
						else
						{
							skip_mouse_hook=true;
							// Собственно, удвоение движение мыши
							
							input.type=INPUT_MOUSE;
							input.mi.dx=g_BKB_MOUSE_X_MULTIPLIER*(pMouseStruct->pt.x-last_x)/10;
							input.mi.dy=g_BKB_MOUSE_Y_MULTIPLIER*(pMouseStruct->pt.y-last_y)/10;
							//input.mi.dx=10;
							//input.mi.dy=10;
							input.mi.mouseData=0; // Нужно для всяких колёс прокрутки 
							//input.mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE;
							input.mi.dwFlags=MOUSEEVENTF_MOVE;
							input.mi.time=0;
							input.mi.dwExtraInfo=0;
							
							//swprintf_s(debug_buf,_countof(debug_buf),L"%ld %ld %ld %ld\r\n",pMouseStruct->pt.x,pMouseStruct->pt.y,input.mi.dx,input.mi.dy);
							//OutputDebugString(debug_buf);

							//
							//last_x=pMouseStruct->pt.x+input.mi.dx; last_y=pMouseStruct->pt.y+input.mi.dy;
							
							// Вынесли перед SendInput, хотя могли бы вообще убрать
							//last_x=pMouseStruct->pt.x; last_y=pMouseStruct->pt.y;
							SendInput(1,&input,sizeof(INPUT));

							skip_mouse_hook=false;
							
							hook_initialized=true;
							return 1;
						}
					}

				
					last_x=pMouseStruct->pt.x; last_y=pMouseStruct->pt.y;
					hook_initialized=true;
					if(last_x<0) last_x=0; if(last_y<0) last_y=0;
					//if(last_x>screenX) last_x=screenX-1;
					//if(last_y>screenX) last_y=screenX-1; // Вот тут баг какой был, а я и не замечал! y и x!
					if(last_x>mouscreenX) last_x=mouscreenX-1;
					if(last_y>mouscreenY) last_y=mouscreenY-1;
					break;
			}
		}
	}

	return CallNextHookEx(NULL,disabled,wParam,lParam);
}
/*
LRESULT OnMouseMove(UINT, WPARAM, LPARAM, BOOL)
{
	//(20150204)
	//-----------------------------------------------------------------------------------------------------
	if (m_track)
	{
		TRACKMOUSEEVENT track;
		track.cbSize = sizeof(track);
		track.dwFlags = TME_HOVER;
		track.dwHoverTime = 50;
		track.hwndTrack = Airhwnd;

		::TrackMouseEvent(&track);

		return 0;
	}
	//-----------------------------------------------------------------------------------------------------	
	//(20150204)
}

LRESULT OnMouseHover(UINT, WPARAM, LPARAM, BOOL)
{
	UINT w=50, h=50;
	::SystemParametersInfo(SPI_GETMOUSEHOVERWIDTH,0,&w,0);
	::SystemParametersInfo(SPI_GETMOUSEHOVERHEIGHT, 0, &h, 0);

	POINT p_next;
	GetCursorPos(&p_next);

	//BKBToolWnd::Init(Airhwnd, p_next);

	return 0;
}
*/
