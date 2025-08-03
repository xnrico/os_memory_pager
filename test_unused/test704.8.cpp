#include <iostream>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <vector>
#include "vm_app.h"
#include "vm_arena.h"

using std::cout;
using std::vector;

const size_t NUM_BLOCKS_TO_READ = 10;

int main() {
    char* filename = (char*)vm_map(nullptr, 0);
    strcpy(filename, "data1.bin");
    vector<char*> virtual_page_addresses(NUM_BLOCKS_TO_READ, nullptr);

    for (size_t i = 0; i < NUM_BLOCKS_TO_READ; ++i) {
        virtual_page_addresses[i] = (char*) vm_map(filename, i);
    }

    // read and write together
    for (size_t i = 0; i < virtual_page_addresses.size(); ++i) {
        for (size_t j = 0 ; j < VM_PAGESIZE ; ++j) {
            cout << virtual_page_addresses[i][j];
            virtual_page_addresses[i][j] = 'Z';
        }
    }
    
    return 0;
}