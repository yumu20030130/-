#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma once
#include "winsock2.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include<regex>
#include <Ws2tcpip.h>
#include <unordered_map>
using namespace std;
#define DEFAULT_PORT 5050 //设置默认端口
#define BUF_SIZE 4096 //设置缓冲区大小
#pragma comment(lib,"ws2_32.lib")
const char NOTFOUND[] = "HTTP/1.1 404 NOT FOUND\r\n";
const char REQERROR[] = "HTTP/1.1 400 Bad Request\r\n";
const char FORMERROR[] = "HTTP/1.1 400 Bad Request\r\n";
const char LOADERROR[] = "HTTP/1.1 400 Bad Request\r\n";
const char FORBIDDEN[] = "HTTP/1.1 403 FORBIDDEN\r\n";
const char path_404[] = "D:/net_test/404.html";
const char path_403[] = "D:/net_test/403.html";
char recvBuf[BUF_SIZE]; ////设置接收缓冲区
string main_path; //主目录
unordered_map<string, string> Content_Type = {
	{".html", "text/html"},
	{".xml", "text/xml"},
	{".css", "text/css"},
	{".txt", "text/plain"},
	{".png", "image/png"},
	{".gif", "image/gif"},
	{".jpg", "image/jpg"},
	{".jpeg", "image/jpeg"},
	{".mp3", "audio/mpeg"}
};
void talk(SOCKET sessionSocket)//进行会话的函数
{
	int sendre;

	//对接受到的消息进行解析
	std::smatch strmatch;//正则表达式结果文本
	std::regex regulation("([A-Za-z]+) +(/.*) +(HTTP/[0-9][.][0-9])");//正则表达式规则，匹配请求报文的请求行
	std::string str(recvBuf);//请求报文原始文本

	int match_num = std::regex_search(str, strmatch, regulation);
	if (match_num == 0)
	{
		std::cout << "Invaild Request!" << endl;
		sendre = send(sessionSocket, REQERROR, strlen(REQERROR), 0);
		//closesocket(sessionSocket);  
		return;
	}
	else//分离 GET url http_version
	{
		cout << "Curret request: " << strmatch[0] << endl;
		std::string msg_get = strmatch[1];
		std::string msg_url = strmatch[2];
		std::smatch filetype;
		std::regex regulation2("\\..*"); // "."是特殊用途符号，正常匹配需要转义（为兼容版本，用两个转义符）
		match_num = regex_search(msg_url, filetype, regulation2);
		if (match_num == 0)
		{
			std::cout << msg_url + " 400 bad request" << endl;
			sendre = send(sessionSocket, REQERROR, strlen(REQERROR), 0);
			//closesocket(sessionSocket);
			return;
		}
		else
		{
			std::ifstream f;
			if (msg_url.find("/private", 0) != string::npos) {
				std::cout << msg_url + " 403 Forbidden!" << endl;
				sendre = send(sessionSocket, FORBIDDEN, strlen(FORBIDDEN), 0);
				string rev_msg;
				rev_msg = rev_msg + "Content-Type:" + Content_Type[".html"] + "\r\n\r\n";//字符串拼接一开始要有字符串变量
				int msg_size = rev_msg.size();
				sendre = send(sessionSocket, rev_msg.c_str(), rev_msg.size(), 0);
				f.open(path_403, std::ios::binary);
				std::filebuf* tmp = f.rdbuf();//读文件流缓冲区
				int size = tmp->pubseekoff(0, f.end, f.in);//将内部位置指针设置为相对f.end偏移为0的位置，并返回新指针的位置
				tmp->pubseekpos(0, f.in); //重新设置为0号绝对位置
				char* buffer = new char[size];
				char* tail = buffer + size;
				tmp->sgetn(buffer, size);
				f.close();
				while (buffer < tail)
				{
					sendre = send(sessionSocket, buffer, size, 0);
					buffer = buffer + sendre;
					size = size - sendre;
				}
				//closesocket(sessionSocket);
				return;
			}
			f.open(main_path + msg_url, std::ios::binary);//以读文件流的形式打开目标文件
			//cout << "-------------" << main_path + msg_url << endl;
			if (!f.is_open())//没有找到文件
			{
				std::cout << msg_url + " 404 NOT FOUND" << endl;
				sendre = send(sessionSocket, NOTFOUND, strlen(NOTFOUND), 0);
				string rev_msg;
				rev_msg = rev_msg + "Content-Type:" + Content_Type[".html"] + "\r\n\r\n";//字符串拼接一开始要有字符串变量
				int msg_size = rev_msg.size();
				sendre = send(sessionSocket, rev_msg.c_str(), rev_msg.size(), 0);
				f.open(path_404, std::ios::binary);
				std::filebuf* tmp = f.rdbuf();//读文件流缓冲区
				int size = tmp->pubseekoff(0, f.end, f.in);//将内部位置指针设置为相对f.end偏移为0的位置，并返回新指针的位置
				tmp->pubseekpos(0, f.in); //重新设置为0号绝对位置
				char* buffer = new char[size];
				char* tail = buffer + size;
				tmp->sgetn(buffer, size);
				f.close();
				while (buffer < tail)
				{
					sendre = send(sessionSocket, buffer, size, 0);
					buffer = buffer + sendre;
					size = size - sendre;
				}
				//closesocket(sessionSocket);
				return;
			}
			else//如果找到了对应的文件
			{
				std::filebuf* tmp = f.rdbuf();//读文件流缓冲区
				int size = tmp->pubseekoff(0, f.end, f.in);//将内部位置指针设置为相对f.end偏移为0的位置，并返回新指针的位置
				tmp->pubseekpos(0, f.in); //重新设置为0号绝对位置
				if (size < 0)
				{
					std::cout << "load file into buf failed!\n";
					sendre = send(sessionSocket, LOADERROR, strlen(LOADERROR), 0);
					//closesocket(sessionSocket);
					return;
				}
				else
				{
					char* buffer = new char[size];
					char* tail = buffer + size;
					tmp->sgetn(buffer, size); 
					f.close();
					string rev_msg;
					cout << "200 OK!" << endl;
					rev_msg = rev_msg + "HTTP/1.1 200 OK\r\n" + "Connection:close\r\n" + "Server: cxm\r\n" + "Content-Length: " + to_string(size)
						+ "\r\n" + "Content-Type:" + Content_Type[filetype[0]] + "\r\n\r\n";//字符串拼接一开始要有字符串变量
					int msg_size = rev_msg.size();
					sendre = send(sessionSocket, rev_msg.c_str(), rev_msg.size(), 0);
					while (buffer < tail)
					{
						sendre = send(sessionSocket, buffer, size, 0);
						buffer = buffer + sendre;
						size = size - sendre;
					}
					//closesocket(sessionSocket);
					return;
				}
			}
		}
	}
	return;
}

int main() {
	WSADATA wsaData;
	/*
		select()机制中提供的fd_set的数据结构，实际上是long类型的数组，
		每一个数组元素都能与一打开的文件句柄（不管是socket句柄，还是其他文件或命名管道或设备句柄）建立联系，建立联系的工作由程序员完成.
		当调用select()时，由内核根据IO状态修改fd_set的内容，由此来通知执行了select()的进程哪个socket或文件句柄发生了可读或可写事件。
	*/
	fd_set rfds;				//用于检查socket是否有数据到来的的文件描述符，用于socket非阻塞模式下等待网络事件通知（有数据到来）
	fd_set wfds;				//用于检查socket是否可以发送的文件描述符，用于socket非阻塞模式下等待网络事件通知（可以发送数据）
	bool first_connetion = true;

	int nRc = WSAStartup(0x0202, &wsaData);

	if (nRc) {
		printf("Winsock  startup failed with error!\n");
	}

	if (wsaData.wVersion != 0x0202) {
		printf("Winsock version is not correct!\n");
	}

	printf("Winsock  startup Ok!\n");


	//监听socket
	SOCKET srvSocket;

	//服务器地址和客户端地址
	sockaddr_in addr, clientAddr;

	//会话socket，负责和client进程通信
	SOCKET sessionSocket;

	//ip地址长度
	int addrLen;

	//创建监听socket
	srvSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (srvSocket != INVALID_SOCKET)
		printf("Socket create Ok!\n");


	//设置服务器的端口和地址
	//sin_addr是一个联合体，S_un.S_addr表示用无符号长整型表示ip地址
	addr.sin_family = AF_INET; //设置协议簇（AF_INET表示IPv4）
	cout << "Do you want to configure port? (y/n)" << endl; //配置端口
	cout << "Default port : " << DEFAULT_PORT << endl;
	string port_config;
	int port_num;
	while (1) {
		int flag = 0;
		getline(cin, port_config);
		for (auto t : port_config) {
			if (t == 'y' || t == 'Y') {
				cout << "input port number:" << endl;
				cin >> port_num;
				flag = 1;
				break;
			}
			else if (t == 'N' || t == 'n') {
				port_num = DEFAULT_PORT;
				flag = 1;
				break;
			}
		}
		if (!flag)
			cout << "Invaild input! Please input again:" << endl;
		else
			break;
	}
	//todo未检查端口号是否合法
	addr.sin_port = htons(port_num);//设置端口

	cout << "Do you want to configure ip? (y/n)" << endl; //配置ip地址
	cout << "Input 'n'，we will choose anyone" << endl;
	string ip_config;
	while (1) {
		getline(cin, ip_config);
		int flag = 0;
		for (auto t : ip_config) {
			if (t == 'y' || t == 'Y') {
				string ip;
				cout << "input ip:" << endl;
				cin >> ip;
				inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.S_un.S_addr);
				flag = 1;
				break;
			}
			else if (t == 'N' || t == 'n') {
				addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //主机上任意一块网卡的IP地址
				flag = 1;
				break;
			}
		}
		if (!flag)
			cout << "Invaild input! Please input again:" << endl;
		else
			break;
	}
	//todo 未检查ip是否合法

	cout << "Please configue main path:" << endl;
	cin >> main_path;
	//todo 检查路径合法性
	cout << "OK!" << endl;


	//binding
	int rtn = bind(srvSocket, (LPSOCKADDR)&addr, sizeof(addr));//将对应的socker和相应的ip地址、端口建立连接
	if (rtn != SOCKET_ERROR)
		printf("Socket bind Ok!\n");

	//监听
	rtn = listen(srvSocket, 5);//对应socker进入被动监听请求状态，请求队列长度为5
	if (rtn != SOCKET_ERROR)
		printf("Socket listen Ok!\n");

	clientAddr.sin_family = AF_INET;
	addrLen = sizeof(clientAddr);

	u_long blockMode = 1;//将srvSock设为非阻塞模式以监听客户连接请求

	//调用ioctlsocket，将srvSocket改为非阻塞模式，改成反复检查fd_set元素的状态，看每个元素对应的句柄是否可读或可写
	if ((rtn = ioctlsocket(srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO：允许或禁止套接口s的非阻塞模式。
		cout << "ioctlsocket() failed with error!\n";
		return 0;
	}
	cout << "ioctlsocket() for server socket ok!	Waiting for client connection and data\n";

	while (true) {
		//清空rfds和wfds数组
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);

		//将srvSocket加入rfds数组
		//即：当客户端连接请求到来时，rfds数组里srvSocket对应的的状态为可读
		//因此这条语句的作用就是：设置等待客户连接请求
		FD_SET(srvSocket, &rfds);

		//如果first_connetion为true，sessionSocket还没有产生
		if (!first_connetion) {
			//将sessionSocket加入rfds数组和wfds数组
			//即：当客户端发送数据过来时，rfds数组里sessionSocket的对应的状态为可读；当可以发送数据到客户端时，wfds数组里sessionSocket的对应的状态为可写
			//因此下面二条语句的作用就是：
			//设置等待会话SOKCET可接受数据或可发送数据
			if (sessionSocket != INVALID_SOCKET) { //如果sessionSocket是有效的
				FD_SET(sessionSocket, &rfds);
				FD_SET(sessionSocket, &wfds);
			}

		}

		/*
			select工作原理：传入要监听的文件描述符集合（可读、可写，有异常）开始监听，select处于阻塞状态。
			当有可读写事件发生或设置的等待时间timeout到了就会返回，返回之前自动去除集合中无事件发生的文件描述符，返回时传出有事件发生的文件描述符集合。
			但select传出的集合并没有告诉用户集合中包括哪几个就绪的文件描述符，需要用户后续进行遍历操作(通过FD_ISSET检查每个句柄的状态)。
		*/
		//开始等待，等待rfds里是否有输入事件，wfds里是否有可写事件
		//The select function returns the total number of socket handles that are ready and contained in the fd_set structure
		//返回总共可以读或写的句柄个数
		int nTotal = select(0, &rfds, &wfds, NULL, NULL);//select似乎可以从两个数组中找出真正可写/可读的socker，其余的会删掉
		//如果srvSock收到连接请求，接受客户连接请求
		//cout << "nTotal = " << nTotal << endl;
		if (first_connetion && FD_ISSET(srvSocket, &rfds)) {
			nTotal--;   //因为客户端请求到来也算可读事件，因此-1，剩下的就是真正有可读事件的句柄个数（即有多少个socket收到了数据）
			//产生会话SOCKET
			sessionSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
			if (sessionSocket != INVALID_SOCKET) {
				printf("Socket listen one client request!\n");

			}
			//把会话SOCKET设为非阻塞模式
			if ((rtn = ioctlsocket(sessionSocket, FIONBIO, &blockMode)) == SOCKET_ERROR) { //FIONBIO：允许或禁止套接口s的非阻塞模式。
				cout << "ioctlsocket() failed with error!\n";
				return 0;
			}
			cout << "ioctlsocket() for session socket ok!	Waiting for client connection and data\n";

			//设置等待会话SOKCET可接受数据或可发送数据
			FD_SET(sessionSocket, &rfds);
			FD_SET(sessionSocket, &wfds);

			first_connetion = false;

		}

		//检查会话SOCKET是否有数据到来或者是否可以发送数据
		if (nTotal > 0) {
			//如果还有有可读事件，说明是会话socket有数据到来，则接受客户的数据;
			if (FD_ISSET(sessionSocket, &rfds)) {
				//receiving data from client
				memset(recvBuf, '\0', 4096);
				rtn = recv(sessionSocket, recvBuf, 4096, 0);
				cout << "rtn : " << rtn << endl;
				if (rtn > 0) {
					//printf("Received %d bytes from client: %s\n", rtn, recvBuf);
					cout << "Current client ip: " << inet_ntoa(clientAddr.sin_addr) << endl;
					cout << "Currect client port: " << clientAddr.sin_port << endl;
					talk(sessionSocket);
					closesocket(sessionSocket);
					first_connetion = true;
				}
				else { //否则是收到了客户端断开连接的请求，也算可读事件。
					printf("Client leaving ...\n");
					closesocket(sessionSocket);  //既然client离开了，就关闭sessionSocket
					nTotal--;	//因为客户端离开也属于可读事件，所以需要-1
					sessionSocket = INVALID_SOCKET; //把sessionSocket设为INVALID_SOCKET
					first_connetion = true;
				}
			}
		}
	}
	return 0;
}
