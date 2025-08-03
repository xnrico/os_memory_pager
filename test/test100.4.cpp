#include <iostream>
#include <cstring>
#include <unistd.h>
#include <vector>
#include "vm_app.h"
#include "vm_arena.h"

using std::cout;
using std::vector;


const size_t NUM_GROUPS = 6;
const size_t ELEMENTS_PER_GROUP = 2;
const size_t NUM_BYTES_TO_READ = 32;

// Virtual pages sharing physical pages!

int main() {
    char* filename = (char*)vm_map(nullptr, 0);
    strcpy(filename, "data1.bin");

    vector<vector<char*>> vectors(6, vector<char*>(2, nullptr));

    // Every group maps to a different {block} in the same filename.
    for (size_t i = 0; i < NUM_GROUPS; ++i) {
        for (size_t j = 0; j < ELEMENTS_PER_GROUP; ++j) {
            vectors[i][j] = (char*) vm_map(filename, i);
        }
    }

    // write first
    for (size_t i = 0; i < NUM_GROUPS; ++i) {
        for (size_t j = 0; j < ELEMENTS_PER_GROUP; ++j) {
            for (size_t k = 0; k < NUM_BYTES_TO_READ; ++k) {
                vectors[i][j][k] = 'G';
            }
        }
    }

    // read later
    for (size_t i = 0; i < NUM_GROUPS; ++i) {
        for (size_t j = 0; j < ELEMENTS_PER_GROUP; ++j) {
            for (size_t k = 0; k < NUM_BYTES_TO_READ; ++k) {
                cout << vectors[i][j][k];
            }
        }
    }

    // do it the byte way too!

    // read first
    size_t byte_to_read = VM_PAGESIZE - 20;
    size_t num_blocks_to_read = NUM_GROUPS * ELEMENTS_PER_GROUP;
    for (size_t i = 0; i < num_blocks_to_read - 1; ++i) {
        for (size_t j = 0; j < 40; ++j) {
            cout << vectors[0][0][byte_to_read];
            ++byte_to_read;
        }
        byte_to_read += VM_PAGESIZE;
    }

    // write later
    byte_to_read = VM_PAGESIZE - 20;
    for (size_t i = 0; i < num_blocks_to_read - 1; ++i) {
        for (size_t j = 0; j < 40; ++j) {
            vectors[0][0][byte_to_read] = 'G';
            ++byte_to_read;
        }
        byte_to_read += VM_PAGESIZE;
    }



    return 0;
}
