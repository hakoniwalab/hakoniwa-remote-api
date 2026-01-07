#pragma once

namespace hakoniwa::api {

enum class HakoSimulationState
{
    HakoSim_Stopped = 0,
    HakoSim_Runnable,
    HakoSim_Running,
    HakoSim_Stopping,
    HakoSim_Resetting,
    HakoSim_Error,
    HakoSim_Terminated,
    HakoSim_Any,
    HakoSim_Count
};

enum class HakoSimulationControlCommand
{
    HakoSimControl_Start = 0,
    HakoSimControl_Stop,
    HakoSimControl_Reset,
    HakoSimControl_Count

};
enum class HakoSimulationAssetEvent
{
    HakoSimAssetEvent_None = 0,
    HakoSimAssetEvent_Start,
    HakoSimAssetEvent_Stop,
    HakoSimAssetEvent_Reset,
    HakoSimAssetEvent_Error,
    HakoSimAssetEvent_Count
};
}
