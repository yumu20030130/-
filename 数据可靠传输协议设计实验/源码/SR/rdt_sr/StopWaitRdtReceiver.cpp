#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtReceiver.h"

StopWaitRdtReceiver::StopWaitRdtReceiver()
{
	lastAckPkt.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++){
		lastAckPkt.payload[i] = '.';
	}
	winsize = Configuration::WINDOW_SIZE;
	base = 0;
	for (int i = 0; i < Configuration::MOD; i++) {
		packet_queue[i].acknum = 0;//需要用值是否为全零标识该位置是否有缓存数据，故必须初始化
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
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);
	if (checkSum != packet.checksum) {
		pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		return;
	}
	if (is_inwindow(packet.seqnum)) {  //在窗口内
		pUtils->printPacket("接收方正确收到发送方的报文", packet);
		if (packet_queue[packet.seqnum].acknum != -1) { //说明该位置还没载入过值
			memcpy(&packet_queue[packet.seqnum], &packet, sizeof packet);
			//packet_queue[packet.seqnum] = packet; ---?
		}
		lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方发送确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		printf("当前接收窗口为[%d,%d]\n", base, (base + winsize - 1) % Configuration::MOD);
		if (base == packet.seqnum) { //当新确认的位置正好是基序号
			while (packet_queue[base].acknum == -1) {
				//取出Message，向上递交给应用层
				Message msg;
				memcpy(msg.data, packet_queue[base].payload, sizeof(packet_queue[base].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				packet_queue[base].acknum = 0;  //算是作一个记号表示删除
				base = (base + 1) % Configuration::MOD;
			}
		}
		printf("当前接收窗口为[%d,%d]\n", base, (base + winsize - 1) % Configuration::MOD);
	}
	else { //不在窗口内（本题序号总数恰为窗口大小两倍，所以不存在第三种情况）
		lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方发送确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
	}
}