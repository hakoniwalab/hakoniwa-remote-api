#ifndef _pdu_ctype_hako_srv_msgs_SimControlRequest_H_
#define _pdu_ctype_hako_srv_msgs_SimControlRequest_H_

#include "pdu_primitive_ctypes.h"

typedef struct {
        char name[HAKO_STRING_SIZE];
        Hako_uint32 op;
} Hako_SimControlRequest;

#endif /* _pdu_ctype_hako_srv_msgs_SimControlRequest_H_ */
