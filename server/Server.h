#pragma once

#include <winsock.h>
#include <iostream>
#include <string>
#include <thread>
#include <mysql.h>
#include <unordered_map>
#include <mutex>
#include <tuple>
#pragma comment(lib, "ws2_32.lib")


using connect_info = std::tuple<bool, std::string, std::string, int>;


class Server {
private:
	int sockfd;
	int client_sock;
	sockaddr_in server_addr;

	// 共享变量: 从name到对应的sockfd的哈希表
	static std::unordered_map<std::string, int> name_sock_map;
	// 共享对象锁, 防止读写冲突
	static std::mutex name_sock_lock;
	


public:
	Server(const char* ip, int port);
	~Server();
	void run();
	static void RecvMsg(int conn);
	static void HandleRequest(int conn, std::string str, connect_info& info);
};