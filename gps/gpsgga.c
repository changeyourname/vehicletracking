#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>     /*标准函数库定义*/
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>

//#define  dev  "/dev/ttyS0"
#define  BUFF_SIZE 512
#define  MAX_COM_NUM 3

int  SectionID=0,i=0;

struct data
{
	char GPS_time[20];         //UTC时间
	char GPS_sv[20];           //定位是否有效
	char GPS_wd[12];           //纬度
	char GPS_jd[12];           //经度
	char GPS_speed[5];         //速度
	char GPS_azimuth[20];	   //方位角
	char GPS_date[8];          //UTC日期          
	char GPS_declination[10];  //磁偏角
	char GPS_mode[10];	   //模式指示

}GPS_DATA;

struct data
{
	char time[20];     //UTC时间
	char wd[12];       //纬度
	char jd[12];       //经度
	char state[2];     //状态
	char number[20];   //正在使用的卫星数量
	char height[10];   //海平面高度

}GPS_GGA;

//设置串口
int set_com_config(int fd,int baud_rate,int data_bits,char parity,int stop_bits)
{
	struct termios new_cfg,old_cfg;
	int speed;
	//保存并测试现有串口参数设置，在这里如果串口号出错，会有相关的出错信息
	if(tcgetattr(fd,&old_cfg)!=0)
	{
		perror("tcgetattr");
		return -1;
	}
	tcflush(fd, TCIOFLUSH);
	new_cfg=old_cfg;
	cfmakeraw(&new_cfg);//配置为原始模式
	new_cfg.c_cflag&=~CSIZE;

	//设置波特率
	switch(baud_rate)
	{
		case 2400:
			{
				speed = B2400;
				break;
			}
		case 4800:
			{
				speed = B4800;
				break;
			}
		case 9600:
			{
				speed = B9600;
				break;
			}
		case 19200:
			{
				speed = B19200;
				break;
			}
		case 38400:
			{
				speed = B38400;
				break;
			}
		case 115200:
			{
				speed = B115200;
				break; 
			}
	}
	cfsetispeed(&new_cfg,speed);
	cfsetospeed(&new_cfg,speed);

	//设置数据位
	switch(data_bits)
	{
		case 7:
			{
				new_cfg.c_cflag|=CS7;
				break;
			}
		case 8:
			{
				new_cfg.c_cflag|=CS8;
				break;
			}
	}

	//设置停止位
	switch(stop_bits)
	{
		case 1:
			{
				new_cfg.c_cflag &=~CSTOPB;
				break;
			}
		case 2:
			{
				new_cfg.c_cflag |=CSTOPB;
				break;
			}
	}

	//设置奇偶校验位
	switch(parity)
	{
		case 'o':
		case 'O':
			{
				new_cfg.c_cflag|=(PARODD|PARENB);
				new_cfg.c_iflag|=(INPCK |ISTRIP);
				break;
			}
		case 'e':
		case 'E':
			{
				new_cfg.c_cflag |=PARENB;
				new_cfg.c_cflag &=~PARODD;
				new_cfg.c_iflag |=(INPCK | ISTRIP);
				break;
			}
		case 's':
		case 'S':
			{
				new_cfg.c_cflag &=~PARENB;
				new_cfg.c_cflag &=~CSTOPB;
				break;
			}
		case 'n':
		case 'N':
			{
				new_cfg.c_cflag &=~PARENB;
				new_cfg.c_iflag &=~INPCK;
				break;
			}
	}

	new_cfg.c_cc[VTIME] =10;
	new_cfg.c_cc[VMIN] =5;
	//处理未接收字符
	tcflush(fd,TCIFLUSH);

	if((tcsetattr(fd,TCSANOW,&new_cfg))!=0)
	{
		perror("tcsetattr");
		return -1;
	}

	return 0;
}

//打开串口函数
int open_port(int com_port)
{
	int fd;
#if 0
#if (COM_TYPE == GNR_COM)  //使用普通串口
	char* dev[] = {"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2"};
#else//使用USB转串口
	char* dev[] = {"/dev/ttyUSB0","/dev/ttyUSB1","/dev/ttyUSB2"};
#endif
	if((com_port<0)||(com_port > MAX_COM_NUM))
	{
		return -1;
	}
	//打开串口
	if((fd=open(dev[com_port-1],O_RDWR|O_NOCTTY|O_NDELAY))<0)
	{
		perror("open serial port");
		return -1;
	}
#endif
	if((fd=open("/dev/ttyUSB0",O_RDWR|O_NOCTTY|O_NDELAY))<0)
	{
		perror("open serial port");
		return -1;
	}
	//恢复串口为堵塞状态
	if(fcntl(fd,F_SETFL,0) <0 )
	{
		perror("fcntl F_SETFL\n");
		return -1;
	}
	//测试是否为终端设备
	if(isatty(STDIN_FILENO)==0)
	{
		perror("standard input is not a terminal device");
	}

	return fd;
}

//UTC时间转换为北京时间
void utc_to_beijing(void)
{
	unsigned char temp,hour_shi,hour_ge;
	temp=(GPS_DATA.GPS_time[0]-'0')*10+(GPS_DATA.GPS_time[1]-'0');
	if(temp<=16)
	{
		temp=temp+8;
		hour_shi=temp/10;
		hour_ge=temp%10;
		GPS_DATA.GPS_time[0]=hour_shi+'0';
		GPS_DATA.GPS_time[1]=hour_ge+'0';	
	}
	else
	{
		temp=temp+8-24;
		GPS_DATA.GPS_time[0]='0';
		GPS_DATA.GPS_time[1]=temp%10+'0';	
	}
}

//往屏幕上打印解析后的信息
void print_info(void)
{
	//打印选择界面，即引导的字符号
	printf("Now the receive time is: %s\n",GPS_DATA.GPS_time);
	//	printf("The star is %c 3\n",GPS_DATA.GPS_sv);
	printf("The state is: 		 %s\n",GPS_DATA.GPS_sv);
	printf("The earth latitude is:   %s\n",GPS_DATA.GPS_wd);
	printf("The earth longitude is:  %s\n",GPS_DATA.GPS_jd);	
	printf("The train speed is: 	 %s\n",(GPS_DATA.GPS_speed));
	printf("The azimuth is: 	 %s\n",(GPS_DATA.GPS_azimuth));
	printf("The date is:		 %s\n",GPS_DATA.GPS_date);
	printf("The declination is:	 %s\n",GPS_DATA.GPS_declination);
	printf("The mode is:		 %s\n",GPS_DATA.GPS_mode);
}

//提取GPRMC中有用的数据
void GPS_resolve_GPRMC(char data)
{
	//$GPRMC,064851.890,A,2239.4457,N,11400.9571,E,0.68,76.18,270116,,,A*52
	if(data==',')
	{
		++SectionID;
		i=0;
	}
	else
	{
		switch(SectionID)
		{
			case 1:		
				GPS_DATA.GPS_time[i++]=data;		
				if(i==2 || i==5)
				{		
					GPS_DATA.GPS_time[i++]=':';		
				}				
				GPS_DATA.GPS_time[8]='\0';
				break;
			case 2:
				if(data=='A')
				{
					GPS_DATA.GPS_sv[0]='\0';
					strcpy(GPS_DATA.GPS_sv,"Effective location");
				}
				else
				{
					GPS_DATA.GPS_sv[0]='\0';
					strcpy(GPS_DATA.GPS_sv,"Invalid location");
				}
				break;
			case 3:
				GPS_DATA.GPS_wd[++i]=data;	
				GPS_DATA.GPS_wd[12]='\0';					
				break;

			case 4:
				if(data=='N')
					GPS_DATA.GPS_wd[0]='N';
				else if(data=='S')
					GPS_DATA.GPS_wd[0]='S';

				break;
			case 5:

				GPS_DATA.GPS_jd[++i]=data;	
				GPS_DATA.GPS_jd[12]='\0';
				break;

			case 6:
				if(data=='E')
					GPS_DATA.GPS_jd[0]='E';
				else if(data=='W')
					GPS_DATA.GPS_jd[0]='W';

				break;
			case 7:
				GPS_DATA.GPS_speed[i++]=data;
				GPS_DATA.GPS_speed[5]='\0';						
				break;
			case 8:
				GPS_DATA.GPS_azimuth[i++]=data;
				GPS_DATA.GPS_azimuth[6]='\0';
				break;

			case 9:
				GPS_DATA.GPS_date[i++]=data;	
				if(i==2 || i==5)						
				{
					GPS_DATA.GPS_date[i++]='-';
				}								
				GPS_DATA.GPS_date[8]='\0';					
				break;
			case 10:
				GPS_DATA.GPS_declination[++i]=data;	
				GPS_DATA.GPS_declination[4]='\0';	
				break;
			case 11:
				GPS_DATA.GPS_declination[0]=data;	
				break;
			case 12:
				if(data=='A')
					strcpy(GPS_DATA.GPS_mode,"自主定位");
				else if(data=='D')
					strcpy(GPS_DATA.GPS_mode,"差分");
				else if(data=='E')
					strcpy(GPS_DATA.GPS_mode,"估算");
				else
					strcpy(GPS_DATA.GPS_mode,"数据无效");
		}
	}		
}

//提取GPGGA中有用的数据
void GPS_resolve_GPGGA(char data)
{
	//$GPGGA,064851.890,2239.4457,N,11400.9571,E,1,04,3.9,113.4,M,-2.4,M,,0000*47

	if(data==',')
	{
		++SectionID;
		i=0;
	}
	else
	{
		switch(SectionID)
		{
			case 1:		
				GPS_DATA.GPS_time[i++]=data;		
				if(i==2 || i==5)
				{		
					GPS_DATA.GPS_time[i++]=':';		
				}				
				GPS_DATA.GPS_time[8]='\0';
				break;
			case 2:
				GPS_DATA.GPS_wd[++i]=data;	
				GPS_DATA.GPS_wd[12]='\0';					
				break;
			case 3:
				if(data=='N')
					GPS_DATA.GPS_wd[0]='N';
				else if(data=='S')
					GPS_DATA.GPS_wd[0]='S';
				break;

			case 4:
				GPS_DATA.GPS_jd[++i]=data;	
				GPS_DATA.GPS_jd[12]='\0';

				break;
			case 5:
				if(data=='E')
					GPS_DATA.GPS_jd[0]='E';
				else if(data=='W')
					GPS_DATA.GPS_jd[0]='W';

				break;

			case 6:
				if(data=='0'){
	
}else if(data=='1'){

}else if(data=='2'){

}else if(data=='3'){

}else
strcpy();
				break;
			case 7:
				GPS_DATA.GPS_speed[i++]=data;
				GPS_DATA.GPS_speed[5]='\0';						
				break;
			case 8:
				GPS_DATA.GPS_azimuth[i++]=data;
				GPS_DATA.GPS_azimuth[6]='\0';
				break;

			case 9:
				GPS_DATA.GPS_date[i++]=data;	
				if(i==2 || i==5)						
				{
					GPS_DATA.GPS_date[i++]='-';
				}								
				GPS_DATA.GPS_date[8]='\0';					
				break;
			case 10:
				GPS_DATA.GPS_declination[++i]=data;	
				GPS_DATA.GPS_declination[4]='\0';	
				break;
			case 11:
				GPS_DATA.GPS_declination[0]=data;	
				break;
			case 12:
				if(data=='A')
					strcpy(GPS_DATA.GPS_mode,"自主定位");
				else if(data=='D')
					strcpy(GPS_DATA.GPS_mode,"差分");
				else if(data=='E')
					strcpy(GPS_DATA.GPS_mode,"估算");
				else
					strcpy(GPS_DATA.GPS_mode,"数据无效");
		}
	}		
}

void read_data(int fd)
{
	char buffer[BUFF_SIZE],dest[1024]; 	 
	char array[10]="$GPRMC";
	int  res,i=0,j=0,k;
	int  data=1,len=0;
	memset(dest,0,sizeof(dest));
	do
	{	 
		memset(buffer,0,sizeof(buffer));
		//$GPRMC,064851.890,A,2239.4457,N,11400.9571,E,0.68,76.18,270116,,,A*52
		if(res=read(fd,buffer,1)>0) //每次读一个字符到buffer
		{		
			strcat(dest,buffer); //把buffer中的那个字符追加到dest
			if(buffer[0]=='\n') //接收完一句,就进入下面进行处理
			{
				i=0;
				if(strncmp(dest,array,6)==0)
				{				
					printf("\n--------------------------------------------------------\n");
					printf("%s",dest);
					len=strlen(dest);
					for(k=0;k<len;k++)
					{
						GPS_resolve_GPRMC(dest[k]);
					}		
					SectionID=0;

					utc_to_beijing();
					print_info();
				}
				bzero(dest,sizeof(dest));
			}
		}
	}while(1);
	close(fd);
}

int main(int argc,char*argv[])
{		
	int fd=0;	   
	int HOST_COM_PORT=1;	 
	fd=open_port(HOST_COM_PORT);
	if(fd<0)	
	{
		perror("open fail!");
	}
	printf("open sucess!\n");
	if((set_com_config(fd,4800,8,'N',1))<0)	
	{
		perror("set_com_config fail!\n");
	}
	printf("The received worlds are:\n");
	read_data(fd);
	return 0;
}
