#ifndef _pdu_ctype_hako_srv_msgs_SimControlResponse_H_
#define _pdu_ctype_hako_srv_msgs_SimControlResponse_H_

#include "pdu_primitive_ctypes.h"

typedef struct {
        Hako_uint32 status_code;
        char message[HAKO_STRING_SIZE];
} Hako_SimControlResponse;

#endif /* _pdu_ctype_hako_srv_msgs_SimControlResponse_H_ */
