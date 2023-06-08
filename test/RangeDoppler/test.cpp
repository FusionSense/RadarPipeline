// g++ -std=c++11 -Wall -Wextra -pedantic -lfftw3f -lm -I../../src/ -o test test.cpp `pkg-config --cflags --libs opencv4`; ./test 
#include "../src/rpl/private-header.hpp"
#define INPUT_SIZE 64 * 512
#define OUTPUT_SIZE 0
int main(int argc, char* argv[])
{   
    // Create the Range Doppler Map Object
    RangeDoppler rdm("blackman");
    
    if (argc > 1){
        if(argc != 3){
            std::cout << "Incorrect number of arguments, format should be : \n    --> ./test max_SNR_THRESHOLD min_SNR_THRESHOLD \n OR --> ./test" << std::endl;
            return 1;
        }
        
        float max = std::stof(argv[1]);
        float min = std::stof(argv[2]);
        rdm.setSNR(max,min);
    }

    rdm.readFile("../data/adc_data/out_DAQ.txt");

    // Activate the process block which reads from a text data file and computes the range doppler map.
    rdm.process(); 
    
    // Receive the pointer to the range doppler map.
    float* in_bufferptr = rdm.getBufferPointer();

    // Initialize the Visualizer Class
    Visualizer rdmplot(INPUT_SIZE,OUTPUT_SIZE); 
    
    // Set the input pointer from the range doppler object to the visualizer object
    rdmplot.setBufferPointer(in_bufferptr);
    rdmplot.setWaitTime(0);

    // Plot the range doppler map.
    rdmplot.process();
    
    std::cout << "Test Complete!" << std::endl;

    return 0;
}
