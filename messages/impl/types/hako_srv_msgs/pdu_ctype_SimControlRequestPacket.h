#ifndef _pdu_ctype_hako_srv_msgs_SimControlRequestPacket_H_
#define _pdu_ctype_hako_srv_msgs_SimControlRequestPacket_H_

#include "pdu_primitive_ctypes.h"
#include "hako_srv_msgs/pdu_ctype_ServiceRequestHeader.h"
#include "hako_srv_msgs/pdu_ctype_SimControlRequest.h"

typedef struct {
        Hako_ServiceRequestHeader header;
        Hako_SimControlRequest body;
} Hako_SimControlRequestPacket;

#endif /* _pdu_ctype_hako_srv_msgs_SimControlRequestPacket_H_ */
