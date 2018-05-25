
#include "amdadl.h"
#include "stdio.h"

int main(int argc, char* argv[])
{
    int retval = 0;

    AMDADL amdadl;
    retval = amdadl.loadDLL();
    if (retval != 0)
        return retval;

    if (argc != 7)
    {
        printf("Wrong number of arguments!\n");
        printf("Usage: index cclock mclock cvddc mvddc fanspeed\n");
        return -1;
    }

    retval = amdadl.setupGPU(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
    if (retval != 0)
        return retval;

    return 0;
}

