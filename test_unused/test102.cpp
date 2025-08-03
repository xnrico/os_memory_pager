#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    char *p0 = (char *)vm_map(nullptr, 0);
    p0[0] = 'H';
    printf("%c", p0[0]);

    char *p1 = (char *)vm_map(nullptr, 0);
    p1[0] = 'A';
    printf("%c", p1[0]);

    char *p2 = (char *)vm_map(nullptr, 0);
    p2[0] = 'S';
    printf("%c", p2[0]);

    char *p3 = (char *)vm_map(nullptr, 0);
    p3[0] = 'A';
    printf("%c", p3[0]);

    char *p4 = (char *)vm_map(nullptr, 0);
    p4[0] = 'N';
    printf("%c", p4[0]);

    char *p5 = (char *)vm_map(nullptr, 0);
    p5[0] = 'B';
    printf("%c", p5[0]);

    char *p6 = (char *)vm_map(nullptr, 0);
    p6[0] = 'E';
    printf("%c", p6[0]);

    char *p7 = (char *)vm_map(nullptr, 0);
    p7[0] = 'R';
    printf("%c", p7[0]);

    char *p8 = (char *)vm_map(nullptr, 0);
    p8[0] = 'K';
    printf("%c", p8[0]);

    printf("\n");
    printf("%c%c%c%c%c%c%c%c\n", p0[0], p1[0], p0[0], p1[0], p0[0], p1[0], p0[0], p1[0]);
    printf("%c%c%c%c%c%c%c%c\n", p2[0], p2[0], p2[0], p2[0], p2[0], p2[0], p2[0], p2[0]);
    printf("%c%c%c%c%c %c%c%c%c\n", p0[0], p1[0], p2[0], p3[0], p4[0], p5[0], p6[0], p7[0], p8[0]);
    printf("%c%c%c%c%c %c%c%c%c\n", p8[0], p8[0], p8[0], p8[0], p8[0], p8[0], p8[0], p8[0], p8[0]);
}