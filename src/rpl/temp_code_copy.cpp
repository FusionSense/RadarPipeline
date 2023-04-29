#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 4096 
#define PORT        4098
#define BYTES_IN_PACKET 1456 // Max packet size - sequence number and byte count = 1466-10 
#define NUM_RX 4
#define NUM_TX 3
#define FAST_TIME 512 //ADC Samples
#define SLOW_TIME 64 //Chirps
#define IQ_DATA 2 //Types of IQ (I and Q)
#define IQ_BYTES 2 


void read_udp_packets();

int main()
{
    read_udp_packets();
}


void read_udp_packets() {

    // Variable initialization
    uint16_t BYTES_IN_FRAME = SLOW_TIME*FAST_TIME*NUM_RX*NUM_TX*IQ_DATA*IQ_BYTES;
    uint16_t BYTES_IN_FRAME_CLIPPED = BYTES_IN_FRAME/BYTES_IN_PACKET*BYTES_IN_PACKET;

    //float PACKETS_FRAME = BYTES_IN_FRAME / BYTES_IN_PACKET;
    uint16_t PACKETS_IN_FRAME_CLIPPED = BYTES_IN_FRAME / BYTES_IN_PACKET;
    uint16_t UINT16_IN_PACKET = BYTES_IN_PACKET / 2;
    uint16_t UINT16_IN_FRAME = BYTES_IN_FRAME / 2;

    // socket file descriptor
    int sockfd; 
    // create buffer array of preset size to hold one packet
    char buffer[BUFFER_SIZE];
    // initialize socket
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    // Create a UDP socket file descriptor which is UNbounded
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) <  0){
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 

    // Filling in the servers (DCA1000EVMs) information
    servaddr.sin_family     = AF_INET;      //this means it is a IPv4 address
    servaddr.sin_addr.s_addr= INADDR_ANY;   //sets address to accept incoming messages
    servaddr.sin_port       = htons(PORT);  //port number to accept from
    
    // Now we bind the socket with the servers (DCA1000EVMs) address 
    // if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) 

    // Bind the socket to any available IP address and a specific port
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    // n is the packet size in bytes (including sequence number and byte count)
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
    buffer[n] = '\0'; // Null-terminate the buffer

    uint64_t max_packet_num = (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer [0];
    uint64_t max_byte_count = (buffer[9] << 8*5) | (buffer[8] << 8*4) | (buffer[7] << 8*3) | (buffer[6] << 8*2) | (buffer[5] << 8) | (buffer[4]);
    //std::cout << "Max Seq:" << max_packet_num << std::endl;
    //std::cout << "Max Bytes:" << max_byte_count << std::endl;
    uint32_t starting_sequence_num = ((buffer[0] & 0xFF) << 0)  |
                                ((buffer[1] & 0xFF) << 8)  |
                                ((buffer[2] & 0xFF) << 16) |
                                ((long) (buffer[3] & 0xFF) << 24);
    //std::cout << "First Sequence Number = " << starting_sequence_num << std::endl;
                            
    // Receive UDP packets and save the data to a buffer
    while (true) {
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);

        buffer[n] = '\0'; // Null-terminate the buffer

        // first four bytes from buffer (the sequence number)
        uint32_t sequence_num = ((buffer[0] & 0xFF) << 0)  |
                                ((buffer[1] & 0xFF) << 8)  |
                                ((buffer[2] & 0xFF) << 16) |
                                ((long) (buffer[3] & 0xFF) << 24);
        // print sequence number (packet count from DCA data) -> misses packets
        std::cout << "Sequence Number = " << sequence_num << std::endl;

        //unpack byte count
        // uint64_t byte_count = ((buffer[4] & 0xFF) << 0)  |
        //                         ((buffer[5] & 0xFF) << 8)  |
        //                         ((buffer[6] & 0xFF) << 16) |
        //                         ((buffer[7] & 0xFF) << 24) |
        //                         ((buffer[8] & 0xFF) << 32) |
        //                         ((buffer[9] & 0xFF) << 40) |
        //                         ((0x00) << 48) |
        //                         ((long) (0x00) << 54);

        // print byte count from DCA data
        std::cout << "Byte Count = " << byte_count << std::endl;



        //std::cout<< frame << std::endl;
        //std::cout << "Sequs:" << pck_num << std::endl;
        //std::cout << "Bytes:" << byt_cnt  << std::endl;
        
        // // Get IQ data from buffer
        // int buffer_len = sizeof(buffer);


        // while(buffer_len >= 4)
        // {
        //     int16_t i_data = (buffer[1] << 8) | buffer[0]; // Assembling the 16 bit I
        //     int16_t q_data = (buffer[3] << 8) | buffer[2]; // Assembling the 16 bit Q
        //     std::cout << "I: " << i_data << ", Q: " << q_data << std::endl;
        //     buffer_len -= 4;
        //     memmove(buffer, buffer + 4, buffer_len);
        // }

        // std::cout << "Received packet from " << inet_ntoa(cliaddr.sin_addr) << ":" << ntohs(cliaddr.sin_port) << std::endl;
        // std::cout << "Data: " << buffer << std::endl;
        // // Do something with the data, e.g., save it to a file or process it in some way
    }

    // Close the socket
    close(sockfd);
}
