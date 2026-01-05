#ifndef _pdu_ctype_hako_srv_msgs_JoinResponse_H_
#define _pdu_ctype_hako_srv_msgs_JoinResponse_H_

#include "pdu_primitive_ctypes.h"

typedef struct {
        Hako_uint32 status_code;
        char message[HAKO_STRING_SIZE];
} Hako_JoinResponse;

#endif /* _pdu_ctype_hako_srv_msgs_JoinResponse_H_ */
