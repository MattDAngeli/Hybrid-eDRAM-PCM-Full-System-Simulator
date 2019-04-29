/*
 * C program to illustrate how a file stored on the disk is read
*/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{

    FILE *fptr;

    char ch;

    /*  open the file for reading */

    fptr = fopen(argv[1], "r");

    if (fptr == NULL)
    {
        printf("Cannot open file \n");

        exit(0);
    }

    ch = fgetc(fptr);

    while (ch != EOF)
    {
        printf ("%c", ch);

        ch = fgetc(fptr);
    }

    fclose(fptr);

}
