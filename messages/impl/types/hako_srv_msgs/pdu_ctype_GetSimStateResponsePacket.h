#ifndef _pdu_ctype_hako_srv_msgs_GetSimStateResponsePacket_H_
#define _pdu_ctype_hako_srv_msgs_GetSimStateResponsePacket_H_

#include "pdu_primitive_ctypes.h"
#include "hako_srv_msgs/pdu_ctype_GetSimStateResponse.h"
#include "hako_srv_msgs/pdu_ctype_ServiceResponseHeader.h"

typedef struct {
        Hako_ServiceResponseHeader header;
        Hako_GetSimStateResponse body;
} Hako_GetSimStateResponsePacket;

#endif /* _pdu_ctype_hako_srv_msgs_GetSimStateResponsePacket_H_ */
