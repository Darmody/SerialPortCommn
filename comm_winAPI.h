/**
* comm_winAPI.h : 定义操作方法
* Description：利用WinAPI实现串口通信，线程级
* Author：蔡寰宇
* Version：2.0 2013.7.26 18:36
*/

//头文件
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
	
		//构造函数
		SPC();

		//析构函数
		~SPC();

	public:
		/**
		* 以重叠（异步）操作方式打开串口
		* @param portName  串口名
		* @return 串口打开成功返回true
		*/
		bool openPortInOverlappedMode(char* portName);

		/**
		* 设置DCB参数
		* @param baudRate 波特率
		* @param fParity 是否允许奇偶校验
		* @param byteSize 数据位数
		* @param parity 校验方式
		* @param stopBits 停止位 
		* @return DCB设置成功返回true
		*/
		bool setupDCB(DWORD baudRate, DWORD fParity, BYTE byteSize, BYTE parity, BYTE stopBits);

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
			DWORD readTotalConstant, DWORD writeTotalMultiplier, DWORD writeTotalConstant);

		/**
		* 从串口读取数据
		* 采用轮询方式读取
		*/
		void readData();

		/**
		* 向串口写入数据
		* @param m_szWriteBuffer 待写入数据的首地址
		* @param m_nToSend 待写入数据的长度 
		*/
		void writeData(BYTE* m_szWriteBuffer, DWORD m_nToSend);

		/**
		* 创建并启动监听事件线程
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

