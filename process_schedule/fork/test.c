#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[])
{
    int opt;

    while((opt = getopt(argc, argv, "wr")) != -1)
    {
        switch(opt)
        {
        case 'w':
        
            break;
        case 'r':
            
            break;
        default:
            break;
        }
    }
}
