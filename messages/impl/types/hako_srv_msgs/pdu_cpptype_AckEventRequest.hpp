#ifndef _pdu_cpptype_hako_srv_msgs_AckEventRequest_HPP_
#define _pdu_cpptype_hako_srv_msgs_AckEventRequest_HPP_

#include "pdu_primitive_ctypes.h"
#include <vector>
#include <array>

typedef struct {
        std::string name;
        Hako_uint32 event_code;
        Hako_uint32 result_code;
} HakoCpp_AckEventRequest;

#endif /* _pdu_cpptype_hako_srv_msgs_AckEventRequest_HPP_ */
