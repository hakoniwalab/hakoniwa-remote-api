#ifndef _pdu_cpptype_hako_srv_msgs_AckEventRequestPacket_HPP_
#define _pdu_cpptype_hako_srv_msgs_AckEventRequestPacket_HPP_

#include "pdu_primitive_ctypes.h"
#include <vector>
#include <array>
#include "hako_srv_msgs/pdu_cpptype_AckEventRequest.hpp"
#include "hako_srv_msgs/pdu_cpptype_ServiceRequestHeader.hpp"

typedef struct {
        HakoCpp_ServiceRequestHeader header;
        HakoCpp_AckEventRequest body;
} HakoCpp_AckEventRequestPacket;

#endif /* _pdu_cpptype_hako_srv_msgs_AckEventRequestPacket_HPP_ */
