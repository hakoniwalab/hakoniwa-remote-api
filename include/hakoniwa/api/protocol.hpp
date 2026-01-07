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

}

