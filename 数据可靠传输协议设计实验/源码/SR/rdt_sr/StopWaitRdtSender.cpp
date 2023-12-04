#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtSender.h"


StopWaitRdtSender::StopWaitRdtSender():expectSequenceNumberSend(0),waitingState(false)
{
	this->base = 0;
	this->winsize = Configuration::WINDOW_SIZE;
	for (int i = 0; i < Configuration::MOD; i++) {
		packet_queue[i].acknum = i; //表示此位置没有数据
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
	
	if (this->waitingState) { //发送方处于等待状态（此处意为队列已满）
		printf("窗口已满，暂时不可发送数据！\n");
		return false;
	}
	this->packetWaitingAck.acknum = -1; //忽略该字段
	this->packetWaitingAck.seqnum = this->expectSequenceNumberSend;
	this->packetWaitingAck.checksum = 0;
	memcpy(this->packetWaitingAck.payload, message.data, sizeof(message.data));
	this->packetWaitingAck.checksum = pUtils->calculateCheckSum(this->packetWaitingAck);
		
	packet_queue[expectSequenceNumberSend] = this->packetWaitingAck; //缓存发送的数据
	printf("当前发送窗口为[%d,%d]\n", base, (base + winsize - 1) % Configuration::MOD);
	pUtils->printPacket("发送方发送报文", this->packetWaitingAck);
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);				//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	pns->startTimer(SENDER, Configuration::TIME_OUT, expectSequenceNumberSend);//启动发送方定时器		
	expectSequenceNumberSend = (expectSequenceNumberSend + 1) % Configuration::MOD;
	if ((base + winsize) % Configuration::MOD == expectSequenceNumberSend) { //窗口已满，不接受数据发送
		waitingState = true;
	}
	return true;
}

void StopWaitRdtSender::receive(const Packet &ackPkt) {
		//检查校验和是否正确
		int checkSum = pUtils->calculateCheckSum(ackPkt);
		if (checkSum != ackPkt.checksum) { //数据差错
			pUtils->printPacket("发送方收到错误报文", ackPkt);
			return;
		}
		if (is_inwindow(ackPkt.acknum)) {
			if (packet_queue[ackPkt.acknum].acknum == -1) {
				pns->stopTimer(SENDER, ackPkt.acknum);
				packet_queue[ackPkt.acknum].acknum = ackPkt.acknum; //标记该位置数据收到
			}
			pUtils->printPacket("发送方正确收到确认", ackPkt);
			printf("当前发送窗口为[%d,%d]\n", base, (base + winsize - 1) % Configuration::MOD);
			while (packet_queue[base].acknum == base && base != expectSequenceNumberSend) { //保证窗口左端为为确认数据
				base = (base + 1) % Configuration::MOD;
				waitingState = false;
			}
			printf("当前发送窗口为[%d,%d]\n", base, (base + winsize - 1) % Configuration::MOD);
		}
		else {
			pUtils->printPacket("发送方收到一个窗口之外的确认报文\n", ackPkt);
		}
}

void StopWaitRdtSender::timeoutHandler(int seqNum) {
	printf("发送方%d号数据定时器时间到，重发未确认的报文\n", seqNum);
	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
	pUtils->printPacket("发送方定时器时间到，重发未确认的报文", packet_queue[seqNum]);
	pns->sendToNetworkLayer(RECEIVER, packet_queue[seqNum]);
}
