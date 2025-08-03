#include <iostream>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    printf("test start");

    /* Allocate swap-backed page from the arena */
    char* filename0 = static_cast<char *>(vm_map(nullptr, 0));
    char* filename1 = static_cast<char *>(vm_map(nullptr, 0));

    assert((uintptr_t) filename0 == (uintptr_t) VM_ARENA_BASEADDR);
    assert((uintptr_t) filename1 == (uintptr_t) VM_ARENA_BASEADDR + (uintptr_t) VM_PAGESIZE);
    
    char* begin0 = (char*) ((uintptr_t) filename0 + (uintptr_t) VM_PAGESIZE - 4);
    char* begin = begin0;
    printf("Address Begins at: %p\n", begin0);
    *begin0++ = 'l';
    *begin0++ = 'a';
    *begin0++ = 'm';
    *begin0++ = 'p';
    *begin0++ = 's';
    *begin0++ = 'o';
    *begin0++ = 'n';
    *begin0++ = '8';
    *begin0++ = '3';
    *begin0++ = '.';
    *begin0++ = 't';
    *begin0++ = 'x';
    *begin0++ = 't';
    *begin0 = '\0';
    printf("Address ends at %p, filename is: %s\n", begin0, begin);

    /* Map a page from the specified file */
    char* p0 = (char*) (vm_map(begin, 0));
    char* p1 = (char*) (vm_map(begin, 0));
    char* p2 = (char*) (vm_map(begin, 0));
    char* p3 = (char*) (vm_map(begin, 0));
    char* p4 = (char*) (vm_map(begin, 0));
    char* p5 = (char*) (vm_map(begin, 0));

    /* Print the first part of the paper */
    for (unsigned int i=0; i<1930; i++) {
	    cout << p0[i];
    }
    printf("\n\n\n\n");
    for (unsigned int i=0; i<1930; i++) {
	    cout << p1[i];
    }
    printf("\n\n\n\n");
    for (unsigned int i=0; i<1930; i++) {
	    cout << p2[i];
    }
    printf("\n\n\n\n");
    for (unsigned int i=0; i<1930; i++) {
	    cout << p3[i];
    }
    printf("\n\n\n\n");
    for (unsigned int i=0; i<1930; i++) {
	    cout << p4[i];
    }
    printf("\n\n\n\n");
    for (unsigned int i=0; i<1930; i++) {
	    cout << p5[i];
    }
    printf("\n\n\n\n");
    for (unsigned int i=0; i<1930; i++) {
	    cout << p0[i];
    }
}