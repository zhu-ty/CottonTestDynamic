#pragma once
#define SERIAL_TIMEOUT	100
#define SERIAL_INTERVAL 40

//MFC Special
#include <afx.h>
#include <afxwin.h>

#include <PvDeviceGEV.h>
#include <PvSystem.h>
#include <PvDeviceSerialPort.h>
#include <PvDeviceAdapter.h>
#include <PvSerialTerminalWnd.h>
#include <PvSerialBridge.h>



class CMVSerial
{
public:
	CMVSerial(PvDeviceGEV *lDevice);
	~CMVSerial();

	PvDeviceGEV							*mDevice;
	PvDeviceSerialPort					mSerialPort;
	int									m_iSerialPort;
	PvDeviceAdapter						*mDeviceAdapter;
	PvSerialPortConfiguration			mSerialComConfig;
	PvSerialBridge						mSerialBridge;

	DWORD OpenSerialPort(BYTE mSystemPort);
	DWORD ShutSerialPort();
	DWORD SerialWrite(BYTE mSystemPort,PBYTE DataBuf,uint32_t SVStartByteNum);
	DWORD SerialRead(BYTE mSystemPort,PBYTE DataBuf,uint32_t ReByteNum);
	DWORD SerialWriteRead(BYTE mSystemPort,PBYTE SDataBuf,uint32_t SendByteNum,PBYTE RDataBuf,uint32_t* ReceiveByteNum);
	DWORD EnableVirtualSerialPort(int mSystemPort,int mBitSize, int mBaudRate, int mStopBits, int mParity, int PvPort);
	DWORD DisableVirtualSerialPort();

	CString Int2Str(int num);
	CString Int2Str(DWORD num);
	CString Byte2Str(BYTE num);
	CString GetASCData(int port, CString lStr); //ASC发送接收
	DWORD SetASCData(int port, CString lStr); //ASC发送

	DWORD SetRegValue(int port, DWORD aAddress,uint32_t aValue);
	DWORD GetRegValue(int port, DWORD lAddr, uint32_t &lVal);

	PvSerialTerminalWnd mSerialTerminalWnd;
	void OpenSerialWindow(HWND lWnd);
};