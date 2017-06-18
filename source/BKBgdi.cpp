#include <Windows.h>
#include "BKBgdi.h"

HPEN red_pen, green_pen, dkyellow_pen; // перья ?кист?для рисования
HBRUSH dkblue_brush, dkblue_brush2, blue_brush;
HFONT hfont;

int screenX, screenY, mouscreenX, mouscreenY;

double screen_scale=1.0;

void BKBgdiInit()
{
	//  Кист?создаё?	red_pen=CreatePen(PS_SOLID,1,RGB(255,100,100));
	green_pen=CreatePen(PS_SOLID,1,RGB(100,255,100));
	dkyellow_pen=CreatePen(PS_SOLID,1,RGB(227,198,2));

	dkblue_brush=CreateSolidBrush(RGB(45,62,90));
	dkblue_brush2=CreateSolidBrush(RGB(100,72,100));
	blue_brush=CreateSolidBrush(RGB(188,199,216));

	hfont = CreateFont( -48, 0, 0, 0, FW_BOLD, 0, 0, 0,
		RUSSIAN_CHARSET,
		0, 0, 0, 0, L"Arial");
	

	// Получи?разрешение экрана
	screenX=GetSystemMetrics(SM_CXSCREEN);
	screenY=GetSystemMetrics(SM_CYSCREEN);

	// Козлин? систем?разрешений экрана ?windows8.1...
	DEVMODE dm;
	ZeroMemory (&dm, sizeof (dm));
	EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &dm);

	screen_scale=((double)screenX)/dm.dmPelsWidth;

	// ?windows 8 пр?HighDPI координаты курсор?отличают? от координа?точк?на экране
	mouscreenX=dm.dmPelsWidth;
	mouscreenY=dm.dmPelsHeight;
}

void BKBgdiHalt()
{
	// удаляем кист?	DeleteObject(red_pen);
	DeleteObject(dkyellow_pen);
	DeleteObject(green_pen);

	DeleteObject(dkblue_brush);
	DeleteObject(blue_brush);

	DeleteObject(hfont);
}