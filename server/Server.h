#pragma once


#include <iostream>
#include <string>
#include <thread>
#include <mysql.h>
#include <unordered_map>
#include <mutex>
#include <tuple>
#include <set>
#include <fstream>
#include <winsock.h>
#include <random>

#include "HiRedis.h";

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "HiRedis.lib")
#define SERVER_PORT 8023

using connect_info = std::tuple<bool, std::string, std::string, int, int>;


class Server {
private:
	int sockfd;
	int client_sock;
	sockaddr_in server_addr;

	static MYSQL *mysql;

	// �������: ��name����Ӧ��sockfd�Ĺ�ϣ��
	static std::unordered_map<std::string, int> name_sock_map;
	// ���������, ��ֹ��д��ͻ
	static std::mutex name_sock_lock;
	// �������: ����Ⱥ�ĺ��뵽��Ӧ�ĳ�Ա
	static std::unordered_map<int, std::set<int>> group_sock_map;
	// ���������, ��ֹ��д��ͻ
	static std::mutex group_sock_lock;
	// �������: �ͻ��˵�cookie�Ĺ�ϣ��
	static std::unordered_map<int, std::string> sock_cookie_map;
	// ���������, ��ֹ��д��ͻ
	static std::mutex sock_cookie_lock;
	


public:
	Server(const char* ip, int port);
	~Server();
	void run();
	static void RecvMsg(int conn);
	static void HandleRequest(int conn, std::string str, connect_info& info);
};