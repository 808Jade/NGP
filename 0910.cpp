#include "Common.h"

int main(int argc, char* argv[])
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) // MAKEWORD(2,2) 0x0202
		return 1;
	printf("[알림] 윈속 초기화 성공\n");

	printf("wVersion : %d\n", wsa.wVersion);
	printf("wHighVersion : %d\n", wsa.wHighVersion);
	printf("szDescription : %s\n", wsa.szDescription);
	printf("szSystemStatus : %s\n", wsa.szSystemStatus);

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");
	printf("[알림] 소켓 생성 성공\n");
	// std::cout <<"소켓 사이즈 : " << sizeof(sock);

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}