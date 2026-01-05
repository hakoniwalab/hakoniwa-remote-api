#ifndef _pdu_cpptype_hako_srv_msgs_GetEventRequestPacket_HPP_
#define _pdu_cpptype_hako_srv_msgs_GetEventRequestPacket_HPP_

#include "pdu_primitive_ctypes.h"
#include <vector>
#include <array>
#include "hako_srv_msgs/pdu_cpptype_GetEventRequest.hpp"
#include "hako_srv_msgs/pdu_cpptype_ServiceRequestHeader.hpp"

typedef struct {
        HakoCpp_ServiceRequestHeader header;
        HakoCpp_GetEventRequest body;
} HakoCpp_GetEventRequestPacket;

#endif /* _pdu_cpptype_hako_srv_msgs_GetEventRequestPacket_HPP_ */
