#ifndef __BKB_TOOLWND
#define __BKB_TOOLWND

#include "Fixation.h"

class BKBToolWnd
{
public: 
	static void Init( POINT p_next);   //static HWND Init();   //(20150128)
	static bool IsItYours(POINT *pnt, BKB_MODE *bm);
	static void OnPaint(HDC hdc=0);
	static void Reset(BKB_MODE *bm);
	static void ScrollCursor(POINT *p);
	static HWND GetHwnd(){return Tlhwnd;};
	static void SleepCheck(POINT *p);

	static bool tool_modifier[4];
	static int current_tool;
	static bool LeftSide() {return left_side;}

	static HWND Tlhwnd;

	static int end_flag;   //(20150324)
protected:
	static TCHAR *tool_modifier_name[4];
	//static HWND Tlhwnd;   //把這行註解(20150317)
	static int screen_x, screen_y;
	static bool left_side;
};

#endif