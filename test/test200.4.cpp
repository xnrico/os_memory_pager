#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

#include "vm_app.h"
#include "vm_arena.h"

// Error checks!

using std::cout;
using std::vector;

const size_t NUM_MAX_SWAP_PAGES = 256;
const size_t NUM_BLOCKS_TO_READ = 10;

// Check filename not completely in the valid part of the arena!
// Check what happens if there are more than MAX NUM SWAP Pages requested

int main() {
    char* filename1 = (char*)vm_map(nullptr, 0);
    char* filename2 = (char*)vm_map(nullptr, 0);
    strcpy(filename2, "data2.bin");

    // vector<char*> swap_pages(NUM_MAX_SWAP_PAGES + 1, nullptr);
    // for (size_t i = 0; i < NUM_MAX_SWAP_PAGES; ++i) {
    //     swap_pages[i] = (char*)vm_map(nullptr, 0);
    // }
    // last two blocks must be not allocated (nullptr) because not enough swap pages!

    filename1 = filename1 - 3;  // filename is not in the valid part of the arena!
    strcpy(filename1, "data1.bin");
    vector<char*> virtual_page_addresses(NUM_BLOCKS_TO_READ, nullptr);

    for (size_t i = 0; i < NUM_BLOCKS_TO_READ; ++i) {
        // all of these are supposed to fail!
        virtual_page_addresses[i] = (char*)vm_map(filename1, i);
    }

    // too many file backed pages, map should return -1
    for (size_t i = 0; i < (VM_ARENA_SIZE / VM_PAGESIZE) + 1; ++i) {
        vm_map(filename2, 0);
    }

    return 0;
}