#ifndef _pdu_ctype_hako_srv_msgs_AckEventRequestPacket_H_
#define _pdu_ctype_hako_srv_msgs_AckEventRequestPacket_H_

#include "pdu_primitive_ctypes.h"
#include "hako_srv_msgs/pdu_ctype_AckEventRequest.h"
#include "hako_srv_msgs/pdu_ctype_ServiceRequestHeader.h"

typedef struct {
        Hako_ServiceRequestHeader header;
        Hako_AckEventRequest body;
} Hako_AckEventRequestPacket;

#endif /* _pdu_ctype_hako_srv_msgs_AckEventRequestPacket_H_ */
