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
#define DEFAULT_PORT 5050 //����Ĭ�϶˿�
#define BUF_SIZE 4096 //���û�������С
#pragma comment(lib,"ws2_32.lib")
const char NOTFOUND[] = "HTTP/1.1 404 NOT FOUND\r\n";
const char REQERROR[] = "HTTP/1.1 400 Bad Request\r\n";
const char FORMERROR[] = "HTTP/1.1 400 Bad Request\r\n";
const char LOADERROR[] = "HTTP/1.1 400 Bad Request\r\n";
const char FORBIDDEN[] = "HTTP/1.1 403 FORBIDDEN\r\n";
const char path_404[] = "D:/net_test/404.html";
const char path_403[] = "D:/net_test/403.html";
char recvBuf[BUF_SIZE]; ////���ý��ջ�����
string main_path; //��Ŀ¼
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
void talk(SOCKET sessionSocket)//���лỰ�ĺ���
{
	int sendre;

	//�Խ��ܵ�����Ϣ���н���
	std::smatch strmatch;//������ʽ����ı�
	std::regex regulation("([A-Za-z]+) +(/.*) +(HTTP/[0-9][.][0-9])");//������ʽ����ƥ�������ĵ�������
	std::string str(recvBuf);//������ԭʼ�ı�

	int match_num = std::regex_search(str, strmatch, regulation);
	if (match_num == 0)
	{
		std::cout << "Invaild Request!" << endl;
		sendre = send(sessionSocket, REQERROR, strlen(REQERROR), 0);
		//closesocket(sessionSocket);  
		return;
	}
	else//���� GET url http_version
	{
		cout << "Curret request: " << strmatch[0] << endl;
		std::string msg_get = strmatch[1];
		std::string msg_url = strmatch[2];
		std::smatch filetype;
		std::regex regulation2("\\..*"); // "."��������;���ţ�����ƥ����Ҫת�壨Ϊ���ݰ汾��������ת�����
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
				rev_msg = rev_msg + "Content-Type:" + Content_Type[".html"] + "\r\n\r\n";//�ַ���ƴ��һ��ʼҪ���ַ�������
				int msg_size = rev_msg.size();
				sendre = send(sessionSocket, rev_msg.c_str(), rev_msg.size(), 0);
				f.open(path_403, std::ios::binary);
				std::filebuf* tmp = f.rdbuf();//���ļ���������
				int size = tmp->pubseekoff(0, f.end, f.in);//���ڲ�λ��ָ������Ϊ���f.endƫ��Ϊ0��λ�ã���������ָ���λ��
				tmp->pubseekpos(0, f.in); //��������Ϊ0�ž���λ��
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
			f.open(main_path + msg_url, std::ios::binary);//�Զ��ļ�������ʽ��Ŀ���ļ�
			//cout << "-------------" << main_path + msg_url << endl;
			if (!f.is_open())//û���ҵ��ļ�
			{
				std::cout << msg_url + " 404 NOT FOUND" << endl;
				sendre = send(sessionSocket, NOTFOUND, strlen(NOTFOUND), 0);
				string rev_msg;
				rev_msg = rev_msg + "Content-Type:" + Content_Type[".html"] + "\r\n\r\n";//�ַ���ƴ��һ��ʼҪ���ַ�������
				int msg_size = rev_msg.size();
				sendre = send(sessionSocket, rev_msg.c_str(), rev_msg.size(), 0);
				f.open(path_404, std::ios::binary);
				std::filebuf* tmp = f.rdbuf();//���ļ���������
				int size = tmp->pubseekoff(0, f.end, f.in);//���ڲ�λ��ָ������Ϊ���f.endƫ��Ϊ0��λ�ã���������ָ���λ��
				tmp->pubseekpos(0, f.in); //��������Ϊ0�ž���λ��
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
			else//����ҵ��˶�Ӧ���ļ�
			{
				std::filebuf* tmp = f.rdbuf();//���ļ���������
				int size = tmp->pubseekoff(0, f.end, f.in);//���ڲ�λ��ָ������Ϊ���f.endƫ��Ϊ0��λ�ã���������ָ���λ��
				tmp->pubseekpos(0, f.in); //��������Ϊ0�ž���λ��
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
						+ "\r\n" + "Content-Type:" + Content_Type[filetype[0]] + "\r\n\r\n";//�ַ���ƴ��һ��ʼҪ���ַ�������
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
		select()�������ṩ��fd_set�����ݽṹ��ʵ������long���͵����飬
		ÿһ������Ԫ�ض�����һ�򿪵��ļ������������socket��������������ļ��������ܵ����豸�����������ϵ��������ϵ�Ĺ����ɳ���Ա���.
		������select()ʱ�����ں˸���IO״̬�޸�fd_set�����ݣ��ɴ���ִ֪ͨ����select()�Ľ����ĸ�socket���ļ���������˿ɶ����д�¼���
	*/
	fd_set rfds;				//���ڼ��socket�Ƿ������ݵ����ĵ��ļ�������������socket������ģʽ�µȴ������¼�֪ͨ�������ݵ�����
	fd_set wfds;				//���ڼ��socket�Ƿ���Է��͵��ļ�������������socket������ģʽ�µȴ������¼�֪ͨ�����Է������ݣ�
	bool first_connetion = true;

	int nRc = WSAStartup(0x0202, &wsaData);

	if (nRc) {
		printf("Winsock  startup failed with error!\n");
	}

	if (wsaData.wVersion != 0x0202) {
		printf("Winsock version is not correct!\n");
	}

	printf("Winsock  startup Ok!\n");


	//����socket
	SOCKET srvSocket;

	//��������ַ�Ϳͻ��˵�ַ
	sockaddr_in addr, clientAddr;

	//�Ựsocket�������client����ͨ��
	SOCKET sessionSocket;

	//ip��ַ����
	int addrLen;

	//��������socket
	srvSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (srvSocket != INVALID_SOCKET)
		printf("Socket create Ok!\n");


	//���÷������Ķ˿ں͵�ַ
	//sin_addr��һ�������壬S_un.S_addr��ʾ���޷��ų����ͱ�ʾip��ַ
	addr.sin_family = AF_INET; //����Э��أ�AF_INET��ʾIPv4��
	cout << "Do you want to configure port? (y/n)" << endl; //���ö˿�
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
	//todoδ���˿ں��Ƿ�Ϸ�
	addr.sin_port = htons(port_num);//���ö˿�

	cout << "Do you want to configure ip? (y/n)" << endl; //����ip��ַ
	cout << "Input 'n'��we will choose anyone" << endl;
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
				addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //����������һ��������IP��ַ
				flag = 1;
				break;
			}
		}
		if (!flag)
			cout << "Invaild input! Please input again:" << endl;
		else
			break;
	}
	//todo δ���ip�Ƿ�Ϸ�

	cout << "Please configue main path:" << endl;
	cin >> main_path;
	//todo ���·���Ϸ���
	cout << "OK!" << endl;


	//binding
	int rtn = bind(srvSocket, (LPSOCKADDR)&addr, sizeof(addr));//����Ӧ��socker����Ӧ��ip��ַ���˿ڽ�������
	if (rtn != SOCKET_ERROR)
		printf("Socket bind Ok!\n");

	//����
	rtn = listen(srvSocket, 5);//��Ӧsocker���뱻����������״̬��������г���Ϊ5
	if (rtn != SOCKET_ERROR)
		printf("Socket listen Ok!\n");

	clientAddr.sin_family = AF_INET;
	addrLen = sizeof(clientAddr);

	u_long blockMode = 1;//��srvSock��Ϊ������ģʽ�Լ����ͻ���������

	//����ioctlsocket����srvSocket��Ϊ������ģʽ���ĳɷ������fd_setԪ�ص�״̬����ÿ��Ԫ�ض�Ӧ�ľ���Ƿ�ɶ����д
	if ((rtn = ioctlsocket(srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO��������ֹ�׽ӿ�s�ķ�����ģʽ��
		cout << "ioctlsocket() failed with error!\n";
		return 0;
	}
	cout << "ioctlsocket() for server socket ok!	Waiting for client connection and data\n";

	while (true) {
		//���rfds��wfds����
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);

		//��srvSocket����rfds����
		//�������ͻ�������������ʱ��rfds������srvSocket��Ӧ�ĵ�״̬Ϊ�ɶ�
		//��������������þ��ǣ����õȴ��ͻ���������
		FD_SET(srvSocket, &rfds);

		//���first_connetionΪtrue��sessionSocket��û�в���
		if (!first_connetion) {
			//��sessionSocket����rfds�����wfds����
			//�������ͻ��˷������ݹ���ʱ��rfds������sessionSocket�Ķ�Ӧ��״̬Ϊ�ɶ��������Է������ݵ��ͻ���ʱ��wfds������sessionSocket�Ķ�Ӧ��״̬Ϊ��д
			//�����������������þ��ǣ�
			//���õȴ��ỰSOKCET�ɽ������ݻ�ɷ�������
			if (sessionSocket != INVALID_SOCKET) { //���sessionSocket����Ч��
				FD_SET(sessionSocket, &rfds);
				FD_SET(sessionSocket, &wfds);
			}

		}

		/*
			select����ԭ������Ҫ�������ļ����������ϣ��ɶ�����д�����쳣����ʼ������select��������״̬��
			���пɶ�д�¼����������õĵȴ�ʱ��timeout���˾ͻ᷵�أ�����֮ǰ�Զ�ȥ�����������¼��������ļ�������������ʱ�������¼��������ļ����������ϡ�
			��select�����ļ��ϲ�û�и����û������а����ļ����������ļ�����������Ҫ�û��������б�������(ͨ��FD_ISSET���ÿ�������״̬)��
		*/
		//��ʼ�ȴ����ȴ�rfds���Ƿ��������¼���wfds���Ƿ��п�д�¼�
		//The select function returns the total number of socket handles that are ready and contained in the fd_set structure
		//�����ܹ����Զ���д�ľ������
		int nTotal = select(0, &rfds, &wfds, NULL, NULL);//select�ƺ����Դ������������ҳ�������д/�ɶ���socker������Ļ�ɾ��
		//���srvSock�յ��������󣬽��ܿͻ���������
		//cout << "nTotal = " << nTotal << endl;
		if (first_connetion && FD_ISSET(srvSocket, &rfds)) {
			nTotal--;   //��Ϊ�ͻ���������Ҳ��ɶ��¼������-1��ʣ�µľ��������пɶ��¼��ľ�����������ж��ٸ�socket�յ������ݣ�
			//�����ỰSOCKET
			sessionSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
			if (sessionSocket != INVALID_SOCKET) {
				printf("Socket listen one client request!\n");

			}
			//�ѻỰSOCKET��Ϊ������ģʽ
			if ((rtn = ioctlsocket(sessionSocket, FIONBIO, &blockMode)) == SOCKET_ERROR) { //FIONBIO��������ֹ�׽ӿ�s�ķ�����ģʽ��
				cout << "ioctlsocket() failed with error!\n";
				return 0;
			}
			cout << "ioctlsocket() for session socket ok!	Waiting for client connection and data\n";

			//���õȴ��ỰSOKCET�ɽ������ݻ�ɷ�������
			FD_SET(sessionSocket, &rfds);
			FD_SET(sessionSocket, &wfds);

			first_connetion = false;

		}

		//���ỰSOCKET�Ƿ������ݵ��������Ƿ���Է�������
		if (nTotal > 0) {
			//��������пɶ��¼���˵���ǻỰsocket�����ݵ���������ܿͻ�������;
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
				else { //�������յ��˿ͻ��˶Ͽ����ӵ�����Ҳ��ɶ��¼���
					printf("Client leaving ...\n");
					closesocket(sessionSocket);  //��Ȼclient�뿪�ˣ��͹ر�sessionSocket
					nTotal--;	//��Ϊ�ͻ����뿪Ҳ���ڿɶ��¼���������Ҫ-1
					sessionSocket = INVALID_SOCKET; //��sessionSocket��ΪINVALID_SOCKET
					first_connetion = true;
				}
			}
		}
	}
	return 0;
}
