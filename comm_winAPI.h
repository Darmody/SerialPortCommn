/**
* comm_winAPI.h : �����������
* Description������WinAPIʵ�ִ���ͨ�ţ��̼߳�
* Author�������
* Version��2.0 2013.7.26 18:36
*/

//ͷ�ļ�
#include<iostream>
#include <process.h>
#include<Windows.h>
#include<string>
#include<fstream>
#include<sstream>
#include<ctime>

using namespace std;

class SPC {
		
	public:

	public:
	
		//���캯��
		SPC();

		//��������
		~SPC();

	public:
		/**
		* ���ص����첽��������ʽ�򿪴���
		* @param portName  ������
		* @return ���ڴ򿪳ɹ�����true
		*/
		bool openPortInOverlappedMode(char* portName);

		/**
		* ����DCB����
		* @param baudRate ������
		* @param fParity �Ƿ�������żУ��
		* @param byteSize ����λ��
		* @param parity У�鷽ʽ
		* @param stopBits ֹͣλ 
		* @return DCB���óɹ�����true
		*/
		bool setupDCB(DWORD baudRate, DWORD fParity, BYTE byteSize, BYTE parity, BYTE stopBits);

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
			DWORD readTotalConstant, DWORD writeTotalMultiplier, DWORD writeTotalConstant);

		/**
		* �Ӵ��ڶ�ȡ����
		* ������ѯ��ʽ��ȡ
		*/
		void readData();

		/**
		* �򴮿�д������
		* @param m_szWriteBuffer ��д�����ݵ��׵�ַ
		* @param m_nToSend ��д�����ݵĳ��� 
		*/
		void writeData(BYTE* m_szWriteBuffer, DWORD m_nToSend);

		/**
		* ���������������¼��߳�
		*/
		void runEventThread();
};

typedef struct dev_param {

	string curBaud;

	string othBaud;

	string msep;

	string hehdt;
	
	string gprmc;

	bool save;

	bool reset;
	
}DEV_PARAM;

