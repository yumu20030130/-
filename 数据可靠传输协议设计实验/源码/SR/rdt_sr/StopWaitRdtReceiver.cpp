#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtReceiver.h"

StopWaitRdtReceiver::StopWaitRdtReceiver()
{
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++){
		lastAckPkt.payload[i] = '.';
	}
	winsize = Configuration::WINDOW_SIZE;
	base = 0;
	for (int i = 0; i < Configuration::MOD; i++) {
		packet_queue[i].acknum = 0;//��Ҫ��ֵ�Ƿ�Ϊȫ���ʶ��λ���Ƿ��л������ݣ��ʱ����ʼ��
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


StopWaitRdtReceiver::~StopWaitRdtReceiver()
{
}

bool StopWaitRdtReceiver::is_inwindow(int num) {
	if (num >= base) {
		return base + winsize - 1 >= num;
	}
	else {
		int tail = (base + winsize - 1) % Configuration::MOD;
		return tail >= num && tail < base;
	}
}

void StopWaitRdtReceiver::receive(const Packet &packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);
	if (checkSum != packet.checksum) {
		pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
		return;
	}
	if (is_inwindow(packet.seqnum)) {  //�ڴ�����
		pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
		if (packet_queue[packet.seqnum].acknum != -1) { //˵����λ�û�û�����ֵ
			memcpy(&packet_queue[packet.seqnum], &packet, sizeof packet);
			//packet_queue[packet.seqnum] = packet; ---?
		}
		lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		printf("��ǰ���մ���Ϊ[%d,%d]\n", base, (base + winsize - 1) % Configuration::MOD);
		if (base == packet.seqnum) { //����ȷ�ϵ�λ�������ǻ����
			while (packet_queue[base].acknum == -1) {
				//ȡ��Message�����ϵݽ���Ӧ�ò�
				Message msg;
				memcpy(msg.data, packet_queue[base].payload, sizeof(packet_queue[base].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				packet_queue[base].acknum = 0;  //������һ���Ǻű�ʾɾ��
				base = (base + 1) % Configuration::MOD;
			}
		}
		printf("��ǰ���մ���Ϊ[%d,%d]\n", base, (base + winsize - 1) % Configuration::MOD);
	}
	else { //���ڴ����ڣ������������ǡΪ���ڴ�С���������Բ����ڵ����������
		lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
	}
}