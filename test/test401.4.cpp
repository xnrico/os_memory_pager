#include <iostream>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <vector>
#include "vm_app.h"
#include "vm_arena.h"

int main()
{
    if (fork())
    {
        char *filename = (char *)vm_map(nullptr, 0);

        strcpy(filename, "data2.bin");

        char *p = (char *)vm_map(filename, 0);
        for (size_t i=0; i<5; ++i) {
            std::cout << p[i];
        }
        std::cout << std::endl;

        strcpy(p, "Hasan");
        vm_yield();

        for (size_t i=0; i<5; ++i) {
            std::cout << p[i];
        }
        std::cout << std::endl;
    }
    else
    {
        char *filename = (char *)vm_map(nullptr, 0);

        strcpy(filename, "datax.bin");

        char *p = (char *)vm_map(filename, 0);
        for (size_t i=0; i<5; ++i) {
            std::cout << p[i];
        }
        std::cout << std::endl;

        strcpy(p, "Iaro");
        vm_yield();

        for (size_t i=0; i<5; ++i) {
            std::cout << p[i];
        }
        std::cout << std::endl;
    }
}
