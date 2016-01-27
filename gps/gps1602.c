#include <REG52.H>
#include <stdio.h>
#include <intrins.h>

unsigned char  flag_rec=0;
unsigned char num_rec=0;

unsigned char code kaijihuamian[]="HLJU-505";
unsigned char code receiving[]="Receiving!";
unsigned char code nodata[]="No GPS data!";

char code TIME_AREA= 8;		//时区
unsigned char flag_data;	//数据标志位

//GPS数据存储数组
unsigned char JD[10];		//经度
unsigned char JD_a;		//经度方向
unsigned char WD[9];		//纬度
unsigned char WD_a;		//纬度方向
unsigned char date[6];		//日期
unsigned char time[6];		//时间
unsigned char time1[6];		//时间
unsigned char speed[5]={'0','0','0','0','0'};		//速度
unsigned char high[6];		//高度
unsigned char angle[5];		//方位角
unsigned char use_sat[2];	//使用的卫星数
unsigned char total_sat[2];	//天空中总卫星数
unsigned char lock;		//定位状态

//串口中断需要的变量
unsigned char seg_count;	//逗号计数器
unsigned char dot_count;	//小数点计数器
unsigned char byte_count;	//位数计数器
unsigned char cmd_number;	//命令类型
unsigned char mode;		//0：结束模式，1：命令模式，2：数据模式
unsigned char buf_full;		//1：整句接收完成，相应数据有效。0：缓存数据无效。
unsigned char cmd[5];		//命令类型存储数组

sbit rs	= P2^4;	
sbit rw = P2^5;
sbit ep = P2^6;

//-----------------------------------------------------------------------------------------------
//延时子程序
void delayms(unsigned char ms)
{
	unsigned char i;
	while(ms--)
	{
		for(i = 0; i < 120; i++);
	}
}

bit lcd_bz(void)
{							// 测试LCD忙碌状态
	bit result;
	rs = 0;
	rw = 1;
	ep = 1;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	result = (bit)(P0 & 0x80);
	ep = 0;
	return result;	
}

void lcd_wcmd(unsigned char cmd)
{							// 写入指令数据到LCD
	while(lcd_bz());
	rs = 0;
	rw = 0;
	ep = 0;
	_nop_();
	_nop_();	
	P0 = cmd;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	ep = 1;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	ep = 0;		
}

void lcd_pos(unsigned char pos)
{							//设定显示位置
	lcd_wcmd(pos | 0x80);
}

void lcd_wdat(unsigned char dat)	
{							//写入字符显示数据到LCD
	while(lcd_bz());
	rs = 1;
	rw = 0;
	ep = 0;
	P0 = dat;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	ep = 1;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	ep = 0;	
}
void lcd_string(unsigned char *ptr,unsigned char pos)
{

	unsigned char i;
	lcd_pos(pos);
	while(*(ptr+i) != '\0')
	{						// 显示字符"welcome!"
		lcd_wdat(*(ptr+i));
		i++;
	}		

}

lcd_init()
{							//LCD初始化设定
	lcd_wcmd(0x38);			//
	delayms(1);
	lcd_wcmd(0x0c);			//
	delayms(1);
	lcd_wcmd(0x06);			//
	delayms(1);
	lcd_wcmd(0x01);			//清除LCD的显示内容
	delayms(1);
}
//--------------------------------------------------------------------------
//发送字节
void sendbyte(unsigned char ptr)
{
	SBUF=ptr;
	while(TI==0);
	TI=0;
}
//---------------------------------------------------------------------------
//发送数据流
void sendstring(unsigned char *ptr,unsigned char len)
{

	int i=0;
	while(i<len)
	{	
		SBUF=*(ptr+i);	             //SUBF接受/发送缓冲器
		while(TI==0);
		TI=0;
		i++;	
	}
	num_rec=0;	

}

void init_all(void)	//9600  11.05926
{

	SCON = 0x50;      //REN=1允许串行接受状态，串口工作模式1    	       	   
	TMOD|= 0x20;      //定时器工作方式2                    
	PCON|= 0x80;                                                          
	TH1 = 0xFa;		//baud*2  /* reload value 9600、数据位8、停止位1。效验位无 (11.0592) 			
	TL1 = 0xF3;         
	TR1  = 1;                                                             
	ES   = 1;        //开串口中断                  
	EA   = 1;        // 开总中断
	lcd_init() ;

} 
//将UTC时间转成BJ时间
void trans_time(void)
{
	unsigned char temp,hour_shi,hour_ge;
	temp=(time[0]-'0')*10+(time[1]-'0');
	if(temp<=16)
	{
		temp=temp+8;
		hour_shi=temp/10;
		hour_ge=temp%10;
		time[0]=hour_shi+'0';
		time[1]=hour_ge+'0';

	}
	else
	{
		temp=temp+8-24;
		time[0]='0';
		time[1]=temp%10+'0';

	}

} 
//判断是否有GPS数据 有1，无0
bit gps_data(void)
{
	if(buf_full&0x01!=0)
		return 1;
	else 
		return 0;	

}

//----------------------------------------------------------------------------
void main (void) 
{	
	unsigned char i;
	init_all();	
	lcd_wcmd(0x01);			//清除LCD的显示内容
	delayms(10);
	i=0	 ;
	lcd_pos(4);			// 设置显示位置
	while(kaijihuamian[i] != '\0')
	{
		lcd_wdat(kaijihuamian[i]);	// 显示字符时间
		i++;
	}
	delayms(200);
	delayms(200);
	delayms(200);
	delayms(200);
	delayms(200);
	delayms(200);
	delayms(200);

	lcd_wcmd(0x01);			//清除LCD的显示内容
	delayms(10);
	while(1)
	{
		if(flag_data==0)// 如果没有数据
		{
			lcd_wcmd(0x01);			//清除LCD的显示内容
			delayms(10);
			i=0	 ;
			lcd_pos(2);			// 设置显示位置
			while(nodata[i] != '\0')
			{
				lcd_wdat(nodata[i]);	// 显示字符时间
				i++;
			}
			delayms(100);
			delayms(100);
			delayms(100);
			delayms(100);
			delayms(100);
			delayms(100);
			delayms(100);
			delayms(100);
			lcd_wcmd(0x01);			//清除LCD的显示内容
			delayms(10); 	

		}
		if(flag_rec==1)             //gps data appear
		{
			flag_rec=0;  //清数据有效标志位
			trans_time();//UTC--BJ TIME
			if (lock==1) //如果已经定位
			{
				//lock=0; //清定位标志位				  
				lcd_pos(0);			// 设置显示位置
				while(JD[i] != '\0')
				{
					lcd_wdat(JD[i]);	// 显示字符经度
					i++;
				}
				delayms(10);
				i=0	 ;
				lcd_pos(10);			// 设置显示位置
				while(time[i] != '\0')
				{
					lcd_wdat(time[i]);	// 显示字符时间
					i++;
				}
				delayms(10);
				i=0	;			//sendstring("\r\n",3);

				lcd_pos(0x40);			// 设置显示位置
				while(WD[i] != '\0')
				{
					lcd_wdat(WD[i]);	// 显示字符 纬度
					i++;
				}
				delayms(10);
				i=0	 ;

				//sendstring(WD,9);
				//	lcd_string(WD,40);
				delayms(10);
				//sendstring("\r\n",3);
				//	sendstring(time,6);
				//lcd_string(time,10);	
				delayms(10);
				//lcd_string("hello",46);
				//sendstring("\r\n",3);

			}
			else if(lock==0)	   //wei dingwei
			{			//display relative message

				lcd_wcmd(0x01);			//清除LCD的显示内容
				delayms(10);
				i=0	 ;
				lcd_pos(3);			// 设置显示位置
				while(receiving[i] != '\0')
				{
					lcd_wdat(receiving[i]);	// 显示字符时间
					i++;
				}
				delayms(100);
				delayms(100);
				delayms(100);
				delayms(100);
				delayms(100);
				lcd_wcmd(0x01);			//清除LCD的显示内容
				delayms(10);



			}

		}

	}
}

//--------------------------------------------------------------------
//串口中断程序
void ser_int (void) interrupt 4 using 1
{

	unsigned char tmp;
	if(RI)
	{
		RI=0;
		tmp=SBUF;
		switch(tmp)
		{
			case '$':
				cmd_number=0;		//命令类型清空
				mode=1;				//接收命令模式
				byte_count=0;		//接收位数清空
				flag_data=1;
				flag_rec=1;		//数据标志位置一
				break;
			case ',':
				seg_count++;		//逗号计数加1
				byte_count=0;
				break;
			case '*':
				switch(cmd_number)
				{
					case 1:
						buf_full|=0x01;
						break;
					case 2:
						buf_full|=0x02;
						break;
					case 3:
						buf_full|=0x04;
						break;
				}
				mode=0;
				break;
			default:
				if(mode==1)	//命令种类判断
				{
					cmd[byte_count]=tmp;			//接收字符放入类型缓存
					if(byte_count>=4)
					{				//如果类型数据接收完毕，判断类型
						if(cmd[0]=='G')
						{
							if(cmd[1]=='N')
							{
								if(cmd[2]=='G')
								{
									if(cmd[3]=='G')
									{
										if(cmd[4]=='A')
										{
											cmd_number=1;      //data type
											mode=2;
											seg_count=0;
											byte_count=0;
										}
									}
									else if(cmd[3]=='S')
									{
										if(cmd[4]=='V')
										{
											cmd_number=2;
											mode=2;
											seg_count=0;
											byte_count=0;
										}
									}
								}
								else if(cmd[2]=='R')
								{
									if(cmd[3]=='M')
									{
										if(cmd[4]=='C')
										{
											cmd_number=3;
											mode=2;
											seg_count=0;
											byte_count=0;
										}
									}
								}
							}
						}
					}
				}
				else if(mode==2)
				{
					//接收数据处理
					switch (cmd_number)
					{
						case 1:				//类型1数据接收。GPGGA
							switch(seg_count)
							{
								case 2:		//纬度处理
									if(byte_count<9)
									{
										WD[byte_count]=tmp;
									}
									break;
								case 3:		//纬度方向处理
									if(byte_count<1)
									{
										WD_a=tmp;
									}
									break;
								case 4:		//经度处理
									if(byte_count<10)
									{
										JD[byte_count]=tmp;
									}
									break;
								case 5:		//经度方向处理
									if(byte_count<1)
									{
										JD_a=tmp;
									}
									break;
								case 6:		//定位判断
									if(byte_count<1)
									{
										lock=tmp;
									}
									break;
								case 7:		//定位使用的卫星数
									if(byte_count<2)
									{
										use_sat[byte_count]=tmp;
									}
									break;
								case 9:		//高度处理
									if(byte_count<6)
									{
										high[byte_count]=tmp;
									}
									break;
							}
							break;
						case 2:				//类型2数据接收。GPGSV
							switch(seg_count)
							{
								case 3:		//天空中的卫星总数
									if(byte_count<2)
									{
										total_sat[byte_count]=tmp;
									}
									break;
							}
							break;
						case 3:				//类型3数据接收。GPRMC
							switch(seg_count)
							{
								case 1:		//时间处理
									if(byte_count<6)
									{				
										time[byte_count]=tmp;	
									}
									break;
								case 2:		//定位判断						
									if(byte_count<1)
									{
										if (tmp=='V') {lock=0;}
										else
										{
											lock=1;
										}
									}
									break;
								case 3:		//纬度处理						
									if(byte_count<9)
									{
										WD[byte_count]=tmp;
									}
									break;
								case 4:		//纬度方向处理						
									if(byte_count<1)
									{
										WD_a=tmp;
									}
									break;
								case 5:		//经度处理						
									if(byte_count<10)
									{
										JD[byte_count]=tmp;
									}
									break;
								case 6:		//经度方向处理						
									if(byte_count<1)
									{
										JD_a=tmp;
									}
									break;
								case 7:		//速度处理						
									if(byte_count<5)
									{
										speed[byte_count]=tmp;
									}
									break;
								case 8:		//方位角处理						
									if(byte_count<5)
									{
										angle[byte_count]=tmp;
									}
									break;
								case 9:		//方位角处理						
									if(byte_count<6)
									{
										date[byte_count]=tmp;
									}
									break;

							}
							break;
					}
				}
				byte_count++;		//接收数位加1
				break;
		}
	}
	//flag_rec=1;
	//trans_time();//UTC--BJ TIME
}
