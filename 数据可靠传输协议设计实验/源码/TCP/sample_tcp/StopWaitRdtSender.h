#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
#include <queue>
class StopWaitRdtSender :public RdtSender
{
private:
	int expectSequenceNumberSend;	// ��һ���������
	bool waitingState;				// �Ƿ��ڵȴ�Ack��״̬
	Packet packetWaitingAck;		//�ѷ��Ͳ��ȴ�Ack�����ݰ�
	int base;						//���ڻ����
	int tail;						//���Ҫ����һ�����ݣ�������������λ�õı��
	int winsize;					//���ڴ�С
	int ack_cnt;					//����ack��Ŀ
	Packet packet_queue[Configuration::MOD];		//���淢����δȷ������
public:

	bool getWaitingState();
	bool send(const Message &message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(const Packet &ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����
	void ack3Handler();									//��������ack�������
public:
	StopWaitRdtSender();
	virtual ~StopWaitRdtSender();
};

#endif

