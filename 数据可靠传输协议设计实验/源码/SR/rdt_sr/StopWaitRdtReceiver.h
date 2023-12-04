#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
class StopWaitRdtReceiver :public RdtReceiver
{
private:
	Packet lastAckPkt;				//上次发送的确认报文
	int base;						//接收窗口基序号
	int winsize;					//窗口长度
	Packet packet_queue[Configuration::MOD]; //缓存接收到的数据包
public:
	StopWaitRdtReceiver();
	virtual ~StopWaitRdtReceiver();

public:
	bool is_inwindow(int num); //判断一个数是否在窗口中
	void receive(const Packet &packet);	//接收报文，将被NetworkService调用
};

#endif

