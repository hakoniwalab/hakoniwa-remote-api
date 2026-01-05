#ifndef _pdu_ctype_hako_srv_msgs_GetEventRequest_H_
#define _pdu_ctype_hako_srv_msgs_GetEventRequest_H_

#include "pdu_primitive_ctypes.h"

typedef struct {
        char name[HAKO_STRING_SIZE];
} Hako_GetEventRequest;

#endif /* _pdu_ctype_hako_srv_msgs_GetEventRequest_H_ */
