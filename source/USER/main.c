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
#include "window.h"
#include "gui.h"
//ALIENTEK 探索者STM32F407开发板 实验57
//UCOSII-信号量和邮箱    --库函数版本
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com  
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK

 

/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   

//主任务
//设置任务优先级
#define MAIN_TASK_PRIO       			4 
//设置任务堆栈大小
#define MAIN_STK_SIZE  					128
//任务堆栈	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//任务函数
void main_task(void *pdata);

//音频任务
//设置任务优先级
#define VIDEO_TASK_PRIO       			3 
//设置任务堆栈大小
#define VIDEO_STK_SIZE  					512
//任务堆栈	
OS_STK VIDEO_TASK_STK[VIDEO_STK_SIZE];
//任务函数
void vedio_task(void *pdata);

//文件管理任务
//设置任务优先级
#define FILE_TASK_PRIO       			3 
//设置任务堆栈大小
#define FILE_STK_SIZE  					64
//任务堆栈	
OS_STK FILE_TASK_STK[FILE_STK_SIZE];
//任务函数
void FILE_task(void *pdata);


//////////////////////////////////////////////////////////////////////////////
OS_EVENT * msg_key;			//按键邮箱事件块指针	 	  
////加载主界面   
//void ucos_load_main_ui(void)
//{
//}	 


int main(void)
{ 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200
	
	KEY_Init(); 				//按键初始化  
 	LCD_Init();					//LCD初始化 
	tp_dev.init();				//触摸屏初始化
	gui_init();
	my_mem_init(SRAMIN);		//初始化内部内存池 
	my_mem_init(SRAMCCM);		//初始化CCM内存池 
	exfuns_init();			//为fatfs相关变量申请内存  
  	f_mount(fs[0],"0:",1); 	//挂载SD卡 
 	f_mount(fs[1],"1:",1); 	//挂载FLASH.
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	
 	LCD_Init();					//LCD初始化  
 	KEY_Init();					//按键初始化   
	W25QXX_Init();				//初始化W25Q128
	WM8978_Init();				//初始化WM8978	
	
	FSMC_SRAM_Init();			//初始化外部SRAM  
	WM8978_ADDA_Cfg(1,0);		//开启DAC
	WM8978_Input_Cfg(0,0,0);	 //关闭输入通道
	WM8978_Output_Cfg(1,0);		//开启DAC输出  
	WM8978_HPvol_Set(25,25);
	WM8978_SPKvol_Set(60);
	TIM3_Int_Init(10000-1,8400-1);//10Khz计数,1秒钟中断一次
	
	my_mem_init(SRAMIN);		//初始化内部内存池 
	my_mem_init(SRAMCCM);		//初始化CCM内存池 
	exfuns_init();				//为fatfs相关变量申请内存  
  	f_mount(fs[0],"0:",1); 		//挂载SD卡  
	POINT_COLOR=RED;      
	while(font_init()) 			//检查字库
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//清除显示	     
		delay_ms(200);	
	}  	 
//  update_font(20,110,16,"0:");//更新字库
	//tp_dev.init();				//触摸屏初始化
	
//	delay_ms(1500);	
	
	//ucos_load_main_ui();		//加载主界面	 
  	OSInit();  	 				//初始化UCOSII
  	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	    
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//开始任务
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 		  
	msg_key=OSMboxCreate((void*)0);	//创建消息邮箱 			  
	OSStatInit();					//初始化统计任务.这里会延时1秒钟左右	
 	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
 //	OSTaskCreate(touch_task,(void *)0,(OS_STK*)&TOUCH_TASK_STK[TOUCH_STK_SIZE-1],TOUCH_TASK_PRIO);	 				   				    				   
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);	 				   
 //	OSTaskCreate(key_task,(void *)0,(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],KEY_TASK_PRIO);	 	
 	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}	  
//任务调度函数
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


//主任务
void main_task(void *pdata)
{				
	OS_CPU_SR cpu_sr=0;
//	u8 *str = "ERROR";
	while(1){
//		LCD_ShowHomePic(); 
//		POINT_COLOR=RED;
//		Show_Str(20,20,120,16,"视频播放器",16,1);		
//		Show_Str(135,20,120,16,"文件查看器",16,1);	
		LCD_Clear(WHITE);
//		_window_obj *win1;
//		win1 = mymalloc(SRAMIN,sizeof(_window_obj));
//		win1 = window_creat(10,10,100,200,0,128,16);
//		window_draw(win1);	
		window_msg_box(0,0,150,200,"ERROR","title",16,RED,3,255);
		while(1)
		{
			LCD_Clear(WHITE);
			//window_msg_box(0,0,100,200,str,"TITLE",12,RED,0,255);
//			if(TP_Scan(0))//先判断有没有触摸事件
//			{
//				if(tp_dev.x[0]>20 && tp_dev.x[0]<70 && tp_dev.y[0]>40 && tp_dev.y[0]<100)
//				{
//						POINT_COLOR=BLUE;
//						Show_Str(20,20,120,16,"视频播放器",16,1);	
//						delay_ms(300);
//						OS_ENTER_CRITICAL();//进入临界段,防止其他任务,打断LCD操作,导致液晶乱序.
//						OSTaskCreate(vedio_task,(void *)0,(OS_STK*)&VIDEO_TASK_STK[VIDEO_STK_SIZE-1],VIDEO_TASK_PRIO);		
//						OS_EXIT_CRITICAL();
//						OSTaskSuspend(MAIN_TASK_PRIO);	//挂起起始任务.
//						break;
//				}else if(tp_dev.x[0]>135 && tp_dev.x[0]<220 && tp_dev.y[0]>40 && tp_dev.y[0]<70)
//				{
//						POINT_COLOR=BLUE;
//						Show_Str(135,20,120,16,"文件查看器",16,1);	
//						delay_ms(300);
//				  	OS_ENTER_CRITICAL();//进入临界段,防止其他任务,打断LCD操作,导致液晶乱序.
//						OSTaskCreate(file_task,(void *)0,(OS_STK*)&FILE_TASK_STK[FILE_STK_SIZE-1],FILE_TASK_PRIO);		
//						OS_EXIT_CRITICAL();
//						OSTaskSuspend(MAIN_TASK_PRIO);	//挂起起始任务.
//						break;
//				}
//			}
			delay_ms(1);
		}
	}
} 



