#include "rpl/private-header.hpp"

#define INPUT_SIZE 4
#define OUTPUT_SIZE 8
 
int main()
{   
    // Upstream frame count
    uint inputframe = 0;
    int inputbuffer[INPUT_SIZE];

    uint* in_frameptr = &inputframe;
    int* in_bufferptr = inputbuffer;

    cout << "Upstream frame: " << inputframe << endl;
    cout << "Address: " << in_frameptr << endl;
    cout << "\n" << endl;

    // New block
    RadarBlock obj(INPUT_SIZE, OUTPUT_SIZE, true);

    // Setting inputs
    obj.setBufferPointer(in_bufferptr);
    obj.setFramePointer(in_frameptr);

    // Pass to next block
    int* out_bufferptr = obj.getBufferPointer();
    uint* out_frameptr = obj.getFramePointer();

    cout << "Output buffer address: " << out_bufferptr << endl;
    cout << "\n" << endl;

    // Multi-threading this block
    thread myThread(&RadarBlock::iteration, &obj);

    // Incrementing the upstream frame
    inputframe++;

    cout << "Upstream frame: " << inputframe << endl;
    cout << "Address: " << in_frameptr << endl;
    cout << "\n" << endl;

    // Exit upon execution of thread
    myThread.join();

    return 0;
}
