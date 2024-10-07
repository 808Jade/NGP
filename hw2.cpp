#include "Common.h"

#include <string>
#include <vector>

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

std::vector<std::string> getIPAddresses(struct hostent* he) 
{
	std::vector<std::string> ipAddresses;

	// IPv4 유효
	if (he->h_addrtype == AF_INET) {
		for (int i = 0; he->h_addr_list[i] != nullptr; i++) {
			char ip[INET_ADDRSTRLEN];	// 22

			inet_ntop(AF_INET, he->h_addr_list[i], ip, sizeof(ip));
			ipAddresses.push_back(ip);
		}
	}

	return ipAddresses;
}

std::vector<std::string> getAliases(struct hostent* he) 
{
	std::vector<std::string> aliases;

	for (char** alias = he->h_aliases; *alias != nullptr; alias++) {
		aliases.push_back(*alias);
	}
	
	return aliases;
}

int main(int argc, char* argv[])
{
	 std::string domain = argv[1];

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	struct hostent* p_he = gethostbyname( domain.c_str() );
	if (p_he == nullptr) {
		std::cerr << "gethostbyname failed: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}
	
	std::vector<std::string> ipAddresses = getIPAddresses(p_he);
	std::vector<std::string> aliases = getAliases(p_he);

	std::cout << "IPv4 Addresses:" << std::endl;
	for (const auto& ip : ipAddresses) {
		std::cout << "  - " << ip << std::endl;
	}

	std::cout << "별명 (CNAMEs):" << std::endl;
	if (!aliases.empty()) {
		for (const auto& alias : aliases) {
			std::cout << "  - " << alias << std::endl;
		}
	}
	else {
		std::cout << "  No aliases found" << std::endl;
	}

	WSACleanup();
	return 0;
}