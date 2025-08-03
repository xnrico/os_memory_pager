#include <unistd.h>

#include <cstring>
#include <iostream>
#include <vector>

#include "vm_app.h"
#include "vm_arena.h"

using std::cout;
using std::vector;

const size_t NUM_GROUPS = 6;
const size_t ELEMENTS_PER_GROUP = 2;
const size_t NUM_BYTES_TO_READ = 32;

// Complicated test case. First parent writes, yields. Then child reads, writes. Exits.
// After, First reads.

int main() {
    char* filename = (char*)vm_map(nullptr, 0);
    strcpy(filename, "data1.bin");

    vector<vector<char*>> vectors(6, vector<char*>(2, nullptr));

    // Every group maps to a different {block} in the same filename.
    for (size_t i = 0; i < ELEMENTS_PER_GROUP; ++i) {
        for (size_t j = 0; j < NUM_GROUPS; ++j) {
            vectors[j][i] = (char*)vm_map(filename, i);
        }
    }

    // read first
    for (size_t i = 0; i < ELEMENTS_PER_GROUP; ++i) {
        for (size_t j = 0; j < NUM_GROUPS; ++j) {
            for (size_t k = 0; k < NUM_BYTES_TO_READ; ++k) {
                cout << vectors[j][i][k];
            }
        }
    }
    // write later
    for (size_t i = 0; i < ELEMENTS_PER_GROUP; ++i) {
        for (size_t j = 0; j < NUM_GROUPS; ++j) {
            for (size_t k = 0; k < NUM_BYTES_TO_READ; ++k) {
                vectors[j][i][k] = 'G';
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
                    cout << vectors[j][i][k];
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
                    vectors[i][j][k] = 'G';
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
                    vectors[i][j][k] = 'G';
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
                    cout << vectors[i][j][k];
                }
            }
        }
    }

    return 0;
}
