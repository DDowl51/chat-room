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
	for (retry = 0; retry < 5 && connect(sockfd, (sockaddr*)&server_addr, sizeof(sockaddr)) < 0; retry++)	// ���ӷ�����ʧ��, ����
		std::cout << "���ӷ�����ʧ��, ������\n�������: " << WSAGetLastError() << std::endl;
	if (retry == 5) {
		std::cout << "���ӷ�����ʧ��!��������˳�\n";
		std::cin.get();
		exit(1);
	}
	std::cout << "���ӷ������ɹ�\n";


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
		if(conn > 0)	// ˽��
			sendbuf = "[message]" + sendbuf;	// ��ʽ��
		else if(conn < 0)	// Ⱥ��
			sendbuf = "[gr_message]" + sendbuf;	// ��ʽ��
		int ret = send(abs(conn), sendbuf.c_str(), sendbuf.length(), 0);
		if (sendbuf == "[message]exit" || sendbuf == "[gr_message]exit" || ret < 0) {
			// �˳�
			if(ret < 0)
				std::cout << "Error code " << WSAGetLastError() << std::endl;
			closesocket(conn);
			std::cout << "��������˳�\n";
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

	// ��鱾���Ƿ����cookie, �������͸�������, ������¼
	std::ifstream file;
	file.open("cookie.txt");
	if (file.is_open()) {
		std::string cookie;
		file >> cookie;
		cookie = "[cookie]" + cookie;
		send(conn, cookie.c_str(), cookie.length(), 0);
		recv(conn, recvbuf, sizeof(recvbuf), 0);	// ���ս��
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
		std::cout << "0. �˳�\n1. ע��\n2. ��¼\n";
		fflush(stdin);
		std::cin >> choice;
		if (0 == choice)
			exit(0);
		if (1 == choice) {
			// [reg]name:pass
			std::string reg, name, pass, passConfirm;

			std::cout << "�������û���>";
			/* ----- ����������ʽ����û����Ƿ���Ϲ淶----- */
			input_and_match(name, "[_a-zA-Z0-9]{4,12}", "�û��������Ϲ淶\nֻ֧��������ĸ�Լ��»��ߣ������ַ�����Ϊ4-12");

			set_password:	// �������벻һ��, �����������õı�ǩ
			std::cout << "����������>";
			/* ----- ����������ʽ��������Ƿ���Ϲ淶----- */
			input_and_match(pass, "[_a-zA-Z0-9]{6,15}", "���벻���Ϲ淶\nֻ֧��������ĸ�Լ��»��ߣ������ַ�����Ϊ6-15");
			std::cout << "��ȷ������>";
			std::cin >> passConfirm;
			if (pass != passConfirm) {
				std::cout << "�����������벻һ��!\n";
				goto set_password;
			}

			// ����ע���ʽ: [reg]name:pass
			reg = "[reg]" + name + ":" + pass;
			send(conn, reg.c_str(), reg.length(), 0);
			// ���յ�¼״̬
			memset(recvbuf, 0, sizeof(recvbuf));
			recv(conn, recvbuf, sizeof(recvbuf), 0);
			// ��ʱ����һ��name����
			name = recvbuf;
			// std::cout << name << std::endl;
			if (name == "[ret]error") {
				std::cout << "ע��ʧ��\n";
			}
			else if (name == "[ret]ok") {
				std::cout << "ע��ɹ�\n���������ּ�������>\n";
			}
		}
		if (2 == choice) {
			// [login]name:pass
			std::string login, name, pass;

			std::cout << "�������û���>";
			std::cin >> name;
			std::cout << "����������>";
			std::cin >> pass;

			login = "[login]" + name + ":" + pass;

			send(conn, login.c_str(), login.length(), 0);

			memset(recvbuf, 0, sizeof(recvbuf));
			recv(conn, recvbuf, sizeof(recvbuf), 0);

			std::string rec = recvbuf;
			// std::cout << name << std::endl;
			if (rec == "[ret]not_found") {
				std::cout << "�û�������\n";
			}
			else if (rec == "[ret]incorrect_password") {
				std::cout << "�������\n";
			}
			else if (rec.substr(0, 7) == "[ret]ok") {
				std::cout << "��½�ɹ�\n";
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
			std::cout << "�û���: " << login_name << std::endl;
			std::cout << "0. �˳�\n1. ˽��\n2. Ⱥ��\n";
			std::cin >> choice;
			fflush(stdin);
			if (0 == choice)
				break;
			if (1 == choice) {	// ˽��
				std::string sendstr;
				std::cout << "������Ҫ˽�ĵĶ���>";
				std::cin >> sendstr;
				fflush(stdin);
				// [target]target
				sendstr = "[target]" + sendstr;
				send(conn, sendstr.c_str(), sendstr.length(), 0);

				memset(recvbuf, 0, sizeof(recvbuf));
				recv(conn, recvbuf, sizeof(recvbuf), 0);
				// ��ʱ����һ��sendstr����
				sendstr = recvbuf;

				//std::cout << sendstr << std::endl;
				//fflush(stdin);
				//std::cin.get();

				if (sendstr == "[ret]ok") {	// �ҵ�target
					std::cout << "������Ҫ���͵���Ϣ>";
					std::thread send_thread(SendMsg, conn), recv_thread(RecvMsg, conn);
					send_thread.join();
					recv_thread.join();
				}
				else if (sendstr == "[ret]not_found") {
					// �û�������
					std::cout << "δ�ҵ��û�\n";
					std::cout << "�����������\n";
					fflush(stdin);
					std::cin.get();
					continue;
				}
			} 
			if (2 == choice) {	// Ⱥ��
				std::string group_num;
				std::cout << "������Ⱥ�ĺ�>";
				// std::cin >> group_num;
				input_and_match(group_num, "[1-9]+[0-9]?", "Ⱥ�ĺ�ֻ֧�������Ҳ���Ϊ0��ͷ");
				group_num = "[group_num]" + group_num;	// [group_num]123456789
				send(conn, group_num.c_str(), group_num.length(), 0);	// ���������

				std::cout << "������Ҫ���͵���Ϣ>";
				std::thread send_thread(SendMsg, -conn)/* connΪ��, ����ΪȺ����Ϣ */, recv_thread(RecvMsg, conn);
				send_thread.join();
				recv_thread.join();
			}

		} else
			break;
	}
	closesocket(sockfd);

}
