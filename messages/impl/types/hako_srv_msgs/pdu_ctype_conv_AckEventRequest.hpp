#ifndef _PDU_CTYPE_CONV_HAKO_hako_srv_msgs_AckEventRequest_HPP_
#define _PDU_CTYPE_CONV_HAKO_hako_srv_msgs_AckEventRequest_HPP_

#include "pdu_primitive_ctypes.h"
#include "ros_primitive_types.hpp"
#include "pdu_primitive_ctypes_conv.hpp"
#include "pdu_dynamic_memory.hpp"
/*
 * Dependent pdu data
 */
#include "hako_srv_msgs/pdu_ctype_AckEventRequest.h"
/*
 * Dependent ros data
 */
#include "hako_srv_msgs/msg/ack_event_request.hpp"

/*
 * Dependent Convertors
 */

/***************************
 *
 * PDU ==> ROS2
 *
 ***************************/

static inline int _pdu2ros_AckEventRequest(const char* heap_ptr, Hako_AckEventRequest &src, hako_srv_msgs::msg::AckEventRequest &dst)
{
    // string convertor
    dst.name = (const char*)src.name;
    // primitive convert
    hako_convert_pdu2ros(src.event_code, dst.event_code);
    // primitive convert
    hako_convert_pdu2ros(src.result_code, dst.result_code);
    (void)heap_ptr;
    return 0;
}

static inline int hako_convert_pdu2ros_AckEventRequest(Hako_AckEventRequest &src, hako_srv_msgs::msg::AckEventRequest &dst)
{
    void* base_ptr = (void*)&src;
    void* heap_ptr = hako_get_heap_ptr_pdu(base_ptr);
    // Validate magic number and version
    if (heap_ptr == nullptr) {
        return -1; // Invalid PDU metadata
    }
    else {
        return _pdu2ros_AckEventRequest((char*)heap_ptr, src, dst);
    }
}

/***************************
 *
 * ROS2 ==> PDU
 *
 ***************************/

static inline bool _ros2pdu_AckEventRequest(hako_srv_msgs::msg::AckEventRequest &src, Hako_AckEventRequest &dst, PduDynamicMemory &dynamic_memory)
{
    try {
        // string convertor
        (void)hako_convert_ros2pdu_array(
            src.name, src.name.length(),
            dst.name, M_ARRAY_SIZE(Hako_AckEventRequest, char, name));
        dst.name[src.name.length()] = '\0';
        // primitive convert
        hako_convert_ros2pdu(src.event_code, dst.event_code);
        // primitive convert
        hako_convert_ros2pdu(src.result_code, dst.result_code);
    } catch (const std::runtime_error& e) {
        std::cerr << "convertor error: " << e.what() << std::endl;
        return false;
    }
    (void)dynamic_memory;
    return true;
}

static inline int hako_convert_ros2pdu_AckEventRequest(hako_srv_msgs::msg::AckEventRequest &src, Hako_AckEventRequest** dst)
{
    PduDynamicMemory dynamic_memory;
    Hako_AckEventRequest out;
    if (!_ros2pdu_AckEventRequest(src, out, dynamic_memory)) {
        return -1;
    }
    int heap_size = dynamic_memory.get_total_size();
    void* base_ptr = hako_create_empty_pdu(sizeof(Hako_AckEventRequest), heap_size);
    if (base_ptr == nullptr) {
        return -1;
    }
    // Copy out on base data
    memcpy(base_ptr, (void*)&out, sizeof(Hako_AckEventRequest));

    // Copy dynamic part and set offsets
    void* heap_ptr = hako_get_heap_ptr_pdu(base_ptr);
    dynamic_memory.copy_to_pdu((char*)heap_ptr);

    *dst = (Hako_AckEventRequest*)base_ptr;
    return hako_get_pdu_meta_data(base_ptr)->total_size;
}

static inline Hako_AckEventRequest* hako_create_empty_pdu_AckEventRequest(int heap_size)
{
    // Allocate PDU memory
    char* base_ptr = (char*)hako_create_empty_pdu(sizeof(Hako_AckEventRequest), heap_size);
    if (base_ptr == nullptr) {
        return nullptr;
    }
    return (Hako_AckEventRequest*)base_ptr;
}
#endif /* _PDU_CTYPE_CONV_HAKO_hako_srv_msgs_AckEventRequest_HPP_ */
