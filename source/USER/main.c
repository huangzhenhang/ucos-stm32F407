#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "lcd.h"
#include "key.h"  
#include "touch.h"	 	
#include "includes.h"
#include "key.h"  
#include "malloc.h" 
#include "w25qxx.h"    
#include "sdio_sdcard.h"
#include "ff.h"  
#include "exfuns.h"    
#include "fontupd.h"
#include "text.h"	 
#include "wm8978.h"	 
#include "videoplayer.h" 
#include "timer.h" 
#include "piclib.h"	
#include "fattester.h"	 
#include "sram.h"
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
#define MAIN_TASK_PRIO       			4 
//���������ջ��С
#define MAIN_STK_SIZE  					128
//�����ջ	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//������
void main_task(void *pdata);

//��Ƶ����
//�����������ȼ�
#define VIDEO_TASK_PRIO       			3 
//���������ջ��С
#define VIDEO_STK_SIZE  					512
//�����ջ	
OS_STK VIDEO_TASK_STK[VIDEO_STK_SIZE];
//������
void vedio_task(void *pdata);

//�ļ���������
//�����������ȼ�
#define FILE_TASK_PRIO       			3 
//���������ջ��С
#define FILE_STK_SIZE  					64
//�����ջ	
OS_STK FILE_TASK_STK[FILE_STK_SIZE];
//������
void FILE_task(void *pdata);


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
	
	KEY_Init(); 				//������ʼ��  
 	LCD_Init();					//LCD��ʼ�� 
	tp_dev.init();				//��������ʼ��
	
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ�� 
	my_mem_init(SRAMCCM);		//��ʼ��CCM�ڴ�� 
	exfuns_init();			//Ϊfatfs��ر��������ڴ�  
  	f_mount(fs[0],"0:",1); 	//����SD�� 
 	f_mount(fs[1],"1:",1); 	//����FLASH.
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	
 	LCD_Init();					//LCD��ʼ��  
 	KEY_Init();					//������ʼ��   
	W25QXX_Init();				//��ʼ��W25Q128
	WM8978_Init();				//��ʼ��WM8978	
	
	FSMC_SRAM_Init();			//��ʼ���ⲿSRAM  
	WM8978_ADDA_Cfg(1,0);		//����DAC
	WM8978_Input_Cfg(0,0,0);	 //�ر�����ͨ��
	WM8978_Output_Cfg(1,0);		//����DAC���  
	WM8978_HPvol_Set(25,25);
	WM8978_SPKvol_Set(60);
	TIM3_Int_Init(10000-1,8400-1);//10Khz����,1�����ж�һ��
	
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ�� 
	my_mem_init(SRAMCCM);		//��ʼ��CCM�ڴ�� 
	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
  	f_mount(fs[0],"0:",1); 		//����SD��  
	POINT_COLOR=RED;      
	while(font_init()) 			//����ֿ�
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//�����ʾ	     
		delay_ms(200);	
	}  	 
//  update_font(20,110,16,"0:");//�����ֿ�
	//tp_dev.init();				//��������ʼ��
	
//	delay_ms(1500);	
	
	//ucos_load_main_ui();		//����������	 
  	OSInit();  	 				//��ʼ��UCOSII
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
 //	OSTaskCreate(touch_task,(void *)0,(OS_STK*)&TOUCH_TASK_STK[TOUCH_STK_SIZE-1],TOUCH_TASK_PRIO);	 				   				    				   
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);	 				   
 //	OSTaskCreate(key_task,(void *)0,(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],KEY_TASK_PRIO);	 	
 	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}	  
//������Ⱥ���
void vedio_task(void *pdata)
{				
	while(1)
	{
		video_play();
		OSTaskResume(MAIN_TASK_PRIO);
		OSTaskDel(VIDEO_TASK_PRIO);	
		delay_ms(1);
	}
} 

void file_task(void *pdata)
{				
	while(1)
	{
		file_manage();
		OSTaskResume(MAIN_TASK_PRIO);
		OSTaskDel(FILE_TASK_PRIO);	
		delay_ms(1);
	}
} 


//������
void main_task(void *pdata)
{				
	OS_CPU_SR cpu_sr=0;
	while(1){
		LCD_ShowHomePic(); 
		POINT_COLOR=RED;
		Show_Str(20,20,120,16,"��Ƶ������",16,1);		
		Show_Str(135,20,120,16,"�ļ��鿴��",16,1);	
		while(1)
		{
			if(TP_Scan(0))//���ж���û�д����¼�
			{
				if(tp_dev.x[0]>20 && tp_dev.x[0]<70 && tp_dev.y[0]>40 && tp_dev.y[0]<100)
				{
						POINT_COLOR=BLUE;
						Show_Str(20,20,120,16,"��Ƶ������",16,1);	
						delay_ms(300);
						OS_ENTER_CRITICAL();//�����ٽ��,��ֹ��������,���LCD����,����Һ������.
						OSTaskCreate(vedio_task,(void *)0,(OS_STK*)&VIDEO_TASK_STK[VIDEO_STK_SIZE-1],VIDEO_TASK_PRIO);		
						OS_EXIT_CRITICAL();
						OSTaskSuspend(MAIN_TASK_PRIO);	//������ʼ����.
						break;
				}else if(tp_dev.x[0]>135 && tp_dev.x[0]<220 && tp_dev.y[0]>40 && tp_dev.y[0]<70)
				{
						POINT_COLOR=BLUE;
						Show_Str(135,20,120,16,"�ļ��鿴��",16,1);	
						delay_ms(300);
				  	OS_ENTER_CRITICAL();//�����ٽ��,��ֹ��������,���LCD����,����Һ������.
						OSTaskCreate(file_task,(void *)0,(OS_STK*)&FILE_TASK_STK[FILE_STK_SIZE-1],FILE_TASK_PRIO);		
						OS_EXIT_CRITICAL();
						OSTaskSuspend(MAIN_TASK_PRIO);	//������ʼ����.
						break;
				}
			}
			delay_ms(1);
		}
	}
} 



