#include "rpl/private-header.hpp"

#define INPUT_SIZE 64 * 512
#define OUTPUT_SIZE 0

// Changing the buffer and increasing frame
void updateBuffer(int* bufferptr, uint* frameptr)
{
    for(;;)
    {
        for (int i = 0; i < INPUT_SIZE; i++)
        {
            bufferptr[i] = rand() % 256; // Generate a random number between 0 and 255
        }

        (*frameptr)++;
    }
}

int main()
{   
    // Upstream frame count
    uint inputframe = 0;
    int inputbuffer[INPUT_SIZE];

    uint* in_frameptr = &inputframe;
    int* in_bufferptr = inputbuffer;

    // New block
    Visualizer obj(INPUT_SIZE, OUTPUT_SIZE, true);

    // Setting inputs
    obj.setBufferPointer(in_bufferptr);
    obj.setFramePointer(in_frameptr);

    // Multi-threading this block
    // thread myThread(updateBuffer, in_bufferptr, in_frameptr);
    // obj.process();

    // Starts Visualizer iteration
    obj.iteration();

    // Exit upon execution of thread
    myThread.join();

    return 0;
}
