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
	// send(sockfd, "[message]exit", 14, 0);
	closesocket(sockfd);
}

void Client::run()
{
	int retry;
	for (retry = 0; retry < 5 && connect(sockfd, (sockaddr*)&server_addr, sizeof(sockaddr)) < 0; retry++)	// 连接服务器失败, 重连
		std::cout << "连接服务器失败, 重连中\n错误代码: " << WSAGetLastError() << std::endl;
	if (retry == 5) {
		std::cout << "连接服务器失败!按任意键退出\n";
		std::cin.get();
		exit(1);
	}
	std::cout << "连接服务器成功\n";


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
		fflush(stdin);
		if(conn > 0)	// 私聊
			sendbuf = "[message]" + sendbuf;	// 格式化
		else if(conn < 0)	// 群聊
			sendbuf = "[gr_message]" + sendbuf;	// 格式化
		int ret = send(abs(conn), sendbuf.c_str(), sendbuf.length(), 0);
		if (sendbuf == "[message]exit" || sendbuf == "[gr_message]exit" || ret < 0) {
			// 退出
			if(ret < 0)
				std::cout << "Error code " << WSAGetLastError() << std::endl;
			closesocket(conn);
			std::cout << "按任意键退出\n";
			std::cin.get();
			fflush(stdin);
			exit(1);
		}
	}
}

void Client::RecvMsg(int conn)
{
	char recvbuf[1000];
	while (1) {
		memset(recvbuf, 0, sizeof(recvbuf));
		int ret = recv(conn, recvbuf, sizeof(recvbuf), 0);
		if (ret < 0) {
			std::cout << "Error code " << WSAGetLastError() << std::endl;
			exit(1);
		}
		if (ret > 0)
			std::cout << recvbuf << std::endl;
	}
}

void Client::HandleClient(int conn) {
	int choice, logined = 0;
	std::string login_name;
	char recvbuf[1000];
	memset(recvbuf, 0, sizeof(recvbuf));

	// 检查本地是否存在cookie, 存在则发送给服务器, 跳过登录
	std::ifstream file;
	file.open("cookie.txt");
	if (file.is_open()) {
		std::string cookie;
		file >> cookie;
		cookie = "[cookie]" + cookie;
		send(conn, cookie.c_str(), cookie.length(), 0);
		recv(conn, recvbuf, sizeof(recvbuf), 0);	// 接收结果
		std::string ret(recvbuf);
		if (ret.substr(0, 7) == "[ret]ok") {
			logined = true;
			login_name = ret.substr(7);
		}
	}
	file.close();

	while (1) {
		if (logined)
			break;
		std::cout << "0. 退出\n1. 注册\n2. 登录\n";
		fflush(stdin);
		std::cin >> choice;
		if (0 == choice)
			exit(0);
		if (1 == choice) {
			// [reg]name:pass
			std::string reg, name, pass, passConfirm;

			std::cout << "请输入用户名>";
			/* ----- 利用正则表达式检测用户名是否符合规范----- */
			input_and_match(name, "[_a-zA-Z0-9]{4,12}", "用户名不符合规范\n只支持数字字母以及下划线，并且字符数量为4-12");

			set_password:	// 两次密码不一致, 用于重新设置的标签
			std::cout << "请输入密码>";
			/* ----- 利用正则表达式检测密码是否符合规范----- */
			input_and_match(pass, "[_a-zA-Z0-9]{6,15}", "密码不符合规范\n只支持数字字母以及下划线，并且字符数量为6-15");
			std::cout << "请确认密码>";
			std::cin >> passConfirm;
			if (pass != passConfirm) {
				std::cout << "两次密码输入不一致!\n";
				goto set_password;
			}

			// 设置注册格式: [reg]name:pass
			reg = "[reg]" + name + ":" + pass;
			send(conn, reg.c_str(), reg.length(), 0);
			// 接收登录状态
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

			std::string rec = recvbuf;
			// std::cout << name << std::endl;
			if (rec == "[ret]not_found") {
				std::cout << "用户不存在\n";
			}
			else if (rec == "[ret]incorrect_password") {
				std::cout << "密码错误\n";
			}
			else if (rec.substr(0, 7) == "[ret]ok") {
				std::cout << "登陆成功\n";
				logined = 1;
				login_name = name;
				std::string cookie = rec.substr(7);
				std::ofstream file;
				file.open("cookie.txt", std::ios::out);
				file.write(cookie.c_str(), cookie.length());
				file.close();
				break;
			}
		}
	}

	system("cls");
	while (1) {
		if (logined) {
			std::cout << "用户名: " << login_name << std::endl;
			std::cout << "0. 退出\n1. 私聊\n2. 群聊\n";
			std::cin >> choice;
			fflush(stdin);
			if (0 == choice)
				break;
			if (1 == choice) {	// 私聊
				std::string sendstr;
				std::cout << "请输入要私聊的对象>";
				std::cin >> sendstr;
				fflush(stdin);
				// [target]target
				sendstr = "[target]" + sendstr;
				send(conn, sendstr.c_str(), sendstr.length(), 0);

				memset(recvbuf, 0, sizeof(recvbuf));
				recv(conn, recvbuf, sizeof(recvbuf), 0);
				// 暂时借用一下sendstr变量
				sendstr = recvbuf;

				//std::cout << sendstr << std::endl;
				//fflush(stdin);
				//std::cin.get();

				if (sendstr == "[ret]ok") {	// 找到target
					std::cout << "请输入要发送的信息>";
					std::thread send_thread(SendMsg, conn), recv_thread(RecvMsg, conn);
					send_thread.join();
					recv_thread.join();
				}
				else if (sendstr == "[ret]not_found") {
					// 用户不存在
					std::cout << "未找到用户\n";
					std::cout << "按任意键继续\n";
					fflush(stdin);
					std::cin.get();
					continue;
				}
			} 
			if (2 == choice) {	// 群聊
				std::string group_num;
				std::cout << "请输入群聊号>";
				// std::cin >> group_num;
				input_and_match(group_num, "[1-9]+[0-9]?", "群聊号只支持数字且不能为0开头");
				group_num = "[group_num]" + group_num;	// [group_num]123456789
				send(conn, group_num.c_str(), group_num.length(), 0);	// 发给服务端

				std::cout << "请输入要发送的信息>";
				std::thread send_thread(SendMsg, -conn)/* conn为负, 代表为群聊消息 */, recv_thread(RecvMsg, conn);
				send_thread.join();
				recv_thread.join();
			}

		} else
			break;
	}
	closesocket(sockfd);

}
