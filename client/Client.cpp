#include "Client.h"

Client::Client(const char* ip, int port)
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);
}

Client::~Client()
{
	closesocket(sockfd);
}

void Client::run()
{
	int ret = connect(sockfd, (sockaddr*)&server_addr, sizeof(sockaddr));
	if (ret == SOCKET_ERROR)
		perror("连接失败");
	else
		std::cout << "连接成功\n";


	HandleClient(sockfd);
	//std::thread sendThread(SendMsg, sockfd);
	//std::thread recvThread(RecvMsg, sockfd);
	//sendThread.join();
	//recvThread.join();

}

void Client::SendMsg(int conn)
{
	std::string sendbuf;
	while (1) {
		sendbuf.clear();
		getline(std::cin, sendbuf);
		send(conn, sendbuf.c_str(), sendbuf.length(), 0);
	}
}

void Client::RecvMsg(int conn)
{
	char recvbuf[1000];
	while (1) {
		memset(recvbuf, 0, sizeof(recvbuf));
		int ret = recv(conn, recvbuf, sizeof(recvbuf), 0);
		if (ret < 0) {
			perror("recv");
		}
		if (ret > 0)
			std::cout << "[server]" << recvbuf << std::endl;
	}
}

void Client::HandleClient(int conn) {
	int choice, logined = 0;

	char recvbuf[1000];
	memset(recvbuf, 0, sizeof(recvbuf));

	while (1) {
		std::cout << "0. 退出\n1. 注册\n2. 登录\n";
		std::cin >> choice;
		if (0 == choice)
			exit(0);
		if (1 == choice) {
			// [reg]name:pass
			std::string reg, name, pass, passConfirm;
			std::cout << "请输入用户名>";
			std::cin >> name;
			while (1) {
				std::cout << "请输入密码>";
				std::cin >> pass;
				std::cout << "请确认密码>";
				std::cin >> passConfirm;
				if (pass == passConfirm)
					break;
				else
					std::cout << "两次密码输入不一致!\n";
			}
			reg = "[reg]" + name + ":" + pass;
			send(conn, reg.c_str(), reg.length(), 0);

			memset(recvbuf, 0, sizeof(recvbuf));
			recv(conn, recvbuf, sizeof(recvbuf), 0);
			// 暂时借用一下name变量
			name = recvbuf;
			// std::cout << name << std::endl;
			if (name == "[ret]error") {
				std::cout << "注册失败\n";
			}
			else if (name == "[ret]ok") {
				std::cout << "注册成功\n请输入数字继续操作>\n";
			}
		}
		if (2 == choice) {
			// [login]name:pass
			std::string login, name, pass;

			std::cout << "请输入用户名>";
			std::cin >> name;
			std::cout << "请输入密码>";
			std::cin >> pass;

			login = "[login]" + name + ":" + pass;

			send(conn, login.c_str(), login.length(), 0);

			memset(recvbuf, 0, sizeof(recvbuf));
			recv(conn, recvbuf, sizeof(recvbuf), 0);
			// 暂时借用一下name变量
			name = recvbuf;
			// std::cout << name << std::endl;
			if (name == "[ret]not_found") {
				std::cout << "用户不存在\n";
			}
			else if (name == "[ret]incorrect_password") {
				std::cout << "密码错误\n";
			}
			else if (name == "[ret]ok") {
				std::cout << "登陆成功\n";
				logined = 1;
				break;
			}
		}
	}

	while (1) {
		if (logined) {
			system("cls");
			std::cout << "0. 退出\n1. 私聊\n2. 群聊\n";
			std::cin >> choice;
			if (0 == choice)
				exit(0);
			if (1 == choice) {
				std::string sendstr;
				std::cout << "请输入要私聊的对象>";
				std::cin >> sendstr;
				// [target]target
				sendstr = "[target]" + sendstr;
				send(conn, sendstr.c_str(), sendstr.length(), 0);

				memset(recvbuf, 0, sizeof(recvbuf));
				recv(conn, recvbuf, sizeof(recvbuf), 0);
				// 暂时借用一下sendstr变量
				sendstr = recvbuf;
				if (sendstr == "[ret]ok") {	// 找到target并且在线
					std::cout << "请输入要发送的信息>";
					std::thread send_thread(SendMsg, conn), recv_thread(RecvMsg, conn);
					send_thread.join();
					recv_thread.join();
				}
				else if (sendstr == "[ret]not_found") {
					// 用户不存在
					std::cout << "用户不存在\n";
					continue;
				}
				else if (sendstr == "[ret]not_online") {
					// 用户不在线
					std::cout << "用户不在线\n";
					continue;
				}

			}

		} else
			break;
	}
	closesocket(sockfd);

}
