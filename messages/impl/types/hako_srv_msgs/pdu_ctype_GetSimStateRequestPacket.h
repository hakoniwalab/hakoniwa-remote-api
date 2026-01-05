#ifndef _pdu_ctype_hako_srv_msgs_GetSimStateRequestPacket_H_
#define _pdu_ctype_hako_srv_msgs_GetSimStateRequestPacket_H_

#include "pdu_primitive_ctypes.h"
#include "hako_srv_msgs/pdu_ctype_GetSimStateRequest.h"
#include "hako_srv_msgs/pdu_ctype_ServiceRequestHeader.h"

typedef struct {
        Hako_ServiceRequestHeader header;
        Hako_GetSimStateRequest body;
} Hako_GetSimStateRequestPacket;

#endif /* _pdu_ctype_hako_srv_msgs_GetSimStateRequestPacket_H_ */
