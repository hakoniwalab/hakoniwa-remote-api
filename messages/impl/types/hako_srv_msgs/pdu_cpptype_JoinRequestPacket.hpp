#ifndef _pdu_cpptype_hako_srv_msgs_JoinRequestPacket_HPP_
#define _pdu_cpptype_hako_srv_msgs_JoinRequestPacket_HPP_

#include "pdu_primitive_ctypes.h"
#include <vector>
#include <array>
#include "hako_srv_msgs/pdu_cpptype_JoinRequest.hpp"
#include "hako_srv_msgs/pdu_cpptype_ServiceRequestHeader.hpp"

typedef struct {
        HakoCpp_ServiceRequestHeader header;
        HakoCpp_JoinRequest body;
} HakoCpp_JoinRequestPacket;

#endif /* _pdu_cpptype_hako_srv_msgs_JoinRequestPacket_HPP_ */
