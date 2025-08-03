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

    strcpy(p0, "Hasan Berk");
    printf("King of 482: %s\n", p0);

    strcpy(p0, "Iaroslav");
    printf("King of 482: %s\n", p0);

    strcpy(p0+1, "Rico");
    printf("King of 482: %s\n", p0);

    char *p1 = (char *)vm_map(nullptr, 0);
    char *p2 = (char *)vm_map(nullptr, 0);
    char *p3 = (char *)vm_map(nullptr, 0);

    strcpy(p0, "Hasan Berk");
    memcpy(p1, p0, strlen("Hasan Berk")+1);
    strcpy(p0, "Iaroslav");
    memcpy(p2, p0, strlen("Iaroslav")+1);
    strcpy(p0, "Rico");
    memcpy(p3, p0, strlen("Rico")+1);

    printf("King of 482: %s\n", p3);
    printf("King of 482: %s\n", p2);
    printf("King of 482: %s\n", p1);
    printf("King of 482: %s\n", p0);

    char *p4 = (char *)vm_map(nullptr, 0);
    char *p5 = (char *)vm_map(nullptr, 0);

    strcpy(p0, "Hasan Berk");
    strcpy(p4, "Berky Berk");
    memcpy(p5 - 3, p0, strlen("Hasan Berk")+1);
    printf("Final king of 482 from %p: %s\n", p5-3, p5-3);
    printf("Final king of 482 from %p: %s\n", p5, p5);
}