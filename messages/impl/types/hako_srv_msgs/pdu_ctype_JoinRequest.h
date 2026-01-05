#ifndef _pdu_ctype_hako_srv_msgs_JoinRequest_H_
#define _pdu_ctype_hako_srv_msgs_JoinRequest_H_

#include "pdu_primitive_ctypes.h"

typedef struct {
        char name[HAKO_STRING_SIZE];
} Hako_JoinRequest;

#endif /* _pdu_ctype_hako_srv_msgs_JoinRequest_H_ */
