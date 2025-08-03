#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

#include "vm_app.h"
#include "vm_arena.h"

using std::cout;
using std::vector;

const size_t NUM_BLOCKS_TO_READ = 4;

const size_t NUM_GROUPS = 3;
const size_t ELEMENTS_PER_GROUP = 2;
const size_t NUM_BYTES_TO_READ = 32;

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

    child_pid = fork();
    if (child_pid == 0) {
        printf("Child...");

        /* OUTSIDE */
        char* filename = (char*)vm_map(nullptr, 0);
        strcpy(filename, "data1.bin");

        vector<vector<char*>> vectors(6, vector<char*>(2, nullptr));

        // Every group maps to a different {block} in the same filename.
        for (size_t i = 0; i < NUM_GROUPS; ++i) {
            for (size_t j = 0; j < ELEMENTS_PER_GROUP; ++j) {
                vectors[i][j] = (char*)vm_map(filename, i);
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
        /* INSIDE */

        char* second_filename = (char*)vm_map(nullptr, 0);
        strcpy(second_filename, "data2.bin");

        vector<char*> blocks_to_read(3, nullptr);
        for (size_t i = 0; i < 3; ++i) {
            blocks_to_read[i] = (char*)vm_map(second_filename, i);
        }

        vm_yield();

        // read first
        for (size_t i = 0; i < NUM_GROUPS; ++i) {
            for (size_t j = 0; j < ELEMENTS_PER_GROUP; ++j) {
                for (size_t k = 0; k < NUM_BYTES_TO_READ; ++k) {
                    cout << vectors[i][j][k];
                }
            }
        }

        // read here
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 128; ++j) {
                blocks_to_read[i][j] = 'X';
            }
        }

        vm_yield();

        // write here
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 128; ++j) {
                blocks_to_read[i][j] = 'X';
            }
        }

        // write later
        for (size_t i = 0; i < NUM_GROUPS; ++i) {
            for (size_t j = 0; j < ELEMENTS_PER_GROUP; ++j) {
                for (size_t k = 0; k < NUM_BYTES_TO_READ; ++k) {
                    vectors[i][j][k] = 'G';
                }
            }
        }

    }  // child
    else {
        // parent
        printf("Parent...");

        /* OUTSIDE */
        char* filename = (char*)vm_map(nullptr, 0);
        strcpy(filename, "data1.bin");

        vector<vector<char*>> vectors(6, vector<char*>(2, nullptr));

        // Every group maps to a different {block} in the same filename.
        for (size_t i = 0; i < NUM_GROUPS; ++i) {
            for (size_t j = 0; j < ELEMENTS_PER_GROUP; ++j) {
                vectors[i][j] = (char*)vm_map(filename, i);
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
        /* INSIDE */

        char* second_filename = (char*)vm_map(nullptr, 0);
        strcpy(second_filename, "data2.bin");

        vector<char*> blocks_to_read(3, nullptr);
        for (size_t i = 0; i < 3; ++i) {
            blocks_to_read[i] = (char*)vm_map(second_filename, i);
            vm_yield();
        }

        // write first
        for (size_t i = 0; i < NUM_GROUPS; ++i) {
            for (size_t j = 0; j < ELEMENTS_PER_GROUP; ++j) {
                for (size_t k = 0; k < NUM_BYTES_TO_READ; ++k) {
                    vectors[i][j][k] = 'G';
                }
            }
        }

        // write here
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 128; ++j) {
                blocks_to_read[i][j] = 'X';
            }
        }

        vm_yield();

        // read here
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 128; ++j) {
                blocks_to_read[i][j] = 'X';
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
    }

    return 0;
}