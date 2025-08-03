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


    // write first
    size_t byte_to_read = VM_PAGESIZE - 20;
    for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
        for (size_t j = 0; j < 40; ++j) {
            virtual_page_addresses[0][byte_to_read] = 'G';
            ++byte_to_read;
        }
        byte_to_read += VM_PAGESIZE;
    }

    // read later
    byte_to_read = VM_PAGESIZE - 20;
    for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
        for (size_t j = 0; j < 40; ++j) {
            cout << virtual_page_addresses[0][byte_to_read];
            ++byte_to_read;
        }
        byte_to_read += VM_PAGESIZE;
    }


    return 0;
}