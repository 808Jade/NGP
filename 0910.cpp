#include "Common.h"

int main(int argc, char* argv[])
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) // MAKEWORD(2,2) 0x0202
		return 1;
	printf("[�˸�] ���� �ʱ�ȭ ����\n");

	printf("wVersion : %d\n", wsa.wVersion);
	printf("wHighVersion : %d\n", wsa.wHighVersion);
	printf("szDescription : %s\n", wsa.szDescription);
	printf("szSystemStatus : %s\n", wsa.szSystemStatus);

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");
	printf("[�˸�] ���� ���� ����\n");
	// std::cout <<"���� ������ : " << sizeof(sock);

	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}