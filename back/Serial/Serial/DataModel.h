#pragma once

#include<iostream>
#include<vector>
#include<queue>
typedef unsigned int uint;
typedef unsigned short ushort;
#define MAX_QUEUE_AVG_LEN 100

const int DATA_WIDTH = 80;
const int DATA_HEIGHT = 100;

#define CAMERA_MODE_ENABLE
#define SERVER_MODE_ENABLE

struct RawDataPack
{
	uint avg = 10;
	uint data_len = 20000;
	//std::queue<std::vector<unsigned short>> data_queue;
	std::vector<ushort> data;
	ushort threshold = 4000;
	//0 : == threshold
	//1 : > threshold
	//2 : < threshold
	ushort compare_way = 1;
};