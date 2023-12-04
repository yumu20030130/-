#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtSender.h"


StopWaitRdtSender::StopWaitRdtSender():expectSequenceNumberSend(0),waitingState(false)
{
	this->base = 0;
	this->expectSequenceNumberSend = 0;
	this->winsize = Configuration::WINDOW_SIZE;
	this->waitingState = false;
}


StopWaitRdtSender::~StopWaitRdtSender()
{
}



bool StopWaitRdtSender::getWaitingState() {
	return waitingState;
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
	packet_queue[expectSequenceNumberSend] = this->packetWaitingAck;
	printf("当前发送窗口为[%d,%d]", base, (base + winsize - 1) % Configuration::MOD);
	pUtils->printPacket("发送方发送报文", this->packetWaitingAck);
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);				//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	if (base == expectSequenceNumberSend) {
		pns->startTimer(SENDER, Configuration::TIME_OUT, 1);//启动发送方定时器
	}			
	expectSequenceNumberSend = (expectSequenceNumberSend + 1) % Configuration::MOD;
	if ((base + winsize) % Configuration::MOD == expectSequenceNumberSend) {
		waitingState = true;
	}
	return true;
}

void StopWaitRdtSender::receive(const Packet &ackPkt) {
		//检查校验和是否正确
		int checkSum = pUtils->calculateCheckSum(ackPkt);
		//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
		if (checkSum == ackPkt.checksum && (ackPkt.acknum + 1) % Configuration::MOD != base) {
			pns->stopTimer(SENDER, 1);
			base = (ackPkt.acknum + 1) % Configuration::MOD;
			pUtils->printPacket("发送方正确收到确认", ackPkt);
			printf("当前发送窗口为[%d,%d]\n", base, (base + winsize - 1) % Configuration::MOD);
			if (base != expectSequenceNumberSend) { //仍有发送但未确认数据
				pns->startTimer(SENDER, Configuration::TIME_OUT, 1);
			}		
			waitingState = false;
		}
		else {
			pUtils->printPacket("发送方收到错误报文", ackPkt);
		}
}

void StopWaitRdtSender::timeoutHandler(int seqNum) {
	//唯一一个定时器,无需考虑seqNum
	printf("发送方定时器时间到，重新发送窗口内所有数据包\n");
	pns->stopTimer(SENDER, 1);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, 1);			//重新启动发送方定时器
	for (int i = base; i != expectSequenceNumberSend; i = (i + 1) % Configuration::MOD) {
		pUtils->printPacket("发送方定时器时间到，重发未确认的报文", packet_queue[i]);
		pns->sendToNetworkLayer(RECEIVER, packet_queue[i]);
	}
}
