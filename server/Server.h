#pragma once
#include "global.h"


class Server {
private:
	int sockfd;
	int client_sock;
	sockaddr_in server_addr;

	static MYSQL *mysql;

	// 共享变量: 从name到对应的sockfd的哈希表
	static std::unordered_map<std::string, int> name_sock_map;
	// 共享对象锁, 防止读写冲突
	static std::mutex name_sock_lock;
	// 共享变量: 保存群聊号码到对应的成员
	static std::unordered_map<int, std::set<int>> group_sock_map;
	// 共享对象锁, 防止读写冲突
	static std::mutex group_sock_lock;
	// 共享变量: 客户端到cookie的哈希表
	static std::unordered_map<int, std::string> sock_cookie_map;
	// 共享对象锁, 防止读写冲突
	static std::mutex sock_cookie_lock;
	


public:
	Server(const char* ip, int port);
	~Server();
	void run();
	static void RecvMsg(int conn);
	static void HandleRequest(int conn, std::string str, connect_info& info);
};