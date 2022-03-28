#pragma once
#include "global.h"

#define SERVER_PORT 8023

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

