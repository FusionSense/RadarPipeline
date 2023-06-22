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
    
    // CONSTRUCTOR INITIATION
    DataAcquisition daq(buffer_size, port, bytes_in_packet, fast_time, slow_time, rx, tx, iq, iq_bytes);

    RangeDoppler rdm(fast_time,slow_time,rx,tx,iq,"blackman");

    Visualizer vis(INPUT_SIZE,OUTPUT_SIZE);

    // BUFFER POINTER INITIATION
    uint16_t *in_bufferptr    = daq.getBufferPointer();
    float    *in_visualizeptr = rdm.getBufferPointer();

    rdm.setBufferPointer(in_bufferptr);
    vis.setBufferPointer(in_visualizeptr);

    // FRAME POINTER INITIATION
    auto frame_daq = daq.getFramePointer();
    rdm.setFramePointer(frame_daq);
    auto frame_rdm = rdm.getFramePointer();
    daq.setFramePointer(frame_rdm);
    // OTHER PARAMS
    // daq.create_bind_socket(); // open the socket for listening
    vis.setWaitTime(1);   

    std::cout << "FRAME --> DAQ: " << *frame_daq << std::endl;
    std::cout << "FRAME --> RDM: "<< *frame_rdm << std::endl;
    // std::cout << "FRAME --> VIS: "<< *frame_vis << std::endl;

    thread daqThread(&DataAcquisition::iteration, &daq);
    thread rdmThread(&RangeDoppler::iteration, &rdm);
    thread visThread(&Visualizer::iteration, &vis);

    // daq.close_socket();

    std::cout << "Test Complete!" << std::endl;

    daqThread.join();
    rdmThread.join();
    visThread.join();

    return 0;
}
