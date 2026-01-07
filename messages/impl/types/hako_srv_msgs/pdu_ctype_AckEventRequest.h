#ifndef _pdu_ctype_hako_srv_msgs_AckEventRequest_H_
#define _pdu_ctype_hako_srv_msgs_AckEventRequest_H_

#include "pdu_primitive_ctypes.h"

typedef struct {
        char name[HAKO_STRING_SIZE];
        Hako_uint32 event_code;
        Hako_uint32 result_code;
} Hako_AckEventRequest;

#endif /* _pdu_ctype_hako_srv_msgs_AckEventRequest_H_ */
