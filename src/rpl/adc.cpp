/* 
This code is a translation of the adc.py 
code in OpenRadar's AD-testing 
*/ 


#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

enum class CMD : uint16_t
{
    RESET_FPGA_CMD_CODE = 0x0100,
    RESET_AR_DEV_CMD_CODE = 0x0200,
    CONFIG_FPGA_GEN_CMD_CODE = 0x0300,
    CONFIG_EEPROM_CMD_CODE = 0x0400,
    RECORD_START_CMD_CODE = 0x0500,
    RECORD_STOP_CMD_CODE = 0x0600,
    PLAYBACK_START_CMD_CODE = 0x0700,
    PLAYBACK_STOP_CMD_CODE = 0x0800,
    SYSTEM_CONNECT_CMD_CODE = 0x0900,
    SYSTEM_ERROR_CMD_CODE = 0x0a00,
    CONFIG_PACKET_DATA_CMD_CODE = 0x0b00,
    CONFIG_DATA_MODE_AR_DEV_CMD_CODE = 0x0c00,
    INIT_FPGA_PLAYBACK_CMD_CODE = 0x0d00,
    READ_FPGA_VERSION_CMD_CODE = 0x0e00
};

struct ADCParams
{
    uint16_t chirps = 64;
    uint16_t rx = 4;
    uint16_t tx = 3;
    uint16_t samples = 512;
    uint16_t IQ = 2;
    uint16_t bytes = 2;
};

// STATIC
const uint16_t MAX_PACKET_SIZE = 4096;
const uint16_t BYTES_IN_PACKET = 1466 - 10;
// DYNAMIC
const uint32_t BYTES_IN_FRAME = (ADCParams().chirps * ADCParams().rx * ADCParams().tx *
                                 ADCParams().IQ * ADCParams().samples * ADCParams().bytes);
const uint32_t BYTES_IN_FRAME_CLIPPED = (BYTES_IN_FRAME / BYTES_IN_PACKET) * BYTES_IN_PACKET;
const uint32_t PACKETS_IN_FRAME = BYTES_IN_FRAME / BYTES_IN_PACKET;
const uint32_t PACKETS_IN_FRAME_CLIPPED = BYTES_IN_FRAME / BYTES_IN_PACKET;
const uint16_t UINT16_IN_PACKET = BYTES_IN_PACKET / 2;
const uint16_t UINT16_IN_FRAME = BYTES_IN_FRAME / 2;


class DCA1000 { 
    public: 
        DCA1000(std::string static_ip="192.168.33.30", std::string adc_ip="192.168.33.180", int data_port=4098, int config_port=4096){

            
    }
    void configure(){

    }

    void close(){
        this->data_socket.close(); 
        this->config_socket.close();
    }

    void read(){

    }

}




int main()
{
    // Example usage
    std::string config_header = "5aa5";
    std::string config_status = "0000";
    std::string config_footer = "aaee";
    ADCParams adc_params;
    
    std::cout << "BYTES_IN_FRAME: " << BYTES_IN_FRAME << std::endl;
    std::cout << "BYTES_IN_FRAME_CLIPPED: " << BYTES_IN_FRAME_CLIPPED << std::endl;
    std::cout << "PACKETS_IN_FRAME: " << PACKETS_IN_FRAME << std::endl;
    std::cout << "PACKETS_IN_FRAME_CLIPPED: " << PACKETS_IN_FRAME_CLIPPED << std::endl;
    std::cout << "UINT16_IN_PACKET: " << UINT16_IN_PACKET << std::endl;
    std::cout << "UINT16_IN_FRAME: " << UINT16_IN_FRAME << std::endl;
    
    return 0;
}