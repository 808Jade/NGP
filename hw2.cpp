#include "Common.h"


// 도메인 이름 -> IPv4 주소
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

// IPv4 주소 -> 도메인 이름
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
	char buffer[256];  // 적절한 크기의 버퍼 선언
	std::cin >> buffer;
	const char* input = buffer;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 도메인 이름 -> IP 주소
	struct in_addr addr;
	if (GetIPAddr(input, &addr)) {
		// 성공이면 결과 출력
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &addr, str, sizeof(str));
		printf("IP 주소(변환 후) = %s\n", str);

		// IP 주소 -> 도메인 이름
		char name[256];
		if (GetDomainName(addr, name, sizeof(name))) {
			// 성공이면 결과 출력
			printf("도메인 이름(다시 변환 후) = %s\n", name);
		}
	}

	// 윈속 종료
	WSACleanup();
	return 0;
}