#include "Client.h"

void initialization() {
	using namespace std;
	//��ʼ���׽��ֿ�
	WORD w_req = MAKEWORD(2, 2);//�汾��
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0)
		cout << "��ʼ���׽��ֿ�ʧ�ܣ�" << endl;
	//���汾��
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		cout << "�׽��ֿ�汾�Ų�����" << endl;
		WSACleanup();
	}
}


int main() {
	initialization();
	Client c("192.168.0.100", SERVER_PORT);
	c.run();
	return 0;
}