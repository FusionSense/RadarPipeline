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
    
    // Create the Range Doppler Map Object
    RangeDoppler rdm(fast_time,slow_time,rx,tx,iq,"blackman");
    
    // Activate the process block which reads from a text data file and computes the range doppler map.
    rdm.process("../data/adc_data/out_DAQ.txt"); 

    // Receive the pointer to the range doppler map.
    float* in_bufferptr = rdm.getBufferPointer();
    
    // Initialize the Visualizer Class
    Visualizer rdmplot(INPUT_SIZE,OUTPUT_SIZE); 
    
    // Set the input pointer from the range doppler object to the visualizer object
    rdmplot.setBufferPointer(in_bufferptr);

    // Plot the range doppler map.
    rdmplot.process();

    
    std::cout << "Test Complete!" << std::endl;

    return 0;
}
