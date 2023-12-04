#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtSender.h"


StopWaitRdtSender::StopWaitRdtSender():expectSequenceNumberSend(0),waitingState(false)
{
	this->base = 0;
	this->tail = 0;
	this->winsize = Configuration::WINDOW_SIZE;
}


StopWaitRdtSender::~StopWaitRdtSender()
{
}



bool StopWaitRdtSender::getWaitingState() {
	return waitingState;
}




bool StopWaitRdtSender::send(const Message &message) {
	if (this->waitingState) { //���ͷ����ڵȴ�״̬���˴���Ϊ����������
		printf("������������ʱ���ɷ������ݣ�\n");
		return false;
	}
	this->packetWaitingAck.acknum = -1; //���Ը��ֶ�
	this->packetWaitingAck.seqnum = this->expectSequenceNumberSend;
	this->packetWaitingAck.checksum = 0;
	memcpy(this->packetWaitingAck.payload, message.data, sizeof(message.data));
	this->packetWaitingAck.checksum = pUtils->calculateCheckSum(this->packetWaitingAck);
	packet_queue[tail] = this->packetWaitingAck;
	printf("��ǰ���ʹ���Ϊ[%d,%d]", base, (base + winsize - 1) % Configuration::MOD);
	pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck);
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);				//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	if (base == tail) {
		pns->startTimer(SENDER, Configuration::TIME_OUT, 1);//�������ͷ���ʱ��
	}			
	//expectSequenceNumberSend += strlen(message.data);
	expectSequenceNumberSend += sizeof packetWaitingAck;
	tail = (tail + 1) % Configuration::MOD;
	if ((base + winsize) % Configuration::MOD == tail) {
		waitingState = true;
	}
	return true;
}

void StopWaitRdtSender::receive(const Packet &ackPkt) {
		//���У����Ƿ���ȷ
		int checkSum = pUtils->calculateCheckSum(ackPkt);
		//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
		if (checkSum == ackPkt.checksum && ackPkt.acknum > packet_queue[base].seqnum) {
			pns->stopTimer(SENDER, 1);
			while (ackPkt.acknum > packet_queue[base].seqnum && base != tail) {
				base = (base + 1) % Configuration::MOD;
			}
			pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
			printf("��ǰ���ʹ���Ϊ[%d,%d]\n", base, (base + winsize - 1) % Configuration::MOD);
			if (base != tail) { //���з��͵�δȷ������
				pns->startTimer(SENDER, Configuration::TIME_OUT, 1);
			}
			ack_cnt = 0;
			waitingState = false;
		}
		else if (checkSum == ackPkt.checksum && ackPkt.acknum == packet_queue[base].seqnum) {
			pUtils->printPacket("���ͷ��յ�����ack����", ackPkt);
			ack_cnt++;//��������ack��Ŀ
			if (ack_cnt == 3) {
				ack3Handler();
			}
		}
		else {
			pUtils->printPacket("���ͷ��յ�������", ackPkt);
		}
}

void StopWaitRdtSender::ack3Handler() {
	printf("3������ack,acknumΪ%d�����·��ʹ������������ݰ�\n", packet_queue[base].seqnum);
	for (int i = base; i != tail; i = (i + 1) % Configuration::MOD) {
		pUtils->printPacket("3������ack���ط�δȷ�ϵı���", packet_queue[i]);
		pns->sendToNetworkLayer(RECEIVER, packet_queue[i]);
	}
	ack_cnt = 0;
}

void StopWaitRdtSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	printf("���ͷ���ʱ��ʱ�䵽�����·��ʹ������������ݰ�\n");
	pns->stopTimer(SENDER, 1);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, 1);			//�����������ͷ���ʱ��
	for (int i = base; i != tail; i = (i + 1) % Configuration::MOD) {
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط�δȷ�ϵı���", packet_queue[i]);
		pns->sendToNetworkLayer(RECEIVER, packet_queue[i]);
	}
	ack_cnt = 0;
}
