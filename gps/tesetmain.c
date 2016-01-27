#include <reg51.h>
#include <stdio.h>
#include <string.h>
#include "GPS.h"
#include "LCD.h"
#include "display.h"

sbit led1 = P2^0;  //接收数据指示灯
sbit led2 = P2^1;  //GPRMC数据有效指示灯
sbit led3 = P2^2;  //GPGGA数据有效指示灯

#define   REV_YES    led1 = 0
#define   REV_NO     led1 = 1
#define   RMC_YES    led2 = 0
#define   RMC_NO     led2 = 1
#define   GGA_YES    led3 = 0
#define   GGA_NO     led3 = 1

char xdatarev_buf[80];       //接收缓存
ucharxdatarev_start = 0;     //接收开始标志
ucharxdatarev_stop  = 0;     //接收停止标志
ucharxdatagps_flag = 0;      //GPS处理标志
ucharxdatachange_page = 0;   //换页显示标志
ucharxdatanum = 0;           //

extern GPS_INFO   GPS;  //在display.c中定义，使用时要加extern

/*****************************/
/********初始化串口***********/
/******************************/
void Uart_Init(void)
{
	TMOD = 0x21;	//0010 0001	定时器1工作在模式2（8位自动重装）
	//定时器0工作在模式1（16位定时器/计数器）
	//	SCON=0X50;
	PCON=0X00;
	TH0=0x3c;
	TL0=0xb0;
	TH1=0xFa;		//	波特率为4800
	TL1=0xFa;		//
	TR1=1;     //开启定时器1
	REN=1;     //允许接收数据

	SM0=0;		//串口工作在第一模式
	SM1=1;

	TI=0;		//发送中断清0
	RI=0;		//接收中断清0

	EA=1;    //开总中断
	ES=1;    //串口1中断允许
	ET0 = 1; //定时器0中断允许
}

/****************************************
  主函数	
****************************************/
void main(void)
{
	ucharerror_num = 0;

	Uart_Init();  //初始化串口
	Lcd_Init();	  //初始化LCD
	GPS_Init();   //初始化GPS
	rev_stop=0;	  //接收停止标志
	REV_NO;       //接收数据指示灯关

	while(1)
	{
		if (rev_stop)   //如果接收完一行
		{
			TR0 = 1;   //开启定时器0
			REV_YES;   //接收数据指示灯开
			if ( change_page % 2 == 1)  //换页
			{
				if (GPS_GGA_Parse(rev_buf, &GPS))  //解析GPGGA
				{
					GGA_YES;
					GPS_DisplayTwo();   //显示第二页信息
					error_num = 0;
					gps_flag  = 0;	    //GPS处理标志
					rev_stop  = 0;		//接收停止标志
					REV_NO;				//接收数据指示灯关
				}
				else
				{
					error_num++;
					if (error_num>= 20) //如果数据无效超过20次
					{
						GGA_NO;			   //GPGGA数据无效指示灯
						error_num = 20;
						GPS_Init();     //返回初始化
					}
					gps_flag = 0;
					rev_stop  = 0;
					REV_NO;
				}

			}
			else
			{
				if (GPS_RMC_Parse(rev_buf, &GPS)) //解析GPRMC
				{
					RMC_YES;

					GPS_DisplayOne();	  //显示GPS第一页信息
					error_num = 0;
					gps_flag  = 0;
					rev_stop  = 0;
					REV_NO;	
				}
				else
				{
					error_num++;
					if (error_num>= 20) //如果数据无效超过20次
					{
						RMC_NO;
						error_num = 20;
						GPS_Init();     //返回初始化
					}
					gps_flag = 0;
					rev_stop  = 0;
					REV_NO;
				}
			}
		}
	}
}
/***************************************************************/
/*************************定时器0用作换页***********************/
/***************************************************************/

void timer0(void) interrupt 1
{
	static uchar count = 0;
	TH0 = 0x3c;
	TL0 = 0xb0;	

	if (count == 200)  //2*5秒钟
	{
		count = 0;
		change_page++;  //换页
		if (change_page == 10)
			change_page = 0;
	}		
} 
/***************************************************************/
/*************************串口数据接收**************************/
/***************************************************************/
void Uart_Receive(void) interrupt 4
{
	ucharch;
	ES = 0;					//串口1中断禁止
	if (RI)					//如果接收完成则进入
	{
		ch = SBUF;
		if ((ch == '$') && (gps_flag == 0))  //如果收到字符'$'，便开始接收
		{
			rev_start = 1;
			rev_stop  = 0;		  //接收停止标志
		}

		if (rev_start == 1)       //标志位为1，开始接收
		{
			rev_buf[num++] = ch;  //字符存到数组中
			if (ch == '\n')       //如果接收到换行
			{
				rev_buf[num] = '\0';
				rev_start = 0;
				rev_stop  = 1;	  //接收停止标志
				gps_flag = 1;
				num = 0;
			}
		}
	}
	RI = 0;	 //RI清0，重新接收
	ES = 1;	 //串口1中断允许
}
