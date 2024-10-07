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
	
	// 명령행 인수 = 사용자가 입력한 파일 이름
	// C:\Users\zztmd\Videos\OBS\Joker.mkv
	if (argc < 2)
		err_quit("Usage: program <filename>");

	const char* file_name = argv[1];
	if (!file_name)
		err_quit("Can't Find Video");

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
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

	// 데이터 통신에 사용할 변수
	int len;

	// 파일 이름 전송하기
	len = (int)strlen(file_name);
	retval = send(sock, (char*)&len, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() - file name length");
	}

	retval = send(sock, file_name, len, 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() - file name data");
	}

	// 파일 총 용량 전송하기
	long file_size = getFileSize(file_name);

	retval = send(sock, (char*)&file_size, sizeof(long), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() - file size");
	}

	// 파일 데이터 전송하기
	const int CHUNK_SIZE = 1024;
	std::ifstream file(file_name, std::ios::binary);
	std::vector<char> buffer(CHUNK_SIZE);

	int totalChunks = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
	int chunkNumber = 0;

	while ( !file.eof() ) {
		file.read(buffer.data(), CHUNK_SIZE);
		int bytesRead = file.gcount();

		// 고정 길이 헤더 전송
		int header[3] = { chunkNumber, bytesRead, totalChunks };
		send(sock, (char*)header, sizeof(header), 0);

		// 가변 길이 데이터 전송
		send(sock, buffer.data(), bytesRead, 0);

		chunkNumber++;
	}

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}