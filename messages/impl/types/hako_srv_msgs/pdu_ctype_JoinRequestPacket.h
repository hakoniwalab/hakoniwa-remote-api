#ifndef _pdu_ctype_hako_srv_msgs_JoinRequestPacket_H_
#define _pdu_ctype_hako_srv_msgs_JoinRequestPacket_H_

#include "pdu_primitive_ctypes.h"
#include "hako_srv_msgs/pdu_ctype_JoinRequest.h"
#include "hako_srv_msgs/pdu_ctype_ServiceRequestHeader.h"

typedef struct {
        Hako_ServiceRequestHeader header;
        Hako_JoinRequest body;
} Hako_JoinRequestPacket;

#endif /* _pdu_ctype_hako_srv_msgs_JoinRequestPacket_H_ */
