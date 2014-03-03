/**
* comm_winAPI.cpp : ʵ�ֲ�������
* Description������WinAPIʵ�ִ���ͨ�ţ��̼߳�
* Author�������
* Version��3.0 2013.8.16 17:03
*/

//ͷ�ļ�
#include "comm_winAPI.h"

//ȫ�ֱ���
OVERLAPPED olEvent;		//�¼��첽��������
OVERLAPPED olWrite;		//д�첽��������
OVERLAPPED olRead;		//���첽��������
HANDLE hComm;			//���ھ��
HANDLE hThreadEvent;	//ʱ���߳̾��
char* readBytes;					//���յ�������
//CRITICAL_SECTION   m_csCommunicationSync; //����������� 
//const UINT SLEEP_TIME_INTERVAL = 0;  //������������ʱ,sleep���´β�ѯ�����ʱ��,��λ:���� 
SYSTEMTIME sysTime;		//ϵͳ��ǰʱ��
string dataFile = "";			//�����ļ���
//DEV_PARAM devParam;
int infoIndex = 1; 
ofstream ocout;
//FILE* dataf;
stringstream dataStream;

/**
* �޲������캯��
*/
//SPC::SPC() {}

/**
* ��������
*/
//SPC::~SPC() {}

/**
* ���ص����첽��������ʽ�򿪴���
* @param portName  ������
* @return ���ڴ򿪳ɹ�����true
*/

bool openPortInOverlappedMode(char* portName) {

	//cout<<"���Դ򿪴��ڣ�"<<portName<<"..."<<endl;

	//����WinAPI��CreateFile�����򿪴���
	hComm = CreateFile(portName, 
									 GENERIC_READ | GENERIC_WRITE, //����Դ��ڽ��ж�д����
									 0,								//dwShareMode:����ģʽ����,���ڲ�����������Ϊ0
									 NULL,						//��ȫ�������ã�NULL��ʾĬ������			
									 OPEN_EXISTING,		//ָ��������ʽ������ֻ����ΪOPEN_EXISTING
									 FILE_FLAG_OVERLAPPED,		//����������Ϊͬ������
									 NULL);						//ָ��ģ���ļ��ľ�����˴���ΪNULL

	//��ʧ��
	if(hComm == INVALID_HANDLE_VALUE) {

		cout<<"���� "<<portName<<" ��ʧ�ܣ�"<<endl;

		CloseHandle(hComm);

		return FALSE;
	}

	//cout<<"���� "<<portName<<" �򿪳ɹ���"<<endl;
	cout<<"�ѿ�������"<<portName<<endl;

	return TRUE;
}

/**
* ����DCB����
* @param baudRate ������
* @param fParity �Ƿ�������żУ��
* @param byteSize ����λ��
* @param parity У�鷽ʽ
* @param stopBits ֹͣλ 
* @return DCB���óɹ�����true
*/
bool setupDCB(DWORD baudRate, DWORD fParity, BYTE byteSize, BYTE parity, BYTE stopBits) {

	//��������ʼ��DCB
	DCB dcb;

	memset(&dcb, 0, sizeof(dcb));

	//cout<<"��ʼ����ͨ�Ų���..."<<endl;

	//��ȡ��ǰ����ͨ�Ų���������ֵ
	if(!GetCommState(hComm, &dcb)) {

		cout<<"ͨ�Ų�����ȡʧ�ܣ�"<<endl;

		return FALSE;
	}

	//����dcbֵ
	dcb.DCBlength = sizeof(dcb);		//DCB�ṹ��С
	dcb.BaudRate = baudRate;			//ͨ�Ų�����
	dcb.fParity = fParity;						//�Ƿ�������żУ��
	dcb.Parity = parity;						//��żУ�鷽ʽ
	dcb.StopBits = stopBits;				//ֹͣλ
	dcb.ByteSize = byteSize;				//����λ��
	dcb.fOutxCtsFlow = FALSE;			//ָ�������������ʱ���Ƿ�ʹ��CTS��Ϊ�������źţ��˴�Ϊ��
	dcb.fDtrControl = DTR_CONTROL_DISABLE;	//ָ��DTR�������źŵĹ�����ʽ���˴�����
	dcb.fDsrSensitivity = FALSE;			//ָ��ͨ�����������DSR�ź��Ƿ����У��˴�Ϊ��
	dcb.fRtsControl = RTS_CONTROL_DISABLE;		//ָ��RTS�����Ʒ�ʽ���˴�����
	dcb.fOutX = FALSE;						//ָ�����ʹ�������ʱ�Ƿ�ʹ��Xon/Xoff�����ƣ��˴�Ϊ��
	dcb.fInX = FALSE;							//ָ�����մ�������ʱ�Ƿ�ʹ��Xon/Xoff�����ƣ��˴�Ϊ��
	dcb.fErrorChar = FALSE;				//����żУ�����ʱ���Ƿ�ʹ��ErrorChar�滻�ô������ݣ��˴�Ϊ��
	dcb.fBinary = TRUE;						//�Ƿ����������ͨ�ţ�����ΪTrue
	dcb.fNull = FALSE;						//�Ƿ���Խ��������еĿ��ַ����˴�Ϊ��
	dcb.fAbortOnError = FALSE;			//ָ������������ʱ���Ƿ���ֹ���ͺͽ��ղ������˴�Ϊ��
	dcb.wReserved = 0;						//������Ϊ0
	dcb.XonLim = 2;							//ָ������Xon�ַ�ǰ���ջ��������������С�ַ�����
	dcb.XoffLim = 4;							//ָ������Xoff�ַ�ǰ���ջ��������������������ַ�����(Ϊ���ջ�����������XoffLim֮��)
	dcb.XonChar = 0x13;					//ָ��ͨ��ʱ����XON�ַ�ʹ�õ��ַ�ֵ
	dcb.XoffChar = 0x19;					//ָ��ͨ��ʱ����XOFF�ַ�ʹ�õ��ַ�ֵ
	dcb.EvtChar = 0;							//ָ���¼��ַ�

	//���ô���ͨ�Ų���
	if(!SetCommState(hComm, &dcb)) {

		cout<<"ͨ�Ų�������ʧ�ܣ�"<<endl;

		return FALSE;
	}

	cout<<"ͨ�Ų�������:";
	cout<<"������-"<<baudRate<<"-"<<endl;
//	cout<<"��żУ��-"<<fParity<<"-";
//	cout<<"��żУ�鷽ʽ-"<<parity<<"-";
//	cout<<"����λ-"<<(int)byteSize<<"-";
//	cout<<"ֹͣλ-"<<stopBits<<endl;

	return TRUE;
}

/**
* ���ó�ʱ����
* @param readInterval ͨ�����������ַ�����֮������ʱ�䣨ms��
* @param readTotalMultiplier ָ��һ��������������������ʱʱ������ֽ������ֵ�ĳ˻���ms��
* @param readTotalConstant ָ��һ��������������������ʱ������ֽ�����readTotalMultiplier�ĳ˻����ϸó�����ms��
* @param writeTotalMultiplier ��readTotalMultiplier���ƣ�����д��������ʱ��
* @param writeTotalConstant ��readTotalConstant���ƣ�����д��������ʱ��
* @return ��ʱ���óɹ�����true
*/
bool setupTimeout(DWORD readInterval, DWORD readTotalMultiplier, 
	DWORD readTotalConstant, DWORD writeTotalMultiplier, DWORD writeTotalConstant) {

		COMMTIMEOUTS timeouts;

		timeouts.ReadIntervalTimeout = readInterval;
		timeouts.ReadTotalTimeoutConstant = readTotalConstant;
		timeouts.ReadTotalTimeoutMultiplier = readTotalMultiplier;
		timeouts.WriteTotalTimeoutConstant = writeTotalConstant;
		timeouts.WriteTotalTimeoutMultiplier = writeTotalMultiplier;

	//	cout<<"��ʼ�趨��ʱ����..."<<endl;

		//���ó�ʱ����
		if(!SetCommTimeouts(hComm, &timeouts)) {

			cout<<"��ʱ����ʧ�ܣ�"<<endl;

			return FALSE;
		}

		cout<<"��ʱ����:";
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

			case 1 : resultData.append("\t").append("UTCʱ��(hhmmss):").append(segment).append("\n");break;
			case 2 : resultData.append("\t").append("��λ״̬:").append(segment).append("\n");break;
			case 3 : resultData.append("\t").append("γ��(ddmm.mmmm):").append(segment).append("\n");break;
			case 4 : resultData.append("\t").append("γ�Ȱ���:").append(segment).append("\n");break;
			case 5 : resultData.append("\t").append("����(ddmm.mmmm):").append(segment).append("\n");break;
			case 6 : resultData.append("\t").append("���Ȱ���:").append(segment).append("\n");break;
			case 7 : resultData.append("\t").append("��������:");
						
						buffer<<segment;
						buffer>>speed;
						buffer.clear();
						x = 1.852 * 1000 / 3600;
						speed *= x;
						buffer<<speed;

						resultData.append(buffer.str()).append("\n");

						break;

			case 8 : resultData.append("\t").append("���溽��:").append(segment).append("\n");break;
			case 9 : resultData.append("\t").append("UTC����(ddmmyy):").append(segment).append("\n");break;
			case 10 : resultData.append("\t").append("��ƫ��:").append(segment).append("\n");break;
			case 11 : resultData.append("\t").append("��ƫ�Ƿ���:").append(segment).append("\n");break;
		//	case 12 : resultData.append("ģʽָʾ:").append(segment);break;
			default:break;
		}

		data = data.substr(end + 1, data.size());

	//	start = end + 1;

		index ++;
	}

	end = data.find_first_of('*');

	string segment = data.substr(start, end);

	resultData.append("\t").append("ģʽָʾ:").append(segment).append("\n");

	return resultData;
}
*/

/**
* ���������ļ���ʽ����
*/
/*
void saveData() {

//	ofstream ocout;

	//��׷�ӷ�ʽ����־�ļ�
//	ocout.open(dataFile, ios::app);
	//����ʱ���
//	ocout<<infoIndex++<<".���ݽ���ʱ��<"<<sysTime.wYear<<"-"<<sysTime.wMonth<<"-"<<sysTime.wDay<<" "
//		<<sysTime.wHour<<":"<<sysTime.wMinute<<":"<<sysTime.wSecond<<":"<<sysTime.wMilliseconds<<">"<<endl
//		<<"������Ϣ:";

	//��������
//	string data = parse(readBytes);

	//��������
//	ocout<<readBytes<<endl;

	//	ocout.close();
	//fprintf(dataf, infoIndex++);

	dataStream<<readBytes;

	}
*/

/**
* �������ݵ��̷߳���
*/
/*
void threadProcSave(void* pParam) {

	EnterCriticalSection(&m_csCommunicationSync);		//�����ٽ���
	//��������
	saveData();

	LeaveCriticalSection(&m_csCommunicationSync);	//�뿪�ٽ���
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
* �Ӵ��ڶ�ȡ����
* �����¼�������ʽ��ȡ
*/
void readData() {

	bool bRead = TRUE;			//�ж��Ƿ�ɶ�
	BOOL bResult = TRUE;		//�ж϶�ȡ�����Ƿ����

	COMSTAT comstat;		//ͨ�Ŷ˿�״̬�ṹ����

	DWORD dwError = 0;		//���صĴ������
	DWORD bytesRead = 0;		//ʵ�ʱ��������ֽ���
	DWORD dwRes;

	char* tmpBytes;

	//���������ȡ����״̬
	ClearCommError(hComm, &dwError, &comstat);
	
	//��ʼ����Ŷ�ȡ���ݵĵ�ַ
	tmpBytes = (char*) malloc(sizeof(char) * (comstat.cbInQue + 1));

	//��ȡ����
	if(ReadFile(hComm, tmpBytes, comstat.cbInQue, (LPDWORD)bytesRead, &olRead)) {

		tmpBytes[comstat.cbInQue] = '\0';

		cout<<tmpBytes<<endl;

	//	_beginthread(threadProcSave, 0, 0); 						
	//	saveData();
	
	} else {

		//��ȡʧ�ܣ������������첽����
		olRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		//����5�볬ʱ�����ȴ�
		dwRes = WaitForSingleObject(olRead.hEvent, 5000);

		switch(dwRes) {

			case WAIT_OBJECT_0 :
				//��������
				if(!GetOverlappedResult(hComm, &olRead, &bytesRead, TRUE)) {

					switch(dwError = GetLastError()) {
						//�ص���ȡ������δ�������ȴ�������������Ϊ���ɶ�״̬
						case ERROR_IO_PENDING : bRead = FALSE;	break;
						default:	cout<<"��ȡ���̳���������룺"<<dwError<<endl;break;
					}
				
				} else {
			
					cout<<tmpBytes<<endl;
				}

				break;

			//��������ʱ
			case WAIT_TIMEOUT : 

				cout<<"��������ʱ��"<<endl;break;

			default : break;
		}
	}

	if(strlen(tmpBytes) > 0) {

		//����ʱ���
		dataStream<<infoIndex++<<".���ݽ���ʱ��<"<<sysTime.wYear<<"-"<<sysTime.wMonth<<"-"<<sysTime.wDay<<" "
		<<sysTime.wHour<<":"<<sysTime.wMinute<<":"<<sysTime.wSecond<<":"<<sysTime.wMilliseconds<<">"<<endl
		<<"������Ϣ:";
		//������д���ڴ���
		dataStream<<tmpBytes<<endl;
	}
}

/**
* �򴮿�д������
* @param m_szWriteBuffer ��д�����ݵ��׵�ַ
* @param m_nToSend ��д�����ݵĳ��� 
*/
void writeData(BYTE* m_szWriteBuffer, DWORD m_nToSend) {

	bool writeCompleted = TRUE;		//�ж�д�����Ƿ����

	DWORD BytesSent = 0;

	//ָ��ƫ������������Ϊ0
	olWrite.Offset = 0;
	olWrite.OffsetHigh = 0;

//	cout<<"��ʼд������..."<<endl;

	//д������
	if(!WriteFile(hComm, m_szWriteBuffer, 
			m_nToSend, &BytesSent, &olWrite)) {

		DWORD dwError = GetLastError();

		switch(dwError) {
			//�ص�д������δ�������ȴ�������������Ϊдδ��״̬	
			case ERROR_IO_PENDING : BytesSent = 0;writeCompleted = FALSE;break;
			default : break;
		}

		//�ȴ�д�������
		if(!writeCompleted) {
					
			GetOverlappedResult(hComm, &olWrite, &BytesSent, TRUE);
		}

		//�ж�ʵ��д��������Ԥ��д�������Ƿ�һ��
		if(BytesSent != m_nToSend) {

			cout<<"���棺ʵ��д��������Ԥ��д�����ݳ��Ȳ�����"<<endl;
			cout<<"Ԥ��д�����ݣ�"<<m_nToSend<<"��ʵ��д�����ݣ�"<<BytesSent<<endl;
		}
	}

//	cout<<"д���ݲ�������"<<endl;
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
* �����¼��̷߳���
*/
void threadProcEvent(void* param) {

	DWORD dwEvtMask, dwRes;

	//�¼��첽���󴴽��¼�
	olEvent.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	while(hComm != INVALID_HANDLE_VALUE) {

		//���������¼�
		WaitCommEvent(hComm, &dwEvtMask, &olEvent);

		//�ȴ����¼���������100ms��ʱ��
		dwRes = WaitForSingleObject(olEvent.hEvent, 100);

		switch(dwRes) {

			//�ɹ��õ������¼����
			case WAIT_OBJECT_0 : 
						
						switch(dwEvtMask) {
								
							//�����������յ��ַ���ִ�ж�������д����
							case EV_RXCHAR : readData();
														break;
						
							default : break;
						 }

			default : break;
		}
	}
}

/**
* ���������������¼��߳�
*/
void runEventThread() {

	//���ü������¼�Ϊ���������յ��ַ�
	if(!SetCommMask(hComm, EV_RXCHAR)) {

		cout<<"�����¼�����ʧ�ܣ�"<<endl;
	}

	//������״̬�������߳�
	if(!_beginthread(threadProcEvent, 0, 0)) {
		
		cout<<"�����¼��߳�ʧ�ܣ�"<<endl;
	}
}

/**
* ��ʼ����������ɴ������ӣ�DCB����ʱ���ã���������ʼ��
*/
bool init() {

	//�򿪴��ڣ����첽��ʽ
	if(!openPortInOverlappedMode("COM3")) {
			
		cout<<"�������� "<<"COM3"<<" ʧ�ܣ�"<<endl;

		return FALSE;
	}

	//����DCB
	if(!setupDCB(19200, FALSE, 8, NOPARITY, ONESTOPBIT)) {

		cout<<"����DCBʧ�ܣ�"<<endl;

		return FALSE;
	}

	//���ó�ʱ
	if(!setupTimeout(0, 0, 0, 0, 0)) {

		cout<<"���ó�ʱʧ�ܣ�"<<endl;

		return FALSE;
	}

	//��ջ�����
	PurgeComm(hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	//��ʼ���ٽ�������
//	InitializeCriticalSection(&m_csCommunicationSync);

	//��ȡϵͳ��ǰʱ��
	GetLocalTime( &sysTime ); 

	//��ʼ���ļ���
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
* ��Ӳ���豸����ָ���������
*/
void setupDevice() {

	cout<<"�豸���ã�"<<endl;

	ifstream ocin;
	//��ָ���ļ�
	ocin.open("dev_param.ins", ios::in | 1);

	char line[100];
	//��ȡָ��
	while(ocin.getline(line, 100)) {

		cout<<line<<endl;
		//����ָ��
		writeData((BYTE*)line, strlen(line));
	}
}

/**
* ����ѯ��ʽִ�ж��������̷߳���
*/
/*
void threadProcReadInPollMode(void* param) {
	
	bool bRead = TRUE;			//�ж��Ƿ�ɶ�
	BOOL bResult = TRUE;		//�ж϶�ȡ�����Ƿ����

	COMSTAT comstat;		//ͨ�Ŷ˿�״̬�ṹ����

	DWORD dwError = 0;		//���صĴ������
	DWORD bytesRead = 0;		//ʵ�ʱ��������ֽ���
	DWORD dwRes;

	cout<<"��ʼ��ȡ���ڵ����ݣ�"<<endl;

	//��ѯ����
	while(true) {
		
		//���������ȡ����״̬
		ClearCommError(hComm, &dwError, &comstat);

		//��ǰ�����ֽ���Ϊ0����ʾ�����ݣ�������ѯ
		if(comstat.cbInQue == 0) {

			Sleep(SLEEP_TIME_INTERVAL);
	
			continue;
		}

		//��ʼ����Ŷ�ȡ���ݵĵ�ַ
		readBytes = (char*) malloc(sizeof(char) * (comstat.cbInQue + 1));

		//��ȡ����
		if(ReadFile(hComm, readBytes, comstat.cbInQue, (LPDWORD)bytesRead, &olRead)) {

			readBytes[comstat.cbInQue] = '\0';

			cout<<readBytes<<endl;

			//��ȡϵͳʱ��
			GetLocalTime( &sysTime ); 
	
			_beginthread(threadProcSave, 0, 0);
	
		} else {

			//��ȡʧ�ܣ������������첽����
			olRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			//����5�볬ʱ�����ȴ�
			dwRes = WaitForSingleObject(olRead.hEvent, 5000);

			switch(dwRes) {

				case WAIT_OBJECT_0 :
					//��������
					if(!GetOverlappedResult(hComm, &olRead, &bytesRead, TRUE)) {

						switch(dwError = GetLastError()) {
							//�ص���ȡ������δ�������ȴ�������������Ϊ���ɶ�״̬
							case ERROR_IO_PENDING : bRead = FALSE;	break;
							default:	cout<<"��ȡ���̳���������룺"<<dwError<<endl;break;
						}
				
					} else {
			
						cout<<readBytes<<endl;

						//��ȡϵͳʱ��
						GetLocalTime( &sysTime ); 
	
						_beginthread(threadProcSave, 0, 0);
				}

				break;

				//��������ʱ
				case WAIT_TIMEOUT : 

					cout<<"��������ʱ��"<<endl;break;

				default : break;
			}
		}
	}
}
*/
int main() {

	//��ʼ������������
	init();

	//�豸����
	setupDevice();

//	_beginthread(threadProcRead, 0, 0);

	runEventThread();

	//_beginthread(threadProcReadInPollMode, 0, 0);

	char q;

	//����ctrl-z������������
	while(cin>>q) {}

	CloseHandle(hComm);
	//���ļ�
	ocout.open(dataFile, ios::app);
	//��������д���ļ�
	ocout<<dataStream.str();

	ocout.close();
	dataStream.clear();

	return 0;
}