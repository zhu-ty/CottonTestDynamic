
//#include "StdAfx.h"
#include "MVGevSource.h"
#include <PvConfigurationReader.h>
#include <MV_BufferSaver.h>
#ifdef _WIN64
	#pragma comment(lib, "MV_BufferSaver64.lib")
#else
	#pragma comment(lib, "MV_BufferSaver.lib")
#endif

#define SKIP_DSPLAY  ( 01 )
#define BUFFER_COUNT ( 32 )
#define JPG_QUALITY  ( 60 )
#define BAYER_LEVEL  ( 01 )
#define RAMSAV_TYPE  ( 00 )
#define ALLOW_DEFECT ( 00 )

CMvImgSaver::CMvImgSaver():
	lbInited(FALSE),
	lbLocked(FALSE),
	lsPath(""),
	lRgbData(NULL),
	lRamPicBuffer(NULL),
	lRamAllocedSize(0),
	lRamPicNumber(0),
	lRamPointerR(0),
	lRamPointerW(0)
{
	lRgbBuffer = new PvBuffer();
	lRamBuffer = new PvBuffer();
}

CMvImgSaver::~CMvImgSaver()
{
	if(lRgbBuffer!=NULL)
	{
		delete lRgbBuffer;
		lRgbBuffer = NULL;
	}
	if(lRgbData)
	{
		delete lRgbData;
		lRgbData = NULL;
	}
	if(lRamBuffer!=NULL)
	{
		delete lRamBuffer;
		lRamBuffer = NULL;
	}
	if(lRamPicBuffer)
	{
		delete lRamPicBuffer;
		lRamPicBuffer = NULL;
	}
}

BOOL CMvImgSaver::SaveImage(PvImage * pImg, CString lFileName, IMG_TYPE lType)
{
	CFileFind find;
	if(find.FindFile(lsPath))
	{
		int lXSize = pImg->GetWidth();
		int lYSize = pImg->GetHeight();
		BYTE* lBuf = pImg->GetDataPointer();
		int lSize = pImg->GetImageSize();
		int lPType = pImg->GetPixelType();
		TRACE("CMvImgSaver Start Save File With Type :%d W:%d H:%d PixelType:0x%x",lType,lXSize,lYSize,lPType);
		CString lFilePath = _T(lsPath +"\\"+lFileName);
		if(lType == FILE_RAW)
		{
			return MVBUF_SaveImageEx(lBuf,lSize,lXSize,lYSize, 2, lFilePath.GetBuffer(), IMG_FILE_RAW);
		}
		else if(lType == FILE_BMP || lType == FILE_JPG)
		{
			if(pImg->GetPixelType()==PvPixelMono8)
			{
				return MVBUF_SaveImageEx(lBuf,lSize,lXSize,lYSize, 1, lFilePath.GetBuffer(), (lType == FILE_BMP)?IMG_FILE_BMP:IMG_FILE_JPG, JPG_QUALITY);
			}
			else //Color Bayer->RGB
			{
				if(lRgbData) {delete lRgbData;}
				BOOL lResSave = TRUE;
				lRgbData = new BYTE[lXSize*lYSize*4];
				lRgbImage = lRgbBuffer->GetImage();
				lRgbImage->Attach(lRgbData,lXSize,lYSize,PvPixelBGR8); //RGB24
				lConverter.SetBayerFilter((PvBayerFilterType)BAYER_LEVEL); //01:2x2 02:3x3
				PvResult lRes = lConverter.Convert(pImg->GetBuffer(),lRgbBuffer);
				if(lRes==PV_OK)
				{
					lResSave = MVBUF_SaveImageEx(lRgbData,lXSize*lYSize*3,lXSize,lYSize, 3, lFilePath.GetBuffer(), (lType == FILE_BMP)?IMG_FILE_BMP:IMG_FILE_JPG, JPG_QUALITY);
					lRgbImage->Detach();
				}
				else
				{					
					lRgbImage->Detach();
					return FALSE;
				}
				return lResSave;
			}
		}
	}
	else
	{
		return FALSE;
	}
	return FALSE;
}

BOOL CMvImgSaver::SetSavePath(CString lPath)
{
	CFileFind find;
	if(!find.FindFile(lPath))
	{
		//::SHCreateDirectoryEx(0,lPath,NULL);
		//TODO: CreatPath
		find.Close();
		lsPath = lPath;
		return FALSE;
	}
	find.Close();
	lsPath = lPath;
	return TRUE;
}

BOOL CMvImgSaver::SaveRamImage(BYTE * pBuf, CString lFileName, IMG_TYPE lType)
{
	if(!lbInited) return FALSE;
	CFileFind find;
	if(find.FindFile(lsPath))
	{
		int lXSize = lRamImgSizeX;
		int lYSize = lRamImgSizeY;
		BYTE* lBuf = pBuf;
		int lSize = lXSize*lYSize;
		//TRACE("CMvImgSaver Start Save File With Type :%d",lType);
		CString lFilePath = _T(lsPath +"\\"+lFileName);
		if(lType == FILE_RAW) //Raw
		{
			return MVBUF_SaveImageEx(pBuf,lSize,lXSize,lYSize, 1, lFilePath.GetBuffer(), IMG_FILE_RAW);
		}
		else if(lType == FILE_BMP || lType == FILE_JPG)
		{
			if(lRamImgPixelType==PvPixelMono8) //Gray
			{
				return MVBUF_SaveImageEx(lBuf,lSize,lXSize,lYSize, 1, lFilePath.GetBuffer(), (lType == FILE_BMP)?IMG_FILE_BMP:IMG_FILE_JPG, JPG_QUALITY);
			}
			else //Color Bayer->RGB
			{
				if(lRgbData) {delete lRgbData;}
				BOOL lResSave = TRUE;
				lRamImage = lRamBuffer->GetImage();
				lRamImage->Attach(lBuf,lXSize,lYSize,lRamImgPixelType); //RGB24

				lRgbData = new BYTE[lXSize*lYSize*4];
				lRgbImage = lRgbBuffer->GetImage();
				lRgbImage->Attach(lRgbData,lXSize,lYSize,PvPixelBGR8); //RGB24

				lConverter.SetBayerFilter(PvBayerFilter3X3);
				PvResult lRes = lConverter.Convert(lRamBuffer,lRgbBuffer);
				if(lRes==PV_OK)
				{
					lResSave = MVBUF_SaveImageEx(lRgbData,lXSize*lYSize*3,lXSize,lYSize, 3, lFilePath.GetBuffer(), (lType == FILE_BMP)?IMG_FILE_BMP:IMG_FILE_JPG, JPG_QUALITY);
					lRgbImage->Detach();
				}
				else
				{	
					lRamImage->Detach();
					lRgbImage->Detach();
					return FALSE;
				}
				return lResSave;
			}
		}
	}
	else
	{
		return FALSE;
	}
	return FALSE;
}

BOOL CMvImgSaver::AllocRamImgBuffer(int lWidth, int lHeight, PvPixelType lPixelType, int lMaxNum)
{
	ResetRamImgBuffer();
	lRamImgSizeX = lWidth;
	lRamImgSizeY = lHeight;
	lRamImgPixelType = lPixelType; //8bit Only
	lRamPicBuffer = new BYTE[lRamImgSizeX*lRamImgSizeY*lMaxNum];

	if(lRamPicBuffer==NULL)
	{
		TRACE("[ERROR] AllocRamImgBuffer Failed @ lMaxNum :%d, No Enough Memory",lMaxNum);
	}
	else
	{
		lRamAllocedSize = lRamImgSizeX*lRamImgSizeY*lMaxNum;
		TRACE("[OK] AllocRamImgBuffer OK @ lMaxNum :%d, Total Alloc Memory %llu",lMaxNum, lRamAllocedSize);
		lbInited = TRUE;
	}
	return TRUE;
}

BOOL CMvImgSaver::ResetRamImgBuffer(BOOL lResetBuf)
{
	if(lRamPicBuffer && lResetBuf)
	{
		delete lRamPicBuffer;
		//VirtualFree(lRamPicBuffer,0,MEM_RELEASE);
		lRamAllocedSize = 0;
		lbInited = FALSE;
	}
	lbLocked = FALSE;
	lRamPicNumber = 0;
	lRamPointerR = lRamPointerW = NULL;
	TRACE("[OK] RamImgBuffer Reset With Flag %d",lResetBuf);
	return TRUE;
}

BOOL CMvImgSaver::WriteRamImgBuffer(PvImage * pImg)
{
	if(lbLocked) 
	{
		return FALSE;
	}
	//Size Control
	if(lRamImgSizeX!=pImg->GetWidth() || lRamImgSizeY!=pImg->GetHeight() || lRamImgPixelType!=pImg->GetPixelType())
	{
		return FALSE;
	}
	//Overflow Control
	if(lRamPointerW + lRamImgSizeX*lRamImgSizeY > lRamAllocedSize)
	{
		TRACE("[ERROR] Write Overflowed Alloc Size %llu @ lRamPicNumber %d", lRamAllocedSize, lRamPicNumber);
	}
	else
	{
		//Write PvImg to RAM
		memcpy(lRamPicBuffer+lRamPointerW, pImg->GetDataPointer(), lRamImgSizeX*lRamImgSizeY);
		lRamPointerW += lRamImgSizeX*lRamImgSizeY;
		TRACE("[OK] WriteRamImgBuffer Ok @ lRamPicNumber :%d", lRamPicNumber);
		lRamPicNumber++;
	}
	return TRUE;
}

BOOL CMvImgSaver::StartSaveRamImg(int lThreadCPU)
{
	HANDLE hSaveThread;
	DWORD dwThreadId;
	TRACE("Start RamImgSaving Process @ Core %d!",lThreadCPU);
	hSaveThread = CreateThread(NULL, 0,RamImgSaving, this, 0, &dwThreadId);
	lbLocked = TRUE; //ReadOnly
	if(lThreadCPU!=-1) //分配CPU给线程
	{
		SetThreadAffinityMask(hSaveThread, 1<<lThreadCPU);
	}
	return TRUE;
}

DWORD WINAPI CMvImgSaver::RamImgSaving(LPVOID lpParam)
{
	CMvImgSaver* pSaver = (CMvImgSaver*)lpParam;
	//TRACE("=======>Ram Pic Number To Save is %d",pSaver->lRamPicNumber);
	for(DWORD m=0; m<pSaver->lRamPicNumber; m++)
	{
		CString lStrRamImgName;
		CString lStrExt = ".jpg";
		if(RAMSAV_TYPE==FILE_BMP) lStrExt = ".bmp";
		if(RAMSAV_TYPE==FILE_RAW) lStrExt = ".raw";
		lStrRamImgName.Format("RAM_IDX_%04d%s",m,lStrExt);
		//TRACE("Ram Pic Number To Save is %d",pSaver->lRamPicNumber);
		pSaver->SaveRamImage(pSaver->lRamPicBuffer + pSaver->lRamPointerR, lStrRamImgName.GetBuffer(),(IMG_TYPE)RAMSAV_TYPE);
		TRACE("Ram Pic [%s] Saved",lStrRamImgName);
		pSaver->lRamPointerR += pSaver->lRamImgSizeX*pSaver->lRamImgSizeY;
	}
	//TRACE("=======>Ram Pic Saved Num %d Over",pSaver->lRamPicNumber);
	return TRUE;
}

CMvGevSource::CMvGevSource(void):
	mpUserData(NULL),
	mpCallback(NULL),
	lDeviceInfo(NULL),
	lDevice(NULL),
	lDeviceParams(NULL),
	lStream(NULL),
	lPipeline(NULL),
	lStreamParams(NULL),
	mhCapThread(NULL) ,
	mbOpen(FALSE),
	mbThreadExit(FALSE),
	mbInnerDisplayEnable(FALSE),
	mSerial(NULL)
{
	mbStart[0] = mbStart[1] = 0;
}
		
CMvGevSource::~CMvGevSource(void)
{
	Close();
}

BOOL CMvGevSource::DisplayWindow(int lDispChanner, HWND hWnd, BOOL lEmbed, char* lStrTitle)
{
	if(mDisplayEx[lDispChanner].GetHandle()!=NULL)
	{
		mDisplayEx[lDispChanner].Close();
	}
	CRect rect;
	if(lEmbed)
	{
		::GetClientRect(hWnd, rect);
		mDisplayEx[lDispChanner].Create( hWnd, 10000+lDispChanner*50);
	}
	else
	{
		::GetWindowRect(hWnd, rect);
		mDisplayEx[lDispChanner].ShowModeless( hWnd );
		
	}
	mDisplayEx[lDispChanner].SetPosition( rect.left, rect.top, rect.Width(), rect.Height() );
	mDisplayEx[lDispChanner].SetBackgroundColor( 0x80, 0x80, 0x80 );
	mDisplayEx[lDispChanner].SetTitle(lStrTitle);

	return 0;
}

BOOL CMvGevSource::DisplayBuffer(int lDispChanner, PvBuffer* lBuffer, char* lStrTitle)
{
	if(mDisplayEx[lDispChanner].GetHandle()!=NULL)
	{
		if(lStrTitle != NULL)
		mDisplayEx[lDispChanner].SetTitle(lStrTitle);
		return mDisplayEx[lDispChanner].Display( *lBuffer ).GetCode();
	}
	else
	{
		return PV_NO_AVAILABLE_DATA;
	}
}

DWORD WINAPI CMvGevSource::GrabFunction(LPVOID lpParam)
{	
	CMvGevSource* pSource = (CMvGevSource*)lpParam;
	while(!pSource->mbThreadExit)
	{
		PvBuffer *lBuffer = NULL;
		PvResult lOperationResult;
		PvResult lResult = pSource->lPipeline->RetrieveNextBuffer( &lBuffer, 1000, &lOperationResult );
		if ( lResult.IsOK() )
		{
			pSource->miCapedNum++;
			if ( lOperationResult.IsOK() || ALLOW_DEFECT)
			{
				PvPayloadType lType;

				// If the buffer contains an image.
				lType = lBuffer->GetPayloadType();

				if ( lType == PvPayloadTypeImage )
				{
					// Get image specific buffer interface.
					PvImage *lImage = lBuffer->GetImage();
					//TRACE("========>BlockID: %llu", lBuffer->GetBlockID());

					if(pSource->mbInnerDisplayEnable)
					{
						if(pSource->miCapedNum % SKIP_DSPLAY == 0)
						{
							pSource->mInnerDisplay.Display( *lBuffer );
						}
					}

					if (pSource->mpCallback!=NULL)
					{
						//OutputDebugString(_T("Gev Capture Callback."));
						pSource->mpCallback(lImage, pSource->mpUserData);
					}
				}
			}
			else
			{
				TRACE("[ERROR] RetrieveNextBuffer Not OK! lRes %x-%s",lOperationResult.GetCode(),lOperationResult.GetCodeString().GetAscii());
			}

			// Release the buffer back to the pipeline
			pSource->lPipeline->ReleaseBuffer( lBuffer );
		}
		else
		{
			Sleep(0);
		}
	}
	OutputDebugString("=====================>GrabFunction Exited!");
	return 0;
}

void CMvGevSource::SetInnerDisplayEnable(BOOL lbEnable) 
{
	mbInnerDisplayEnable = lbEnable;
}

void CMvGevSource::SetCallback(VideoCallback pVideoCallback, void* pUserData) 
{		
	if (pVideoCallback == NULL)
		return;
	mpCallback = pVideoCallback;
	mpUserData = pUserData;
}

BOOL CMvGevSource::OpenConfig(CString sFile)
{
	PvConfigurationReader lReader;
	PvResult lResult = lReader.Load( (LPCTSTR)sFile );
	if ( !lResult.IsOK() )
	{
		return FALSE;
	}

	if (lReader.GetDeviceCount() > 0)
	{
		lReader.Restore(0, lDevice);		
	}

	if (lReader.GetStreamCount() > 0)
	{
		lReader.Restore(0, lStream);		
	}

	return TRUE;
}

BOOL CMvGevSource::Start(int streamChannel, BOOL lbController, int lThreadCPU) 
{
	if (!mbStart[streamChannel] && mbOpen)
	{
		lDeviceParams = lDevice->GetParameters();

		PvGenEnum * lSourceSelector = dynamic_cast<PvGenEnum *>(lDeviceParams->Get("SourceSelector"));
		if(lbController)
		{
			if(lSourceSelector!=NULL)
			{
				if (streamChannel == 0) 
				{
					lSourceSelector->SetValue("Source1");
				}
				else if (streamChannel == 1) 
				{
					lSourceSelector->SetValue("Source2");
				}
				else
				{
					return FALSE;
				}
			}

			// Map the GenICam AcquisitionStart and AcquisitionStop commands
			lStart = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStart" ) );
			lStop = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStop" ) );
		}

		lStreamParams = lStream->GetParameters();

		// Map a few GenICam stream stats counters
		lFrameRate = dynamic_cast<PvGenFloat *>( lStreamParams->Get( "AcquisitionRate" ) );
		lBandwidth = dynamic_cast<PvGenFloat *>( lStreamParams->Get( "Bandwidth" ) );
			
		if(lbController)
		{
			uint32_t lSize = lDevice->GetPayloadSize();
			lPipeline->SetBufferSize( lSize );
		}

		lPipeline->Start();

		if(lbController)
		{
			lDevice->StreamEnable();
			lStart->Execute();
		}

		if((lbController && (streamChannel==0) ) || (!lbController && (streamChannel==1)))
		{
			mbThreadExit = FALSE;
			mhCapThread = CreateThread(NULL, 0, GrabFunction, this, 0, NULL);

			if(lThreadCPU!=-1)
			{
				SetThreadAffinityMask(mhCapThread, 1<<lThreadCPU);
			}
		}

		mbStart[streamChannel] = TRUE;
		miCapedNum = 0;

		return TRUE;
	}
	return FALSE;
}

int CMvGevSource::GetWidth()
{
	return miWidth;
}
	
int CMvGevSource::GetHeight()
{
	return miHeight;
}

PvPixelType CMvGevSource::GetPixelFormat()
{
	return mPixelType;
}

void CMvGevSource::Stop(int streamChannel, BOOL lbController)
{
	if (mbStart[streamChannel])
	{			
		mbThreadExit = TRUE;
		// Close Thread
		if ( WAIT_TIMEOUT == WaitForSingleObject(mhCapThread, 1000))
		{
			TerminateThread(mhCapThread, 0);
		}
		CloseHandle(mhCapThread);

		PvGenEnum * lSourceSelector = dynamic_cast<PvGenEnum *>(lDeviceParams->Get("SourceSelector"));
		if(lbController)
		{
			if(lSourceSelector!=NULL)
			{
				if (streamChannel == 0) 
				{
					lSourceSelector->SetValue("Source1");
				}
				else if (streamChannel == 1) 
				{
					lSourceSelector->SetValue("Source2");
					
				}
			}
			// Map the GenICam AcquisitionStart and AcquisitionStop commands
			lStart = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStart" ) );
			lStop = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStop" ) );
		}

		// Stop
		if(lbController)
		{
			lStop->Execute();
			lDevice->StreamDisable();
			lPipeline->Stop();
		}

		mbStart[streamChannel] = FALSE;
	}
}

int64_t CreateIpAddress(const char * string)
{
	int64_t value;
	int64_t ip[4];
	memset(ip,0,sizeof(ip));
	::sscanf_s(string,"%d.%d.%d.%d",&ip[3], &ip[2], &ip[1], &ip[0]); 
	value = ip[3];

	for (int i=2 ; i>=0; i--)
	{
		value = value << 8;
		value += ip[i];
	}
	return value;
}

void CMvGevSource::ShowParams()
{
	PvGenBrowserWnd *aWnd = new PvGenBrowserWnd;
	aWnd->SetTitle( PvString( "Control" ) );
	aWnd->SetGenParameterArray( lDevice->GetParameters() );
	aWnd->ShowModeless(  );
}

void CMvGevSource::ShowStream()
{
	PvGenBrowserWnd *aWnd = new PvGenBrowserWnd;
	aWnd->SetTitle( PvString( "Stream" ) );
	aWnd->SetGenParameterArray( lStream->GetParameters() );
	aWnd->ShowModeless(  );
}

BOOL CMvGevSource::Resize(int streamChannel, int lWidth, int lHeight)
{
	BOOL lbNeedRestart = FALSE;
	if(mbStart[streamChannel])
	{
		lbNeedRestart = TRUE;
		Stop(streamChannel,1-streamChannel);
	}
	PvGenParameterArray *pParams = lDevice->GetParameters();
	pParams->SetIntegerValue( "Width", lWidth);
	pParams->SetIntegerValue( "Height", lHeight);
	pParams->GetIntegerValue( "Width", miWidth);
	pParams->GetIntegerValue( "Height", miHeight);
	if(lbNeedRestart)
	{
		Start(streamChannel,1-streamChannel);
	}
	return TRUE;
}

BOOL CMvGevSource::Open(HWND hWnd, TCHAR* sConfigFile, int lChannel)
{	
	if (mbOpen)
	{
		return TRUE;
	}
	if (mbStart && (lChannel==0))
	{
		Stop(lChannel,0);
	}
		
	lDeviceInfo = SelectDevice( &lDeviceFinderWnd );

	if ( NULL != lDeviceInfo )
	{
		CString CStringinfo;
		CStringinfo.Format("%s",lDeviceInfo->GetDisplayID().GetAscii());
		int pos=0;
		CString CStrIPNum;
		CStrIPNum=CStringinfo.Tokenize(".",pos);
		for (int i=0;i<3;i++)
		{
			ip.push_back(CStrIPNum);
			CStrIPNum=CStringinfo.Tokenize(".",pos);
		}
		if ( lDevice = ConnectToDevice( lDeviceInfo ,lChannel==0?TRUE:FALSE) )
		{
			if ( lStream = OpenStream( lDeviceInfo, lChannel ) )
			{
				ConfigureStream( lDevice, lStream , lChannel);
				lPipeline = CreatePipeline( lDevice, lStream,  lChannel);
			}
		}
	}
	else
	{
		return FALSE;
	}

	if(!lDevice)
	{
		return FALSE;
	}

	PvGenParameterArray *pParams = lDevice->GetParameters();

	pParams->GetIntegerValue( "Width", miWidth);
	pParams->GetIntegerValue( "Height", miHeight);

	int64_t lFormat;
	pParams->GetEnumValue( "PixelFormat", lFormat );
	mPixelType =  static_cast<PvPixelType>( lFormat );

	//dynamic_cast<PvGenInteger *>(pParams->Get("GevStreamChannelSelector"))->SetValue(0);
	//dynamic_cast<PvGenInteger *>(pParams->Get("GevSCPHostPort"))->SetValue(50000);

	/*
	PvString lLocalIP = lStream->GetLocalIPAddress();
	TRACE("lLocalIP is %s",lLocalIP.GetAscii());
	int64_t ipAddr= CreateIpAddress(lLocalIP.GetAscii());
	PvResult lResult = dynamic_cast<PvGenInteger *>(pParams->Get("GevSCDA"))->SetValue(ipAddr);
	*/

	mbOpen = TRUE;

	CRect rect;
	::GetClientRect(hWnd, rect);
	mInnerDisplay.Create( hWnd, 10000 );
	mInnerDisplay.SetPosition( rect.left, rect.top, rect.Width(), rect.Height() );
	mInnerDisplay.SetBackgroundColor( 0x80, 0x80, 0x80 );

	mSerial = new CMVSerial(lDevice);
	
	return TRUE;
}

PvStreamGEV* CMvGevSource::GetStream()
{
	return lStream;
}

void CMvGevSource::Close()
{
	if (mbOpen)
	{
		mbThreadExit = TRUE;
		Sleep(1500);
		mInnerDisplay.Close();

		for(int m=0; m<CHANNEL_COUNT; m++)
		mDisplayEx[m].Close();

		//delete mDisplayThread;
		OutputDebugString(_T("Gev Capture Close."));

		if ( lPipeline != NULL )
		{
			if ( lPipeline->IsStarted() ) 
			{
				lPipeline->Stop();
			}

			delete lPipeline;
			lPipeline = NULL;
		}


		if(mSerial)
		{
			delete mSerial;
			mSerial = NULL;
		}

		// Disconnect the device
		if(lDevice!=NULL)
		{
			lDevice->Disconnect();
			PvDevice::Free( lDevice );
		}

		lStream->Close();
		if(lStream)
		{
			delete lStream;
			lStream = NULL;
		}
	}

	mbOpen = FALSE;
}

BOOL CMvGevSource::FindExePath( CString &sPath )
{
	TCHAR exeFullPath[MAX_PATH]; 
	CString strPath; 
	GetModuleFileName(NULL,exeFullPath,MAX_PATH); 
	strPath=(CString)exeFullPath; 
	int position=strPath.ReverseFind('\\'); 
	sPath=strPath.Left(position+1); 
	return TRUE;
}

const PvDeviceInfo *CMvGevSource::SelectDevice( PvDeviceFinderWnd *aDeviceFinderWnd )
{
	const PvDeviceInfo *lDeviceInfo = NULL;
	PvResult lResult;

	if (NULL != aDeviceFinderWnd)
	{
		// Display list of GigE Vision and USB3 Vision devices
		lResult = aDeviceFinderWnd->ShowModal();
		if ( !lResult.IsOK() )
		{
			return NULL;
		}

		// Get the selected device information.
		lDeviceInfo = aDeviceFinderWnd->GetSelected();
	}

	return lDeviceInfo;
}

PvDeviceGEV *CMvGevSource::ConnectToDevice( const PvDeviceInfo *aDeviceInfo , BOOL lController)
{
	PvDeviceGEV *lDevice;
	PvResult lResult;

	lDevice = new PvDeviceGEV();
	// Connect to the GigE Vision or USB3 Vision device
	//lDevice = PvDevice::CreateAndConnect( aDeviceInfo, &lResult );
	lResult = lDevice->Connect( aDeviceInfo, lController?PvAccessControl: PvAccessReadOnly);
	if ( lDevice == NULL )
	{
		CString sInfo;
		sInfo.Format(_T("Unable to connect to %s"), aDeviceInfo->GetDisplayID().GetAscii());
		OutputDebugString(sInfo);
	}

	return lDevice;
}
	
PvStreamGEV *CMvGevSource::OpenStream( const PvDeviceInfo *aDeviceInfo, int streamChannel )
{
	PvResult lResult;

	// Open stream to the GigE Vision or USB3 Vision device
	//lStream = PvStream::CreateAndOpen( aDeviceInfo->GetConnectionID(), &lResult );
	lStream = new PvStreamGEV;
	lStream->Open( aDeviceInfo->GetConnectionID() ,0, streamChannel );
	if ( lStream == NULL )
	{
		CString sInfo;
		sInfo.Format(_T("Unable to connect to %s @ Channel %d"), aDeviceInfo->GetConnectionID().GetAscii(), streamChannel);
		OutputDebugString(sInfo);
	}

	return lStream;
}

void CMvGevSource::ConfigureStream( PvDevice *aDevice, PvStream *aStream , int lChannel)
{
	// If this is a GigE Vision device, configure GigE Vision specific streaming parameters
	PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV *>( aDevice );
	if ( lDeviceGEV != NULL )
	{
		PvStreamGEV *lStreamGEV = static_cast<PvStreamGEV *>( aStream );

		// Negotiate packet size
		lDeviceGEV->NegotiatePacketSize();

		// Configure device streaming destination
		lDeviceGEV->SetStreamDestination( lStreamGEV->GetLocalIPAddress(), lStreamGEV->GetLocalPort() ,lChannel);
		TRACE("SetStreamDestination ip:%s @Channel :%d @Port %d\n",lStreamGEV->GetLocalIPAddress().GetAscii(), lChannel, lStreamGEV->GetLocalPort());
	}
}

PvPipeline *CMvGevSource::CreatePipeline( PvDevice *aDevice, PvStream *aStream ,  int lChannel)
{
	// Create the PvPipeline object
	PvPipeline* lPipeline = new PvPipeline( aStream );

	// Reading payload size from device
	uint32_t lSize = aDevice->GetPayloadSize();

	// Set the Buffer count and the Buffer size
	lPipeline->SetBufferCount( BUFFER_COUNT );
	if(lChannel==0)
	{
		lPipeline->SetBufferSize( lSize );
	}

	return lPipeline;
}