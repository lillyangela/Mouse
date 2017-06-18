﻿#ifndef __BKB_MAGNIFYWND
#define __BKB_MAGNIFYWND

class BKBMagnifyWnd
{
public:
	static void Init();
	static bool FixPoint(POINT *pnt);
	static void Reset();
	static bool IsVisible(){return mgf_visible;};
protected:
	static bool mgf_visible;
	static HWND Mghwnd;
	static int x_size, y_size, size_amendment;
	static int screen_x, screen_y;
	static int midpoint_x, midpoint_y;
};

#endif