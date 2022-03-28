#include "Client.h"

void initialization() {
	using namespace std;
	//³õÊ¼»¯Ì×½Ó×Ö¿â
	WORD w_req = MAKEWORD(2, 2);//°æ±¾ºÅ
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0)
		cout << "³õÊ¼»¯Ì×½Ó×Ö¿âÊ§°Ü£¡" << endl;
	//¼ì²â°æ±¾ºÅ
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		cout << "Ì×½Ó×Ö¿â°æ±¾ºÅ²»·û£¡" << endl;
		WSACleanup();
	}
}


int main() {
	initialization();
	Client c("192.168.0.100", SERVER_PORT);
	c.run();
	return 0;
}