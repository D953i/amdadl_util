
#ifndef AMDADL_H_
#define AMDADL_H_

#include <windows.h>
#include <adl_sdk.h>
#include <adl_structures.h>

#include <vector>

class AMDADL
{

typedef struct _amd_adapters_
{
    int id;
    int iAdapterIndex;
    int iBusNumber;
} amd_adapters;

public:
    AMDADL();
    ~AMDADL();

    int loadDLL();
    int findGPUs();
    int setupGPU(int index, int cclk, int mclk, int cvddc, int mvdcc, int fan_speed);


private:
    HINSTANCE m_hDLL;

    std::vector<amd_adapters*> m_gpus;
};

#endif //AMDADL_H_
