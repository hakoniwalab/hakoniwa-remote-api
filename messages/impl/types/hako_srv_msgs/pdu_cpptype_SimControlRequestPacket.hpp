#ifndef _pdu_cpptype_hako_srv_msgs_SimControlRequestPacket_HPP_
#define _pdu_cpptype_hako_srv_msgs_SimControlRequestPacket_HPP_

#include "pdu_primitive_ctypes.h"
#include <vector>
#include <array>
#include "hako_srv_msgs/pdu_cpptype_ServiceRequestHeader.hpp"
#include "hako_srv_msgs/pdu_cpptype_SimControlRequest.hpp"

typedef struct {
        HakoCpp_ServiceRequestHeader header;
        HakoCpp_SimControlRequest body;
} HakoCpp_SimControlRequestPacket;

#endif /* _pdu_cpptype_hako_srv_msgs_SimControlRequestPacket_HPP_ */
