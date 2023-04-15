#include "rpl/private-header.hpp"

int main()
{
    int sockfd, connfd;

    // host the server
    tie(sockfd, connfd) = host();
   
    // Function for chatting between client and server
    calc_speed(connfd);
   
    // After chatting close the socket
    close(sockfd);
}
