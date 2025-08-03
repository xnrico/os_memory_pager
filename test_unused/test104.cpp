#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    char *p0 = (char *)vm_map(nullptr, 0);
    p0[0] = 'H';
    printf("%c\n", p0[0]);

    strcpy(p0, "test000.cpp");
    printf("Where to find King of 482: %s\n", p0);

    char *p1 = (char *)vm_map(p0, 0);
    char *p2 = (char *)vm_map(p0, 0);
    char *p3 = (char *)vm_map(p0, 1);
    char *p4 = (char *)vm_map(p0, 1);

    char *p5 = (char *)vm_map(nullptr, 0);
    char *p6 = (char *)vm_map(nullptr, 0);

    for (size_t i=0; i<9; ++i) {
        static char* tmp = p1;
        printf("%c", *tmp++);
    }

    printf("\n");

    memcpy(p5, p1, strlen("HasanBerk")+1);
    memcpy(p6, p3, strlen("HasanBerk")+1);
    p5[strlen("HasanBerk")] = '\0';
    printf("King of 482 from p5: %s\n", p5);
    printf("King of xxx from p6: %c%c%c%c\n", *p6, *(p6+1), *(p6+2), *(p6+3));

    memcpy(p4, p2, strlen("HasanBerk")+1);
    p4[strlen("HasanBerk")] = '\0';
    printf("King of 482 from p2: %s\n", p2);
    printf("King of 482 from p4: %s\n", p4);

    strcpy(p1+150, "We all love 482, or is that not trueeeeee!?");
    printf("Message from p1: %s\n", p1+150);

    char *p7 = (char *)vm_map(nullptr, 0);
    p7[0] = 'I';
    char *p8 = (char *)vm_map(nullptr, 0);
    p8[0] = 'A';
    char *p9 = (char *)vm_map(nullptr, 0);
    p9[0] = 'R';
    char *p10 = (char *)vm_map(nullptr, 0);
    p10[0] = 'O';
    char *p11 = (char *)vm_map(nullptr, 0);
    p11[0] = '!';

    printf("%c%c%c%c%c\n", p7[0], p8[0], p9[0], p10[0], p11[0]);
    // Now p1 should have been evicted, read from p2 (backed by the same file)
    printf("Message from p2: %s\n", p2+150);
    printf("Amazing!\n");
}