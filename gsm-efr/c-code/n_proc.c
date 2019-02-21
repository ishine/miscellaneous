#include <stdio.h>
#include <stdlib.h>

void proc_head (char *mes)
{
    fprintf(stderr, "\n/**************************************************************\n\n");
    fprintf(stderr, "     European digital cellular telecommunications system\n");
    fprintf(stderr, "                12200 bits/s speech codec for\n");
    fprintf(stderr, "          enhanced full rate speech traffic channels\n\n");
    fprintf(stderr, "     Bit-Exact C Simulation Code - %s\n", mes);
    fprintf(stderr, "     Version 5.1.0\n");
    fprintf(stderr, "     June 26, 1996\n\n");
    fprintf(stderr, "**************************************************************/\n\n");
}
