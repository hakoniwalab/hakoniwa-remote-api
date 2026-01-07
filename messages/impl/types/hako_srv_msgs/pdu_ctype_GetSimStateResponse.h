#ifndef _pdu_ctype_hako_srv_msgs_GetSimStateResponse_H_
#define _pdu_ctype_hako_srv_msgs_GetSimStateResponse_H_

#include "pdu_primitive_ctypes.h"

typedef struct {
        Hako_uint32 sim_state;
        Hako_int64 master_time;
        Hako_bool is_pdu_created;
        Hako_bool is_simulation_mode;
        Hako_bool is_pdu_sync_mode;
} Hako_GetSimStateResponse;

#endif /* _pdu_ctype_hako_srv_msgs_GetSimStateResponse_H_ */
