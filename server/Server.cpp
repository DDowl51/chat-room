#include "Server.h"

Server::Server(const char* ip, int port)
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);



}

Server::~Server()
{
	closesocket(sockfd);
}

void Server::run()
{
	sockaddr_in client_addr;
	int ret;
	ret = bind(sockfd, (sockaddr*)&server_addr, sizeof(sockaddr));
	listen(sockfd, SOMAXCONN);
	int len = sizeof(SOCKADDR);

	while (1) {
		client_sock = accept(sockfd, (SOCKADDR*)&client_addr, &len);
		if (client_sock < 0) {
			std::cout << WSAGetLastError() << std::endl;
		}
		else
			std::cout << "\n��ͻ���[" << client_sock << "]����, ׼����������\n";


		std::thread recvThread(RecvMsg, client_sock);
		recvThread.detach();
	}
}

void Server::RecvMsg(int conn)
{
	char recvbuf[1000];
	while (1) {
		memset(recvbuf, 0, sizeof(recvbuf));
		int ret = recv(conn, recvbuf, sizeof(recvbuf), 0);
		if (ret > 0) {
			std::cout << "�յ���Ϣ: " << recvbuf << std::endl;
			HandleRequest(conn, recvbuf);
		}
	}
}

void Server::HandleRequest(int conn, std::string str) {
	// connect to mysql server
	MYSQL mysql;
	mysql_init(&mysql);
	if (mysql_real_connect(&mysql, "127.0.0.1", "root", "0814", "chatroom", 3306, NULL, 0)) {
		std::cout << "�ɹ����ӵ�mysql���ݿ�\n";
	}
	else
		std::cout << "���ӵ����ݿ�ʧ��\n";

	std::string sendstr;


	// Register, str="[reg]name:pass"
	if (str.find("[reg]") == 0) {
		std::string name, pass;
		int seperator = str.find(":"), length = str.length();
		pass = str.substr(seperator + 1);
		name = str.substr(5, length - pass.length() - 6/* sizeof("[reg]") + sizeof(":") */);
		std::cout << "name: " << name << std::endl;
		std::cout << "pass: " << pass << std::endl;
		// INSERT INTO users VALUES( "name", "pass" );
		str = "INSERT INTO users VALUES( \"" + name + "\", \"" + pass + "\");";
		std::cout << "SQL���: " << str << std::endl;
		int ret = mysql_query(&mysql, str.c_str());
		if (ret) {
			std::cout << "INSERT failed\n";
			sendstr = "[ret]error";	// register failed
		}
		else {
			std::cout << "INSERT success\n";
			sendstr = "[ret]ok";
		}
		send(conn, sendstr.c_str(), sendstr.length(), 0);
	}
	// Login, str="[login]name:pass"
	else if (str.find("[login]") == 0) {
		std::string name, pass, sqlstatement;
		int seperator = str.find(":"), length = str.length();
		pass = str.substr(seperator + 1);
		name = str.substr(7, length - pass.length() - 8/* sizeof("[reg]") + sizeof(":") */);
		std::cout << "name: " << name << std::endl;
		std::cout << "pass: " << pass << std::endl;
		// SELECT name, pass FROM users WHERE name="name";
		sqlstatement = "SELECT name, pass FROM users WHERE name=\"" + name + "\";";
		int ret = mysql_query(&mysql, sqlstatement.c_str());
		MYSQL_RES* result = mysql_store_result(&mysql);
		MYSQL_ROW sql_row = mysql_fetch_row(result);
		if (!ret && sql_row) {	// �ҵ����
			std::cout << "MYSQL�е�����: name=" << sql_row[0] << ", pass=" << sql_row[1] << std::endl;
			if (pass == sql_row[1]) {
				// ������ȷ
				std::cout << "������ȷ\n";
				sendstr = "[ret]ok";
			}
			else { // �������
				sendstr = "[ret]incorrect_password";
				std::cout << "�������\n";
			} 
		}
		else {	// û�ҵ����û�
			std::cout << "��������Ϊ" << name << "���û�\n";
			sendstr = "[ret]not_found";
		}
		send(conn, sendstr.c_str(), sendstr.length(), 0);
	}

}
