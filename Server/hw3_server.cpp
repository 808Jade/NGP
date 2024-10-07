#define _CRT_SECURE_NO_WARNINGS
#include <iomanip>
#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    512

void displayProgress(long long receivedBytes, long long totalBytes) {
	double percentage = (double)receivedBytes / totalBytes * 100;
	std::cout << "\r���� �����: " << std::fixed << std::setprecision(2) << percentage << "% ";
	std::cout << "(" << receivedBytes << " / " << totalBytes << " bytes)" << std::flush;
}

int main(int argc, char* argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
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

	// ������ ��ſ� ����� ����
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

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",	addr, ntohs(clientaddr.sin_port));

		// ���� �̸� ���� �ޱ�(���� ����)
		retval = recv(client_sock, (char*)&len, sizeof(int), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		}

		// ���� �̸� ������ �ޱ�(���� ����)
		retval = recv(client_sock, file_name, len, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		}
	
		// ���� ������ ���
		file_name[retval] = '\0';
		printf("[TCP/%s:%d File Name] %s\n", addr, ntohs(clientaddr.sin_port), file_name);


		// ���� ũ�� �ޱ�(���� ����)
		retval = recv(client_sock, (char*)&file_size, sizeof(long), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
		}

		// ���� ������ ���
		printf("[TCP/%s:%d File Size] %ld\n", addr, ntohs(clientaddr.sin_port), file_size);
		

		// ���� ������ �ޱ�
		FILE* fp = fopen("C:\\Users\\zztmd\\Videos\\OBS\\wb", "wb");
		if (fp == NULL) {
			err_quit("���� ���� ����");
		}
		long totalReceived = 0;
		int header[3];
		char buf[1024];  // CHUNK_SIZE�� �����ϰ� ����

		while (true) {
			// ��� ���� ����
			retval = recv(client_sock, (char*)header, sizeof(header), MSG_WAITALL);
			if (retval == SOCKET_ERROR || retval == 0) {
				err_display("recv() - header");
				break;
			}

			int chunkNumber = header[0];
			int bytesRead = header[1];
			int totalChunks = header[2];

			// ûũ ������ ����
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

			// ����� ǥ��
			displayProgress(totalReceived, file_size);

			// ��� ûũ�� �޾Ҵ��� Ȯ��
			if (chunkNumber == totalChunks - 1) {
				break;
			}
		}
		fclose(fp);

		// ���� �ݱ�
		closesocket(client_sock);
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// ���� �ݱ�
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}