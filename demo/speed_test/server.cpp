#include <stdio.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#include SERVER_PARAMS
#include CLIENT_PARAMS
#define SA struct sockaddr
   
// Function designed for chat between client and server.
void func(int connfd)
{
    uint8_t buff[CUBE];
    uint8_t temp_buff[CUBE];

    bzero(buff, sizeof(buff));
    bzero(temp_buff, sizeof(buff));

    clock_t start = clock();
    clock_t end;

    float mbps;
    float timer;
    int status;
    
    for (;;) {
        // Reads from the TCP connection
        status = recv(connfd, temp_buff, sizeof(buff), 0);

        // Checks to see if the packet is new
        if(memcmp(temp_buff, buff, sizeof(buff)) != 0)
        {
            // End timer
            end = clock();

            memcpy(buff, temp_buff, sizeof(buff));
            timer = (float)(end - start) / CLOCKS_PER_SEC;
            mbps = (float)sizeof(buff) * 8 / 1048576 / (float)timer;
            printf("From client: %.1f KB \t %.6fs \t %.3f Mbps\n", (float)status / 1024, timer, mbps);
            
            //Start new timer
            start = clock();
        }
    }
}
   
// Driver function
int main()
{
    int sockfd, connfd;
    socklen_t len;
    struct sockaddr_in servaddr, cli;
   
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(TCP_PORT);
   
    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");
   
    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);
   
    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");
   
    // Function for chatting between client and server
    func(connfd);
   
    // After chatting close the socket
    close(sockfd);
}
