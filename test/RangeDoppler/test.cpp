// g++ -std=c++11 -Wall -Wextra -pedantic -lfftw3f -lm -I../../src/ -o test test.cpp `pkg-config --cflags --libs opencv4`; ./test 
#include "../src/rpl/private-header.hpp"
#define INPUT_SIZE 64 * 512
#define OUTPUT_SIZE 0

#define THRESHOLD 19.0
#define GUARD_LEN 4
#define NOISE_LEN 8


int cfar(float* rdmap, char* cfarmap) {
    // allocate new array for range CFAR
    float* cfar_range = reinterpret_cast<float*>(calloc(SIZE, sizeof(float)));
    // allocate new array for doppler CFAR
    float* cfar_doppler = reinterpret_cast<float*>(calloc(SIZE, sizeof(float)));

    // IDEA, instead of normalizing (making the areaa of the kernel =
    // 10 by scaling down every single convolution
    const float normalize=1.0/(2*NOISE_LEN);
    int start_left, end_left, start_right, end_right;
    int row_start;
    // perform the range convolution
    for (int j=0; j<DOPPLER_BINS*NUM_RX*NUM_TX; j++) {
	for (int i=0; i<RANGE_BINS; i++) {
	    // left-hand-side convolution
	    row_start=j*RANGE_BINS;
	    start_left=row_start+std::max(0, i-GUARD_LEN-NOISE_LEN);
	    end_left=row_start+std::max(0, i-GUARD_LEN);
	    for (int z=start_left; z<end_left; z++) {
		cfar_range[row_start+i]=rdmap[row_start+z]*normalize;
	    }
	    // right-hand-side convolution
	    start_right=row_start+std::min(RANGE_BINS, i+GUARD_LEN+1);
	    end_right=row_start+std::min(RANGE_BINS, i+GUARD_LEN+NOISE_LEN+1);
	    for (int z=start_right; z<end_right; z++) {
		cfar_range[row_start+i]=rdmap[row_start+z]*normalize;
	    }
	    
	}
    }

    // perform the doppler convolution (harder since indexes are noncontiguous)
    // maybe we only need a range convolution??

    // TO BE IMPLEMENTED
    
    // compare to threshold to create bitmap
    for (int i=0; i<SIZE; i++) {
	cfarmap[i]=cfar_range[i]>THRESHOLD && cfar_doppler[i]>THRESHOLD;
    }

    return 0;
}




int main()
{   
    // const int FAST_TIME = 512;
    // const int SLOW_TIME = 64;
    // const int RX = 4;
    // const int TX = 3;
    // const int IQ = 2;
    
    
    // Create the Range Doppler Map Object
    RangeDoppler rdm(FAST_TIME,SLOW_TIME,RX,TX,IQ,"blackman");

    // Initialize the Visualizer Class
    Visualizer rdmplot(INPUT_SIZE,OUTPUT_SIZE); 
    
    // Activate the process block which reads from a text data file and computes the range doppler map.
    rdm.process("../data/adc_data/out_DAQ.txt");
    
    for(int i=0; i<500; i++) {
      rdm.process("../data/adc_data/out_DAQ.txt");
    }
    // Receive the pointer to the range doppler map.
    float* in_bufferptr = rdm.getBufferPointer();
    // Set the input pointer from the range doppler object to the visualizer object
    rdmplot.setBufferPointer(in_bufferptr);

    // Plot the range doppler map.
    // rdmplot.process();

    std::cout << "Test Complete!" << std::endl;

    return 0;
}
