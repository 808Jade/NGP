#include "Common.h"
#include "resource.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE 1024

long long getFileSize(const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (file == NULL) return -1;

	fseek(file, 0, SEEK_END);
	long long size = ftell(file);
	fclose(file);

	return size;
}

INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

DWORD WINAPI ClientMain(LPVOID arg);

HWND hFileEdit;		// IDC_MFCEDITBROWSE1
HWND hProgressBar;  // IDC_PROGRESS2
HWND hSendButton;   // IDC_BUTTON1

HANDLE hSendEvent, hRcevEvent; // �̺�Ʈ

char file_name[MAX_PATH];
char szFile[MAX_PATH] = "";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// GUI ����
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	// ���� ����
	WSACleanup();
	return 0;
}

// ��ȭ���� ���ν���
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		hFileEdit = GetDlgItem(hDlg, IDC_EDIT1);
		hSendButton = GetDlgItem(hDlg, IDC_BUTTON1);
		hProgressBar = GetDlgItem(hDlg, IDC_PROGRESS1);
		SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON2: // ��ư : ���� ����
			OPENFILENAMEA  ofn;

			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "��� ����(*.*)\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			if (GetOpenFileNameA(&ofn)) {
				SetWindowTextA(hFileEdit, szFile);
				strcpy_s(file_name, MAX_PATH, szFile);
				EnableWindow(hSendButton, TRUE);
			}
			return TRUE;

		case IDC_BUTTON1: // ��ư : ����
			EnableWindow(hSendButton, FALSE);
			CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL); // ��ȭ���� �ݱ�
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

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
	std::cout << "before send file_size : " << file_size << std::endl;

	retval = send(sock, (char*)&file_size, sizeof(long long), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() - file size");
	}

	// ���� ������ �����ϱ�
	std::ifstream file(file_name, std::ios::binary);
	std::vector<char> buffer(BUFSIZE);

	int totalChunks = (file_size + BUFSIZE - 1) / BUFSIZE;
	int chunkNumber = 0;
	long long totalBytesSent = 0;

	while (!file.eof()) {
		file.read(buffer.data(), BUFSIZE);
		int bytesRead = file.gcount();

		// ���� ���� ��� ����
		int header[3] = { chunkNumber, bytesRead, totalChunks };
		send(sock, (char*)header, sizeof(header), 0);

		// ���� ���� ������ ����
		send(sock, buffer.data(), bytesRead, 0);

		//// ����� ��� �� ProgressBar ������Ʈ
		//totalBytesSent += bytesRead;
		//int progressPercentage = (int)((totalBytesSent * 100) / file_size);
		//SendMessage(hProgressBar, PBM_SETPOS, progressPercentage, 0);

		//chunkNumber++;

		//Sleep(10);

		// �����κ��� ����� ����
		int progress;
		retval = recv(sock, (char*)&progress, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv() - progressPercentage");
			break;
		}

		// ProgressBar ������Ʈ
		SendMessage(hProgressBar, PBM_SETPOS, progress, 0);
	}

	// ���� �Ϸ� �� ProgressBar�� 100%�� ����
	// SendMessage(hProgressBar, PBM_SETPOS, 100, 0);

	return 0;
}