#include "Server.h"


std::unordered_map<std::string, int> Server::name_sock_map;
std::unordered_map<int, std::set<int>> Server::group_sock_map;
std::unordered_map<int, std::string> Server::sock_cookie_map;
std::mutex Server::name_sock_lock;
std::mutex Server::group_sock_lock;
std::mutex Server::sock_cookie_lock;

MYSQL *Server::mysql = mysql_init(mysql);

Server::Server(const char* ip, int port)
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);
	
	if (mysql_real_connect(mysql, "127.0.0.1", "root", "0814", "chatroom", 3306, NULL, 0))
		std::cout << "�ɹ����ӵ�mysql���ݿ�\n";
	else
		std::cout << "���ӵ����ݿ�ʧ��\n";

	if (RedisConnect("127.0.0.1", "AUTH 123654") == "OK")
		std::cout << "�ɹ����ӵ�Redis������\n";
	else
		std::cout << "���ӵ�Redis������ʧ��\n";
}

Server::~Server()
{
	for (auto& c : name_sock_map) {
		closesocket(c.second);	// c.second�ǿͻ��˵�sockfd
	}
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
	connect_info info;
	/* 
	 * bool logined;			�ͻ����Ƿ��Ѿ���¼
	 * std::string login_name;	�ͻ��˵�¼�û���
	 * std::string target_name;	Ŀ��ͻ����û���
	 * int target_conn��			Ŀ��ͻ����׽���
	 * int group_num			�ͻ�������Ⱥ
	 */
	std::get<0>(info) = false;	// logined = false;
	std::get<3>(info) = -1;		// target_conn = -1;

	while (1) {
		memset(recvbuf, 0, sizeof(recvbuf));
		int ret = recv(conn, recvbuf, sizeof(recvbuf), 0);
		if (ret > 0) {
			std::cout << "�յ���Ϣ: " << recvbuf << std::endl;
			HandleRequest(conn, recvbuf, info);
		}
		else {
			std::cout << "ret=" << ret << ", �ر���ͻ���[" << conn << "]������\n";
			closesocket(conn);
			name_sock_map.erase(std::get<1>(info));
			break;
		}
	}
}

// �ͻ��˵��׽���, ���յ�����Ϣ, ������������Ϣ
void Server::HandleRequest(int conn, std::string str, connect_info& info) {

	// ������������Ϣ
	bool logined				= std::get<0>(info);
	std::string login_name		= std::get<1>(info);
	std::string target_name		= std::get<2>(info);
	int target_conn				= std::get<3>(info);
	int group_num				= std::get<4>(info);

	// connect to mysql server

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
		int ret = mysql_query(mysql, str.c_str());
		if (ret) {
			std::cout << "INSERT failed\n";
			sendstr = "[ret]error";	// register failed
		}
		else {
			std::cout << "INSERT success\n";
			sendstr = "[ret]ok";
		}
		send(conn, sendstr.c_str(), sendstr.length(), 0);	// ��ͻ��˷��ؽ��
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
		int ret = mysql_query(mysql, sqlstatement.c_str());
		MYSQL_RES* result = mysql_store_result(mysql);
		MYSQL_ROW sql_row = mysql_fetch_row(result);
		if (!ret && sql_row) {	// �ҵ����
			std::cout << "MYSQL�е�����: name=" << sql_row[0] << ", pass=" << sql_row[1] << std::endl;
			if (pass == sql_row[1]) {
				// ������ȷ
				std::cout << "������ȷ\n";
				sendstr = "[ret]ok";

				// ����name_sock_map��
				name_sock_lock.lock();
				name_sock_map[name] = conn;	// ���뵽name_sock_map����, �����û��ѵ�¼
				name_sock_lock.unlock();

				logined = true;
				login_name = name;

				// ����cookie
				srand(time(NULL));	// ��ʱ����Ϊ����㷨������
				std::string cookie;
				for (int i = 0; i < 10; i++) {	// ʮλ��cookie
					int type = rand() % 3;	// 0:����, 1:Сд��ĸ, 2:��д��ĸ
					int random;
					switch (type) {
					case 0:
						random = rand() % 10;
						cookie += '0' + random;
						break;
					case 1:
						random = rand() % 26;
						cookie += 'a' + random;
						break;
					case 2:
						random = rand() % 26;
						cookie += 'A' + random;
						break;
					}
				}
				std::cout << "����cookie: " << cookie << std::endl;
				std::string rCmd = "HSET session " + cookie + " " + login_name;	// "HGET session cookie login_name"
				std::string result = RedisCommand(rCmd.c_str());	// û�з���
				std::cout << "Redis���: " << rCmd << std::endl;
				if (result != "(null)")
					std::cout << "Redis: " << result << std::endl;

				sendstr += cookie;	// �����ɵ�cookie���͵��ͻ���

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
		send(conn, sendstr.c_str(), sendstr.length(), 0);	// ��ͻ��˷��ؽ��
	}
	else if (str.find("[target]") == 0) {	// ����˽�Ķ��� 
		std::string target = str.substr(8);
		std::cout << "׼��������" << login_name << "��" << target << "������\n";
		if (name_sock_map.find(target) == name_sock_map.end()) {
			// Ŀ��δ�ҵ�
			std::cout << "δ�ҵ���Ϊ" << target << "��Ŀ��\n";
			sendstr = "[ret]not_found";
		}
		else {
			// �ҵ�Ŀ��, ��������
			std::cout << "������" << login_name << "��" << target_name << "������\n";
			target_name = target;
			target_conn = name_sock_map[target];
			sendstr = "[ret]ok";
		}
		send(conn, sendstr.c_str(), sendstr.length(), 0);	// ��ͻ��˷��ؽ��
	}
	else if (str.find("[message]") == 0) {
		// ����˽����Ϣ
		sendstr = str.substr(9);	// ȥ��ǰ׺"[message]"
		if (sendstr == "exit") {
			// �ͻ�������, ����name_sock_map��
			std::cout << login_name << "����\n";
			closesocket(conn);
			name_sock_map.erase(login_name);
		} else {
			std::cout << "��" << login_name << "ת����Ϣ:\"" << sendstr << "\"��" << target_name << std::endl;
			sendstr = "[" + login_name + "]" + sendstr;	// ��ʽ��Ϊ"[login_name]message..."
			send(target_conn, sendstr.c_str(), sendstr.length(), 0);	// ����Ϣת����Ŀ��ͻ���
		}
	}
	else if (str.find("[group_num]") == 0) {	// ��Ⱥ�ĺ�
		group_num = std::stoi(str.substr(11));	// ȥ��ǰ׺[group], ת��������

		std::cout << "�ͻ���[" << conn << "]����Ⱥ��" << group_num << std::endl;

		group_sock_lock.lock();
		group_sock_map[group_num].insert(conn);	// ����ǰ�ͻ��˵�sockfd����Ⱥ����
		group_sock_lock.unlock();
	}
	else if (str.find("[gr_message]") == 0)	{	// Ⱥ����Ϣ
		sendstr = str.substr(12);	// ȥ��ǰ׺[gr_message]
		std::cout << "����" << login_name << "��Ⱥ" << group_num << "������Ϣ: " << sendstr << std::endl;
		sendstr = "[" + login_name + "]" + sendstr;
		for (auto c : group_sock_map[group_num]) {
			if (c != conn)
				send(c, sendstr.c_str(), sendstr.length(), 0);
		}
	}
	else if (str.find("[cookie]") == 0) {	// cookie��¼
		std::string cookie = str.substr(8);
		std::cout << "�յ��ͻ���[" << conn << "]��cookie: " << cookie << std::endl;
		std::string rCmd = "HGET session " + cookie;
		std::string result = RedisCommand(rCmd.c_str());	// Ӧ�÷���cookie��Ӧ���û���, ���򷵻�"(null)"
		if (result != "(null)" && result.substr(0, 3) != "ERR") {	// �ҵ���Ӧ�û�, ֱ�ӵ�¼
			logined = true;
			login_name = result;
			std::cout << "�ҵ��û�" << login_name << std::endl;
			result = "[ret]ok" + result;	// "[ret]oklogin_name" �� [ret]ok+�û���
		}
		else
			std::cout << "û���ҵ�cookie��Ӧ���û�" << std::endl;
		// û���ҵ�cookie��Ӧ���û�����cookie����, ֱ�ӷ���"(null)", �ͻ��˽���������
		send(conn, result.c_str(), result.length(), 0);
	}


	// ���»�����Ϣinfo
	std::get<0>(info) = logined;
	std::get<1>(info) = login_name;
	std::get<2>(info) = target_name;
	std::get<3>(info) = target_conn;
	std::get<4>(info) = group_num;
}
