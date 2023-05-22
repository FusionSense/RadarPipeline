// g++ -std=c++11 -Wall -Wextra -pedantic -lfftw3f -lm -I../../src/ -o test test.cpp `pkg-config --cflags --libs opencv4`; ./test 
#include "../src/rpl/private-header.hpp"
#define INPUT_SIZE 64 * 512
#define OUTPUT_SIZE 0
int main()
{   
    int fast_time = 512;
    int slow_time = 64;
    int rx = 4;
    int tx = 3;
    int iq = 2;
    int buffer_size = 4096; 
    int port = 4098; 
    int bytes_in_packet = 1456; 
    int iq_bytes = 2;
    
    // Create the Range Doppler Map Object
    RangeDoppler rdm(fast_time,slow_time,rx,tx,iq,"blackman");
    // float* in_bufferptr = rdm.getBufferPointer();
    // Receive the pointer to the range doppler map.
    uint16_t* in_bufferptr = new uint16_t[rx*tx*iq*slow_time*fast_time];
    
    DataAcquisition daq(in_bufferptr, buffer_size, port, bytes_in_packet, fast_time, slow_time, rx, tx, iq, iq_bytes);

    Visualizer rdmplot(INPUT_SIZE,OUTPUT_SIZE);

    float* in_visualizeptr = rdm.getVisualizePointer();

    rdmplot.setBufferPointer(in_visualizeptr);
    daq.create_bind_socket();
    for(int i = 0; i<20 ; i++){
        std::cout<<"LOOP = " << i << "                                      OTHER : " << in_visualizeptr[100] <<  std::endl;
        daq.process();
        rdm.process(nullptr, in_bufferptr); 
        rdmplot.process(180);
        
        // cv::destroyAllWindows();
    }
    daq.close_socket();
    
    
    std::cout << "Test Complete!" << std::endl;

    return 0;
}
