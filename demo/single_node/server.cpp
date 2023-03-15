#include <stdio.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#include <opencv2/opencv.hpp>
#include "../network_params.h"
#define PORT 8080
#define SA struct sockaddr

using namespace std;
using namespace cv;

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
    
    for (;;) {
        // Reads from the TCP connection
        recv(connfd, temp_buff, sizeof(buff), 0);

        // Checks to see if the packet is new
        if(memcmp(temp_buff, buff, sizeof(buff)) != 0)
        {
            // End timer
            end = clock();

            memcpy(buff, temp_buff, sizeof(buff));
            mbps = sizeof(buff) * 8 / 1000000 / (float)(end - start) * CLOCKS_PER_SEC;
            printf("From client: %.1f kB - %.6fs - %.3f Mbps\n", (float)sizeof(buff) / 1000, time, mbps);
            
            //Start new timer
            start = clock();
        }
    }
}
   
// Driver function
int main()
{
    Mat original_image = imread("../image.png");
    
    Mat image;
    
    for(int i=0; i<1000; i++) {
        
        original_image.copyTo(image);
        
        for(int x=10+i; x<300+i; x++)
            for(int y=20+i; y<200+i; y++)
            image.at<Vec3b>(y, x) = {0, 255, 0};
        
        imshow("bruh", image);
        waitKey(16);
    }
    
    // int sockfd, connfd, len;
    // struct sockaddr_in servaddr, cli;
   
    // // socket create and verification
    // sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sockfd == -1) {
    //     printf("socket creation failed...\n");
    //     exit(0);
    // }
    // else
    //     printf("Socket successfully created..\n");
    // bzero(&servaddr, sizeof(servaddr));
   
    // // assign IP, PORT
    // servaddr.sin_family = AF_INET;
    // servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // servaddr.sin_port = htons(PORT);
   
    // // Binding newly created socket to given IP and verification
    // if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
    //     printf("socket bind failed...\n");
    //     exit(0);
    // }
    // else
    //     printf("Socket successfully binded..\n");
   
    // // Now server is ready to listen and verification
    // if ((listen(sockfd, 5)) != 0) {
    //     printf("Listen failed...\n");
    //     exit(0);
    // }
    // else
    //     printf("Server listening..\n");
    // len = sizeof(cli);
   
    // // Accept the data packet from client and verification
    // connfd = accept(sockfd, (SA*)&cli, &len);
    // if (connfd < 0) {
    //     printf("server accept failed...\n");
    //     exit(0);
    // }
    // else
    //     printf("server accept the client...\n");
   
    // // Function for chatting between client and server
    // func(connfd);
   
    // // After chatting close the socket
    // close(sockfd);
}
