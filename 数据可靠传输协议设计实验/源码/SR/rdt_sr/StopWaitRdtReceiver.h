#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
class StopWaitRdtReceiver :public RdtReceiver
{
private:
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���
	int base;						//���մ��ڻ����
	int winsize;					//���ڳ���
	Packet packet_queue[Configuration::MOD]; //������յ������ݰ�
public:
	StopWaitRdtReceiver();
	virtual ~StopWaitRdtReceiver();

public:
	bool is_inwindow(int num); //�ж�һ�����Ƿ��ڴ�����
	void receive(const Packet &packet);	//���ձ��ģ�����NetworkService����
};

#endif

