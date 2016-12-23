#include "stdafx.h"
#include "MVSerialPack.h"

CMVSerial::CMVSerial(PvDeviceGEV *lDevice)
{
	m_iSerialPort = 0;
	mDevice = lDevice;
	mDeviceAdapter = new PvDeviceAdapter( mDevice );
	OutputDebugString(_T("====================》MVSerial Init OK!"));
}

CMVSerial::~CMVSerial()
{
	OutputDebugString(_T("====================》MVSerial Destory OK!"));
	if(mSerialPort.IsOpened()) 
	{
		mSerialPort.Close();
	}
	mSerialBridge.Stop();
	if ( mSerialTerminalWnd.GetHandle() != 0 )
	{
		mSerialTerminalWnd.Close();
	}
	if(mDeviceAdapter)
	{
		delete mDeviceAdapter;
		mDeviceAdapter = NULL;
	}
}

//串口通讯 Serial Port Communications
DWORD CMVSerial::OpenSerialPort(BYTE mSystemPort)
{
	PvResult  RetVal;
	if(mSerialPort.IsOpened()&&(m_iSerialPort==mSystemPort))
	{
		return PV_OK;
	}
	else 
	{
		if(mSerialPort.IsOpened()) 
		{
			mSerialPort.Close();
		}

		m_iSerialPort = mSystemPort;
		RetVal = mSerialPort.Open(mDeviceAdapter,(PvDeviceSerial)(PvDeviceSerial0 + m_iSerialPort) );
		TRACE("=====>mSystemPort 0 Open RetVal is 0x%x",RetVal);
	}
	return RetVal.GetCode();
}

DWORD CMVSerial::ShutSerialPort()
{
	PvResult  RetVal;
	RetVal = mSerialPort.Close();
	return RetVal.GetCode();
}

DWORD CMVSerial::EnableVirtualSerialPort(int mSystemPort,int mBitSize, int mBaudRate,int mStopBits,int mParity, int PvPort)
{
	mSerialComConfig.mBaudRate = mBaudRate;
	mSerialComConfig.mParity = (PvParity)mParity;
	mSerialComConfig.mByteSize = mBitSize;
	mSerialComConfig.mStopBits = mStopBits;
	ShutSerialPort();
	CString lPortStr;
	lPortStr.Format("COM%d",mSystemPort);
	PvResult lResult = mSerialBridge.Start( lPortStr.GetBuffer(), mSerialComConfig, mDeviceAdapter, (PvDeviceSerial)PvPort );
	TRACE("EnableVirtualSerialPort %s lResult is %x",lPortStr,lResult.GetCode());
	return lResult.GetCode();
}

DWORD CMVSerial::DisableVirtualSerialPort()
{
	PvResult lResult = mSerialBridge.Stop();
	return lResult.GetCode();
}

DWORD CMVSerial::SerialWriteRead(BYTE mSystemPort,PBYTE SDataBuf,uint32_t SendByteNum,PBYTE RDataBuf,uint32_t* ReceiveByteNum)
{
	PvResult lResult1 = PV_OK;
	PvResult lResult2= PV_OK;

	if(ReceiveByteNum == NULL) return PV_INVALID_PARAMETER;
	if(SDataBuf == NULL) return PV_INVALID_PARAMETER;
	if(RDataBuf == NULL) return PV_INVALID_PARAMETER;

	PvResult lResult = PV_OK;

	OpenSerialPort(mSystemPort);

	mSerialPort.FlushRxBuffer();
	lResult1 = mSerialPort.Write(SDataBuf,SendByteNum, SendByteNum);// 
	if(!lResult1.IsOK()) 
	{
		TRACE("[MVC_GEV:Error] SerialWriteRead Write Failed! %s",lResult1.GetCodeString().GetAscii());
		return lResult1.GetCode();
	}

	::Sleep(30);
	uint32_t Lens = 0;
	for(int m=0;m<10;m++)
	{
		mSerialPort.GetRxBytesReady(Lens);
		if(Lens>=*ReceiveByteNum)
		{
			break;
		}
		::Sleep(30);
	}

	//读Reg命令
	if(*ReceiveByteNum == 0)
	{
		*ReceiveByteNum = Lens;
	}
	//lResult2 = mSerialPort.Read(RDataBuf,*ReceiveByteNum,(PvUInt32&)Lens,m_iSerialTimeout);

	int lTotalBytesRead = 0;
	int lToReadLens = *ReceiveByteNum;
	while ( lTotalBytesRead < lToReadLens )
	{
		uint32_t lBytesRead = 0;
		lResult2 = mSerialPort.Read(RDataBuf + lTotalBytesRead ,lToReadLens - lTotalBytesRead,lBytesRead,100);
		if ( lResult2.GetCode() == PvResult::Code::TIMEOUT )
		{
			break;
		}
		lTotalBytesRead += lBytesRead;
	}
	*ReceiveByteNum = lTotalBytesRead;

	if(!lResult2.IsOK()) 
	{
		TRACE("[MVC_GEV:Error] SerialWriteRead Read Failed! %s",lResult2.GetCodeString().GetAscii());
	}

	return lResult2.GetCode();
} 


DWORD CMVSerial::SerialWrite(BYTE mSystemPort,PBYTE DataBuf,uint32_t SVStartByteNum)
{
	PvResult lResult = PV_OK;

	OpenSerialPort(mSystemPort);

	mSerialPort.FlushRxBuffer();
	lResult = mSerialPort.Write(DataBuf,SVStartByteNum,SVStartByteNum);

	return lResult.GetCode();
}

DWORD CMVSerial::SerialRead(BYTE mSystemPort,PBYTE DataBuf,uint32_t ReByteNum) 
{
	PvResult lResult = PV_OK;

	OpenSerialPort(mSystemPort);

	if(ReByteNum == 0)
	{
		mSerialPort.GetRxBytesReady(ReByteNum);
	}
	lResult = mSerialPort.Read(DataBuf,ReByteNum,ReByteNum,SERIAL_TIMEOUT);


	return lResult.GetCode();
}

CString CMVSerial::Int2Str(int num)
{
	CString ks;
	char tmp[255];
	sprintf_s(tmp,"%d",num);
	ks = tmp;
	return ks;
}

CString CMVSerial::Int2Str(DWORD num)
{
	CString ks;
	char tmp[255];
	sprintf_s(tmp,"%d",num);
	ks = tmp;
	return ks;
}

CString CMVSerial::Byte2Str(BYTE num)
{
	CString ks;
	char tmp[255];
	sprintf_s(tmp,"%02x",num);
	ks = tmp;
	return ks;
}

DWORD CMVSerial::SetASCData(int port, CString lStr)
{
	BYTE SendBuf[255];
	DWORD SendNum = 0;

	memset(SendBuf,0,255);//按255字节清空
	for(DWORD m=0;m<SendNum;m++)
	{
		SendBuf[m] = (BYTE)lStr.GetAt(m);
	}

	SendNum = lStr.GetLength();
	return SerialWrite(port,SendBuf,SendNum);
}

CString CMVSerial::GetASCData(int port, CString lStr)
{
	BYTE SendBuf[255];
	BYTE ReadBuf[255];
	DWORD SendNum = 0;
	DWORD ReadNum = 0;

	memset(SendBuf,0,255);//按255字节清空
	memset(ReadBuf,0,255);//按255字节清空

	SendNum = lStr.GetLength();
	for(DWORD m=0;m<SendNum;m++)
	{
		SendBuf[m] = (BYTE)lStr.GetAt(m);
	}
	ReadNum = 255;
	
	DWORD RetVal1 = SerialWrite(port,SendBuf,SendNum);
	if(RetVal1!=PV_OK) 
	{
		return _T("Write Err");
	}

	Sleep(SERIAL_INTERVAL*10);

	DWORD RetVal2 = SerialRead(port,ReadBuf,ReadNum);
	OutputDebugString(_T("ReadNum is ")+Int2Str(ReadNum)+_T("\n"));
	if(RetVal2!=PV_OK) 
	{
		return _T("Read Err");
	}
	
	CString StrRead;
	for(DWORD k=0; k<ReadNum; k++)
	{
		CString ltemp;
		ltemp.Format(_T("%c"),ReadBuf[k]);
		StrRead = StrRead + ltemp;
	}
	return StrRead;
}

DWORD CMVSerial::GetRegValue(int port, DWORD aAddress,uint32_t &aValue)
{
	char DbgStr[255];
	DWORD RetVal;

	aValue = 0;

	DWORD SENDSIZE = 7;
	unsigned char lData[14],RBuf[14];
	uint32_t RBufSize;

	lData[0] = 0xb0;
	lData[1] = (unsigned char) ( ( aAddress >> 24 ) & 0x00FF );//0x00;
	lData[2] = (unsigned char) ( ( aAddress >> 16 ) & 0x00FF );//0x00;
	lData[3] = (unsigned char) ( ( aAddress >> 8 ) & 0x00FF );
	lData[4] = (unsigned char) ( aAddress & 0x00FF );
	lData[5] = 0x1d;
	lData[6] = 0x1a;

	RBufSize = 6;
	RetVal = SerialWriteRead(port,lData,7,RBuf,&RBufSize);
	if(RetVal == PV_OK)
	{
		aValue = (DWORD)RBuf[3] + ((DWORD)RBuf[2]<<8) + ((DWORD)RBuf[1]<<16) + ((DWORD)RBuf[0]<<24);
		sprintf_s(DbgStr,"ReadComand @ aAddress 0x%x OK: %x %x %x %x %x %x(%x)",aAddress, RBuf[0],RBuf[1],RBuf[2],RBuf[3],RBuf[4],RBuf[5],aValue);
		//OutputDebugString(DbgStr);
	}
	else if((RBuf[4] == 0x55)&&(RBuf[5] == 0xAA))
	{
		aValue = (DWORD)RBuf[3] + ((DWORD)RBuf[2]<<8) + ((DWORD)RBuf[1]<<16) + ((DWORD)RBuf[0]<<24);
		sprintf_s(DbgStr,"ReadComand @ aAddress 0x%x Res(%d): %x %x %x %x %x %x(%x)",aAddress, RetVal, RBuf[0],RBuf[1],RBuf[2],RBuf[3],RBuf[4],RBuf[5],aValue);
		OutputDebugString(DbgStr);
	}
	else
	{
		sprintf_s(DbgStr,"ReadComand @ aAddress 0x%x Failed!",aAddress);
		OutputDebugString(DbgStr);
	}

	//TODO:Sleep?
	//Sleep(100);

	return RetVal;
}

DWORD CMVSerial::SetRegValue(int port, DWORD aAddress,uint32_t aValue)
{
	DWORD RetVal;
	char DbgStr[255];

	DWORD SENDSIZE = 14;
	unsigned char lData[14],RBuf[4];
	uint32_t RBufSize;

	lData[0] = 0xa0;
	lData[1] = (unsigned char) ( ( aAddress >> 24 ) & 0x00FF );//0x00;
	lData[2] = (unsigned char) ( ( aAddress >> 16 ) & 0x00FF );//0x00;
	lData[3] = (unsigned char) ( ( aAddress >> 8 ) & 0x00FF );
	lData[4] = (unsigned char) ( aAddress & 0x00FF );
	lData[5] = 0x0d;
	lData[6] = 0x0a;

	lData[7] = 0xa1;
	lData[8] = (unsigned char) ( ( aValue >> 24 ) & 0x00FF );//0x00;
	lData[9] = (unsigned char) ( ( aValue >> 16 ) & 0x00FF );//0x00;
	lData[10] = (unsigned char) ( ( aValue >> 8 ) & 0x00FF );
	lData[11] = (unsigned char) ( aValue & 0x00FF );
	lData[12] = 0x0d;
	lData[13] = 0x0a;

	RBufSize = 2;
	RetVal = SerialWriteRead(port,lData,14,RBuf,&RBufSize);
	if(RetVal == PV_OK)
	{
		//OutputDebugString("Send SendComand Ok !");
	}
	else
	{
		sprintf_s(DbgStr,"SendComand @ aAddress 0x%x With aValue 0x%x Failed (%d)!",aAddress, aValue, RetVal);
		OutputDebugString(DbgStr);
	}

	return RetVal;
}

void CMVSerial::OpenSerialWindow(HWND lWnd)
{
	if(mSerialPort.IsOpened()) 
	{
		mSerialPort.Close();
	}
	if ( mSerialTerminalWnd.GetHandle() != 0 )
	{
		mSerialTerminalWnd.Close();
	}
	else
	{
		mSerialTerminalWnd.SetDevice( mDeviceAdapter );
		mSerialTerminalWnd.ShowModeless( lWnd );
	}
}