#ifndef _pdu_cpptype_hako_srv_msgs_GetSimStateRequestPacket_HPP_
#define _pdu_cpptype_hako_srv_msgs_GetSimStateRequestPacket_HPP_

#include "pdu_primitive_ctypes.h"
#include <vector>
#include <array>
#include "hako_srv_msgs/pdu_cpptype_GetSimStateRequest.hpp"
#include "hako_srv_msgs/pdu_cpptype_ServiceRequestHeader.hpp"

typedef struct {
        HakoCpp_ServiceRequestHeader header;
        HakoCpp_GetSimStateRequest body;
} HakoCpp_GetSimStateRequestPacket;

#endif /* _pdu_cpptype_hako_srv_msgs_GetSimStateRequestPacket_HPP_ */
