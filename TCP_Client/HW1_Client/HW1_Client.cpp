#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512

// 2017�� 1�б� ��Ʈ��ũ���α׷��� ���� 1�� Ŭ���̾�Ʈ
// ����: �̻��� �й�: 13012667
// �۵��ϴ� ������ ����: www.netflix.com / www.naver.com / www.fb.com

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while(left > 0){
		received = recv(s, ptr, left, flags);
		if(received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if(received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("connect()");

	// ������ ��ſ� ����� ����
	char buf[BUFSIZE];
	int len;

	// ������ ������ ���
	while(1){
		// ������ �Է�
		printf("\n[Domain Name]: ");
		if(fgets(buf, BUFSIZE+1, stdin) == NULL)
			break;

		// '\n' ���� ����
		len = strlen(buf);
		if(buf[len-1] == '\n')
			buf[len-1] = '\0';
		if(strlen(buf) == 0)
			break;
		
		retval = send(sock, (char *)&len, sizeof(int), 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
			return 0;
		}

		// ������ ������(���� ����)
		retval = send(sock, buf, len, 0);
		if(retval == SOCKET_ERROR){
			err_display("send()");
			return 0;
		}

		while(1){
			//������ �ޱ�(���� ����)
			retval = recvn(sock, (char *)&len, sizeof(int), 0);
			if(retval == SOCKET_ERROR){
				err_display("recv()");
				break;
			}
			if(retval == 0){
				break;
			}
		
			//������ �ޱ�(���� ����)
			retval = recv(sock, buf, len, 0);
			if(retval == SOCKET_ERROR){
				err_display("recv()");
				break;
			}
			if(retval == 0)
				break;

			//���� ���� ��.
			buf[retval] = '\0';
			if(strcmp(buf,"End") == 0)
				break;
			else if(strcmp(buf,"close") == 0){

				printf("Ŭ���̾�Ʈ�� �����մϴ�.\n");

				// closesocket()
				closesocket(sock);

				// ���� ����
				WSACleanup();

				return 0;
			}
			else{
				// ���� ������ ���
				printf("%s\n", buf);
			}
		}	
	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();

	return 0;
}