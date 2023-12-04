#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtSender.h"


StopWaitRdtSender::StopWaitRdtSender():expectSequenceNumberSend(0),waitingState(false)
{
	this->base = 0;
	this->winsize = Configuration::WINDOW_SIZE;
	for (int i = 0; i < Configuration::MOD; i++) {
		packet_queue[i].acknum = i; //��ʾ��λ��û������
	}
}


StopWaitRdtSender::~StopWaitRdtSender()
{
}



bool StopWaitRdtSender::getWaitingState() {
	return waitingState;
}

bool StopWaitRdtSender::is_inwindow(int num) {
	if (num >= base) {
		return base + winsize - 1 >= num;
	}
	else {
		int tail = (base + winsize - 1) % Configuration::MOD;
		return tail >= num && tail < base;
	}
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
		
	packet_queue[expectSequenceNumberSend] = this->packetWaitingAck; //���淢�͵�����
	printf("��ǰ���ʹ���Ϊ[%d,%d]\n", base, (base + winsize - 1) % Configuration::MOD);
	pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck);
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);				//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	pns->startTimer(SENDER, Configuration::TIME_OUT, expectSequenceNumberSend);//�������ͷ���ʱ��		
	expectSequenceNumberSend = (expectSequenceNumberSend + 1) % Configuration::MOD;
	if ((base + winsize) % Configuration::MOD == expectSequenceNumberSend) { //�������������������ݷ���
		waitingState = true;
	}
	return true;
}

void StopWaitRdtSender::receive(const Packet &ackPkt) {
		//���У����Ƿ���ȷ
		int checkSum = pUtils->calculateCheckSum(ackPkt);
		if (checkSum != ackPkt.checksum) { //���ݲ��
			pUtils->printPacket("���ͷ��յ�������", ackPkt);
			return;
		}
		if (is_inwindow(ackPkt.acknum)) {
			if (packet_queue[ackPkt.acknum].acknum == -1) {
				pns->stopTimer(SENDER, ackPkt.acknum);
				packet_queue[ackPkt.acknum].acknum = ackPkt.acknum; //��Ǹ�λ�������յ�
			}
			pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
			printf("��ǰ���ʹ���Ϊ[%d,%d]\n", base, (base + winsize - 1) % Configuration::MOD);
			while (packet_queue[base].acknum == base && base != expectSequenceNumberSend) { //��֤�������ΪΪȷ������
				base = (base + 1) % Configuration::MOD;
				waitingState = false;
			}
			printf("��ǰ���ʹ���Ϊ[%d,%d]\n", base, (base + winsize - 1) % Configuration::MOD);
		}
		else {
			pUtils->printPacket("���ͷ��յ�һ������֮���ȷ�ϱ���\n", ackPkt);
		}
}

void StopWaitRdtSender::timeoutHandler(int seqNum) {
	printf("���ͷ�%d�����ݶ�ʱ��ʱ�䵽���ط�δȷ�ϵı���\n", seqNum);
	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط�δȷ�ϵı���", packet_queue[seqNum]);
	pns->sendToNetworkLayer(RECEIVER, packet_queue[seqNum]);
}
