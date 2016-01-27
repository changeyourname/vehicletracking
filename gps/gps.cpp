//获取子字符串个数
int GetSubStrCount(CString str,char cFlag)
{
	cFlag = ',';
	int i = 0;
	BOOL isHas = FALSE;

	for (int iStart = -1; -1 != (iStart = str.Find(cFlag,iStart+1)) ; i++)
	{
		isHas = TRUE;
	}

	if (!isHas)
	{
		return 0;
	}
	else
	{
		return i+1;
	}
}

//获取子字符串
// i 序号 0
CString GetSubStr(CString str,int i,char cFlag)
{
	cFlag = ',';
	int iStart = -1;
	int iEnd = 0;
	int j = 0;
	int iStrCount;

	iStrCount = GetSubStrCount(str,cFlag);

	if (i>iStrCount -1 || i<0)
	{
		str = "";
		return str;
	}
	else
	{
		//do nothing
	}

	if (i == iStrCount-1)
	{
		i = iStrCount;

		for (;j<i-1;j++)
		{
			iStart = str.Find(cFlag , iStart+1);
		}

		return   str.Mid(iStart+1 , str.GetLength()-iStart-1);
	}
	else
	{
		//do nothing
	}

	for (; j<i; j++)
	{
		iStart = str.Find(cFlag , iStart+1);
	}

	iEnd = str.Find(cFlag , iStart+1); 
	return str.Mid(iStart+1 , iEnd-iStart-1);
}

//数据解析
CString CGpsDataView::Analyzing(CString str)
{
	CString subStr[20];
	char cFlag = ',';
	int j = GetSubStrCount(str,cFlag);          //得到该行的子字符串个数
	CStdioFile wFile;
	wFile.Open("save.txt",CFile::modeCreate | CFile::modeWrite | CFile::typeText);//将数据写入文件

	for (int i=0;i<j;i++)
	{
		subStr[i] = GetSubStr(str,i,cFlag);                                           
	}

	//GPGGA数据
	if (subStr[0] == "$GPGGA")
	{
		CoordCovert(subStr[2],subStr[4]);
		//提取时间
		subStr[1].Insert(2,':');
		subStr[1].Insert(5,':');
		subStr[1].Insert(0," UTC时间:");

		//提取纬度
		if (subStr[3] == 'N')
		{
			subStr[2].Insert(11,"分");
			subStr[2].Insert(2,"度");
			subStr[2].Insert(0,"  北纬");
		}
		else if (subStr[3] == 'S')
		{
			subStr[2].Insert(11,"分");
			subStr[2].Insert(2,"度");
			subStr[2].Insert(0,"  南纬");
		}

		//提取经度
		if (subStr[5] == 'E')
		{
			subStr[4].Insert(12,"分");
			subStr[4].Insert(3,"度");
			subStr[4].Insert(0,"  东经");
		}
		else if (subStr[5] == 'W')
		{
			subStr[4].Insert(12,"分");
			subStr[4].Insert(3,"度");
			subStr[4].Insert(0,"  西经");
		}

		//判断GPS状态
		CString GpsState;

		if (subStr[6] == '0')
		{
			GpsState = "  GPS状态:无定位.";
		}
		else if (subStr[6] == '1')
		{
			GpsState = "  GPS状态:无差分校正定位.";
		}
		else if (subStr[6] == '2')
		{
			GpsState = "  GPS状态:差分校正定位.";
		}
		else if (subStr[6] == '9')
		{
			GpsState = "  GPS状态:用星历计算定位.";
		}

		//提取卫星数
		subStr[7].Insert(0," 卫星数:");      

		//提取平面位置精度因子
		subStr[8].Insert(0,"  平面位置精度因子:");          

		//天线海拔高度
		subStr[9].Insert(strlen(subStr[9]),subStr[10]);
		subStr[9].Insert(0," 天线海拔高度:");

		//海平面分离度
		subStr[11].Insert(strlen(subStr[11]),subStr[12]);
		subStr[11].Insert(0," 海平面分离度:");

		subStr[0] += subStr[1];
		subStr[0] += subStr[2];
		subStr[0] += subStr[4];
		subStr[0] += GpsState;
		subStr[0] += subStr[7];
		subStr[0] += subStr[8];
		subStr[0] += subStr[9];
		subStr[0] += subStr[11];
		//////////////////////////////////////MessageBox(subStr[0]);
		wFile.WriteString(subStr[0]);//将数据写入文件

	}

	//GPZDA数据
	else if (subStr[0] == "$GPZDA")
	{
		//提取时间
		subStr[1].Insert(2,':');
		subStr[1].Insert(5,':');
		subStr[1].Insert(0," UTC时间:");

		//提取日期
		subStr[2].Insert(strlen(subStr[2]),"日");
		subStr[2].Insert(0,"月");
		subStr[2].Insert(0,subStr[3]);
		subStr[2].Insert(0,"年");
		subStr[2].Insert(0,subStr[4]); 
		subStr[2].Insert(0,' ');

		//当地时域描述
		subStr[5].Insert(strlen(subStr[5]),"小时");

		if (strlen(subStr[6]) > 3)
		{
			subStr[6] = subStr[6].Left(2);
		}
		else
		{
			subStr[6] = '0';
		}

		subStr[6] += "分";
		subStr[6].Insert(0,subStr[5]);
		subStr[6].Insert(0," 当地时域:");

		subStr[0] += subStr[1];
		subStr[0] += subStr[2];
		subStr[0] += subStr[6];
		//////////////////////////////MessageBox(subStr[0]);
		wFile.WriteString(subStr[0]);//将数据写入文件
	}

	//GPGSA数据
	else if (subStr[0] == "$GPGSA")
	{
		//卫星捕获模式，以及定位模式
		CString CatchLocation;

		if (subStr[1] == 'M')
		{
			if (subStr[2] == '1')
			{
				CatchLocation = " 手动捕获卫星，未定位!";
			}
			else if (subStr[2] == '2')
			{
				CatchLocation = "  手动捕获卫星，2D定位!";
			}
			else if (subStr[2] == '3')
			{
				CatchLocation = "  手动捕获卫星，3D定位!";
			}
		}
		else if (subStr[1] == 'A')
		{
			if (subStr[2] == '1')
			{
				CatchLocation ="  自动捕获卫星，未定位!";
			}
			else if (subStr[2] == '2')
			{
				CatchLocation ="  自动捕获卫星，2D定位!";
			}
			else if (subStr[2] == '3')
			{
				CatchLocation ="  自动捕获卫星，3D定位!";
			}
		}

		//各卫星定位结果
		subStr[3].Insert(0,"  各卫星定位结果:");
		subStr[3] += ' ';
		subStr[4].Insert(0,subStr[3]);
		subStr[4] += ' ';
		subStr[5].Insert(0,subStr[4]);
		subStr[5] += ' ';
		subStr[6].Insert(0,subStr[5]);
		subStr[6] += ' ';
		subStr[7].Insert(0,subStr[6]);
		subStr[7] += ' ';
		subStr[8].Insert(0,subStr[7]);
		subStr[8] += ' ';
		subStr[9].Insert(0,subStr[8]);
		subStr[9] += ' ';
		subStr[10].Insert(0,subStr[9]);
		subStr[10] += ' ';
		subStr[11].Insert(0,subStr[10]);
		subStr[11] += ' ';
		subStr[12].Insert(0,subStr[11]);
		subStr[12] += ' ';
		subStr[13].Insert(0,subStr[12]);
		subStr[13] += ' ';
		subStr[14].Insert(0,subStr[13]);
		subStr[14] += ' ';

		//空间（三维）位置精度因子
		subStr[15].Insert(0,"  空间（三维）位置精度因子:");

		//平面位置精度因子
		subStr[16].Insert(0,"   平面位置精度因子:");

		//高度位置精度因子
		subStr[17] = subStr[17].Left(3);
		subStr[17].Insert(0,"  高度位置精度因子:");

		subStr[0] += CatchLocation;
		subStr[0] += subStr[14];
		subStr[0] += subStr[15];
		subStr[0] += subStr[16];
		subStr[0] += subStr[17];
		/////////////////////////////MessageBox(subStr[0]);
		wFile.WriteString(subStr[0]);//将数据写入文件                    
	}

	//GPGSV数据
	else if (subStr[0] == "$GPGSV")
	{
		///////////////////////////MessageBox(subStr[0]);

		//卫星编号、卫星仰角（0~90度）、卫星方位角（0~359度）、信噪比
		subStr[4].Insert(0,"卫星编号:");
		subStr[5].Insert(0," 仰角:");
		subStr[6].Insert(0," 方位角:");
		subStr[7].Insert(0," 信噪比:");
		subStr[4] += subStr[5];
		subStr[4] += subStr[6];
		subStr[4] += subStr[7];
		///////////////////MessageBox(subStr[4]);
		subStr[8].Insert(0,"卫星编号:");
		subStr[9].Insert(0," 仰角:");
		subStr[10].Insert(0," 方位角:");
		subStr[11].Insert(0," 信噪比:");
		subStr[8] += subStr[9];
		subStr[8] += subStr[10];
		subStr[8] += subStr[11];
		////////////////////////MessageBox(subStr[8]);
		subStr[12].Insert(0,"卫星编号:");
		subStr[13].Insert(0," 仰角:");
		subStr[14].Insert(0," 方位角:");
		subStr[15].Insert(0," 信噪比:");
		subStr[12] += subStr[13];
		subStr[12] += subStr[14];
		subStr[12] += subStr[15];
		/////////////////////MessageBox(subStr[12]);
		subStr[16].Insert(0,"卫星编号:");
		subStr[17].Insert(0," 仰角:");
		subStr[18].Insert(0," 方位角:");

		if (strlen(subStr[19]) > 3)
		{
			subStr[19] = subStr[19].Left(2);
		}
		else
		{
			subStr[19] = '0';
		}

		subStr[19].Insert(0," 信噪比:");
		subStr[16] += subStr[17];
		subStr[16] += subStr[18];    
		subStr[16] += subStr[19];
		/////////////////////////////////MessageBox(subStr[16]);
		wFile.WriteString(subStr[16]);//将数据写入文件
	}
	return str;
}


//读取文件数据并解析
void CGpsDataView::OnFileRead()
{
	// TODO: 在此添加命令处理程序代码
	CStdioFile myFile;
	CString oneLine;    
	char cFlag = ',';
	CString subStr[20];

	//读取GPS数据文件
	if(!myFile.Open(("gps.txt"),CFile::modeRead | CFile::typeText))
	{
		AfxMessageBox(_T("打开文件错误!"));
		return;
	}
	else
	{
		/*do nothing*/
	}

	while (myFile.ReadString(oneLine))//读一行
	{            
		//////////MessageBox(oneLine);
		int j = GetSubStrCount(oneLine,cFlag);         //得到该行的子字符串个数

		//校验
		if(CheckNum(oneLine))
		{
			////////////MessageBox(_T("数据校验...接收正确!..."));

			for (int i=0;i<j;i++)
			{
				subStr[i] = GetSubStr(oneLine,i,cFlag);
				//MessageBox(subStr[i]);                          
			}

			Analyzing(oneLine);        //解析
		}
		else
		{
			AfxMessageBox(_T("数据校验..接收错误!..."));
		}
	}

	myFile.Close();
}

//***********************************************************************************************
//坐标转换

//度分秒--弧度
double Dms2Rad(double Dms)
{
	double Degree, Miniute;
	double Second;
	int   Sign;
	double Rad;

	if(Dms >= 0)
	{
		Sign = 1;
	}
	else
	{
		Sign = -1;
	}

	Dms = fabs(Dms); //绝对值
	Degree = floor(Dms); // 取度 floor（2.800） = 2.0000
	Miniute = floor(fmod(Dms * 100.0, 100.0)); //fmod 计算余数
	Second  = fmod(Dms * 10000.0, 100.0);
	Rad = Sign * (Degree + Miniute / 60.0 + Second / 3600.0) * PI / 180.0;
	return Rad;
}

double Rad2Dms(double Rad)
{
	double Degree, Miniute;
	double Second;
	int   Sign;
	double Dms;

	if(Rad >= 0)
	{
		Sign = 1;
	}
	else
	{
		Sign = -1;
	}

	Rad   = fabs(Rad * 180.0 / PI);
	Degree   = floor(Rad);
	Miniute  = floor(fmod(Rad * 60.0, 60.0));
	Second   = fmod(Rad * 3600.0, 60.0);
	Dms   = Sign * (Degree + Miniute / 100.0 + Second / 10000.0);
	return  Dms;
}

//正算公式
bool   GpsPoint::BL2xy()
{
	//大地测量学基础 （吕志平 乔书波 北京：测绘出版社 2010.03）

	double X; //由赤道至纬度为B的子午线弧长   （P106   5-41）
	double N; //椭球的卯酉圈曲率半径
	double t;
	double t2;
	double m;
	double m2;
	double ng2;
	double cosB;
	double sinB;  

	X   = A1 * B * 180.0 / PI + A2 * sin(2 * B) 
		+ A3 * sin(4 * B) + A4 * sin(6 * B);

	sinB = sin(B);
	cosB = cos(B);
	t   =  tan(B);
	t2  =  t * t;

	N   = a /sqrt(1 - e2 * sinB * sinB);
	m   =  cosB * (L - L0);
	m2  = m * m;
	ng2 = cosB * cosB * e2 / (1 - e2);

	//P156  （6-63公式）
	x   = X + N * t *(( 0.5 + (    (5 - t2 + 9 * ng2 + 4 * ng2 * ng2)
					/ 24.0 + (61 - 58 * t2 + t2 * t2) * m2 / 720.0) * m2)* m2);

	y   = N * m * ( 1 + m2 * ( (1 - t2 + ng2) / 6.0 + m2 * ( 5 - 18 * t2 + t2 * t2
					+ 14 * ng2 - 58 *  ng2 * t2 ) / 120.0));

	//y   += 500000;

	return   true;
}

//反算公式
bool   GpsPoint::xy2BL()
{
	double sinB;
	double cosB;
	double t;
	double t2;
	double N; //椭球的卯酉圈曲率半径
	double ng2;
	double V;
	double yN;
	double preB0;
	double B0;
	double eta;           
	//y   -=  500000;
	B0   = x / A1;

	do
	{
		preB0 =  B0;
		B0   = B0 * PI / 180.0;
		B0   = (x - (A2 * sin(2 * B0) + A3 * sin(4 * B0) + A4 * sin(6 * B0))) / A1;
		eta   = fabs(B0 - preB0);
	}while(eta > 0.000000001);

	B0  = B0 * PI / 180.0;
	B   = Rad2Dms(B0);
	sinB = sin(B0);
	cosB = cos(B0);
	t   =  tan(B0);
	t2   = t * t;
	N   = a / sqrt(1 - e2 * sinB * sinB);
	ng2   = cosB * cosB * e2 / (1 - e2);
	V   =   sqrt(1 + ng2);
	yN   = y / N;

	B   = B0 - (yN * yN - (5 + 3 * t2 + ng2 - 9 * ng2 * t2) * yN * yN * yN * yN
			/ 12.0 + (61 + 90 * t2 + 45 * t2 * t2) * yN * yN * yN * yN * yN * yN / 360.0)
		* V * V * t / 2;

	L   = L0 + (yN - (1 + 2 * t2 + ng2) * yN * yN * yN / 6.0 + (5 + 28 * t2 + 24
				* t2 * t2 + 6 * ng2 + 8 * ng2 * t2) * yN * yN * yN * yN * yN / 120.0) / cosB;
	return   true;
}

//设置中央子午线
bool   GpsPoint::SetL0(double dL0)
{
	L0   =   Dms2Rad(dL0);
	return   true;
}

//将度分秒经纬度转换为弧度后再转换为平面坐标
bool   GpsPoint::SetBL(double dB, double dL)
{
	B   =   Dms2Rad(dB);
	L   =   Dms2Rad(dL); 
	BL2xy();
	return   true;
}

bool   GpsPoint::GetBL(double *dB, double *dL)
{
	*dB   =   Rad2Dms(B);
	*dL   =   Rad2Dms(L);
	return   true;
}

//将平面坐标转换为（弧度）经纬度
bool   GpsPoint::Setxy(double dx, double dy)
{
	x   =   dx;
	y   =   dy;
	xy2BL();
	return   true;
}

bool   GpsPoint::Getxy(double *dx, double *dy)
{
	*dx   =   x;
	*dy   =   y;
	return   true;
}

GpsPoint_Krasovsky::GpsPoint_Krasovsky()
{
	a   =   6378245;                                                   //长半径
	f   =   298.3;                                                               //扁率的倒数 （扁率：(a-b)/a）   
	e2   =   1 - ((f - 1) / f) * ((f - 1) / f);        //第一偏心率的平方
	e12   =   (f / (f - 1)) * (f / (f - 1)) - 1;       //第二偏心率的平方

	// 克拉索夫斯基椭球
	A1   =   111134.8611;                                          
	A2   =   -16036.4803;
	A3   =   16.8281;
	A4   =   -0.0220;
}

//*************坐标转换
bool CGpsDataView::CoordCovert(CString latitude, CString longitude)
{
	double bbb = atof(latitude);
	double lll = atof(longitude);

	//度分格式转换为度分秒格式
	bbb = Dm2Dms(bbb);
	lll = Dm2Dms(lll);

	double   MyL0 ;   //中央子午线
	double   MyB   =   bbb ;    //33 d 44 m 55.6666 s
	double   MyL   =   lll ;  //3度带，109 d 22 m 33.4444 s

	//计算当地中央子午线 ,3度带
	MyL0 = fabs(MyL);
	MyL0 = floor(MyL);
	MyL0 = 3 * floor(MyL0 / 3 );

	GpsPoint_Krasovsky  MyPrj;
	MyPrj.SetL0(MyL0);
	MyPrj.SetBL(MyB,   MyL);               
	double   OutMyX;               
	double   OutMyY;
	OutMyX   =   MyPrj.x;           //正算结果:坐标x
	OutMyY   =   MyPrj.y;           //结果：坐标y

	CString strTemp1;
	CString strTemp2;
	CString strTemp3;
	CString strTemp4;

	strTemp1.Format("%f",OutMyX);   
	strTemp2.Format("%f",OutMyY);
	strTemp1.Insert(0,"x = ");
	strTemp2.Insert(0," , y = ");
	strTemp2.Insert(0,strTemp1);
	strTemp2.Insert(0," 坐标转换: ");

	strTemp3.Format("%f12",MyB);
	strTemp4.Format("%f12",MyL);
	strTemp3.Insert(0,"B = ");
	strTemp4.Insert(0,"  L = ");
	strTemp4.Insert(0,strTemp3);
	strTemp2.Insert(0,strTemp4);
	//MessageBox(strTemp2);

	DrawPoint(MyPrj.x,MyPrj.y);

	return true;
}

//==================================
//度分格式转换为度分秒格式
double CGpsDataView::Dm2Dms(double Dm)
{
	double Dms;
	double temp; 
	temp = Dm - floor(Dm);
	temp = (temp * 60) / 100;   
	Dm   =  floor(Dm);
	Dm   += temp;    
	Dm   =  Dm /100;      
	Dms  = Dm;
	return Dms;
}

//*************绘制线路  显示出路线
int count1=0;
bool bFirst = true;
double xTemp;
double yTemp;
void CGpsDataView::DrawPoint(double X, double Y)
{     
	Sleep(100);

	//X = (X - floor(X))*100;
	//Y = (Y - floor(Y))*100;

	if (bFirst)
	{
		xTemp=X;
		yTemp=Y;
		bFirst=false;
	}

	CDC *pDC=GetDC();
	CPen pen(PS_SOLID,3,RGB(255,20,20));
	CPen *pOldPen;
	CBrush *pOldBrush;
	CBrush *pBrush=CBrush::FromHandle( (HBRUSH)GetStockObject(NULL_BRUSH) );

	pOldPen=pDC->SelectObject(&pen);
	pOldBrush=pDC->SelectObject(pBrush);

	int a=(int)(100.0-(X-xTemp)*800.0);
	int b=(int)(100.0+(Y-yTemp)*800.0);

	//绘制点显示路径
	pDC->Ellipse(a,b,a+5,b+5);

	//计数
	count1=count1+1;
	pDC->SelectObject( pOldBrush );
	pDC->SelectObject( pOldPen );

	CString str;
	str.Format("%.1f,%.1f,%d,%d,%d",X-xTemp,Y-yTemp,a,b,count1);
	pDC->TextOut(10,10,str);
}
