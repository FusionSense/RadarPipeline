#include "rpl/private-header.hpp"
 
int main()
{   
    // Upstream frame count
    uint inputframe = 0;
    uint* inputptr = &inputframe;

    cout << "Upstream frame: " << inputframe << endl;
    cout << "Address: " << inputptr << endl;
    cout << "\n" << endl;

    // New block
    RadarBlock obj(8);
    obj.setInputPointer(inputptr);

    int* bufferptr = obj.getBufferPointer();
    uint* frameptr = obj.getFramePointer();

    cout << "Output buffer address: " << bufferptr << endl;
    cout << "\n" << endl;

    // Multi-threading this block
    thread myThread(&RadarBlock::iteration, &obj, 42);

    // Incrementing the upstream frame
    inputframe++;

    cout << "Upstream frame: " << inputframe << endl;
    cout << "Address: " << inputptr << endl;
    cout << "\n" << endl;

    // Exit upon execution of thread
    myThread.join();

    return 0;
}
