// Взаимодействие (гироскопической) мышью вместо Tobii REX
#ifndef __BKB_AIRMOUSE
#define __BKB_AIRMOUSE

class BKBAirMouse
{
public:
	static int Init(HWND hwnd); // Инициализация работы с устройством
	static int Halt(HWND hwnd); // Завершение работы с устройством
	//static void OnTimer();   //(原來的)
	static int OnTimer();   //改寫OnTimer() , 目的要回傳滑鼠的x座標 (20150202)
	//static int has_created;   //(20150302)
protected:
	static bool initialized;
	//static HWND Airhwnd;   //(20150202)
};

#endif