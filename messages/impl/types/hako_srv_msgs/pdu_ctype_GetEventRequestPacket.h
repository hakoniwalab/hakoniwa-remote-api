#ifndef _pdu_ctype_hako_srv_msgs_GetEventRequestPacket_H_
#define _pdu_ctype_hako_srv_msgs_GetEventRequestPacket_H_

#include "pdu_primitive_ctypes.h"
#include "hako_srv_msgs/pdu_ctype_GetEventRequest.h"
#include "hako_srv_msgs/pdu_ctype_ServiceRequestHeader.h"

typedef struct {
        Hako_ServiceRequestHeader header;
        Hako_GetEventRequest body;
} Hako_GetEventRequestPacket;

#endif /* _pdu_ctype_hako_srv_msgs_GetEventRequestPacket_H_ */
