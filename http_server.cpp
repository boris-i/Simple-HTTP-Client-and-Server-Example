#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <memory>

#pragma comment(lib,"ws2_32.lib")

static bool _verbose = true;

int main() 
{

    WSADATA wsa;

    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("ERROR: Failed to initialize Winsock. Error Code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    UINT_PTR server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0)
        printf("ERROR: Can't open socket\n");

    struct sockaddr_in server = { 0 };
    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    server.sin_addr.s_addr = htons(INADDR_ANY);

    if(bind(server_fd, (struct sockaddr*)&server, sizeof(server)) == -1)
    {
        closesocket(server_fd);
        printf("ERROR: Can't bind\n");
    }

    struct sockaddr_in cli_addr = { 0 };
    int sin_len = sizeof(cli_addr);

    listen(server_fd, 1);

    const int recvbuf_maxlen = 65536;
    std::unique_ptr<char[]> recv_buf = std::make_unique<char[]>(recvbuf_maxlen);

    const int response_maxlen = 65536;
    std::unique_ptr<char[]> response = std::make_unique<char[]>(response_maxlen);

    const int rstring_maxlen = 65536;
    std::unique_ptr<char[]> rstring = std::make_unique<char[]>(rstring_maxlen);

    const int req_maxlen = 256;
    std::unique_ptr<char[]> req = std::make_unique<char[]>(req_maxlen);

    for(;;) 
    {
        UINT_PTR client_fd = accept(server_fd, (struct sockaddr*)&cli_addr, &sin_len);
        if(_verbose) printf("got connection\n");

        if(client_fd == -1)
        {
            printf("ERROR: Can't accept\n");
            continue;
        }

        // receive the request
        int rcvd = recv(client_fd, recv_buf.get(), 65535, 0);
        if(rcvd)
        {
            recv_buf[rcvd] = 0;
            if(_verbose) printf("RECEIVED: <%s>\n", recv_buf.get());

            // check if this was this a get command
            if(rcvd > 5 && _strnicmp(recv_buf.get(), "GET ", 4) == 0)
            {
                // extract what was requested
                int i, loc;
                for(loc = 4, i = 0; (recv_buf[loc] != 0) && (i < req_maxlen); loc++, i++)
                {
                    if(recv_buf[loc] == ' ')
                        break;
                    
                    req[i] = recv_buf[loc];
                }

                // end the request string
                req[i] = 0;

                if(i == req_maxlen)
                {
                    printf("ERROR: Invalid GET, too long - will treat as '/'.\n");
                    recv_buf[0] = '/';
                    recv_buf[1] = 0;
                }
                else
                {
                    if(_verbose) printf("GET <%s>\n", req.get());
                }

                // we can response diffently to different requests
                if(_strnicmp(req.get(), "/store", 6) == 0)
                {
                    sprintf_s(rstring.get(), rstring_maxlen, "Stored <%s>!", req.get());
                }
                else // default 
                {
                    strcpy_s(rstring.get(), rstring_maxlen, "Hello HTTP!");
                }

                sprintf_s(response.get(), response_maxlen, "HTTP/1.1 200 OK\r\nContent-Length: %i\r\nConnection: close\r\n\r\n%s", (int)strlen(rstring.get()), rstring.get());
                int response_len = (int)strlen(response.get());
                for(int sent = 0; sent < response_len; sent += send(client_fd, response.get() + sent, response_len - sent, 0));
                if(_verbose) printf("Sent response: <%s>\n", response.get());
            }

            closesocket(client_fd);
        }
        else if(rcvd < 0)
        {
            printf("ERROR: Could not recv\n");
        }
    }

    WSACleanup();

    return 0;
}
