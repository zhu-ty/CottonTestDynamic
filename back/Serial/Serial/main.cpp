#define _CRT_SECURE_NO_WARNINGS

#include<iostream>
#include<mutex>
#include"MVGevSource.h"
#include"SerialBKServer.h"
#include"DataModel.h"
#include<PvPixelType.h>
#include<queue>
#include<vector>

uint cap_num = 0;
std::shared_ptr<mutex> mMutex;
std::shared_ptr<RawDataPack> data_pack;
std::vector<std::vector<ushort>> data_queue;
std::vector<ushort> data_temp;
int CapCallBack(PvImage* pData, void* pUserData);
int recording = 0;//0:no,-1:ready to yes,1:yes
FILE *out;

int main()
{
	printf("HelloWorld!\n");
	char *pszCurrTime = (char*)malloc(sizeof(char) * 30);
	memset(pszCurrTime, 0, sizeof(char) * 30);

	out = fopen("data.txt", "w");
	
	std::shared_ptr<CMvGevSource> mCamera;

	mCamera.reset(new CMvGevSource());
	mMutex.reset(new mutex());
	data_pack.reset(new RawDataPack());
#ifdef CAMERA_MODE_ENABLE

	if (mCamera->Open(NULL, NULL, 0))
	{
		mCamera->ip[2];
		mCamera->SetCallback(CapCallBack, nullptr);
	}
	else
	{
		printf("Camera open failed\n");
		system("pause");
		return 0;
	}
	auto para = mCamera->lDevice->GetParameters();
	para->SetEnumValue("PixelFormat", PvPixelMono12);
	para->SetIntegerValue("Height", DATA_HEIGHT);
	para->SetIntegerValue("Width", DATA_WIDTH);
	para->SetIntegerValue("OffsetX", 0);
	para->SetIntegerValue("OffsetY", 0);
	mCamera->mSerial->SetRegValue(0, 0xff79, DATA_HEIGHT);
	mCamera->mSerial->SetRegValue(0, 0xff80, DATA_WIDTH);
	mCamera->Start(0, 1);
#endif
#ifdef SERVER_MODE_ENABLE
	SerialBKServer server(mCamera, mMutex, data_pack);
	server.start();
#endif
	char command[100] = { 0 };
	while (true)
	{
		memset(command, 0, sizeof(command));
		scanf_s("%s", command, 99);
		if (memcmp(command, "exit", 4) == 0)
			break;
	}
	mCamera->Stop(0, 1);
	fclose(out);
	printf("system ended\n");
	system("pause");
	return 0;
}

int CapCallBack(PvImage* pData, void* pUserData)
{
	cap_num++;
	PBYTE lDataPtr = pData->GetDataPointer();
	if (cap_num % 31 == 0)
	{
		for (int i = 0; i < 12; i++)
			printf("0x%02x ", *(lDataPtr + i));
		printf("\n");
	}
	//TODO:可以考虑在C++层做好数据抽样或横向平均(目前全部传到C#层做）
	for (int i = 0; i < DATA_HEIGHT; i++)
	{
		for (int j = 0; j < DATA_WIDTH; j++)
		{
			UCHAR *pt = lDataPtr + i * DATA_WIDTH * 2 + j * 2;
			ushort val = SerialBKServer::ByteToshort(pt);
			if (recording == 1)
			{
				data_temp.push_back(val);
				//TODO:目前是所有采集的数据（即被recording到的数据）全部保存到data.txt文件中
				//     未来应当由用户选择保存时间点并给文件赋不同的名字

				//可用的方案：
				//1.保存改为C#层做
				//2.由C#传入保存开始和结束标识符，保存加入日期作为文件名字符串
				fprintf(out, "%d ", val);
				mMutex->lock();
				uint LEN = data_pack->data_len;
				uint AVG = data_pack->avg;
				mMutex->unlock();
				if (data_temp.size() >= LEN)//recording end
				{
					recording = 0;
					//TODO:与保存相关的修改，参见上一个TODO
					fprintf(out, "\n");
					if (data_queue.size() >= MAX_QUEUE_AVG_LEN)//queue is full
					{
						data_queue.erase(data_queue.begin());
					}
					data_queue.push_back(data_temp);
					data_temp.clear();
					//re-calculate the average
					if (data_queue.size() >= AVG)//enough data
					{
						bool enough_len = true;
						for (int k = 0; k < AVG; k++)
						{
							if (data_queue[data_queue.size() - k - 1].size() < LEN)
							{
								enough_len = false;
								break;
							}
						}
						if (!enough_len)//Some of the record(s) don't match the data lenth
						{
							mMutex->lock();
							data_pack->data.clear();
							mMutex->unlock();
						}
						else//Finally we have enough data
						{
							std::vector<uint> to_copy;
							for (int k = 0; k < LEN; k++)
							{
								to_copy.push_back(0);
							}
							for (int k = 0; k < AVG; k++)
							{
								for (int kk = 0; kk < LEN; kk++)
								{
									to_copy[kk] += data_queue[data_queue.size() - k - 1][kk];
								}
							}
							mMutex->lock();
							data_pack->data.clear();
							for (int kk = 0; kk < LEN; kk++)
							{
								data_pack->data.push_back((ushort)(to_copy[kk] / AVG));
							}
							mMutex->unlock();
						}
					}
					else//Not enough data
					{
						mMutex->lock();
						data_pack->data.clear();
						mMutex->unlock();
					}
				}
			}
			else
			{
				mMutex->lock();
				ushort WAY = data_pack->compare_way;
				ushort THR = data_pack->threshold;
				mMutex->unlock();
				if (recording == -1)
					recording = 1;
				else if (WAY == 0 && val == THR)
					recording = (j % 2 == 0) ? -1 : 1;
				else if (WAY == 1 && val > THR)
					recording = (j % 2 == 0) ? -1 : 1;
				else if (WAY == 2 && val < THR)
					recording = (j % 2 == 0) ? -1 : 1;
			}
		}
	}
	return true;
}