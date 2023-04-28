#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024



void read_udp_packets();

int main()
{
    read_udp_packets();
}

void read_udp_packets() {
    int sockfd, n;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // Bind the socket to any available IP address and a specific port
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(4098);
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    // Receive UDP packets and save the data to a buffer
    while (true) {
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0'; // Null-terminate the buffer
        // Get IQ data from buffer
        int buffer_len = sizeof(buffer);

        while(buffer_len >= 4)
        {
            int16_t i_data = (buffer[1] << 8) | buffer[0];
            int16_t q_data = (buffer[3] << 8) | buffer[2];
            std::cout << "I: " << i_data << ", Q: " << q_data << std::endl;
            buffer_len -= 4;
            memmove(buffer, buffer + 4, buffer_len);
        }

        std::cout << "Received packet from " << inet_ntoa(cliaddr.sin_addr) << ":" << ntohs(cliaddr.sin_port) << std::endl;
        std::cout << "Data: " << buffer << std::endl;
        // Do something with the data, e.g., save it to a file or process it in some way
    }

    // Close the socket
    close(sockfd);
}
