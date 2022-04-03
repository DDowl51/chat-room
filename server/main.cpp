#include "Server.h"

void initialization() {
	using namespace std;
	//��ʼ���׽��ֿ�
	WORD w_req = MAKEWORD(2, 2);//�汾��
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0)
		log(LogLevel::error, "��ʼ���׽��ֿ�ʧ��!");
	//���汾��
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		log(LogLevel::error, "�׽��ֿ�汾�Ų���!");
		WSACleanup();
	}
}


int main() {
	// log(LogLevel::warning, "{}", "Test");
	initialization();
	Server s("192.168.0.100", SERVER_PORT);
	s.run();
	
	//std::cout << RedisConnect("127.0.0.1", "auth 123654") << std::endl;
	//std::cout << RedisCommand("HSET session abcd ddowl") << std::endl;
	//system("pause");

	return 0;
}