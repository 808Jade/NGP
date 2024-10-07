#include "Common.h"


// ������ �̸� -> IPv4 �ּ�
bool GetIPAddr(const char* name, struct in_addr* addr)
{
	struct hostent* ptr = gethostbyname(name);

	for (char** alias = ptr->h_aliases; *alias != NULL; alias++) {
		std::cout << "alias : " << *alias << std::endl;
	}


	if (ptr == NULL) {
		err_display("gethostbyname()");
		return false;
	}
	if (ptr->h_addrtype != AF_INET)
		return false;
	memcpy(addr, ptr->h_addr, ptr->h_length);
	return true;
}

// IPv4 �ּ� -> ������ �̸�
bool GetDomainName(struct in_addr addr, char* name, int namelen)
{
	struct hostent* ptr = gethostbyaddr((const char*)&addr, sizeof(addr), AF_INET);

	for (char** alias = ptr->h_aliases; *alias != NULL; alias++) {
		std::cout << "alias : " << *alias << std::endl;
	}

	if (ptr == NULL) {
		err_display("gethostbyaddr()");
		return false;
	}
	if (ptr->h_addrtype != AF_INET)
		return false;
	strncpy(name, ptr->h_name, namelen);
	return true;
}

int main(int argc, char* argv[])
{
	// std::cout << "input : " << argv[1] << std::endl;

	// const char* input = argv[1];
	char buffer[256];  // ������ ũ���� ���� ����
	std::cin >> buffer;
	const char* input = buffer;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ������ �̸� -> IP �ּ�
	struct in_addr addr;
	if (GetIPAddr(input, &addr)) {
		// �����̸� ��� ���
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &addr, str, sizeof(str));
		printf("IP �ּ�(��ȯ ��) = %s\n", str);

		// IP �ּ� -> ������ �̸�
		char name[256];
		if (GetDomainName(addr, name, sizeof(name))) {
			// �����̸� ��� ���
			printf("������ �̸�(�ٽ� ��ȯ ��) = %s\n", name);
		}
	}

	// ���� ����
	WSACleanup();
	return 0;
}