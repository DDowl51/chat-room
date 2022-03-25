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
		perror("����ʧ��");
	else
		std::cout << "���ӳɹ�\n";


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
		std::cout << "0. �˳�\n1. ע��\n2. ��¼\n";
		std::cin >> choice;
		if (0 == choice)
			exit(0);
		if (1 == choice) {
			// [reg]name:pass
			std::string reg, name, pass, passConfirm;

			/* ��whileѭ�����������û��� */
			while (1) {
				std::cout << "�������û���>";
				std::cin >> name;

				/* ----- ����������ʽ����û����Ƿ���Ϲ淶----- */
				// ֻ֧��������ĸ�Լ��»��ߣ������ַ�����Ϊ4-12
				std::regex name_regex("[_a-zA-Z0-9]{4,12}");
				if (!std::regex_match(name, name_regex)) {
					// �û��������Ϲ淶
					std::cout << "�û��������Ϲ淶\n";
					std::cout << "ֻ֧��������ĸ�Լ��»��ߣ������ַ�����Ϊ4-12\n";
					continue;
				}
				else // �û������óɹ�, ����ѭ��
					break;
			}
			/* ��whileѭ�����������û��� */

			/* ��whileѭ�������������� */
			while (1) {
				std::cout << "����������>";
				std::cin >> pass;
				std::cout << "��ȷ������>";
				std::cin >> passConfirm;
				if (pass != passConfirm) {
					std::cout << "�����������벻һ��!\n";
					continue;
				}

				/* ----- ����������ʽ��������Ƿ���Ϲ淶----- */
				// ֻ֧��������ĸ�Լ��»��ߣ������ַ�����Ϊ6-15
				std::regex pass_regex("[_a-zA-Z0-9]{6,15}");
				if (!std::regex_match(pass, pass_regex)) {
					// ���벻���Ϲ淶
					std::cout << "���벻���Ϲ淶\n";
					std::cout << "ֻ֧��������ĸ�Լ��»��ߣ������ַ�����Ϊ6-15\n";
					continue;
				}
				else // �������óɹ�, ����ѭ��
					break;
			}
			/* ��whileѭ�������������� */

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
			// ��ʱ����һ��name����
			name = recvbuf;
			// std::cout << name << std::endl;
			if (name == "[ret]not_found") {
				std::cout << "�û�������\n";
			}
			else if (name == "[ret]incorrect_password") {
				std::cout << "�������\n";
			}
			else if (name == "[ret]ok") {
				std::cout << "��½�ɹ�\n";
				logined = 1;
				break;
			}
		}
	}

	while (1) {
		if (logined) {
			system("cls");
			std::cout << "0. �˳�\n1. ˽��\n2. Ⱥ��\n";
			std::cin >> choice;
			if (0 == choice)
				exit(0);
			if (1 == choice) {
				std::string sendstr;
				std::cout << "������Ҫ˽�ĵĶ���>";
				std::cin >> sendstr;
				// [target]target
				sendstr = "[target]" + sendstr;
				send(conn, sendstr.c_str(), sendstr.length(), 0);

				memset(recvbuf, 0, sizeof(recvbuf));
				recv(conn, recvbuf, sizeof(recvbuf), 0);
				// ��ʱ����һ��sendstr����
				sendstr = recvbuf;
				if (sendstr == "[ret]ok") {	// �ҵ�target��������
					std::cout << "������Ҫ���͵���Ϣ>";
					std::thread send_thread(SendMsg, conn), recv_thread(RecvMsg, conn);
					send_thread.join();
					recv_thread.join();
				}
				else if (sendstr == "[ret]not_found") {
					// �û�������
					std::cout << "�û�������\n";
					continue;
				}
				else if (sendstr == "[ret]not_online") {
					// �û�������
					std::cout << "�û�������\n";
					continue;
				}

			}

		} else
			break;
	}
	closesocket(sockfd);

}
