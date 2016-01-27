#include "display.h"

GPS_INFO   GPS;  //GPS信息结构体	 GPS为结构体类型变量


uchar code beiwei[]     = "北纬";
uchar code nanwei[]     = "南纬";
uchar code dongjing[]   = "东经";
uchar code xijing[]     = "西经";
uchar code sudu[]       = "速度: ";
uchar code hangxiang[]  = "航向: ";
uchar code gaodu[]      = "高度: ";
uchar code jiaodu[]     = "角度: ";
uchar code haiba[]      = "海拔: ";
uchar code du[]         = "度";
uchar code meter[]      = "米";
uchar code kmperhour[]  = "km/h";
uchar code date[]       = "    年月日  ";

void Show_Float(float fla, uchar x, uchar y);


void GPS_DispTime(void)
{
	uchari = 0;
	ucharch;
	char time[5];

	Lcd_DispLine(0, 0, date);  //年月日
	Int_To_Str(GPS.D.year,time);  //将年转换成字符串，存在time中
	Lcd_SetPos(0, 0);             //设置显示地址
	if(strlen(time)==4)				//判断接收数据是否有效，有效则显示
	{
		i = 0;
		while(time[i] != '\0')
		{
			ch = time[i++];
			Lcd_WriteDat(ch);	      //显示年
		}
	}

	Int_To_Str(GPS.D.month,time);
	Lcd_SetPos(0, 3);
	if(strlen(time)==2)						//判断接收数据是否有效，有效则显示
	{
		i = 0;
		while(time[i] != '\0')
		{
			ch =time[i++];
			Lcd_WriteDat(ch);	
		}
	}
	Int_To_Str(GPS.D.day,time);
	Lcd_SetPos(0, 5);
	if(strlen(time)==2)					//判断接收数据是否有效，有效则显示
	{
		i = 0;
		while(time[i] != '\0')
		{
			ch =time[i++];
			Lcd_WriteDat(ch);	
		}
	}

	Int_To_Str(GPS.D.hour,time);
	Lcd_SetPos(1, 1);
	if(strlen(time)==2)				//判断接收数据是否有效，有效则显示
	{
		i = 0;
		while(time[i] != '\0')
		{
			ch =time[i++];
			Lcd_WriteDat(ch);	
		}
	}
	Lcd_WriteDat(' ');
	Lcd_WriteDat(':');

	Int_To_Str(GPS.D.minute,time);
	Lcd_SetPos(1, 3);
	if(strlen(time)==2)			//判断接收数据是否有效，有效则显示
	{
		i = 0;
		while(time[i] != '\0')
		{
			ch =time[i++];
			Lcd_WriteDat(ch);	
		}
	}
	Lcd_WriteDat(' ');
	Lcd_WriteDat(':');

	Int_To_Str(GPS.D.second,time);
	Lcd_SetPos(1, 5);
	if(strlen(time)==2)			//判断接收数据是否有效，有效则显示
	{
		i = 0;
		while(time[i] != '\0')
		{
			ch =time[i++];
			Lcd_WriteDat(ch);	
		}
	}
}


void GPS_DisplayOne(void)					 //第一屏
{
	ucharch, i;
	char info[10];
	ET0=0;								 //T0的溢出中断允许位，禁止T0的溢出中断
	clr_screen();//Lcd_WriteCmd(0x01);    //清屏

	GPS_DispTime();		//显示日期，时间

	if (GPS.NS == 'N')              //判断是北纬还是南纬
		Lcd_DispLine(2, 0, beiwei);
	else if (GPS.NS == 'S')
		Lcd_DispLine(2, 0, nanwei);

	if (GPS.EW == 'E')              //判断是东经还是西经
		Lcd_DispLine(3, 0, dongjing);
	else if (GPS.EW == 'W')
		Lcd_DispLine(3, 0, xijing);


	Int_To_Str(GPS.latitude_Degree,info);  //纬度
	Lcd_SetPos(2, 2);
	if(strlen(info)==2)
	{						  //只有正常显示纬度，才显示纬分
		i = 0;
		while(info[i] != '\0')
		{
			ch = info[i++];
			Lcd_WriteDat(ch);
		}
		Lcd_WriteDat(' ');
		Lcd_WriteDat(' ');
		Lcd_WriteDat(0xA1);
		Lcd_WriteDat(0xE3);		  //显示度的符号

		Int_To_Str(GPS.latitude_Cent,info);  //纬分
		if(strlen(info)==2)
		{					  //只有正常显示纬分，才显示纬秒
			i = 0;
			while(info[i] != '\0')
			{
				ch = info[i++];
				Lcd_WriteDat(ch);
			}
			Lcd_WriteDat(0xA1);
			Lcd_WriteDat(0xE4);			//显示秒的符号

			Int_To_Str(GPS.latitude_Second,info);  //纬秒
			if(strlen(info)==2)
			{
				i = 0;
				while(info[i] != '\0')
				{
					ch = info[i++];
					Lcd_WriteDat(ch);
				}
			}
		}	
	} 


	Int_To_Str(GPS.longitude_Degree,info);  //经度
	if(strlen(info)==3)
	{
		Lcd_DispLine(3, 2, info);
		Lcd_WriteDat(' ');
		Lcd_WriteDat(0xA1);
		Lcd_WriteDat(0xE3);

		Int_To_Str(GPS.longitude_Cent,info);  //经分
		if(strlen(info)==2)
		{
			Lcd_DispLine(3, 5, info);
			Lcd_WriteDat(0xA1);
			Lcd_WriteDat(0xE4);

			Int_To_Str(GPS.longitude_Second,info);  //经秒
			if(strlen(info)==2)
			{
				Lcd_DispLine(3, 7, info);
			}
		}
	} 
	ET0=1;
}


void GPS_DisplayTwo(void)					 //第二屏用于显示单位
{
	clr_screen();//Lcd_WriteCmd(0x01);    //清屏
	ET0=0;
	Lcd_DispLine(0, 0, sudu);
	Lcd_DispLine(1, 0, hangxiang);
	Lcd_DispLine(2, 0, gaodu);
	Lcd_DispLine(3, 0, haiba);

	Show_Float(GPS.speed, 0, 3);
	Lcd_DispLine(0, 6, kmperhour);

	Show_Float(GPS.direction, 1, 3);
	Lcd_DispLine(1, 6, du);

	Show_Float(GPS.height_ground, 2, 3);
	Lcd_DispLine(2, 6, meter);

	Show_Float(GPS.height_sea, 3, 3);
	Lcd_DispLine(3, 6, meter);
	ET0=1;	
}


void Show_Float(float fla, uchar x, uchar y)
{
	intintegar;
	char Info[10],ch;
	uchari;

	Lcd_SetPos(x, y);		
	integar = (int)fla;		// 显示整数部分
	Int_To_Str(fla,Info);  //显示整数部分
	i = 0;
	while(Info[i] !='\0')
	{
		ch=Info[i++];
		Lcd_WriteDat(ch);
	}
	Lcd_WriteDat('.');   //显示小数点

	fla = fla - integar; //显示小数部分

	fla =  fla * 10;     //0.1                	// 显示 0.1
	integar = (int) fla;
	fla = fla - integar;		// 改变fla的值,使fla总是小于1
	ch = integar + 0x30; 	
	Lcd_WriteDat(ch);

	fla =  fla*10;	    //0.01			// 显示 0.01
	integar = (int) fla;
	fla = fla - integar;		// 改变fla的值,使fla总是小于1
	ch = integar + 0x30 ; 
	Lcd_WriteDat(ch);
}

