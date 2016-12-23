#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include"MVGevSource.h"

#include<Winsock2.h>
#include<vector>
#include<thread>
#include<mutex>
#include<iostream>
#include"DataModel.h"

using namespace std;

#pragma comment(lib,"ws2_32.lib")

#define DATA_LEN 16

class SerialBKServer
{
public:
	SerialBKServer(shared_ptr<CMvGevSource> camera, shared_ptr<mutex> mtx, shared_ptr<RawDataPack> rdp);
	~SerialBKServer();
	const int port = 986;

	void start();
private:
	SOCKET listen_socket;
	SOCKET communicate_socket;
	thread *listen_thread = nullptr;
	thread *communicate_thread = nullptr;

	shared_ptr<mutex> _mtx;
	//shared_ptr<int> _x, _y;
	shared_ptr<CMvGevSource> _camera;
	shared_ptr<RawDataPack> _rdp;

	void ListenThread();
	void CommunicateThread(LPVOID lparam);
public:
	//Ll Lm Ml MM
	static void intToByte(unsigned int i, char *bytes);
	//Ll Lm Ml MM
	static unsigned int ByteToint(char *bytes);

	static ushort ByteToshort(unsigned char * bytes);

	static void shortToByte(ushort i, unsigned char * bytes);

};


