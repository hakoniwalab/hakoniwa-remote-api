#ifndef _pdu_ctype_hako_srv_msgs_SimControlResponsePacket_H_
#define _pdu_ctype_hako_srv_msgs_SimControlResponsePacket_H_

#include "pdu_primitive_ctypes.h"
#include "hako_srv_msgs/pdu_ctype_ServiceResponseHeader.h"
#include "hako_srv_msgs/pdu_ctype_SimControlResponse.h"

typedef struct {
        Hako_ServiceResponseHeader header;
        Hako_SimControlResponse body;
} Hako_SimControlResponsePacket;

#endif /* _pdu_ctype_hako_srv_msgs_SimControlResponsePacket_H_ */
