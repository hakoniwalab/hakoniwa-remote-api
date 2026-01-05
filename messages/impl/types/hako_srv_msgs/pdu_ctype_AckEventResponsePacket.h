#ifndef _pdu_ctype_hako_srv_msgs_AckEventResponsePacket_H_
#define _pdu_ctype_hako_srv_msgs_AckEventResponsePacket_H_

#include "pdu_primitive_ctypes.h"
#include "hako_srv_msgs/pdu_ctype_AckEventResponse.h"
#include "hako_srv_msgs/pdu_ctype_ServiceResponseHeader.h"

typedef struct {
        Hako_ServiceResponseHeader header;
        Hako_AckEventResponse body;
} Hako_AckEventResponsePacket;

#endif /* _pdu_ctype_hako_srv_msgs_AckEventResponsePacket_H_ */
