#define _CRT_SECURE_NO_WARNINGS
#include <iomanip>
#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    512

void displayProgress(long long receivedBytes, long long totalBytes) {
	double percentage = (double)receivedBytes / totalBytes * 100;
	std::cout << "\r수신 진행률: " << std::fixed << std::setprecision(2) << percentage << "% ";
	std::cout << "(" << receivedBytes << " / " << totalBytes << " bytes)" << std::flush;
}

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	int len;
	char buf[BUFSIZE + 1];

	long file_size;
	char file_name[BUFSIZE + 1];

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",	addr, ntohs(clientaddr.sin_port));

		// 파일 이름 길이 받기(고정 길이)
		retval = recv(client_sock, (char*)&len, sizeof(int), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		}

		// 파일 이름 데이터 받기(가변 길이)
		retval = recv(client_sock, file_name, len, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		}
	
		// 받은 데이터 출력
		file_name[retval] = '\0';
		printf("[TCP/%s:%d File Name] %s\n", addr, ntohs(clientaddr.sin_port), file_name);


		// 파일 크기 받기(고정 길이)
		retval = recv(client_sock, (char*)&file_size, sizeof(long), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		}

		// 받은 데이터 출력
		printf("[TCP/%s:%d File Size] %ld\n", addr, ntohs(clientaddr.sin_port), file_size);
		

		// 파일 데이터 받기
		FILE* fp = fopen("C:\\Users\\zztmd\\Videos\\OBS\\wb", "wb");
		if (fp == NULL) {
			err_quit("파일 열기 실패");
		}
		long totalReceived = 0;
		int header[3];
		char buf[1024];  // CHUNK_SIZE와 동일하게 설정

		while (true) {
			// 헤더 정보 수신
			retval = recv(client_sock, (char*)header, sizeof(header), MSG_WAITALL);
			if (retval == SOCKET_ERROR || retval == 0) {
				err_display("recv() - header");
				break;
			}

			int chunkNumber = header[0];
			int bytesRead = header[1];
			int totalChunks = header[2];

			// 청크 데이터 수신
			retval = recv(client_sock, buf, bytesRead, MSG_WAITALL);
			if (retval == SOCKET_ERROR || retval == 0) {
				err_display("recv() - chunk data");
				break;
			}

			size_t written = fwrite(buf, 1, bytesRead, fp);
			if (written < (size_t)bytesRead) {
				err_display("fwrite()");
				break;
			}

			totalReceived += bytesRead;

			// 진행률 표시
			displayProgress(totalReceived, file_size);

			// 모든 청크를 받았는지 확인
			if (chunkNumber == totalChunks - 1) {
				break;
			}
		}
		fclose(fp);

		// 소켓 닫기
		closesocket(client_sock);
		printf("\n[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}