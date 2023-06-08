// g++ -std=c++11 -Wall -Wextra -pedantic -lfftw3f -lm -I../../src/ -o test test.cpp `pkg-config --cflags --libs opencv4`; ./test 
#include "../src/rpl/private-header.hpp"
#define INPUT_SIZE 64 * 512
#define OUTPUT_SIZE 0
int main(int argc, char* argv[])
{   

    // CONSTRUCTOR INITIATION
    DataAcquisition daq;

    RangeDoppler rdm("blackman");

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
    if (argc > 1){
        if(argc != 3){
            std::cout << "Incorrect number of arguments, format should be : \n    --> ./test max_SNR_THRESHOLD min_SNR_THRESHOLD \n OR --> ./test" << std::endl;
            return 1;
        }
        float max = std::stof(argv[1]);
        float min = std::stof(argv[2]);
        rdm.setSNR(max,min);
    }
    vis.setWaitTime(1);   

    rdm.process();
    
    while(true){
        daq.process();
        rdm.process();
        vis.process();
    }

    return 0;
}
