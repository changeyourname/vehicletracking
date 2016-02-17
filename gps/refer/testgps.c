#include "GPS.h"
#include "LCD.h"
#include <string.h>

uchar code init1[] = {"BASE-MCU GPS终端"};
uchar code init2[] = {"商院电信 091班 "};
uchar code init3[] = {"GPS 初始化......"};
uchar code init4[] = {"搜索定位卫星...."};

static ucharGetComma(ucharnum,char* str);
static double Get_Double_Number(char *s);	  //得到双精度的数据
static float  Get_Float_Number(char *s); 	 //得到单精度的数据
static void   UTC2BTC(DATE_TIME *GPS);		 //格林时间到北京时间的转换

void GPS_Init(void)			   //GPS初始化
{
	Lcd_DispLine(0, 0, init1);
	Lcd_DispLine(1, 0, init2);
	Lcd_DispLine(2, 0, init3);
	Lcd_DispLine(3, 0, init4);
}

intGPS_RMC_Parse(char *line,GPS_INFO *GPS)	//运输定位数据最大帧70
{						   //*GPS为结构体类型的指针
	ucharch, status, tmp;
	float lati_cent_tmp, lati_second_tmp;	 //经度的分、秒临时变量
	float long_cent_tmp, long_second_tmp;	 //纬度的分、秒临时变量
	float speed_tmp;			 //速度的临时变量
	char *buf = line;
	ch = buf[5];
	status = buf[GetComma(2, buf)];		 //定位状态给status，判断是否是A，

	if (ch == 'C')  //如果第五个字符是C，($GPRMC)推荐最小定位信息
	{
		if (status == 'A')  //如果数据有效，则分析	   ‘V’无效，‘A’有效
		{
			GPS -> NS       = buf[GetComma(4, buf)];
			GPS -> EW       = buf[GetComma(6, buf)];

			GPS->latitude   = Get_Double_Number(&buf[GetComma(3, buf)]);	//经度
			GPS->longitude  = Get_Double_Number(&buf[GetComma(5, buf)]);    //纬度

			GPS->latitude_Degree  = (int)GPS->latitude / 100;       //分离纬度
			lati_cent_tmp         = (GPS->latitude - GPS->latitude_Degree * 100);
			GPS->latitude_Cent    = (int)lati_cent_tmp;
			lati_second_tmp       = (lati_cent_tmp - GPS->latitude_Cent) * 60;
			GPS->latitude_Second  = (int)lati_second_tmp;

			GPS->longitude_Degree = (int)GPS->longitude / 100;	//分离经度
			long_cent_tmp         = (GPS->longitude - GPS->longitude_Degree * 100);
			GPS->longitude_Cent   = (int)long_cent_tmp;    
			long_second_tmp       = (long_cent_tmp - GPS->longitude_Cent) * 60;
			GPS->longitude_Second = (int)long_second_tmp;

			speed_tmp      = Get_Float_Number(&buf[GetComma(7, buf)]);    //速度(单位：海里/时)
			GPS->speed     = speed_tmp * 1.85;                           //1海里=1.85公里
			GPS->direction = Get_Float_Number(&buf[GetComma(8, buf)]); //角度			

			GPS->D.hour    = (buf[7] - '0') * 10 + (buf[8] - '0');		//时间
			GPS->D.minute  = (buf[9] - '0') * 10 + (buf[10] - '0');
			GPS->D.second  = (buf[11] - '0') * 10 + (buf[12] - '0');
			tmp = GetComma(9, buf);
			GPS->D.day     = (buf[tmp + 0] - '0') * 10 + (buf[tmp + 1] - '0'); //日期
			GPS->D.month   = (buf[tmp + 2] - '0') * 10 + (buf[tmp + 3] - '0');
			GPS->D.year    = (buf[tmp + 4] - '0') * 10 + (buf[tmp + 5] - '0')+2000;

			UTC2BTC(&GPS->D);

			return 1;
		}		
	}
	return 0;
}

intGPS_GGA_Parse(char *line,GPS_INFO *GPS)			//全球定位数据，最大帧为72
{								//丛这个数据得到海拔的数据
	ucharch, status;
	char *buf = line;
	ch = buf[4];
	status = buf[GetComma(2, buf)];

	if (ch == 'G')           //$GPGGA	丛这个数据得到海拔的数据
	{
		if (status != ',')
		{
			GPS->height_sea = Get_Float_Number(&buf[GetComma(9, buf)]);
			GPS->height_ground = Get_Float_Number(&buf[GetComma(11, buf)]);
			return 1;
		}
	}
	return 0;
}

static float Str_To_Float(char *buf)	  //字符串到单精度的转换函数
{
	float rev = 0;
	float dat;
	int integer = 1;
	char *str = buf;		 //
	inti;
	while(*str != '\0')
	{
		switch(*str)
		{
			case '0':
				dat = 0;
				break;
			case '1':
				dat = 1;
				break;
			case '2':
				dat = 2;
				break;		
			case '3':
				dat = 3;
				break;
			case '4':
				dat = 4;
				break;
			case '5':
				dat = 5;
				break;
			case '6':
				dat = 6;
				break;
			case '7':
				dat = 7;
				break;
			case '8':
				dat = 8;
				break;
			case '9':
				dat = 9;
				break;
			case '.':
				dat = '.';
				break;
		}
		if(dat == '.')
		{
			integer = 0;
			i = 1;
			str ++;
			continue;
		}
		if( integer == 1 )
		{
			rev = rev * 10 + dat;
		}
		else
		{
			rev = rev + dat / (10 * i);
			i = i * 10 ;
		}
		str ++;
	}
	return rev;

}

static float Get_Float_Number(char *s)			
{
	char buf[10];
	uchari;
	float rev;
	i=GetComma(1, s);
	i = i - 1;
	strncpy(buf, s, i);
	buf[i] = 0;
	rev=Str_To_Float(buf);
	return rev;	
}

static double Str_To_Double(char *buf)			  //字符串到双精度的转换函数
{
	double rev = 0;
	double dat;
	int integer = 1;
	char *str = buf;
	inti;
	while(*str != '\0')
	{
		switch(*str)
		{
			case '0':
				dat = 0;
				break;
			case '1':
				dat = 1;
				break;
			case '2':
				dat = 2;
				break;		
			case '3':
				dat = 3;
				break;
			case '4':
				dat = 4;
				break;
			case '5':
				dat = 5;
				break;
			case '6':
				dat = 6;
				break;
			case '7':
				dat = 7;
				break;
			case '8':
				dat = 8;
				break;
			case '9':
				dat = 9;
				break;
			case '.':
				dat = '.';
				break;
		}
		if(dat == '.')
		{
			integer = 0;
			i = 1;
			str ++;
			continue;
		}
		if( integer == 1 )
		{
			rev = rev * 10 + dat;
		}
		else
		{
			rev = rev + dat / (10 * i);
			i = i * 10 ;
		}
		str ++;
	}
	return rev;
}

static double Get_Double_Number(char *s)
{
	char buf[10];
	uchari;
	double rev;
	i=GetComma(1, s);
	i = i - 1;
	strncpy(buf, s, i);
	buf[i] = 0;
	rev=Str_To_Double(buf);
	return rev;	
}

static ucharGetComma(ucharnum,char *str)
{
	uchari,j = 0;
	intlen=strlen(str);				   //字符串的长度

	for(i = 0;i <len;i ++)
	{
		if(str[i] == ',')			   //计算字符串中的逗号的个数
			j++;
		if(j == num)
			return i + 1;			   //把逗号后的字符下标值返回
	}

	return 0;	
}

static void UTC2BTC(DATE_TIME *GPS)
{
	GPS->second ++;  
	if(GPS->second > 59)
	{
		GPS->second = 0;
		GPS->minute ++;
		if(GPS->minute > 59)
		{
			GPS->minute = 0;
			GPS->hour ++;
		}
	}	

	GPS->hour = GPS->hour + 8;
	if(GPS->hour > 23)
	{
		GPS->hour -= 24;
		GPS->day += 1;
		if(GPS->month == 2 ||
				GPS->month == 4 ||
				GPS->month == 6 ||
				GPS->month == 9 ||
				GPS->month == 11 )
		{
			if(GPS->day > 30)
			{
				GPS->day = 1;
				GPS->month++;
			}
		}
		else
		{
			if(GPS->day > 31)
			{	
				GPS->day = 1;
				GPS->month ++;
			}
		}
		if(GPS->year % 4 == 0 )
		{
			if(GPS->day > 29 && GPS->month == 2)
			{		
				GPS->day = 1;
				GPS->month ++;
			}
		}
		else
		{
			if(GPS->day > 28 &&GPS->month == 2)
			{
				GPS->day = 1;
				GPS->month ++;
			}
		}
		if(GPS->month > 12)
		{
			GPS->month -= 12;
			GPS->year ++;
		}		
	}
}

void Int_To_Str(intx,char *Str)
{
	int t;
	char *Ptr,Buf[5];
	inti = 0;
	Ptr = Str;
	if(x < 10)		// 当整数小于10时,转化为"0x"的格式
	{
		*Ptr ++ = '0';
		*Ptr ++ = x+0x30;
	}
	else
	{
		while(x > 0)
		{
			t = x % 10;
			x = x / 10;
			Buf[i++] = t+0x30;	// 通过计算把数字转化成ASCII码形式
		}
		i -- ;
		for(;i>= 0;i --) 		// 将得到的字符串倒序
		{
			*(Ptr++) = Buf[i];
		}
	}
	*Ptr = '\0';
}
