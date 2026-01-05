#ifndef _pdu_ctype_hako_srv_msgs_JoinResponsePacket_H_
#define _pdu_ctype_hako_srv_msgs_JoinResponsePacket_H_

#include "pdu_primitive_ctypes.h"
#include "hako_srv_msgs/pdu_ctype_JoinResponse.h"
#include "hako_srv_msgs/pdu_ctype_ServiceResponseHeader.h"

typedef struct {
        Hako_ServiceResponseHeader header;
        Hako_JoinResponse body;
} Hako_JoinResponsePacket;

#endif /* _pdu_ctype_hako_srv_msgs_JoinResponsePacket_H_ */
