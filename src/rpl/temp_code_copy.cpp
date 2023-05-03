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


uint16_t* read_udp_packets();

int main()
{
    uint16_t* frame_ptr = read_udp_packets();

    return 0;
}


uint16_t* read_udp_packets() {

    // Variable initialization
    uint64_t BYTES_IN_FRAME = SLOW_TIME*FAST_TIME*NUM_RX*NUM_TX*IQ_DATA*IQ_BYTES;
    uint64_t BYTES_IN_FRAME_CLIPPED = BYTES_IN_FRAME/BYTES_IN_PACKET*BYTES_IN_PACKET;
    //std::cout << "Bytes in frame = " << BYTES_IN_FRAME << std::endl;
    //std::cout << "Bytes in frame clipped = " << BYTES_IN_FRAME_CLIPPED << std::endl;

    //float PACKETS_FRAME = BYTES_IN_FRAME / BYTES_IN_PACKET;
    uint16_t PACKETS_IN_FRAME_CLIPPED = BYTES_IN_FRAME / BYTES_IN_PACKET;
    uint16_t UINT16_IN_PACKET = BYTES_IN_PACKET / 2;
    uint64_t UINT16_IN_FRAME = BYTES_IN_FRAME / 2;

    uint64_t packets_read = 0;

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

    uint16_t frame_data[(PACKETS_IN_FRAME_CLIPPED*UINT16_IN_PACKET)];

    // n is the packet size in bytes (including sequence number and byte count)
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
    buffer[n] = '\0'; // Null-terminate the buffer

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
        uint32_t packet_num = ((buffer[0] & 0xFF) << 0)  |
                                ((buffer[1] & 0xFF) << 8)  |
                                ((buffer[2] & 0xFF) << 16) |
                                ((long) (buffer[3] & 0xFF) << 24);
        // print packet number (packet count from DCA data) -> misses packets
        //std::cout << "Packet Number = " << packet_num << std::endl;

        //unpack byte count
        uint64_t byte_count = ((buffer[4] & 0xFF) << 0)  |
                                ((buffer[5] & 0xFF) << 8)  |
                                ((buffer[6] & 0xFF) << 16) |
                                ((buffer[7] & 0xFF) << 24) |
                                ((unsigned long long) (buffer[8] & 0xFF) << 32) |
                                ((unsigned long long) (buffer[9] & 0xFF) << 40) |
                                ((unsigned long long) (0x00) << 48) |
                                ((unsigned long long) (0x00) << 54);

        // print byte count from DCA data
        //std::cout << "Byte Count = " << byte_count << std::endl;
 

        //std::cout << "Bytes since last packet = " << byte_count - byte_count_old << std::endl;
        //byte_count_old = byte_count;

        uint16_t packet_data[(sizeof(buffer)-10)];
        //Read packet data from buffer
        for (int i = 0; i< sizeof(buffer); i++)
        {
            packet_data[i] =  buffer[2*i+10] | (buffer[2*i+11] << 8);
        }
        packets_read++;
        //std::cout << "First I value = " << packet_data[0] << std::endl;
        //std::cout << "Second I value = " << packet_data[1] << std::endl;

        //Do something with the data, e.g., save it to a file or process it in some way

        // Add packet_data to ret_frame
        // uint16_t start_frame_idx = UINT16_IN_PACKET*(packets_read-1);
        // for (int i = start_frame_idx; i < start*packets_read+sizeof(packet_data); i++)
        // {
        //     frame_data[i] = packet_data[i-1024*packets_read];
        // }
        uint64_t byte_mod = byte_count % BYTES_IN_FRAME_CLIPPED;
        if (byte_mod == 0) //end of frame found
        {
            std::cout << "Modulus Thing = " << byte_mod << std::endl;
            return frame_data;
        }
    }

    // Close the socket
    close(sockfd);
}

