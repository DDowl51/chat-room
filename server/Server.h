#pragma once

#include <winsock.h>
#include <iostream>
#include <string>
#include <thread>
#include <mysql.h>
#pragma comment(lib, "ws2_32.lib")


class Server {
private:
	int sockfd;
	int client_sock;
	sockaddr_in server_addr;

public:
	Server(const char* ip, int port);
	~Server();
	void run();
	static void RecvMsg(int conn);
	static void HandleRequest(int conn, std::string str);
};