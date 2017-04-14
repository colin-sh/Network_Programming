#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512

// 2017�� 1�б� ��Ʈ��ũ���α׷��� ���� 1�� ����
// ����: �̻��� �й�: 13012667

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
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE+1];
	HOSTENT *ptr;
	int len;

	while(1){
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET){
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// Ŭ���̾�Ʈ�� ������ ���
		while(1){
			//���� ������ �����͸� �޴´�.(������������ ���̸� �����Ѵ�.)
			retval = recvn(client_sock, (char *)&len, sizeof(int), 0);
			if(retval == SOCKET_ERROR){
				err_display("recv()");
				break;
			}
			else if(retval == 0)
				break;

			// ���� �����͸� �޴´�.
			retval = recv(client_sock, buf,len, 0);
			if(retval == SOCKET_ERROR){
				err_display("recv()");
				break;
			}
			else if(retval == 0)
				break;

			// ���� ������ ���
			buf[retval] = '\0';
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
				ntohs(clientaddr.sin_port), buf);

			if(strcmp(buf,"exit") == 0 || strcmp(buf,"quit") == 0){
				//close�� buf�� ä���
				char *title = "close";
				len = strlen(title);
				strncpy(buf, title, len);
				buf[len] = '\0';

				//���� ���� ����
				retval = send(client_sock, (char *)&len, sizeof(int), 0);
				if(retval == SOCKET_ERROR){
					err_display("send()");
					return 0;
				}

				//���� ���� ����
				retval = send(client_sock, buf, len, 0);
				if(retval == SOCKET_ERROR){
					err_display("send()");
					return 0;
				}
				printf("������ �����մϴ�.\n");
				// closesocket()
				closesocket(listen_sock);

				// ���� ����
				WSACleanup();
				return 0;
			}
			else{
				//hostent ����ü ȹ��
				ptr = gethostbyname(buf);
				if(ptr == NULL){
					char *title = "[gethostbyname()] �˷��� ȣ��Ʈ�� �����ϴ�.";
					len = strlen(title);
					strncpy(buf, title, len);
					buf[len] = '\0';

					//���� ���� ����
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//���� ���� ����
					retval = send(client_sock, buf, len, 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//End�� buf�� ä���
					title = "End";
					len = strlen(title);
					strncpy(buf, title, len);
					buf[len] = '\0';

					//���� ���� ����
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//���� ���� ����
					retval = send(client_sock, buf, len, 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}
				}
				else{
					//Official Name title�� buf�� ä���
					char *title = "[Official Name]";
					len = strlen(title);
					strncpy(buf, title, len);
					buf[len] = '\0';

					//Official Name title ������
					//���� ���� ����
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//���� ���� ����
					retval = send(client_sock, buf, len, 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//Official Name�� buf�� ä���
					len = strlen(ptr->h_name);
					strncpy(buf, ptr->h_name, len);
					buf[len] = '\0';

					//Official Name ������
					//���� ���� ����
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//���� ���� ����
					retval = send(client_sock, buf, len, 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//Aliases�� buf�� ä���
					title = "[Aliases]";
					len = strlen(title);
					strncpy(buf, title, len);
					buf[len] = '\0';

					//Aliases title ������
					//���� ���� ����
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//���� ���� ����
					retval = send(client_sock, buf, len, 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//aliases ȹ��
					char **aliasesPtr = ptr->h_aliases;
					while(*aliasesPtr){
						len = strlen(*aliasesPtr);
						strncpy(buf, *aliasesPtr, len);
						buf[len] = '\0';

						//������ ������(���� ����)
						retval = send(client_sock, (char *)&len, sizeof(int), 0);
						if(retval == SOCKET_ERROR){
							err_display("send()");
							break;
						}

						//������ ������(���� ����)
						retval = send(client_sock, buf, len, 0);
						if(retval == SOCKET_ERROR){
							err_display("send()");
							break;
						}
						++aliasesPtr;
					}

					//IP addresses�� buf�� ä���
					title = "[IP addresses]";
					len = strlen(title);
					strncpy(buf, title, len);
					buf[len] = '\0';

					//IP addresses title ������
					//���� ���� ����
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//���� ���� ����
					retval = send(client_sock, buf, len, 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//IP Adressses ȹ��
					IN_ADDR addr;
					char *addrStr;
					char **ipPtr = ptr->h_addr_list;
					while(*ipPtr){
						memcpy(&addr, *ipPtr, ptr->h_length);
						addrStr = inet_ntoa(addr);

						len = strlen(addrStr);
						strncpy(buf, addrStr, len);
						buf[len] = '\0';

						//������ ������(���� ����)
						retval = send(client_sock, (char *)&len, sizeof(int), 0);
						if(retval == SOCKET_ERROR){
							err_display("send()");
							break;
						}

						//������ ������(���� ����)
						retval = send(client_sock, buf, len, 0);
						if(retval == SOCKET_ERROR){
							err_display("send()");
							break;
						}
						++ipPtr;
					}

					//End�� buf�� ä���
					title = "End";
					len = strlen(title);
					strncpy(buf, title, len);
					buf[len] = '\0';

					//���� ���� ����
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//���� ���� ����
					retval = send(client_sock, buf, len, 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}
				}
			}
		}

		// closesocket()
		closesocket(client_sock);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}