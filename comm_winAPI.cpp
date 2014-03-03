/**
* comm_winAPI.cpp : 实现操作方法
* Description：利用WinAPI实现串口通信，线程级
* Author：蔡寰宇
* Version：3.0 2013.8.16 17:03
*/

//头文件
#include "comm_winAPI.h"

//全局变量
OVERLAPPED olEvent;		//事件异步操作对象
OVERLAPPED olWrite;		//写异步操作对象
OVERLAPPED olRead;		//读异步操作对象
HANDLE hComm;			//串口句柄
HANDLE hThreadEvent;	//时间线程句柄
char* readBytes;					//接收到的数据
//CRITICAL_SECTION   m_csCommunicationSync; //互斥操作串口 
//const UINT SLEEP_TIME_INTERVAL = 0;  //当串口无数据时,sleep至下次查询间隔的时间,单位:毫秒 
SYSTEMTIME sysTime;		//系统当前时间
string dataFile = "";			//数据文件名
//DEV_PARAM devParam;
int infoIndex = 1; 
ofstream ocout;
//FILE* dataf;
stringstream dataStream;

/**
* 无参数构造函数
*/
//SPC::SPC() {}

/**
* 析构函数
*/
//SPC::~SPC() {}

/**
* 以重叠（异步）操作方式打开串口
* @param portName  串口名
* @return 串口打开成功返回true
*/

bool openPortInOverlappedMode(char* portName) {

	//cout<<"尝试打开串口："<<portName<<"..."<<endl;

	//调用WinAPI的CreateFile方法打开串口
	hComm = CreateFile(portName, 
									 GENERIC_READ | GENERIC_WRITE, //允许对串口进行读写操作
									 0,								//dwShareMode:共享模式设置,串口不允许共享，必须为0
									 NULL,						//安全属性设置，NULL表示默认设置			
									 OPEN_EXISTING,		//指定创建方式，串口只能设为OPEN_EXISTING
									 FILE_FLAG_OVERLAPPED,		//将串口设置为同步操作
									 NULL);						//指向模板文件的句柄，此处设为NULL

	//打开失败
	if(hComm == INVALID_HANDLE_VALUE) {

		cout<<"串口 "<<portName<<" 打开失败！"<<endl;

		CloseHandle(hComm);

		return FALSE;
	}

	//cout<<"串口 "<<portName<<" 打开成功！"<<endl;
	cout<<"已开启串口"<<portName<<endl;

	return TRUE;
}

/**
* 设置DCB参数
* @param baudRate 波特率
* @param fParity 是否允许奇偶校验
* @param byteSize 数据位数
* @param parity 校验方式
* @param stopBits 停止位 
* @return DCB设置成功返回true
*/
bool setupDCB(DWORD baudRate, DWORD fParity, BYTE byteSize, BYTE parity, BYTE stopBits) {

	//创建并初始化DCB
	DCB dcb;

	memset(&dcb, 0, sizeof(dcb));

	//cout<<"开始设置通信参数..."<<endl;

	//获取当前串口通信参数的设置值
	if(!GetCommState(hComm, &dcb)) {

		cout<<"通信参数获取失败！"<<endl;

		return FALSE;
	}

	//设置dcb值
	dcb.DCBlength = sizeof(dcb);		//DCB结构大小
	dcb.BaudRate = baudRate;			//通信波特率
	dcb.fParity = fParity;						//是否允许奇偶校验
	dcb.Parity = parity;						//奇偶校验方式
	dcb.StopBits = stopBits;				//停止位
	dcb.ByteSize = byteSize;				//数据位数
	dcb.fOutxCtsFlow = FALSE;			//指定串口输出数据时，是否使用CTS作为流控制信号，此处为否
	dcb.fDtrControl = DTR_CONTROL_DISABLE;	//指定DTR流控制信号的工作方式，此处禁用
	dcb.fDsrSensitivity = FALSE;			//指定通信驱动程序对DSR信号是否敏感，此处为否
	dcb.fRtsControl = RTS_CONTROL_DISABLE;		//指定RTS流控制方式，此处禁用
	dcb.fOutX = FALSE;						//指定发送串口数据时是否使用Xon/Xoff流控制，此处为否
	dcb.fInX = FALSE;							//指定接收串口数据时是否使用Xon/Xoff流控制，此处为否
	dcb.fErrorChar = FALSE;				//当奇偶校验出错时，是否使用ErrorChar替换该错误数据，此处为否
	dcb.fBinary = TRUE;						//是否允许二进制通信，必须为True
	dcb.fNull = FALSE;						//是否忽略接收数据中的空字符，此处为否
	dcb.fAbortOnError = FALSE;			//指定当发生错误时，是否终止发送和接收操作，此处为否
	dcb.wReserved = 0;						//必须设为0
	dcb.XonLim = 2;							//指定发送Xon字符前接收缓冲区中允许的最小字符数量
	dcb.XoffLim = 4;							//指定发送Xoff字符前接收缓冲区中允许允许的最大字符数量(为接收缓冲区长度与XoffLim之差)
	dcb.XonChar = 0x13;					//指定通信时发送XON字符使用的字符值
	dcb.XoffChar = 0x19;					//指定通信时发送XOFF字符使用的字符值
	dcb.EvtChar = 0;							//指定事件字符

	//设置串口通信参数
	if(!SetCommState(hComm, &dcb)) {

		cout<<"通信参数设置失败！"<<endl;

		return FALSE;
	}

	cout<<"通信参数设置:";
	cout<<"波特率-"<<baudRate<<"-"<<endl;
//	cout<<"奇偶校验-"<<fParity<<"-";
//	cout<<"奇偶校验方式-"<<parity<<"-";
//	cout<<"数据位-"<<(int)byteSize<<"-";
//	cout<<"停止位-"<<stopBits<<endl;

	return TRUE;
}

/**
* 设置超时设置
* @param readInterval 通信线上两个字符到达之间的最大时间（ms）
* @param readTotalMultiplier 指定一个乘数，读操作的总限时时间等于字节数与该值的乘积（ms）
* @param readTotalConstant 指定一个常数，读操作的总限时间等于字节数与readTotalMultiplier的乘积加上该常数（ms）
* @param writeTotalMultiplier 与readTotalMultiplier相似，决定写操作总限时间
* @param writeTotalConstant 与readTotalConstant相似，决定写操作总限时间
* @return 超时设置成功返回true
*/
bool setupTimeout(DWORD readInterval, DWORD readTotalMultiplier, 
	DWORD readTotalConstant, DWORD writeTotalMultiplier, DWORD writeTotalConstant) {

		COMMTIMEOUTS timeouts;

		timeouts.ReadIntervalTimeout = readInterval;
		timeouts.ReadTotalTimeoutConstant = readTotalConstant;
		timeouts.ReadTotalTimeoutMultiplier = readTotalMultiplier;
		timeouts.WriteTotalTimeoutConstant = writeTotalConstant;
		timeouts.WriteTotalTimeoutMultiplier = writeTotalMultiplier;

	//	cout<<"开始设定超时设置..."<<endl;

		//设置超时参数
		if(!SetCommTimeouts(hComm, &timeouts)) {

			cout<<"超时设置失败！"<<endl;

			return FALSE;
		}

		cout<<"超时设置:";
		cout<<readInterval<<",";
		cout<<readTotalMultiplier<<",";
		cout<<readTotalConstant<<",";
		cout<<writeTotalMultiplier<<",";
		cout<<writeTotalConstant<<endl;

		return TRUE;
}
/**
string parse(char* readBytes) {

	string data = string(readBytes);
	string resultData;

	int index = 1;

	int start = 0;

	int end = 0;

	while((end = data.find_first_of(',')) >= 0) {

		string segment = data.substr(start, end);

		double speed;
		double x;
		stringstream  buffer;
		
		switch(index) {

			case 1 : resultData.append("\t").append("UTC时间(hhmmss):").append(segment).append("\n");break;
			case 2 : resultData.append("\t").append("定位状态:").append(segment).append("\n");break;
			case 3 : resultData.append("\t").append("纬度(ddmm.mmmm):").append(segment).append("\n");break;
			case 4 : resultData.append("\t").append("纬度半球:").append(segment).append("\n");break;
			case 5 : resultData.append("\t").append("经度(ddmm.mmmm):").append(segment).append("\n");break;
			case 6 : resultData.append("\t").append("经度半球:").append(segment).append("\n");break;
			case 7 : resultData.append("\t").append("地面速率:");
						
						buffer<<segment;
						buffer>>speed;
						buffer.clear();
						x = 1.852 * 1000 / 3600;
						speed *= x;
						buffer<<speed;

						resultData.append(buffer.str()).append("\n");

						break;

			case 8 : resultData.append("\t").append("地面航向:").append(segment).append("\n");break;
			case 9 : resultData.append("\t").append("UTC日期(ddmmyy):").append(segment).append("\n");break;
			case 10 : resultData.append("\t").append("磁偏角:").append(segment).append("\n");break;
			case 11 : resultData.append("\t").append("磁偏角方向:").append(segment).append("\n");break;
		//	case 12 : resultData.append("模式指示:").append(segment);break;
			default:break;
		}

		data = data.substr(end + 1, data.size());

	//	start = end + 1;

		index ++;
	}

	end = data.find_first_of('*');

	string segment = data.substr(start, end);

	resultData.append("\t").append("模式指示:").append(segment).append("\n");

	return resultData;
}
*/

/**
* 将数据以文件形式保存
*/
/*
void saveData() {

//	ofstream ocout;

	//以追加方式打开日志文件
//	ocout.open(dataFile, ios::app);
	//输入时间戳
//	ocout<<infoIndex++<<".数据接收时间<"<<sysTime.wYear<<"-"<<sysTime.wMonth<<"-"<<sysTime.wDay<<" "
//		<<sysTime.wHour<<":"<<sysTime.wMinute<<":"<<sysTime.wSecond<<":"<<sysTime.wMilliseconds<<">"<<endl
//		<<"数据信息:";

	//解析数据
//	string data = parse(readBytes);

	//输入数据
//	ocout<<readBytes<<endl;

	//	ocout.close();
	//fprintf(dataf, infoIndex++);

	dataStream<<readBytes;

	}
*/

/**
* 保存数据的线程方法
*/
/*
void threadProcSave(void* pParam) {

	EnterCriticalSection(&m_csCommunicationSync);		//进入临界区
	//保存数据
	saveData();

	LeaveCriticalSection(&m_csCommunicationSync);	//离开临界区
}
*/
/*
void readPropertiesToInitial() {

	ifstream ocin;
	
	ocin.open("dev_param.properties", ios::in | 1);

	char line[100];

	while(ocin.getline(line, 100)) {

		string strLine = string(line);

		int i = strLine.find('=');

		string attrName = strLine.substr(0, i);
		
		string attrValue = strLine.substr(i + 1, strLine.length());

		if(strLine.compare("$jbaud")) {

			devParam.curBaud = attrValue;

			continue;
		}

		if(strLine.compare("$jbaudOthr")) {

			devParam.othBaud = attrValue;

			continue;
		}

		if(strLine.compare("$jatt,msep")) {

			devParam.msep = attrValue;

			continue;
		}

		if(strLine.compare("$jasc,hehdt")) {

			devParam.hehdt = attrValue;

			continue;
		}

		if(strLine.compare("$jasc,gprmc")) {

			devParam.gprmc = attrValue;

			continue;
		}

		if(strLine.compare("$jsave") == 0) {

			if(attrValue.compare("true") == 0) {

				devParam.save = true;

			} else {

				devParam.save = false;
			}

			continue;
		}

		if(strLine.compare("$jreset") == 0) {
			
			if(attrValue.compare("true") == 0) {

				devParam.reset = true;

			} else {

				devParam.reset = false;
			}

			continue;
		}
	}
}
*/
/**
* 从串口读取数据
* 采用事件驱动方式读取
*/
void readData() {

	bool bRead = TRUE;			//判断是否可读
	BOOL bResult = TRUE;		//判断读取过程是否出错

	COMSTAT comstat;		//通信端口状态结构变量

	DWORD dwError = 0;		//返回的错误代码
	DWORD bytesRead = 0;		//实际被读出的字节数
	DWORD dwRes;

	char* tmpBytes;

	//清除错误或获取串口状态
	ClearCommError(hComm, &dwError, &comstat);
	
	//初始化存放读取数据的地址
	tmpBytes = (char*) malloc(sizeof(char) * (comstat.cbInQue + 1));

	//读取数据
	if(ReadFile(hComm, tmpBytes, comstat.cbInQue, (LPDWORD)bytesRead, &olRead)) {

		tmpBytes[comstat.cbInQue] = '\0';

		cout<<tmpBytes<<endl;

	//	_beginthread(threadProcSave, 0, 0); 						
	//	saveData();
	
	} else {

		//读取失败，创建读操作异步对象
		olRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		//设置5秒超时操作等待
		dwRes = WaitForSingleObject(olRead.hEvent, 5000);

		switch(dwRes) {

			case WAIT_OBJECT_0 :
				//操作出错
				if(!GetOverlappedResult(hComm, &olRead, &bytesRead, TRUE)) {

					switch(dwError = GetLastError()) {
						//重叠读取操作还未结束，等待操作结束，置为不可读状态
						case ERROR_IO_PENDING : bRead = FALSE;	break;
						default:	cout<<"读取过程出错！错误代码："<<dwError<<endl;break;
					}
				
				} else {
			
					cout<<tmpBytes<<endl;
				}

				break;

			//读操作超时
			case WAIT_TIMEOUT : 

				cout<<"读操作超时！"<<endl;break;

			default : break;
		}
	}

	if(strlen(tmpBytes) > 0) {

		//输入时间戳
		dataStream<<infoIndex++<<".数据接收时间<"<<sysTime.wYear<<"-"<<sysTime.wMonth<<"-"<<sysTime.wDay<<" "
		<<sysTime.wHour<<":"<<sysTime.wMinute<<":"<<sysTime.wSecond<<":"<<sysTime.wMilliseconds<<">"<<endl
		<<"数据信息:";
		//将数据写入内存流
		dataStream<<tmpBytes<<endl;
	}
}

/**
* 向串口写入数据
* @param m_szWriteBuffer 待写入数据的首地址
* @param m_nToSend 待写入数据的长度 
*/
void writeData(BYTE* m_szWriteBuffer, DWORD m_nToSend) {

	bool writeCompleted = TRUE;		//判断写操作是否完成

	DWORD BytesSent = 0;

	//指定偏移量，串口设为0
	olWrite.Offset = 0;
	olWrite.OffsetHigh = 0;

//	cout<<"开始写入数据..."<<endl;

	//写入数据
	if(!WriteFile(hComm, m_szWriteBuffer, 
			m_nToSend, &BytesSent, &olWrite)) {

		DWORD dwError = GetLastError();

		switch(dwError) {
			//重叠写操作还未结束，等待操作结束，置为写未完状态	
			case ERROR_IO_PENDING : BytesSent = 0;writeCompleted = FALSE;break;
			default : break;
		}

		//等待写操作完成
		if(!writeCompleted) {
					
			GetOverlappedResult(hComm, &olWrite, &BytesSent, TRUE);
		}

		//判断实际写入数据与预期写入数据是否一致
		if(BytesSent != m_nToSend) {

			cout<<"警告：实际写入数据与预期写入数据长度不符！"<<endl;
			cout<<"预期写入数据："<<m_nToSend<<"，实际写入数据："<<BytesSent<<endl;
		}
	}

//	cout<<"写数据操作结束"<<endl;
}

/*
void threadProcWrite(void* pParam) {

	while(hComm != INVALID_HANDLE_VALUE) {

		string input  = "";

		while(cin>>input) {

			writeData((BYTE*)input.c_str(), input.size());
		//	writeData((BYTE*)readBytes, strlen(readBytes));
		}
	}
}
*/

/**
* 监听事件线程方法
*/
void threadProcEvent(void* param) {

	DWORD dwEvtMask, dwRes;

	//事件异步对象创建事件
	olEvent.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	while(hComm != INVALID_HANDLE_VALUE) {

		//监听串口事件
		WaitCommEvent(hComm, &dwEvtMask, &olEvent);

		//等待的事件对象句柄（100ms超时）
		dwRes = WaitForSingleObject(olEvent.hEvent, 100);

		switch(dwRes) {

			//成功得到监听事件结果
			case WAIT_OBJECT_0 : 
						
						switch(dwEvtMask) {
								
							//当缓冲区接收到字符，执行读操作及写操作
							case EV_RXCHAR : readData();
														break;
						
							default : break;
						 }

			default : break;
		}
	}
}

/**
* 创建并启动监听事件线程
*/
void runEventThread() {

	//设置监听的事件为缓冲区接收到字符
	if(!SetCommMask(hComm, EV_RXCHAR)) {

		cout<<"建立事件掩码失败！"<<endl;
	}

	//监听到状态，开启线程
	if(!_beginthread(threadProcEvent, 0, 0)) {
		
		cout<<"创建事件线程失败！"<<endl;
	}
}

/**
* 初始化操作，完成串口连接，DCB，超时设置，及其他初始化
*/
bool init() {

	//打开串口，以异步方式
	if(!openPortInOverlappedMode("COM3")) {
			
		cout<<"开启串口 "<<"COM3"<<" 失败！"<<endl;

		return FALSE;
	}

	//设置DCB
	if(!setupDCB(19200, FALSE, 8, NOPARITY, ONESTOPBIT)) {

		cout<<"设置DCB失败！"<<endl;

		return FALSE;
	}

	//设置超时
	if(!setupTimeout(0, 0, 0, 0, 0)) {

		cout<<"设置超时失败！"<<endl;

		return FALSE;
	}

	//清空缓冲区
	PurgeComm(hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	//初始化临界区变量
//	InitializeCriticalSection(&m_csCommunicationSync);

	//获取系统当前时间
	GetLocalTime( &sysTime ); 

	//初始化文件名
	ostringstream  buffer;

	buffer<<sysTime.wYear;
	buffer<<"-";
	buffer<<sysTime.wMonth;
	buffer<<"-";
	buffer<<sysTime.wDay;
	buffer<<"-";
	buffer<<sysTime.wHour;
	buffer<<"-";
	buffer<<sysTime.wMinute;
	buffer<<"-";
	buffer<<sysTime.wSecond;
	buffer<<"-";
	buffer<<sysTime.wMilliseconds;
	buffer<<".log";

	dataFile.append(buffer.str());

	readBytes = "";

//	devParam.curBaud = "";
//	devParam.gprmc = "";
//	devParam.hehdt = "";
//	devParam.msep = "";
//	devParam.othBaud = "";
//	devParam.reset = false;
//	devParam.save = false;
}

/**
* 对硬件设备发出指令进行设置
*/
void setupDevice() {

	cout<<"设备设置："<<endl;

	ifstream ocin;
	//打开指令文件
	ocin.open("dev_param.ins", ios::in | 1);

	char line[100];
	//读取指令
	while(ocin.getline(line, 100)) {

		cout<<line<<endl;
		//发送指令
		writeData((BYTE*)line, strlen(line));
	}
}

/**
* 以轮询方式执行读操作的线程方法
*/
/*
void threadProcReadInPollMode(void* param) {
	
	bool bRead = TRUE;			//判断是否可读
	BOOL bResult = TRUE;		//判断读取过程是否出错

	COMSTAT comstat;		//通信端口状态结构变量

	DWORD dwError = 0;		//返回的错误代码
	DWORD bytesRead = 0;		//实际被读出的字节数
	DWORD dwRes;

	cout<<"开始读取串口的数据："<<endl;

	//轮询访问
	while(true) {
		
		//清除错误或获取串口状态
		ClearCommError(hComm, &dwError, &comstat);

		//当前串口字节数为0，表示无数据，继续轮询
		if(comstat.cbInQue == 0) {

			Sleep(SLEEP_TIME_INTERVAL);
	
			continue;
		}

		//初始化存放读取数据的地址
		readBytes = (char*) malloc(sizeof(char) * (comstat.cbInQue + 1));

		//读取数据
		if(ReadFile(hComm, readBytes, comstat.cbInQue, (LPDWORD)bytesRead, &olRead)) {

			readBytes[comstat.cbInQue] = '\0';

			cout<<readBytes<<endl;

			//获取系统时间
			GetLocalTime( &sysTime ); 
	
			_beginthread(threadProcSave, 0, 0);
	
		} else {

			//读取失败，创建读操作异步对象
			olRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			//设置5秒超时操作等待
			dwRes = WaitForSingleObject(olRead.hEvent, 5000);

			switch(dwRes) {

				case WAIT_OBJECT_0 :
					//操作出错
					if(!GetOverlappedResult(hComm, &olRead, &bytesRead, TRUE)) {

						switch(dwError = GetLastError()) {
							//重叠读取操作还未结束，等待操作结束，置为不可读状态
							case ERROR_IO_PENDING : bRead = FALSE;	break;
							default:	cout<<"读取过程出错！错误代码："<<dwError<<endl;break;
						}
				
					} else {
			
						cout<<readBytes<<endl;

						//获取系统时间
						GetLocalTime( &sysTime ); 
	
						_beginthread(threadProcSave, 0, 0);
				}

				break;

				//读操作超时
				case WAIT_TIMEOUT : 

					cout<<"读操作超时！"<<endl;break;

				default : break;
			}
		}
	}
}
*/
int main() {

	//初始化并开启串口
	init();

	//设备设置
	setupDevice();

//	_beginthread(threadProcRead, 0, 0);

	runEventThread();

	//_beginthread(threadProcReadInPollMode, 0, 0);

	char q;

	//输入ctrl-z结束接收数据
	while(cin>>q) {}

	CloseHandle(hComm);
	//打开文件
	ocout.open(dataFile, ios::app);
	//将数据流写入文件
	ocout<<dataStream.str();

	ocout.close();
	dataStream.clear();

	return 0;
}