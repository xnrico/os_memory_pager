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

    /* Print the first part of the paper */
    for (unsigned int i=0; i<20; i++) {
	    cout << p0[i];
    }
    printf("\n\n");
    for (unsigned int i=0; i<20; i++) {
	    cout << p1[i];
    }
    printf("\n\n");
    for (unsigned int i=0; i<20; i++) {
	    cout << p2[i];
    }
    printf("\n\n");
    for (unsigned int i=0; i<20; i++) {
        cout << p3[i];
    }
    printf("\n\n");
    for (unsigned int i=0; i<20; i++) {
	    cout << p0[i];
    }

    pid_t pid0 = fork();

    if (pid0 != 0) {
        printf("\n\n*********fork()*********\n\n");
        printf("forked pid: %d\n", pid0);
        printf("current pid: %d\n", getpid());
         /* Allocate swap-backed page from the arena */
        char* filename00 = static_cast<char *>(vm_map(nullptr, 0));

        char* begin00 = (char*) ((uintptr_t) filename00 + (uintptr_t) VM_PAGESIZE - 4);
        char* beginn = begin00;
        printf("Address Begins at: %p\n", begin00);
        *begin00++ = 'l';
        *begin00++ = 'a';
        *begin00++ = 'm';
        *begin00++ = 'p';
        *begin00++ = 's';
        *begin00++ = 'o';
        *begin00++ = 'n';
        *begin00++ = '8';
        *begin00++ = '3';
        *begin00++ = '.';
        *begin00++ = 't';
        *begin00++ = 'x';
        *begin00++ = 't';
        *begin00 = '\0';
        printf("Address ends at %p, filename is: %s\n", begin00, beginn);

        /* Map a page from the specified file */
        char* p00 = (char*) (vm_map(begin, 0));
        char* p11 = (char*) (vm_map(begin, 0));
        char* p22 = (char*) (vm_map(begin, 0));
        char* p33 = (char*) (vm_map(begin, 0));

        /* Print the first part of the paper */
        for (unsigned int i=0; i<20; i++) {
            cout << p00[i];
        }
        printf("\n\n");
        for (unsigned int i=0; i<20; i++) {
            cout << p11[i];
        }
        printf("\n\n");
        for (unsigned int i=0; i<20; i++) {
            cout << p22[i];
        }
        printf("\n\n");
        for (unsigned int i=0; i<20; i++) {
            cout << p33[i];
        }
        printf("\n\n");
        for (unsigned int i=0; i<20; i++) {
            cout << p00[i];
        }

        pid_t pid1 = fork();

        if (pid1 != 0) {
            printf("\n\n*********fork()*********\n\n");
            char* f = static_cast<char *>(vm_map(nullptr, 0));

            /* Write the name of the file that will be mapped */
            strcpy(f, "lampson83.txt");

            /* Map a page from the specified file */
            char* p = static_cast<char *>(vm_map (f, 0));

            /* Print the first part of the paper */
            for (unsigned int i=0; i<19; i++) {
                cout << p[i];
            }
        }
        printf("\n Finished!\n");
    }

    return 0;
}