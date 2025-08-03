#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>
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
        virtual_page_addresses[i] = (char*)vm_map(filename, i);
    }

    // read first
    size_t byte_to_read = VM_PAGESIZE - 20;
    for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
        for (size_t j = 0; j < 40; ++j) {
            cout << virtual_page_addresses[0][byte_to_read];
            ++byte_to_read;
        }
        byte_to_read += VM_PAGESIZE;
    }

    // write later
    byte_to_read = VM_PAGESIZE - 20;
    for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
        for (size_t j = 0; j < 40; ++j) {
            virtual_page_addresses[0][byte_to_read] = 'G';
            ++byte_to_read;
        }
        byte_to_read += VM_PAGESIZE;
    }

    pid_t child_pid = fork();
    if (child_pid == 0) {
        printf("Child...");
        char* random_virtual_mapping1 = (char*)vm_map(nullptr, 0);
        char* random_virtual_mapping2 = (char*)vm_map(nullptr, 0);
        char* random_virtual_mapping3 = (char*)vm_map(nullptr, 0);

        size_t byte_to_read = VM_PAGESIZE - 20;
        for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
            for (size_t j = 0; j < 40; ++j) {
                cout << virtual_page_addresses[0][byte_to_read];
                ++byte_to_read;
            }
            byte_to_read += VM_PAGESIZE;
        }

        // write later
        byte_to_read = VM_PAGESIZE - 20;
        for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
            for (size_t j = 0; j < 40; ++j) {
                virtual_page_addresses[0][byte_to_read] = 'B';
                ++byte_to_read;
            }
            byte_to_read += VM_PAGESIZE;
        }

    }  // child
    else {
        // parent
        printf("Parent...");
        char* random_virtual_mapping1 = (char*)vm_map(nullptr, 0);
        char* random_virtual_mapping2 = (char*)vm_map(nullptr, 0);
        vm_yield();
        char* random_virtual_mapping3 = (char*)vm_map(nullptr, 0);

        size_t byte_to_read = VM_PAGESIZE - 20;
        for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
            for (size_t j = 0; j < 40; ++j) {
                cout << virtual_page_addresses[0][byte_to_read];
                ++byte_to_read;
            }
            byte_to_read += VM_PAGESIZE;
        }

        // write later
        byte_to_read = VM_PAGESIZE - 20;
        for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
            for (size_t j = 0; j < 40; ++j) {
                virtual_page_addresses[0][byte_to_read] = 'Z';
                ++byte_to_read;
            }
            byte_to_read += VM_PAGESIZE;
            vm_yield();
        }
    }

    child_pid = fork();
    if (child_pid == 0) {
        printf("Child...");
        char* random_virtual_mapping1 = (char*)vm_map(nullptr, 0);
        char* random_virtual_mapping2 = (char*)vm_map(nullptr, 0);
        char* random_virtual_mapping3 = (char*)vm_map(nullptr, 0);

        size_t byte_to_read = VM_PAGESIZE - 20;
        for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
            for (size_t j = 0; j < 40; ++j) {
                cout << virtual_page_addresses[0][byte_to_read];
                ++byte_to_read;
            }
            vm_yield();
            byte_to_read += VM_PAGESIZE;
        }

        // write later
        byte_to_read = VM_PAGESIZE - 20;
        for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
            for (size_t j = 0; j < 40; ++j) {
                virtual_page_addresses[0][byte_to_read] = 'F';
                ++byte_to_read;
            }
            byte_to_read += VM_PAGESIZE;
        }

    } else {
        // parent
        printf("Parent...");
        char* random_virtual_mapping1 = (char*)vm_map(nullptr, 0);
        char* random_virtual_mapping2 = (char*)vm_map(nullptr, 0);
        vm_yield();
        char* random_virtual_mapping3 = (char*)vm_map(nullptr, 0);

        size_t byte_to_read = VM_PAGESIZE - 20;
        for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
            for (size_t j = 0; j < 40; ++j) {
                cout << virtual_page_addresses[0][byte_to_read];
                ++byte_to_read;
            }
            vm_yield();
            byte_to_read += VM_PAGESIZE;
        }

        // write later
        byte_to_read = VM_PAGESIZE - 20;
        for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
            for (size_t j = 0; j < 40; ++j) {
                virtual_page_addresses[0][byte_to_read] = 'U';
                ++byte_to_read;
            }
            byte_to_read += VM_PAGESIZE;
        }
    }

    return 0;
}