#ifndef _pdu_cpptype_hako_srv_msgs_AckEventResponsePacket_HPP_
#define _pdu_cpptype_hako_srv_msgs_AckEventResponsePacket_HPP_

#include "pdu_primitive_ctypes.h"
#include <vector>
#include <array>
#include "hako_srv_msgs/pdu_cpptype_AckEventResponse.hpp"
#include "hako_srv_msgs/pdu_cpptype_ServiceResponseHeader.hpp"

typedef struct {
        HakoCpp_ServiceResponseHeader header;
        HakoCpp_AckEventResponse body;
} HakoCpp_AckEventResponsePacket;

#endif /* _pdu_cpptype_hako_srv_msgs_AckEventResponsePacket_HPP_ */
