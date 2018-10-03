#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "stm32lcd.h"
#include "key.h"  
#include "touch.h"	 	
#include "includes.h"
#include "malloc.h" 
//#include "w25qxx.h"    
//#include "sdio_sdcard.h"
//#include "ff.h"  
//#include "exfuns.h"    
//#include "fontupd.h"
//#include "text.h"	 
//#include "wm8978.h"	 
//#include "videoplayer.h" 
#include "timer.h" 
//#include "piclib.h"	
//#include "fattester.h"	 
#include "sram.h"
#include "gui.h"
#include "GUI_X.h"
//ALIENTEK ̽����STM32F407������ ʵ��57
//UCOSII-�ź���������    --�⺯���汾
//����֧�֣�www.openedv.com
//�Ա����̣�http://eboard.taobao.com  
//������������ӿƼ����޹�˾  
//���ߣ�����ԭ�� @ALIENTEK

 

/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 			   

//������
//�����������ȼ�
#define MAIN_TASK_PRIO       			2 
//���������ջ��С
#define MAIN_STK_SIZE  					128
//�����ջ	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//������
void main_task(void *pdata);

//�������
#define TOUCH_TASK_PRIO                 3 
//��������С
#define TOUCH_STK_SIZE                      64
//�������
OS_STK TOUCH_TASK_STK[TOUCH_STK_SIZE];
//????
void touch_task(void *pdata);


////��Ƶ����
////�����������ȼ�
//#define VIDEO_TASK_PRIO       			3 
////���������ջ��С
//#define VIDEO_STK_SIZE  					512
////�����ջ	
//OS_STK VIDEO_TASK_STK[VIDEO_STK_SIZE];
////������
//void vedio_task(void *pdata);

////�ļ���������
////�����������ȼ�
//#define FILE_TASK_PRIO       			3 
////���������ջ��С
//#define FILE_STK_SIZE  					64
////�����ջ	
//OS_STK FILE_TASK_STK[FILE_STK_SIZE];
////������
//void FILE_task(void *pdata);


//////////////////////////////////////////////////////////////////////////////
OS_EVENT * msg_key;			//���������¼���ָ��	 	  
////����������   
//void ucos_load_main_ui(void)
//{
//}	 


int main(void)
{ 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  //��ʼ����ʱ����
	uart_init(115200);		//��ʼ�����ڲ�����Ϊ115200
	FSMC_SRAM_Init();
	KEY_Init(); 				//������ʼ��  
 	STM_LCD_Init();					//LCD��ʼ�� 
	tp_dev.init();				//��������ʼ��
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ�� 
	my_mem_init(SRAMEX);		//��ʼ���ڲ��ڴ�� 
	my_mem_init(SRAMCCM);		//��ʼ��CCM�ڴ�� 
	exfuns_init();			//Ϊfatfs��ر��������ڴ� 	
  	f_mount(fs[0],"0:",1); 	//����SD�� 
 	f_mount(fs[1],"1:",1); 	//����FLASH.
	
//	TIM3_Int_Init(100-1,8400-1);//10Khz?????,??100??10ms
// 	LCD_Init();					//LCD��ʼ��  
// 	KEY_Init();					//������ʼ��   
//	W25QXX_Init();				//��ʼ��W25Q128
//	WM8978_Init();				//��ʼ��WM8978	
//	
//	FSMC_SRAM_Init();			//��ʼ���ⲿSRAM  
//	WM8978_ADDA_Cfg(1,0);		//����DAC
//	WM8978_Input_Cfg(0,0,0);	 //�ر�����ͨ��
//	WM8978_Output_Cfg(1,0);		//����DAC���  
//	WM8978_HPvol_Set(25,25);
//	WM8978_SPKvol_Set(60);
//	TIM3_Int_Init(10000-1,8400-1);//10Khz����,1�����ж�һ��
	
	POINT_COLOR=RED;      

	GUI_Init();  //GUI��ʼ��
	GUI_SetBkColor(GUI_BLUE);//?????
	GUI_SetColor(GUI_WHITE);
	GUI_Clear();
	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_DispStringAt("- - STM32F- -",4,10);
	GUI_DispStringAt("- - UCGUI3.98 - -",4,40);
	GUI_DispStringAt("- - SUCCESSFULLY - -",4,70);
	delay_ms(1500);	
	GUI_Clear();
	GUI_CURSOR_Show();
	while(font_init()) 			//����ֿ�
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//�����ʾ	     
		delay_ms(200);	
	}  	 
//  update_font(20,110,16,"0:");//�����ֿ�
	
//	delay_ms(1500);	
	
	//ucos_load_main_ui();		//����������	 
  	OSInit();  	 				//��ʼ��UCOSII
		GUI_X_InitOS();
  	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();	    
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//��ʼ����
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 		  
	msg_key=OSMboxCreate((void*)0);	//������Ϣ���� 			  
	OSStatInit();					//��ʼ��ͳ������.�������ʱ1��������	
 	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    				   				    				   
	OSTaskCreate(touch_task,(void *)0,(OS_STK*)&TOUCH_TASK_STK[TOUCH_STK_SIZE-1],TOUCH_TASK_PRIO);
	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);
 //	OSTaskCreate(key_task,(void *)0,(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],KEY_TASK_PRIO);	 	
 	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}	  
//������Ⱥ���
//void vedio_task(void *pdata)
//{				
//	while(1)
//	{
//		video_play();
//		OSTaskResume(MAIN_TASK_PRIO);
//		OSTaskDel(VIDEO_TASK_PRIO);	
//		delay_ms(1);
//	}
//} 

//void file_task(void *pdata)
//{				
//	while(1)
//	{
//		file_manage();
//		OSTaskResume(MAIN_TASK_PRIO);
//		OSTaskDel(FILE_TASK_PRIO);	
//		delay_ms(1);
//	}
//} 


//������
void main_task(void *pdata)
{				
	OS_CPU_SR cpu_sr=0;
//	u8 *str = "ERROR";
	while(1){
//		LCD_ShowHomePic(); 
//		POINT_COLOR=RED;
//		Show_Str(20,20,120,16,"��Ƶ������",16,1);		
//		Show_Str(135,20,120,16,"�ļ��鿴��",16,1);	
//		_window_obj *win1;
//		win1 = mymalloc(SRAMIN,sizeof(_window_obj));
//		win1 = window_creat(10,10,100,200,0,128,16);
//		window_draw(win1);	
		int num=0;
		while(1)
		{
			 
			//window_msg_box(0,0,100,200,str,"TITLE",12,RED,0,255);
//			if(TP_Scan(0))//���ж���û�д����¼�
//			{
//				if(tp_dev.x[0]>20 && tp_dev.x[0]<70 && tp_dev.y[0]>40 && tp_dev.y[0]<100)
//				{
//						POINT_COLOR=BLUE;
//						Show_Str(20,20,120,16,"��Ƶ������",16,1);	
//						delay_ms(300);
//						OS_ENTER_CRITICAL();//�����ٽ��,��ֹ��������,���LCD����,����Һ������.
//						OSTaskCreate(vedio_task,(void *)0,(OS_STK*)&VIDEO_TASK_STK[VIDEO_STK_SIZE-1],VIDEO_TASK_PRIO);		
//						OS_EXIT_CRITICAL();
//						OSTaskSuspend(MAIN_TASK_PRIO);	//������ʼ����.
//						break;
//				}else if(tp_dev.x[0]>135 && tp_dev.x[0]<220 && tp_dev.y[0]>40 && tp_dev.y[0]<70)
//				{
//						POINT_COLOR=BLUE;
//						Show_Str(135,20,120,16,"�ļ��鿴��",16,1);	
//						delay_ms(300);
//				  	OS_ENTER_CRITICAL();//�����ٽ��,��ֹ��������,���LCD����,����Һ������.
//						OSTaskCreate(file_task,(void *)0,(OS_STK*)&FILE_TASK_STK[FILE_STK_SIZE-1],FILE_TASK_PRIO);		
//						OS_EXIT_CRITICAL();
//						OSTaskSuspend(MAIN_TASK_PRIO);	//������ʼ����.
//						break;
//				}
//			}
//			GUI_DispStringAt("- - SUCCESSFULLY - -",4,70);
			LCD_ShowNum(0,20,num,10,16);
			num++;
			if(num>=200)num=0;
			delay_ms(10);
			
		}
	}
} 


void touch_task(void *pdata)
{     
		int r=10,y=50;
    while(1)
    {
        delay_ms(100);
//				GUI_Clear();
//				GUI_DispStringAt("- - SFULLY - -",4,110);
//				GUI_TOUCH_Exec();
//        GUI_Exec();//GUI
				GUI_DrawCircle(150,y,r);
				r+=10;
				if(r>=120){
				r=0;
				y+=10;
				
				}
    }
}


























