#include <stdio.h>
#include <winsock2.h> 
#include <ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib")

int main()
{
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server = { 0 } ;

	const int server_reply_maxlen = 1024;
	char server_reply[server_reply_maxlen];
	int recv_size;

	if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0) 
	{
		printf("ERROR: Initialising Winsock failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	if((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("ERROR: Could not create socket : %d", WSAGetLastError());
	}

	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr.s_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(80);

	// connect to above server
	if(connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		printf("ERROR: connect error");
		return 1;
	}

	// send some data - in this case bogus store command
	const char *message = "GET /store&param100=3.14159265359 HTTP/1.1\r\n\r\n";
	if(send(s, message, strlen(message), 0) < 0)
	{
		printf("ERROR: Send failed");
		return 1;
	}

	if((recv_size = recv(s, server_reply, server_reply_maxlen, 0)) == SOCKET_ERROR)
	{
		printf("ERROR: Recv failed");
	}

	server_reply[recv_size] = '\0';
	printf("Reply to request was:\n-----\n%s\n-----\n", server_reply);

	WSACleanup();

	return 0;
}
