#ifndef _pdu_cpptype_hako_srv_msgs_GetSimStateResponse_HPP_
#define _pdu_cpptype_hako_srv_msgs_GetSimStateResponse_HPP_

#include "pdu_primitive_ctypes.h"
#include <vector>
#include <array>

typedef struct {
        Hako_uint32 sim_state;
        Hako_int64 master_time;
        Hako_bool is_pdu_created;
        Hako_bool is_simulation_mode;
        Hako_bool is_pdu_sync_mode;
} HakoCpp_GetSimStateResponse;

#endif /* _pdu_cpptype_hako_srv_msgs_GetSimStateResponse_HPP_ */
