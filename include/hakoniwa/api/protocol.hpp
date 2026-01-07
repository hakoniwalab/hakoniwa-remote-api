#pragma once

#include <cstdint>
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

struct HakoSimulationStateInfo
{
    HakoSimulationState sim_state;
    int64_t master_time;
    bool is_pdu_created;
    bool is_simulation_mode;
    bool is_pdu_sync_mode;
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
