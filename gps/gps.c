#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>     /*标准函数库定义*/
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>

#include <sqlite3.h>

#define  BUFF_SIZE 512

int  SectionID=0,i=0;

struct data{
	int hour;		//UTC时间
	int minute;
	int second; 
	int year;		//UTC日期
	int month;
	int day;
	char check[20];         //定位是否有效
	char latitude[12];      //纬度
	char longitude[12];     //经度

}GPS;

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
int open_port(void)
{
	int fd;
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
	GPS.hour += 8;
	if(GPS.hour > 23){
		GPS.hour -= 24;
		GPS.day ++;
		switch(GPS.month){
			case 2: if((GPS.year%4==0&&GPS.year%100!=0)||GPS.year%400==0){
					if(GPS.day < 30)
						break;
				}
				else{
					if(GPS.day <29)
						break;
				}
			case 4:
			case 6:
			case 9:
			case 11: if(GPS.day<31)
				 	break;
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12: if(GPS.day < 32){
				 	break;
				 }
			default:
				 	GPS.day = 1;
					GPS.month++;

		}
		if(GPS.month > 12){
			GPS.month = 1;
			GPS.year++;
		}
	}

}

//往屏幕上打印解析后的信息
void print_info(void)
{
	//打印选择界面，即引导的字符号
	printf("Now the receive Beijing time is: %d/%d/%d %d:%d:%d\n",GPS.year,GPS.month,GPS.day,GPS.hour,GPS.minute,GPS.second);
	printf("The state is: 		 %s\n",GPS.check);
	printf("The earth latitude is:   %s\n",GPS.latitude);
	printf("The earth longitude is:  %s\n",GPS.longitude);	
}

//提取GPRMC中有用的数据
void GPS_resolve_GPRMC(char data)
{
	//$GPRMC,064851.890,A,2239.4457,N,11400.9571,E,0.68,76.18,270116,,,A*52
	char temp_time[20];
	char temp_date[20];
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
				temp_time[i++]=data;		
				temp_time[6]='\0';
				break;
			case 2:
				if(data=='A')
				{
					GPS.check[0]='\0';
					strcpy(GPS.check,"Effective");
				}
				else
				{
					GPS.check[0]='\0';
					strcpy(GPS.check,"Invalid");
				}
				break;
			case 3:
				GPS.latitude[++i]=data;	
				GPS.latitude[12]='\0';					
				break;

			case 4:
				if(data=='N')
					GPS.latitude[0]='N';
				else if(data=='S')
					GPS.latitude[0]='S';
				break;
			case 5:

				GPS.longitude[++i]=data;	
				GPS.longitude[12]='\0';
				break;

			case 6:
				if(data=='E')
					GPS.longitude[0]='E';
				else if(data=='W')
					GPS.longitude[0]='W';
				break;
			case 9:
				temp_date[i++]=data;	
				temp_date[6]='\0';					
				break;
			case 12:
				GPS.hour = (temp_time[0] - '0') * 10 + temp_time[1] - '0';
				GPS.minute = (temp_time[2] - '0') * 10 + temp_time[3] - '0';
				GPS.second = (temp_time[4] - '0') * 10 + temp_time[5] - '0';
				GPS.year = (temp_date[4] - '0') * 10 + temp_date[5] - '0' + 2000;
				GPS.month = (temp_date[2] - '0') * 10 + temp_date[3] - '0';
				GPS.day = (temp_date[0] - '0') * 10 + temp_date[1] - '0';
				break;
		}
	}		
}

//读取数据，并把数据存到数据库
void read_data(int fd)
{
	sqlite3 *db;
	int ret;
	char *errmsg;
	char sql[200] = {'\0'};

	char buffer[BUFF_SIZE],dest[1024]; 	 
	char array[10]="$GPRMC";
	int  res,i=0,j=0,k;
	int  data=1,len=0;
	memset(dest,0,sizeof(dest));
	
	sqlite3_open("./data.db",&db);

	sprintf(sql, "create table if not exists gpsdata(id integer primary key, status varchar(10), latitude varchar(10), longitude varchar(10), time timestamp not null default (datetime('now','localtime')))");
	ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if(SQLITE_OK != ret){
		printf("create:%s\n", errmsg);
	        exit(0);
	}

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
					printf("%s",dest);
					len=strlen(dest);
					for(k=0;k<len;k++)
					{
						GPS_resolve_GPRMC(dest[k]);
					}		
					SectionID=0;

					utc_to_beijing();
					print_info(); 		//往屏幕上输出提取的信息

				        sprintf(sql, "insert into gpsdata(status, latitude, longitude) values('%s', '%s', '%s')",GPS.check, GPS.latitude, GPS.longitude);
				        ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
				        if(SQLITE_OK != ret){
				                printf("insert:%s\n", errmsg);
				                exit(0);
				        }
				}
				bzero(dest,sizeof(dest));
			}
		}
	}while(1);
	close(fd);
	sqlite3_close(db);
}

int main(int argc,char*argv[])
{		
	int fd=0;	   
	fd=open_port();
	if(fd<0)	
	{
		perror("open_port fail!");
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

