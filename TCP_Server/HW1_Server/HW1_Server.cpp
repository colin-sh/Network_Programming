#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512

// 2017년 1학기 네트워크프로그래밍 숙제 1번 서버
// 성명: 이상헌 학번: 13012667

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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
// 사용자 정의 데이터 수신 함수
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

	// 윈속 초기화
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

	// 데이터 통신에 사용할 변수
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

		// 접속한 클라이언트 정보 출력
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while(1){
			//고정 길이의 데이터를 받는다.(가변데이터의 길이를 저장한다.)
			retval = recvn(client_sock, (char *)&len, sizeof(int), 0);
			if(retval == SOCKET_ERROR){
				err_display("recv()");
				break;
			}
			else if(retval == 0)
				break;

			// 가변 데이터를 받는다.
			retval = recv(client_sock, buf,len, 0);
			if(retval == SOCKET_ERROR){
				err_display("recv()");
				break;
			}
			else if(retval == 0)
				break;

			// 받은 데이터 출력
			buf[retval] = '\0';
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
				ntohs(clientaddr.sin_port), buf);

			if(strcmp(buf,"exit") == 0 || strcmp(buf,"quit") == 0){
				//close를 buf에 채우기
				char *title = "close";
				len = strlen(title);
				strncpy(buf, title, len);
				buf[len] = '\0';

				//고정 길이 전송
				retval = send(client_sock, (char *)&len, sizeof(int), 0);
				if(retval == SOCKET_ERROR){
					err_display("send()");
					return 0;
				}

				//가변 길이 전송
				retval = send(client_sock, buf, len, 0);
				if(retval == SOCKET_ERROR){
					err_display("send()");
					return 0;
				}
				printf("서버를 종료합니다.\n");
				// closesocket()
				closesocket(listen_sock);

				// 윈속 종료
				WSACleanup();
				return 0;
			}
			else{
				//hostent 구조체 획득
				ptr = gethostbyname(buf);
				if(ptr == NULL){
					char *title = "[gethostbyname()] 알려진 호스트가 없습니다.";
					len = strlen(title);
					strncpy(buf, title, len);
					buf[len] = '\0';

					//고정 길이 전송
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//가변 길이 전송
					retval = send(client_sock, buf, len, 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//End를 buf에 채우기
					title = "End";
					len = strlen(title);
					strncpy(buf, title, len);
					buf[len] = '\0';

					//고정 길이 전송
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//가변 길이 전송
					retval = send(client_sock, buf, len, 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}
				}
				else{
					//Official Name title를 buf에 채우기
					char *title = "[Official Name]";
					len = strlen(title);
					strncpy(buf, title, len);
					buf[len] = '\0';

					//Official Name title 보내기
					//고정 길이 전송
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//가변 길이 전송
					retval = send(client_sock, buf, len, 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//Official Name를 buf에 채우기
					len = strlen(ptr->h_name);
					strncpy(buf, ptr->h_name, len);
					buf[len] = '\0';

					//Official Name 보내기
					//고정 길이 전송
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//가변 길이 전송
					retval = send(client_sock, buf, len, 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//Aliases를 buf에 채우기
					title = "[Aliases]";
					len = strlen(title);
					strncpy(buf, title, len);
					buf[len] = '\0';

					//Aliases title 보내기
					//고정 길이 전송
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//가변 길이 전송
					retval = send(client_sock, buf, len, 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//aliases 획득
					char **aliasesPtr = ptr->h_aliases;
					while(*aliasesPtr){
						len = strlen(*aliasesPtr);
						strncpy(buf, *aliasesPtr, len);
						buf[len] = '\0';

						//데이터 보내기(고정 길이)
						retval = send(client_sock, (char *)&len, sizeof(int), 0);
						if(retval == SOCKET_ERROR){
							err_display("send()");
							break;
						}

						//데이터 보내기(가변 길이)
						retval = send(client_sock, buf, len, 0);
						if(retval == SOCKET_ERROR){
							err_display("send()");
							break;
						}
						++aliasesPtr;
					}

					//IP addresses를 buf에 채우기
					title = "[IP addresses]";
					len = strlen(title);
					strncpy(buf, title, len);
					buf[len] = '\0';

					//IP addresses title 보내기
					//고정 길이 전송
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//가변 길이 전송
					retval = send(client_sock, buf, len, 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//IP Adressses 획득
					IN_ADDR addr;
					char *addrStr;
					char **ipPtr = ptr->h_addr_list;
					while(*ipPtr){
						memcpy(&addr, *ipPtr, ptr->h_length);
						addrStr = inet_ntoa(addr);

						len = strlen(addrStr);
						strncpy(buf, addrStr, len);
						buf[len] = '\0';

						//데이터 보내기(고정 길이)
						retval = send(client_sock, (char *)&len, sizeof(int), 0);
						if(retval == SOCKET_ERROR){
							err_display("send()");
							break;
						}

						//데이터 보내기(가변 길이)
						retval = send(client_sock, buf, len, 0);
						if(retval == SOCKET_ERROR){
							err_display("send()");
							break;
						}
						++ipPtr;
					}

					//End를 buf에 채우기
					title = "End";
					len = strlen(title);
					strncpy(buf, title, len);
					buf[len] = '\0';

					//고정 길이 전송
					retval = send(client_sock, (char *)&len, sizeof(int), 0);
					if(retval == SOCKET_ERROR){
						err_display("send()");
						return 0;
					}

					//가변 길이 전송
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
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}