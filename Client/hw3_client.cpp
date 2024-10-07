#include "Common.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    50

long getFileSize(const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (file == NULL) return -1;

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fclose(file);

	return size;
}

int main(int argc, char* argv[])
{
	int retval;
	
	// ����� �μ� = ����ڰ� �Է��� ���� �̸�
	// C:\Users\zztmd\Videos\OBS\Joker.mkv
	if (argc < 2)
		err_quit("Usage: program <filename>");

	const char* file_name = argv[1];
	if (!file_name)
		err_quit("Can't Find Video");

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
	long file_size = getFileSize(file_name);

	retval = send(sock, (char*)&file_size, sizeof(long), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() - file size");
	}

	// ���� ������ �����ϱ�
	const int CHUNK_SIZE = 1024;
	std::ifstream file(file_name, std::ios::binary);
	std::vector<char> buffer(CHUNK_SIZE);

	int totalChunks = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
	int chunkNumber = 0;

	while ( !file.eof() ) {
		file.read(buffer.data(), CHUNK_SIZE);
		int bytesRead = file.gcount();

		// ���� ���� ��� ����
		int header[3] = { chunkNumber, bytesRead, totalChunks };
		send(sock, (char*)header, sizeof(header), 0);

		// ���� ���� ������ ����
		send(sock, buffer.data(), bytesRead, 0);

		chunkNumber++;
	}

	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}