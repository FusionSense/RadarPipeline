#include "rpl/private-header.hpp"
 
int main()
{   
    int sockfd;

    // connect to server
    sockfd = connect();
 
    // function for chat
    send_rand(sockfd);
 
    // close the socket
    close(sockfd);
}
