#include <unistd.h>

#include <cstring>
#include <iostream>
#include <vector>

#include "vm_app.h"
#include "vm_arena.h"

using std::cout;
using std::vector;

const size_t NUM_GROUPS = 3;
const size_t ELEMENTS_PER_GROUP = 2;
const size_t NUM_BYTES_TO_READ = 16;

// Virtual pages sharing physical pages!

int main() {
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

    filename = (char*)vm_map(nullptr, 0);
    strcpy(filename, "data3.bin");

    vector<vector<char*>> vectors2(6, vector<char*>(2, nullptr));

    // Every group maps to a different {block} in the same filename.
    for (size_t i = 0; i < ELEMENTS_PER_GROUP; ++i) {
        for (size_t j = 0; j < NUM_GROUPS; ++j) {
            vectors2[j][i] = (char*)vm_map(filename, i);
        }
    }

    // read first
    for (size_t i = 0; i < ELEMENTS_PER_GROUP; ++i) {
        for (size_t j = 0; j < NUM_GROUPS; ++j) {
            for (size_t k = 0; k < NUM_BYTES_TO_READ; ++k) {
                cout << vectors2[j][i][k];
            }
        }
    }
    // write later
    for (size_t i = 0; i < ELEMENTS_PER_GROUP; ++i) {
        for (size_t j = 0; j < NUM_GROUPS; ++j) {
            for (size_t k = 0; k < NUM_BYTES_TO_READ; ++k) {
                vectors2[j][i][k] = 'G';
            }
        }
    }

    pid_t child_pid = fork();
    if (child_pid == 0) {
        printf("Child...");
        char* second_filename = (char*)vm_map(nullptr, 0);
        strcpy(second_filename, "data2.bin");

        vector<char*> blocks_to_read(3, nullptr);
        for (size_t i = 0; i < 3; ++i) {
            blocks_to_read[i] = (char*)vm_map(second_filename, i);
        }

        // read first
        for (size_t i = 0; i < ELEMENTS_PER_GROUP; ++i) {
            for (size_t j = 0; j < NUM_GROUPS; ++j) {
                for (size_t k = 0; k < NUM_BYTES_TO_READ; ++k) {
                    cout << vectors2[j][i][k];
                }
            }
        }

        // read here
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 128; ++j) {
                cout << blocks_to_read[i][j];
            }
        }

        // write here
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 128; ++j) {
                if (blocks_to_read[i][j] == 'A') {
                    blocks_to_read[i][j] = 'X';
                } else {
                    blocks_to_read[i][j] = 'Q';
                }
            }
        }

        // write later
        for (size_t i = 0; i < NUM_GROUPS; ++i) {
            for (size_t j = 0; j < ELEMENTS_PER_GROUP; ++j) {
                for (size_t k = 0; k < NUM_BYTES_TO_READ; ++k) {
                    vectors2[i][j][k] = 'G';
                }
            }
        }

    }  // child
    else {
        // parent
        printf("Parent...");
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
                    vectors2[i][j][k] = 'G';
                }
            }
        }

        // write here
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 128; ++j) {
                blocks_to_read[i][j] = 'A';
            }
        }

        vm_yield();

        // read here, shouldn't have the data it thinks it has! (written to by another
        // process!)
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 128; ++j) {
                cout << blocks_to_read[i][j];
            }
        }

        // read some other stuff later later
        for (size_t i = 0; i < NUM_GROUPS; ++i) {
            for (size_t j = 0; j < ELEMENTS_PER_GROUP; ++j) {
                for (size_t k = 0; k < NUM_BYTES_TO_READ; ++k) {
                    cout << vectors2[i][j][k];
                }
            }
        }
    }

    return 0;
}
