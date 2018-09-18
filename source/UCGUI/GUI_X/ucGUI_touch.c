#include "GUI_Private.h"
#include "LCD_Private.h"      /* inter modul definitions & Config */
#include "GUI_Protected.h"
#include"sys.h"
#include"touch.h"
#include"stdio.h"

void GUI_TOUCH_X_ActivateX(void)
{

}
void GUI_TOUCH_X_ActivateY(void)
{

}
void GUI_TOUCH_X_Disable(void)
{

}
int  GUI_TOUCH_X_MeasureX(void)
{
	u16 x;
	x=TP_Read_XOY(CMD_RDX);
//	printf("x:%d ",x);
	return x;
}
int  GUI_TOUCH_X_MeasureY(void)
{
	u16 y;
	y=TP_Read_XOY(CMD_RDY);
//	printf("y:%d ",y);
	return y;
}

