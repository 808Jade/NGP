#include "Common.h"

#define MAKEWORD(a, b)      ((WORD)(((BYTE)( ((DWORD_PTR)(a)) & 0xff) ) | ( (WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))

int main(int argc, char* argv[])
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// ���α׷��� �䱸�ϴ� �ֻ��� ���� ���� . ���� 8 ��Ʈ�� �� ������ , ���� 8 ��Ʈ�� �� ������ �־� ����
	// 0 0 0 0 0 0 1 0 | 0 0 0 0 0 0 1 0

	printf("wVersion : %x\n", wsa.wVersion);
	printf("wHighVersion : %x\n", wsa.wHighVersion);
	printf("szDescription : %s\n", wsa.szDescription);
	printf("szSystemStatus : %s\n", wsa.szSystemStatus);

	WSACleanup();
	return 0;
}