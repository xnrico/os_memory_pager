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
const size_t NUM_BYTES_TO_READ = VM_PAGESIZE / 1024;

int main() {
    char* filename = (char*)vm_map(nullptr, 0);
    strcpy(filename, "data1.bin");
    vector<char*> virtual_page_addresses(NUM_BLOCKS_TO_READ, nullptr);

    for (size_t i = 0; i < NUM_BLOCKS_TO_READ; ++i) {
        virtual_page_addresses[i] = (char*)vm_map(filename, i);
    }

    // write and read together
    for (size_t i = 0; i < virtual_page_addresses.size(); ++i) {
        for (size_t j = 0; j < NUM_BYTES_TO_READ; ++j) {
            virtual_page_addresses[i][j] = 'A';
            cout << virtual_page_addresses[i][j];
        }
        vm_yield();
    }

    pid_t child_pid = fork();
    if (child_pid == 0) {
        printf("Child...");
        char* random_virtual_mapping1 = (char*)vm_map(nullptr, 0);
        char* random_virtual_mapping2 = (char*)vm_map(nullptr, 0);
        char* random_virtual_mapping3 = (char*)vm_map(nullptr, 0);

        // write and read together
        for (size_t i = 0; i < virtual_page_addresses.size(); ++i) {
            for (size_t j = 0; j < NUM_BYTES_TO_READ; ++j) {
                virtual_page_addresses[i][j] = 'B';
                vm_yield();
                cout << virtual_page_addresses[i][j];
            }
        }

    }  // child
    else {
        // parent
        printf("Parent...");
        char* random_virtual_mapping1 = (char*)vm_map(nullptr, 0);
        char* random_virtual_mapping2 = (char*)vm_map(nullptr, 0);
        vm_yield();
        char* random_virtual_mapping3 = (char*)vm_map(nullptr, 0);

        // write and read together
        for (size_t i = 0; i < virtual_page_addresses.size(); ++i) {
            for (size_t j = 0; j < NUM_BYTES_TO_READ; ++j) {
                virtual_page_addresses[i][j] = 'C';
                cout << virtual_page_addresses[i][j];
            }
            vm_yield();
        }
    }

    child_pid = fork();
    if (child_pid == 0) {
        printf("Child...");
        char* random_virtual_mapping1 = (char*)vm_map(nullptr, 0);
        char* random_virtual_mapping2 = (char*)vm_map(nullptr, 0);
        char* random_virtual_mapping3 = (char*)vm_map(nullptr, 0);

        // write and read together
        for (size_t i = 0; i < virtual_page_addresses.size(); ++i) {
            for (size_t j = 0; j < NUM_BYTES_TO_READ; ++j) {
                virtual_page_addresses[i][j] = 'D';
                vm_yield();
                cout << virtual_page_addresses[i][j];
            }
        }
    } else {
        // parent
        printf("Parent...");
        char* random_virtual_mapping1 = (char*)vm_map(nullptr, 0);
        char* random_virtual_mapping2 = (char*)vm_map(nullptr, 0);
        vm_yield();
        char* random_virtual_mapping3 = (char*)vm_map(nullptr, 0);

        // write and read together
        for (size_t i = 0; i < virtual_page_addresses.size(); ++i) {
            for (size_t j = 0; j < NUM_BYTES_TO_READ; ++j) {
                virtual_page_addresses[i][j] = 'E';
                cout << virtual_page_addresses[i][j];
            }
            vm_yield();
        }
    }

    return 0;
}