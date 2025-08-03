#include <iostream>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

int main()
{
    char *filename = (char *)vm_map(nullptr, 0);
    // filename = strcpy(filename, "datax.bin");
    for (size_t i=0; i<20; ++i) {
        std::cout << filename[i];
    }
    std::cout << std::endl;

    char *p1 = (char *)vm_map(filename, 0);
    char *p2 = (char *)vm_map(filename, 1);
    char *p3 = (char *)vm_map(filename, 2);
    strcpy(p1, "EECS 482");
    strcpy(p2, "is");
    strcpy(p3, "SSA!!");
    char *p4 = (char *)vm_map(nullptr, 0);
    p4[0] = 'J';
    for (size_t i=0; i<8; ++i)
    {
        std::cout << p1[i];
    }

    std::cout << std::endl;
    std::cout << p4[0] << std::endl;
}