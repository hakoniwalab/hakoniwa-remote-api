#ifndef _pdu_cpptype_hako_srv_msgs_SimControlResponsePacket_HPP_
#define _pdu_cpptype_hako_srv_msgs_SimControlResponsePacket_HPP_

#include "pdu_primitive_ctypes.h"
#include <vector>
#include <array>
#include "hako_srv_msgs/pdu_cpptype_ServiceResponseHeader.hpp"
#include "hako_srv_msgs/pdu_cpptype_SimControlResponse.hpp"

typedef struct {
        HakoCpp_ServiceResponseHeader header;
        HakoCpp_SimControlResponse body;
} HakoCpp_SimControlResponsePacket;

#endif /* _pdu_cpptype_hako_srv_msgs_SimControlResponsePacket_HPP_ */
