#pragma once

#include <winsock.h>
#include <iostream>
#include <string>
#include <thread>
#include <mysql.h>


#pragma comment(lib, "ws2_32.lib")


class Client {
private:
	int sockfd;
	sockaddr_in server_addr;

public:
	Client(const char* ip, int port);
	~Client();
	void run();
	static void SendMsg(int conn);
	static void RecvMsg(int conn);
	void HandleClient(int conn);
};

