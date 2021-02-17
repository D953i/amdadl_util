
#include "amdadl.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define AMD_VENDOR_ID (1002)

typedef int (*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int (*ADL_MAIN_CONTROL_DESTROY)();
typedef int (*ADL_FLUSH_DRIVER_DATA)(int);
typedef int (*ADL2_ADAPTER_ACTIVE_GET) (ADL_CONTEXT_HANDLE, int, int*);
typedef int (*ADL_ADAPTER_NUMBEROFADAPTERS_GET) (int*);
typedef int (*ADL_ADAPTER_ADAPTERINFO_GET) (LPAdapterInfo, int);
typedef int (*ADL2_ADAPTER_ACTIVE_GET) (ADL_CONTEXT_HANDLE, int, int*);
typedef int (*ADL_ADAPTERX2_CAPS) (int, int*);
typedef int (*ADL2_OVERDRIVE_CAPS) (ADL_CONTEXT_HANDLE context, int index, int * supported, int * enabled, int * version);
typedef int (*ADL2_OVERDRIVEN_FANCONTROL_GET) (ADL_CONTEXT_HANDLE, int, ADLODNFanControl*);
typedef int (*ADL2_OVERDRIVEN_FANCONTROL_SET) (ADL_CONTEXT_HANDLE, int, ADLODNFanControl*);
typedef int (*ADL2_OVERDRIVEN_POWERLIMIT_GET) (ADL_CONTEXT_HANDLE, int, ADLODNPowerLimitSetting*);
typedef int (*ADL2_OVERDRIVEN_POWERLIMIT_SET) (ADL_CONTEXT_HANDLE, int, ADLODNPowerLimitSetting*);
typedef int (*ADL2_OVERDRIVEN_TEMPERATURE_GET) (ADL_CONTEXT_HANDLE, int, int, int*);
typedef int (*ADL2_OVERDRIVEN_CAPABILITIES_GET) (ADL_CONTEXT_HANDLE, int, ADLODNCapabilities*);
typedef int (*ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET) (ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int (*ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET) (ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int (*ADL2_OVERDRIVEN_MEMORYCLOCKS_GET) (ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int (*ADL2_OVERDRIVEN_MEMORYCLOCKS_SET) (ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int (*ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET) (ADL_CONTEXT_HANDLE, int, ADLODNPerformanceStatus*);

ADL_MAIN_CONTROL_CREATE  ADL_Main_Control_Create = NULL;
ADL_MAIN_CONTROL_DESTROY ADL_Main_Control_Destroy = NULL;

ADL_ADAPTERX2_CAPS   ADL_AdapterX2_Caps = NULL;
ADL2_OVERDRIVE_CAPS ADL2_Overdrive_Caps = NULL;
ADL_ADAPTER_ADAPTERINFO_GET ADL_AdapterInfo_Get = NULL;
ADL2_ADAPTER_ACTIVE_GET ADL2_Adapter_Active_Get = NULL;
ADL2_OVERDRIVEN_FANCONTROL_GET ADL2_FanControl_Get = NULL;
ADL2_OVERDRIVEN_FANCONTROL_SET ADL2_FanControl_Set = NULL;
ADL2_OVERDRIVEN_POWERLIMIT_GET ADL2_PowerLimit_Get = NULL;
ADL2_OVERDRIVEN_POWERLIMIT_SET ADL2_PowerLimit_Set = NULL;
ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_NumberOfAdapters_Get = NULL;
ADL2_OVERDRIVEN_CAPABILITIES_GET ADL2_Capabilities_Get = NULL;
ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET ADL2_SystemClocks_Get = NULL;
ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET ADL2_SystemClocks_Set = NULL;
ADL2_OVERDRIVEN_MEMORYCLOCKS_GET ADL2_MemoryClocks_Get = NULL;
ADL2_OVERDRIVEN_MEMORYCLOCKS_GET ADL2_MemoryClocks_Set = NULL;
ADL2_OVERDRIVEN_TEMPERATURE_GET ADL2_Temperature_Get = NULL;
ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET ADL2_PerformanceStatus_Get = NULL;

//Memory allocation function
void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
    void* lpBuffer = malloc(iSize);
    return lpBuffer;
}

//Optional Memory de-allocation function
void __stdcall ADL_Main_Memory_Free(void** lpBuffer)
{
    if (NULL != *lpBuffer)
    {
        free(*lpBuffer);
        *lpBuffer = NULL;
    }
}

AMDADL::AMDADL()
{
    m_hDLL = 0;
}

AMDADL::~AMDADL()
{
    for (size_t i = 0; i < m_gpus.size(); i++)
        delete[] m_gpus[i];

    if (m_hDLL != 0)
    {
        ADL_Main_Control_Destroy();
        FreeLibrary(m_hDLL);
    }
    //printf("AMD ADL library unloaded successfully.\n");
}

int AMDADL::loadDLL()
{
    //Destroy previously loaded instance.
    if (m_hDLL != 0)
    {
        ADL_Main_Control_Destroy();
        FreeLibrary(m_hDLL);
    }

    // Load the ADL dll
    m_hDLL = LoadLibrary(TEXT("atiadlxx.dll"));
    if (m_hDLL == NULL)
    {
        // A 32 bit calling application on 64 bit OS will fail to LoadLibrary.
        // Try to load the 32 bit library (atiadlxy.dll) instead
        m_hDLL = LoadLibrary(TEXT("atiadlxy.dll"));
    }

    if (NULL == m_hDLL)
    {
        printf("Failed to load ADL library\n");
        return -1;
    }

    ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(m_hDLL,"ADL_Main_Control_Create");
    ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(m_hDLL,"ADL_Main_Control_Destroy");
    ADL_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(m_hDLL,"ADL_Adapter_NumberOfAdapters_Get");
    ADL_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(m_hDLL,"ADL_Adapter_AdapterInfo_Get");
    ADL_AdapterX2_Caps = (ADL_ADAPTERX2_CAPS)GetProcAddress(m_hDLL, "ADL_AdapterX2_Caps");
    ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(m_hDLL, "ADL2_Adapter_Active_Get");
    ADL2_Capabilities_Get = (ADL2_OVERDRIVEN_CAPABILITIES_GET)GetProcAddress(m_hDLL, "ADL2_OverdriveN_Capabilities_Get");
    ADL2_SystemClocks_Get = (ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET)GetProcAddress(m_hDLL, "ADL2_OverdriveN_SystemClocks_Get");
    ADL2_SystemClocks_Set = (ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET)GetProcAddress(m_hDLL, "ADL2_OverdriveN_SystemClocks_Set");
    ADL2_MemoryClocks_Get = (ADL2_OVERDRIVEN_MEMORYCLOCKS_GET)GetProcAddress(m_hDLL, "ADL2_OverdriveN_MemoryClocks_Get");
    ADL2_MemoryClocks_Set = (ADL2_OVERDRIVEN_MEMORYCLOCKS_SET)GetProcAddress(m_hDLL, "ADL2_OverdriveN_MemoryClocks_Set");
    ADL2_PerformanceStatus_Get = (ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET)GetProcAddress(m_hDLL,"ADL2_OverdriveN_PerformanceStatus_Get");
    ADL2_FanControl_Get = (ADL2_OVERDRIVEN_FANCONTROL_GET)GetProcAddress(m_hDLL, "ADL2_OverdriveN_FanControl_Get");
    ADL2_FanControl_Set = (ADL2_OVERDRIVEN_FANCONTROL_SET)GetProcAddress(m_hDLL, "ADL2_OverdriveN_FanControl_Set");
    ADL2_PowerLimit_Get = (ADL2_OVERDRIVEN_POWERLIMIT_GET)GetProcAddress(m_hDLL, "ADL2_OverdriveN_PowerLimit_Get");
    ADL2_PowerLimit_Set = (ADL2_OVERDRIVEN_POWERLIMIT_SET)GetProcAddress(m_hDLL, "ADL2_OverdriveN_PowerLimit_Set");
    ADL2_Temperature_Get = (ADL2_OVERDRIVEN_TEMPERATURE_GET)GetProcAddress(m_hDLL, "ADL2_OverdriveN_Temperature_Get");
    ADL2_Overdrive_Caps = (ADL2_OVERDRIVE_CAPS) GetProcAddress(m_hDLL, "ADL2_Overdrive_Caps");

    if (NULL == ADL_Main_Control_Create || NULL == ADL_Main_Control_Destroy ||
        NULL == ADL_NumberOfAdapters_Get || NULL == ADL2_Overdrive_Caps ||
        NULL == ADL_AdapterInfo_Get || NULL == ADL_AdapterX2_Caps ||
        NULL == ADL2_Adapter_Active_Get || NULL == ADL2_Capabilities_Get ||
        NULL == ADL2_SystemClocks_Get || NULL == ADL2_SystemClocks_Set ||
        NULL == ADL2_MemoryClocks_Get || NULL == ADL2_MemoryClocks_Set ||
        NULL == ADL2_FanControl_Get || NULL == ADL2_FanControl_Set ||
        NULL == ADL2_PerformanceStatus_Get)
    {
        printf("Failed to get ADL function pointers\n");
        return -2;
    }

    int retval = ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1);
    if (retval != ADL_OK)
    {
        printf("Failed to initialize nested ADL2 context");
        return -3;
    }

    //printf("AMD ADL library loaded successfully.\n");

    this->findGPUs();

    return 0;
}

int AMDADL::findGPUs()
{
    int amdCards = 0;
    int numberAdapters = 0;
    LPAdapterInfo info = 0;

    if (m_hDLL == 0)
        return -1;

    // Obtain the number of adapters for the system
    if (ADL_NumberOfAdapters_Get(&numberAdapters) != ADL_OK)
    {
        printf("Cannot get the number of adapters!\n");
        return -2;
    }

    if (numberAdapters <= 0)
    {
        printf("No AMD adapters found!\n");
        return -3;
    }

    info = (LPAdapterInfo)malloc(numberAdapters * sizeof(AdapterInfo));
    memset(info, '\0', numberAdapters * sizeof(AdapterInfo));

    // Get the AdapterInfo structure for all adapters in the system
    ADL_AdapterInfo_Get(info, numberAdapters * sizeof(AdapterInfo));

    //Iterate over iNumberAdapters and find id's of real devices.
    for (int i = 0; i < numberAdapters; i++)
    {
        if (info[i].iVendorID != AMD_VENDOR_ID)
            continue;

        int index = m_gpus.size();
        if (index > 0 && (m_gpus[index - 1]->iBusNumber == info[i].iBusNumber))
            continue;

        amd_adapters* gpu = new amd_adapters;
        gpu->id = i;
        gpu->iAdapterIndex = info[i].iAdapterIndex;
        gpu->iBusNumber = info[i].iBusNumber;

        m_gpus.push_back(gpu);
        amdCards++;

        printf("Found %s: DeviceID: %02d, BusID: %d, UDID: %s\n",
                info[i].strAdapterName, i, info[i].iBusNumber, info[i].strUDID);

        ADL_CONTEXT_HANDLE context = NULL;
        amd_adapters* adapter = m_gpus.at(index);
        int iSupported = -1;
        int iEnabled = -1;
        int iVersion = -1;
        int retval = 0;

        ADL2_Overdrive_Caps(context, adapter->iAdapterIndex, &iSupported, &iEnabled, &iVersion);
        //printf("adapter[%02d]: iSupported is %d, iEnabled is %d, iVersion is %d\n", index, iSupported, iEnabled, iVersion);

        if (iVersion != 7)
        {
            printf("adapter[%d]: Doesn't support v7, Quit!\n", index);
            return -5;
        }

        ADLODNFanControl fanCtrl;
        memset(&fanCtrl, 0, sizeof(ADLODNFanControl));

        retval = ADL2_FanControl_Get(context, gpu->iAdapterIndex, &fanCtrl);
        if (retval != ADL_OK)
        {
            printf("ADL2_FanControl_Get is failed\n");
            return -7;
        }

        ADLODNCapabilities gpuCap;
        memset(&gpuCap, 0, sizeof(ADLODNCapabilities));

        retval = ADL2_Capabilities_Get(context, gpu->iAdapterIndex, &gpuCap);
        if (retval != ADL_OK)
        {
            printf("ADL2_Capabilities_Get is failed\n");
            return -6;
        }

        printf("AMD GPU #%d: CClock Default: %-6d Min: %-6d Max: %-6d\n",
                index, gpuCap.sEngineClockRange.iDefault, gpuCap.sEngineClockRange.iMin, gpuCap.sEngineClockRange.iMax);

        printf("AMD GPU #%d: MClock Default: %-6d Min: %-6d Max: %-6d\n",
                index, gpuCap.sMemoryClockRange.iDefault, gpuCap.sMemoryClockRange.iMin, gpuCap.sMemoryClockRange.iMax);

        printf("AMD GPU #%d:   VDDC Default: %-6d Min: %-6d Max: %-6d\n\n",
                index, gpuCap.svddcRange.iDefault, gpuCap.svddcRange.iMin, gpuCap.svddcRange.iMax);

        printf("AMD GPU #%d:   Fans Current: %-6d Min: %-6d Max: %-6d\n",
                index, fanCtrl.iCurrentFanSpeed, fanCtrl.iMinFanLimit, gpuCap.fanSpeed.iMax);

        printf("AMD GPU #%d: %-6d iCurrentFanSpeed \n", index, fanCtrl.iCurrentFanSpeed);
        printf("AMD GPU #%d: %-6d iCurrentFanSpeedMode \n", index, fanCtrl.iCurrentFanSpeedMode);
        printf("AMD GPU #%d: %-6d iFanControlMode \n", index, fanCtrl.iFanControlMode);
        printf("AMD GPU #%d: %-6d iMinFanLimit \n", index, fanCtrl.iMinFanLimit);
        printf("AMD GPU #%d: %-6d iMinPerformanceClock \n", index, fanCtrl.iMinPerformanceClock);
        printf("AMD GPU #%d: %-6d iMode \n", index, fanCtrl.iMode);
        printf("AMD GPU #%d: %-6d iTargetFanSpeed \n", index, fanCtrl.iTargetFanSpeed);
        printf("AMD GPU #%d: %-6d iTargetTemperature \n", index, fanCtrl.iTargetTemperature);
        //printf("AMD GPU #%d: %-6d powerTuneTemperature \n", index, gpuCap.powerTuneTemperature.iMax);


    }


    ADL_Main_Memory_Free((void**)&info);
    return 0;
}

int AMDADL::setupGPU(int index, int cclk, int mclk, int cvddc, int mvddc, int fanMin, int fanMax, int temp)
{
    int retval = 0;
    int iSupported = -1;
    int iEnabled = -1;
    int iVersion = -1;
    ADL_CONTEXT_HANDLE context = NULL;

    if (index >= (int)m_gpus.size())
    {
        printf("Adapter index specified is %d, but %u AMD GPU's found!\n", index, (uint32_t)m_gpus.size());
        return -4;
    }
    amd_adapters* gpu = m_gpus.at(index);

    ADL2_Overdrive_Caps(context, gpu->iAdapterIndex, &iSupported, &iEnabled, &iVersion);
    if (iVersion != 7)
    {
        printf("adapter[%d]: Doesn't support v7, Quit!\n", index);
        return -5;
    }

    ADLODNCapabilities gpuCap;
    memset(&gpuCap, 0, sizeof(ADLODNCapabilities));

    retval = ADL2_Capabilities_Get(context, gpu->iAdapterIndex, &gpuCap);
    if (retval != ADL_OK)
    {
        printf("ADL2_Capabilities_Get is failed\n");
        return -6;
    }

    ADLODNFanControl fanCtrl;
    memset(&fanCtrl, 0, sizeof(ADLODNFanControl));

    retval = ADL2_FanControl_Get(context, gpu->iAdapterIndex, &fanCtrl);
    if (retval != ADL_OK)
    {
        printf("ADL2_FanControl_Get is failed\n");
        return -7;
    }

    if (fanMin > fanMax)
        fanMin = fanMax - 100;

    fanCtrl.iMode = ADLODNControlType::ODNControlType_Manual;
    fanCtrl.iFanControlMode = ADLODNControlType::ODNControlType_Manual;
    fanCtrl.iMinFanLimit = (int)((fanMin * gpuCap.fanSpeed.iMax) / 100);
    fanCtrl.iTargetFanSpeed = (int)((fanMax * gpuCap.fanSpeed.iMax) / 100);
    fanCtrl.iTargetTemperature = temp;



    retval = ADL2_FanControl_Set(context, gpu->iAdapterIndex, &fanCtrl);
    if (retval != ADL_OK)
    {
        printf("ADL2_FanControl_Set is failed\n");
        return -8;
    }

    retval = ADL2_FanControl_Get(context, gpu->iAdapterIndex, &fanCtrl);
    if (retval != ADL_OK)
    {
        printf("ADL2_FanControl_Get is failed\n");
        return -9;
    }

    //Get Core Clocks and Voltages.
    ADLODNPerformanceLevels* corePerfLevels;
    int perfLevels = gpuCap.iMaximumNumberOfPerformanceLevels;

    int size = sizeof(ADLODNPerformanceLevels) + sizeof(ADLODNPerformanceLevel)* (perfLevels - 1);
    void* corePerfLevelsBuffer = new char[size];
    memset(corePerfLevelsBuffer, 0, size);
    corePerfLevels = (ADLODNPerformanceLevels*)corePerfLevelsBuffer;
    corePerfLevels->iSize = size;
    corePerfLevels->iNumberOfPerformanceLevels = perfLevels;

    retval = ADL2_SystemClocks_Get(context, gpu->iAdapterIndex, corePerfLevels);
    if (retval != ADL_OK)
    {
        printf("ADL2_SystemClocks_Set is failed\n");
        return -10;
    }

    //Get Memory Clocks and Voltages.
    ADLODNPerformanceLevels* memPerfLevels;
    int memLevesSize = sizeof(ADLODNPerformanceLevels) + sizeof(ADLODNPerformanceLevel)* (perfLevels - 1);
    void* memPerfLevelsBuffer = new char[memLevesSize];
    memset(memPerfLevelsBuffer, 0, memLevesSize);
    memPerfLevels = (ADLODNPerformanceLevels*)memPerfLevelsBuffer;
    memPerfLevels->iSize = memLevesSize;
    memPerfLevels->iMode = 0; //current
    memPerfLevels->iNumberOfPerformanceLevels = gpuCap.iMaximumNumberOfPerformanceLevels;

    retval = ADL2_MemoryClocks_Get(context, gpu->iAdapterIndex, memPerfLevels);
    if (retval != ADL_OK)
    {
        printf("ADL2_MemoryClocks_Get is failed\n");
        return -11;
    }

    if (cclk < gpuCap.sEngineClockRange.iMin || cclk > gpuCap.sEngineClockRange.iMax)
    {
        printf("AMD GPU #%d: cclock of %d not in range (%d to %d).\n",
                index, cclk, gpuCap.sEngineClockRange.iMin, gpuCap.sEngineClockRange.iMax);
        return -12;
    }

    if (mclk < gpuCap.sMemoryClockRange.iMin || mclk > gpuCap.sMemoryClockRange.iMax)
    {
        printf("AMD GPU #%d: mclock of %d not in range (%d to %d).\n",
                index, mclk, gpuCap.sMemoryClockRange.iMin, gpuCap.sMemoryClockRange.iMax);
        return -13;
    }

    if (cvddc < gpuCap.svddcRange.iMin || cvddc > gpuCap.svddcRange.iMax)
    {
        printf("AMD GPU #%d: cvddc of %d not in range (%d to %d).\n",
                index, cvddc, gpuCap.svddcRange.iMin, gpuCap.svddcRange.iMax);
        return -14;
    }

    corePerfLevels->iMode = ADLODNControlType::ODNControlType_Manual;
    for (int i = 1; i < perfLevels; i++)
    {
        corePerfLevels->aLevels[i].iEnabled = 1;
        corePerfLevels->aLevels[i].iClock = cclk;
        corePerfLevels->aLevels[i].iVddc = cvddc;
    }

    retval = ADL2_SystemClocks_Set(context, gpu->iAdapterIndex, corePerfLevels);
    if (retval != ADL_OK)
    {
        printf("ADL2_SystemClocks_Set is failed\n");
        return -15;
    }

    memPerfLevels->iMode = ADLODNControlType::ODNControlType_Manual;
    for (int i = 1; i < perfLevels; i++)
    {
        memPerfLevels->aLevels[i].iClock = mclk;
        memPerfLevels->aLevels[i].iVddc = mvddc;
    }

    retval = ADL2_MemoryClocks_Set(context, gpu->iAdapterIndex, memPerfLevels);
    if (retval != ADL_OK)
    {
        printf("ADL2_MemoryClocks_Set is failed\n");
        return -16;
    }

    retval = ADL2_SystemClocks_Get(context, gpu->iAdapterIndex, corePerfLevels);
    if (retval != ADL_OK)
    {
        printf("ADL2_OverdriveN_GPUClocks_Get is failed\n");
        return -17;
    }

    retval = ADL2_MemoryClocks_Get(context, gpu->iAdapterIndex, memPerfLevels);
    if (retval != ADL_OK)
    {
        printf("ADL2_MemoryClocks_Get is failed\n");
        return -18;
    }

    printf("\nAMD GPU #%d:\n", index);
    printf("Enabled: ");
    for (int i = 0; i < perfLevels; i++)
        printf("%-6d  ", corePerfLevels->aLevels[i].iEnabled);
    printf("\n");

    printf("CClock : ");
    for (int i = 0; i < perfLevels; i++)
        printf("%-6d  ", corePerfLevels->aLevels[i].iClock);
    printf("\n");

    printf("MClock : ");
    for (int i = 0; i < perfLevels; i++)
        printf("%-6d  ", memPerfLevels->aLevels[i].iClock);
    printf("\n");

    printf("CVddc  : ");
    for (int i = 0; i < perfLevels; i++)
        printf("%-6d  ", corePerfLevels->aLevels[i].iVddc);
    printf("\n");

    printf("MVddc  : ");
    for (int i = 0; i < perfLevels; i++)
        printf("%-6d  ", memPerfLevels->aLevels[i].iVddc);
    printf("\n");

    printf("Fans   : %dRPM (Max %dRPM)\n", fanCtrl.iMinFanLimit, gpuCap.fanSpeed.iMax);

    return 0;
}
