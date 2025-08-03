#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    /* Allocate swap-backed page from the arena */
    char* filename = static_cast<char *>(vm_map(nullptr, 0));

    strcpy(filename, "lampson83.txt");
    char* p0 = static_cast<char *>(vm_map (filename, 0));
    for (unsigned int i=0; i<1930; i++) {
	    cout << p0[i];
    }

    printf("\n\n");

    strcpy(filename, "data1.bin");
    char* p1 = static_cast<char *>(vm_map (filename, 0));
    for (unsigned int i=0; i<512; i++) {
	    cout << p1[i];
    }

    printf("\n\n");

    strcpy(filename, "data2.bin");
    char* p2 = static_cast<char *>(vm_map (filename, 0));
    for (unsigned int i=0; i<100; i++) {
	    cout << p2[i];
    }

    printf("\n\n");

    strcpy(filename, "data3.bin");
    char* p3 = static_cast<char *>(vm_map (filename, 0));
    for (unsigned int i=0; i<500; i++) {
	    cout << p3[i];
    }

    printf("\n\n");

    strcpy(filename, "data4.bin");
    char* p4 = static_cast<char *>(vm_map (filename, 0));
    for (unsigned int i=0; i<874; i++) {
	    cout << p4[i];
    }
    printf("\n\n");
}