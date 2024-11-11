#define _CRT_SECURE_NO_WARNINGS
#include <iomanip>
#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    512
#define CHUNKSIZE 1024

int global_client_num = 0;
CRITICAL_SECTION global_cs;

#include <windows.h>

void moveCursorVertically(int lines) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	// ���� Ŀ�� ��ġ ��������
	if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
		COORD newPosition;
		newPosition.X = 0;
		newPosition.Y = csbi.dwCursorPosition.Y + lines; // ����� �Ʒ���, ������ ���� �̵�

		// �� ��ġ�� �ܼ� ������ ����� �ʵ��� Ȯ��
		if (newPosition.Y < 0) newPosition.Y = 0;
		if (newPosition.Y >= csbi.dwSize.Y) newPosition.Y = csbi.dwSize.Y - 1;

		SetConsoleCursorPosition(hConsole, newPosition);
	}
}

void displayProgress(const char* addr, long long received_bytes, long long total_bytes, int client_num) 
{
	EnterCriticalSection(&global_cs);

	// Ŭ���̾�Ʈ ��ȣ�� �ش��ϴ� �ٷ� �̵�
	moveCursorVertically(client_num);

	double percentage = (double)received_bytes / total_bytes * 100;
	std::cout << "\r[Client "<< client_num <<" | "<< addr << "] ���� ����� : " << std::fixed << std::setprecision(2) << percentage << " % ";
	std::cout << "(" << received_bytes << " / " << total_bytes << " bytes)" << std::flush;

	moveCursorVertically(-client_num);

	LeaveCriticalSection(&global_cs);
}

DWORD WINAPI ProcessClient(LPVOID arg)
{
	int client_num;

	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[BUFSIZE + 1];
	int len;

	long long file_size;
	char file_name[BUFSIZE + 1];

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	client_num = global_client_num++;

	while (1) {
		// ���� �̸� ���� �ޱ�(���� ����)
		retval = recv(client_sock, (char*)&len, sizeof(int), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv() - get file name len");
		}

		// ���� �̸� ������ �ޱ�(���� ����)
		retval = recv(client_sock, file_name, len, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv() - get file name data");
		}
		file_name[retval] = '\0';
		// printf("[TCP/%s:%d File Name] %s\n", addr, ntohs(clientaddr.sin_port), file_name);

		// ���� ũ�� �ޱ�(���� ����)
		retval = recv(client_sock, (char*)&file_size, sizeof(long long), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv() - get file size");
		}
		// printf("[TCP/%s:%d File Size] %ld\n", addr, ntohs(clientaddr.sin_port), file_size);

		char* fn;
		fn = strrchr(file_name, '\\');
		if (fn != NULL) {
			fn++; // '\'�� �ǳʶٰ� ���� ���� �̸��� �������� �̵�
		}
		else {
			fn = file_name; // '\'�� ���� ��� ��ü ���ڿ��� ���� �̸����� ���
		}

		FILE* fp = fopen(fn, "wb");
		if (fp == NULL) {
			err_quit("fopen() - open file");
		}

		// ���� ������ ����� ����
		long totalReceived = 0;
		int header[3];
		char buf[CHUNKSIZE];

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
			displayProgress(addr ,totalReceived, file_size, client_num);

			//// ����� ���
			//int progress = (int)((totalReceived * 100) / file_size);

			//// Ŭ���̾�Ʈ���� ����� ����
			//retval = send(client_sock, (char*)&progress, sizeof(int), 0);
			//if (retval == SOCKET_ERROR) {
			//	err_display("send() - progress");
			//	break;
			//}

			// ��� ûũ�� �޾Ҵ��� Ȯ��
			if (chunkNumber == totalChunks - 1) {
				break;
			}
		}
		fclose(fp);
	}

	// ���� �ݱ�
	closesocket(client_sock);
	// printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d, ���� �������� Ŭ���̾�Ʈ ��=\n", addr, ntohs(clientaddr.sin_port), client_num);
	return 0;
}

int main(int argc, char* argv[])
{
	int retval;

	//Critical section �ʱ�ȭ
	InitializeCriticalSection(&global_cs);

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

	HANDLE hThread;


	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// ������ ����
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);
		if (hThread == NULL) closesocket(client_sock);
		else CloseHandle(hThread);
	}

	// ���� �ݱ�
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}