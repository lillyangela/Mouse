// Обрабатывает фиксацию взгляда в контексте текущего режима
#ifndef __BKB_FIXATION
#define __BKB_FIXATION

#include <stdint.h> // Это для uint64_t

typedef enum {BKB_MODE_LCLICK, BKB_MODE_LCLICK_PLUS, BKB_MODE_RCLICK, BKB_MODE_DOUBLECLICK, BKB_MODE_DOUBLECLICK_PLUS, BKB_MODE_DRAG, BKB_MODE_SCROLL, 
	BKB_MODE_KEYBOARD, BKB_MODE_NONE, BKB_MODE_SWAP, BKB_MODE_SLEEP} BKB_MODE;

class Fixation
{
public:
	static bool Fix(POINT p, POINT pAction); // Взгляд зафиксировался
	static BKB_MODE CurrentMode(){return BKB_Mode; };
	static void Scroll(uint64_t timelag, int direction);
	static BKB_MODE BKB_Mode;

protected:
	static void LeftClick(POINT p, bool skip_modifier_press=false, bool skip_modifier_unpress=false);
	static void RightClick(POINT p);
	static void DoubleClick(POINT p);
	static bool Drag(POINT p, POINT pAction); // true - значит закончил, можно сбрасывать режим (true - means finished, you can reset mode)
	static void ClickModifiers(bool press); // нажать-отпустить модификаторы клика

	
};

#endif