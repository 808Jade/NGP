#include "Common.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    50
#define CHUNKSIZE 1024

long long getFileSize(const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (file == NULL) return -1;

	fseek(file, 0, SEEK_END);
	long long size = ftell(file);
	fclose(file);

	return size;
}

int main(int argc, char* argv[])
{
	int retval;
	
	// C:\Users\zztmd\Videos\OBS\Joker.mkv
	
	const char* file_name = "C:\\Users\\zztmd\\Videos\\OBS\\Joker.mkv";

	// ����� �μ� ó��
	if (argc < 2 || argc > 3) {
		err_quit("Usage: program [server ip] <filename> ");
	}

	if (argc == 2) {
		file_name = argv[1];
		std::cout << "file name(argv[1]) : " << argv[1] << std::endl;
	}
	else if (argc == 3) {
		SERVERIP = argv[1];
		file_name = argv[2];
		std::cout << "server ip : " << argv[1] << std::endl;
		std::cout << "file name : " << argv[2] << std::endl;
	}

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// ������ ��ſ� ����� ����
	int len;

	// ���� �̸� �����ϱ�
	len = (int)strlen(file_name);
	retval = send(sock, (char*)&len, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() - file name length");
	}

	retval = send(sock, file_name, len, 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() - file name data");
	}

	// ���� �� �뷮 �����ϱ�
	long long file_size = getFileSize(file_name);
	std::cout <<"before send file_size : " << file_size << std::endl;

	retval = send(sock, (char*)&file_size, sizeof(long long), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() - file size");
	}

	// ���� ������ �����ϱ�
	std::ifstream file(file_name, std::ios::binary);
	std::vector<char> buffer(CHUNKSIZE);

	int totalChunks = (file_size + CHUNKSIZE - 1) / CHUNKSIZE;
	int chunkNumber = 0;

	while ( !file.eof() ) {
		file.read(buffer.data(), CHUNKSIZE);
		int bytesRead = file.gcount();

		// ���� ���� ��� ����
		int header[3] = { chunkNumber, bytesRead, totalChunks };
		send(sock, (char*)header, sizeof(header), 0);

		// ���� ���� ������ ����
		send(sock, buffer.data(), bytesRead, 0);

		chunkNumber++;

		Sleep(100);
	}

	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}