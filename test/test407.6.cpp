#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

#include "vm_app.h"
#include "vm_arena.h"

using std::cout;
using std::vector;

// Random, complicated test case
// Should only work on the 6 credit version!

const size_t NUM_BLOCKS_TO_READ = 4;

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

        char* filename1 = (char*)vm_map(nullptr, 0);
        strcpy(filename1, "data2.bin");
        char* filename2 = (char*)vm_map(nullptr, 0);
        strcpy(filename2, "data3.bin");
        char* filename3 = (char*)vm_map(nullptr, 0);
        strcpy(filename3, "data4.bin");

        char* file1 = (char*)vm_map(filename1, 0);
        char* file2 = (char*)vm_map(filename2, 1);
        char* file3 = (char*)vm_map(filename3, 0);

        for (size_t i = 0; i < 2; ++i) {
            for (size_t j = 0; j < 32; ++j) {
                cout << file1[j];
            }
            vm_yield();

            for (size_t j = 0; j < 32; ++j) {
                file1[j] = 'L';
            }
        }

        for (size_t i = 0; i < 2; ++i) {
            for (size_t j = 0; j < 32; ++j) {
                cout << file2[j];
            }
            vm_yield();

            for (size_t j = 0; j < 32; ++j) {
                file2[j] = 'L';
            }
        }

        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 32; ++j) {
                cout << file3[j];
            }
            vm_yield();

            for (size_t j = 0; j < 32; ++j) {
                file3[j] = 'L';
            }
        }

        vm_yield();

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

        char* filename1 = (char*)vm_map(nullptr, 0);
        strcpy(filename1, "data2.bin");
        char* filename2 = (char*)vm_map(nullptr, 0);
        strcpy(filename2, "data3.bin");
        char* filename3 = (char*)vm_map(nullptr, 0);
        strcpy(filename3, "data4.bin");

        char* file1 = (char*)vm_map(filename1, 1);
        char* file2 = (char*)vm_map(filename2, 0);
        char* file3 = (char*)vm_map(filename3, 0);

        for (size_t i = 0; i < 2; ++i) {
            for (size_t j = 0; j < 32; ++j) {
                cout << file1[j];
            }
            vm_yield();

            for (size_t j = 0; j < 32; ++j) {
                file1[j] = 'L';
            }
        }

        for (size_t i = 0; i < 2; ++i) {
            for (size_t j = 0; j < 32; ++j) {
                cout << file2[j];
            }
            vm_yield();

            for (size_t j = 0; j < 32; ++j) {
                file2[j] = 'L';
            }
        }

        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 32; ++j) {
                cout << file3[j];
            }
            vm_yield();

            for (size_t j = 0; j < 32; ++j) {
                file3[j] = 'L';
            }
        }

        vm_yield();

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

        size_t byte_to_read = VM_PAGESIZE - 20;
        for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
            for (size_t j = 0; j < 4; ++j) {
                cout << virtual_page_addresses[0][byte_to_read];
                ++byte_to_read;
            }
            vm_yield();
            byte_to_read += VM_PAGESIZE;
        }

        // write later
        byte_to_read = VM_PAGESIZE - 20;
        for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
            for (size_t j = 0; j < 4; ++j) {
                virtual_page_addresses[0][byte_to_read] = 'F';
                ++byte_to_read;
                vm_yield();
            }
            byte_to_read += VM_PAGESIZE;
        }

    } else {
        // parent
        printf("Parent...");

        vm_yield();

        size_t byte_to_read = VM_PAGESIZE - 20;
        for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
            for (size_t j = 0; j < 4; ++j) {
                cout << virtual_page_addresses[0][byte_to_read];
                ++byte_to_read;
            }
            vm_yield();
            byte_to_read += VM_PAGESIZE;
        }

        // write later
        byte_to_read = VM_PAGESIZE - 20;
        for (size_t i = 0; i < NUM_BLOCKS_TO_READ - 1; ++i) {
            for (size_t j = 0; j < 4; ++j) {
                virtual_page_addresses[0][byte_to_read] = 'U';
                ++byte_to_read;
            }
            vm_yield();
            byte_to_read += VM_PAGESIZE;
        }
    }

    return 0;
}