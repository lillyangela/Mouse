#include <windows.h>
#include "BKBRepErr.h"
#include "Internat.h"
#include "TobiiREX.h"
#include "MagnifyWnd.h"
#include "ToolWnd.h"
#include "TranspWnd.h"
#include "KeybWnd.h"
#include "AirMouse.h"
#include "BKBgdi.h"
#include "TET.h"
#include "Click.h"
#include "BKBSettings.h"

// Прототип WndProc решил в .h не включать (WndProc prototype has decided not to include in the .h)
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
// И вот это тоже (And this, too,)
int StartupDialog();

// Глобальные переменные, которые могут потребоваться везде
//TCHAR		BKBAppName=L"Клавиатура и мышь для управления глазами : сборка B / Keyboard & Mouse control with the eyes : Release B";
HINSTANCE	BKBInst;
HWND		BKBhwnd;
int flag_using_airmouse;

// Имя класса окна
//static const char BKBWindowCName[]="BKB0B"; 

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE,LPSTR cline,INT)
// Командную строку не обрабатываем
{
	MSG msg; // Сообщение

	// Сразу делаем Instance доступной всем
	BKBInst=hInst;

	// Если есть другой язык - загружаем его
	Internat::LoadMessages();
	BKBKeybWnd::Load(); // Если есть клавиатура - загружаем её
	BKBSettings::OpenBKBConfig(); // Пробуем прочитать параметры
	BKBSettings::ActualizeLoad(); // Прочитанные параметры считаем верными и пускаем их в бой

	// Что будем использовать?
	flag_using_airmouse=StartupDialog();

	switch(flag_using_airmouse)
	{
	case 0: // Tobii
		// Инициализируем работу с Tobii REX
		// Если произошла ошибка, переходим на работу с [аэро]мышью
		if(BKBTobiiREX::Init()) flag_using_airmouse=1; // пробуем с TheEyeTribe (запоминаем, что гасить в конце)
		else break; // всё прошло успешно

	case 1: // TheEyeTribe
		if (BKBTET::Init())
		//{
			flag_using_airmouse = 2; // пробуем с TheEyeTribe (запоминаем, что гасить в конце)
			//BKBAirMouse::Init(BKBhwnd);   //這個是一開始為甚麼小正方形位在(0,0)的關鍵 (原本沒有這個註解)//(20150128)
		//}
		else break; // всё прошло успешно
		
	case 2: // просто мышь... ничего здесь не делаем
		// Запускаем эмуляцию работы устройства Tobii аэромышью
		// Это сделаем в WM_CREATE ToolBar'a
		break;
	}

	// Кисти-фонты, screen_scale
	BKBgdiInit();
	// Звуки
	BKBClick::Init();

	// Создаем окна
	//BKBMagnifyWnd::Init(); // Увеличительное
	/*
	HWND master_hwnd=BKBToolWnd::Init(); // Инструменты с правой стороны
	BKBMagnifyWnd::Init(master_hwnd); // Увеличительное
	BKBKeybWnd::Init(master_hwnd); // Клавиатура
	BKBTranspWnd::Init(master_hwnd); // Прозрачное окно
	*/

	//嘗試調動順序(2015/01/27)
	
	//HWND master_hwnd = BKBTranspWnd::Init(); // Прозрачное окно

	//要在這裡加入判斷 , 如果滑鼠在某個範圍內不動 , 則跳出tools (最終目的)

	//if (SetTimer(master_hwnd, 5, 10000, NULL))
	//{
	//if (BKBTranspWnd::for_BKB001()==1900)
		//{
			//Sleep(2000);   //測試 , 嘗試睡2秒
			//BKBToolWnd::Init(master_hwnd); // Инструменты с правой стороны
			BKBTranspWnd::Init();
			BKBMagnifyWnd::Init(); // Увеличительное  (會發動BKBToolWnd::Reset函式 , 並隱藏工具箱)
			BKBKeybWnd::Init(); // Клавиатура
			//BKBKeybWnd::Init(master_hwnd); // Клавиатура	
		//}
	//}
	

	



	//Цикл обработки сообщений
	while(GetMessage(&msg,NULL,0,0)) 
    {
		TranslateMessage( &msg );
        DispatchMessage( &msg );
	}// while !WM_QUIT

	// Чистим за собой

	// Звуки
	BKBClick::Halt();
	// Кисти-фонты
	BKBgdiHalt();

	switch(flag_using_airmouse)
	{
	case 0:
		BKBTobiiREX::Halt();
		break;

	case 1:
		BKBTET::Halt();
		break;
	}

	Internat::Unload();

	return 0;
}