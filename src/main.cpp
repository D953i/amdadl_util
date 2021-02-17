
#include "amdadl.h"
#include "stdio.h"

int main(int argc, char* argv[])
{
    int retval = 0;

    AMDADL amdadl;
    retval = amdadl.loadDLL();
    if (retval != 0)
        return retval;

    if (argc != 9)
    {
        printf("Wrong number of arguments!\n");
	printf("Works with AMD driver version 18.7.1\n");
        printf("Usage: index cclock mclock cvddc mvddc fanmin fanmax temp\n");

        //return -1;

        //amdadl.setupGPU(0, 136600, 205000, 980, 950, 60, 95, 60);
        //amdadl.setupGPU(1, 136600, 205000, 980, 950, 60, 95, 55);
        return 0;
    }

    retval = amdadl.setupGPU(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]), atoi(argv[8]));
    if (retval != 0)
        return retval;

    return 0;
}

