#ifndef MVGEVSOURCE_H_
#define MVGEVSOURCE_H_

//MFC Special
#include <afx.h>
#include <afxwin.h>

#include <PvDeviceGEV.h>
#include <PvStreamGEV.h>
#include <PvPipeline.h>
#include <PvBuffer.h>
#include <PvStreamGEV.h>
#include <PvDeviceFinderWnd.h>
#include <PvDisplayWnd.h>
#include <PvGenBrowserWnd.h>
#include <PvBufferConverter.h>
//#include "DisplayThread.h"
#include <vector>
#include "MVSerialPack.h"



#define CHANNEL_COUNT 4

typedef BOOL (*VideoCallback)( PvImage* pData, void* pUserData);

enum IMG_TYPE
{
	FILE_RAW = 0,
	FILE_BMP = 1,
	FILE_JPG = 2
};

class CMvImgSaver
{
public:
	CMvImgSaver();
	~CMvImgSaver();
private:
	PvBufferConverter lConverter;
	CString lsPath;
	PvBuffer* lRgbBuffer;
	PvImage* lRgbImage;
	BYTE* lRgbData;
public:
	BOOL SetSavePath(CString lPath);
	BOOL SaveImage(PvImage * pImg, CString lFileName, IMG_TYPE lType);
private:
	//RAM SAVING
	PvBuffer* lRamBuffer;
	PvImage* lRamImage;
	int64_t lRamAllocedSize;

	BYTE* lRamPicBuffer;
	int64_t lRamPicNumber;
	int64_t lRamPointerW;
	int64_t lRamPointerR;

	int64_t lRamImgSizeX;
	int64_t lRamImgSizeY;
	PvPixelType lRamImgPixelType;

	BOOL lbInited;
	BOOL lbLocked;

public:
	//RAM SAVING
	BOOL AllocRamImgBuffer(int lWidth, int lHeight, PvPixelType lPixelType, int lMaxNum);
	BOOL WriteRamImgBuffer(PvImage * pImg);
	BOOL ResetRamImgBuffer(BOOL lResetBuf=TRUE);
	BOOL SaveRamImage(BYTE * pBuf, CString lFileName, IMG_TYPE lType);
	BOOL StartSaveRamImg(int lThreadCPU=-1);
	int GetRam_iImgNumber(){return lRamPicNumber;};

	static DWORD WINAPI RamImgSaving(LPVOID lpParam);
};

class CMvGevSource:public CWnd
{
public:
	CMvGevSource(void);
	~CMvGevSource(void);

	static DWORD WINAPI GrabFunction(LPVOID lpParam);
public:
	void SetCallback(VideoCallback pVideoCallback, void* pUserData) ;
	CMvImgSaver		 mImageSaver;
	CMVSerial*		 mSerial;
	PvDeviceGEV		*lDevice;
	BOOL Open(HWND hWnd, TCHAR* sConfigFile = NULL, int lChannel = 0);
	BOOL Start(int streamChannel, BOOL lbController, int lThreadCPU=-1);
	void Stop(int streamChannel, BOOL lbController) ;
	BOOL Resize(int streamChannel, int lWidth, int lHeight);
	BOOL IsCapturing(int streamChannel){return mbStart[streamChannel];};
	std::vector <CString> ip;
	void Close();
	void ShowParams();
	void ShowStream();
	void SetInnerDisplayEnable(BOOL lbEnable);
	BOOL DisplayWindow(int lDispChanner, HWND hWnd, BOOL lEmbed, char* lStrTitle="");
	BOOL DisplayBuffer(int lDispChanner, PvBuffer* lBuffer, char* lStrTitle="");

	int GetCapNum(){return miCapedNum;};

	PvStreamGEV* GetStream();

	BOOL FindExePath( CString &sPath );
	BOOL OpenConfig(CString sFile);
	BOOL GetOpenStatus(){return mbOpen;};
	int GetWidth();
	int GetHeight();
	PvPixelType GetPixelFormat();

	// Gev ²Ù×÷º¯Êý
	const PvDeviceInfo *SelectDevice( PvDeviceFinderWnd *aDeviceFinderWnd );
	PvDeviceGEV *ConnectToDevice( const PvDeviceInfo *aDeviceInfo , BOOL lController = TRUE);
	PvStreamGEV *OpenStream( const PvDeviceInfo *aDeviceInfo , int streamChannel );
	void ConfigureStream( PvDevice *aDevice, PvStream *aStream, int lChannel=0 );
	PvPipeline* CreatePipeline( PvDevice *aDevice, PvStream *aStream , int lChannel=0 );

private:
	PvDisplayWnd		lDisplayWnd;
	VideoCallback		mpCallback;
	void*				mpUserData;
	BOOL				mbInnerDisplayEnable;

	int64_t miWidth;
	int64_t miHeight;
	PvPixelType mPixelType;

	PvDeviceFinderWnd	lDeviceFinderWnd;
	const PvDeviceInfo	*lDeviceInfo;
			
	PvStreamGEV			*lStream;
	PvPipeline			*lPipeline;

	PvGenCommand		*lStart;
	PvGenCommand		*lStop;
	PvGenParameterArray *lDeviceParams;
	PvGenParameterArray *lStreamParams;
	PvGenFloat			*lFrameRate;
	PvGenFloat			*lBandwidth;

	HANDLE mhCapThread;
	BOOL mbThreadExit;
	BOOL mbStart[2];
	BOOL mbOpen;

	int miCapedNum;

	PvDisplayWnd	 mInnerDisplay;
	PvDisplayWnd	 mDisplayEx[CHANNEL_COUNT+1];
};
#endif //MVGEVSOURCE_H_