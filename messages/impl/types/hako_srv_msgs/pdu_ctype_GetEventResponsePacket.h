#ifndef _pdu_ctype_hako_srv_msgs_GetEventResponsePacket_H_
#define _pdu_ctype_hako_srv_msgs_GetEventResponsePacket_H_

#include "pdu_primitive_ctypes.h"
#include "hako_srv_msgs/pdu_ctype_GetEventResponse.h"
#include "hako_srv_msgs/pdu_ctype_ServiceResponseHeader.h"

typedef struct {
        Hako_ServiceResponseHeader header;
        Hako_GetEventResponse body;
} Hako_GetEventResponsePacket;

#endif /* _pdu_ctype_hako_srv_msgs_GetEventResponsePacket_H_ */
