#ifndef _PDU_CTYPE_CONV_HAKO_hako_srv_msgs_JoinResponse_HPP_
#define _PDU_CTYPE_CONV_HAKO_hako_srv_msgs_JoinResponse_HPP_

#include "pdu_primitive_ctypes.h"
#include "ros_primitive_types.hpp"
#include "pdu_primitive_ctypes_conv.hpp"
#include "pdu_dynamic_memory.hpp"
/*
 * Dependent pdu data
 */
#include "hako_srv_msgs/pdu_ctype_JoinResponse.h"
/*
 * Dependent ros data
 */
#include "hako_srv_msgs/msg/join_response.hpp"

/*
 * Dependent Convertors
 */

/***************************
 *
 * PDU ==> ROS2
 *
 ***************************/

static inline int _pdu2ros_JoinResponse(const char* heap_ptr, Hako_JoinResponse &src, hako_srv_msgs::msg::JoinResponse &dst)
{
    // primitive convert
    hako_convert_pdu2ros(src.status_code, dst.status_code);
    // string convertor
    dst.message = (const char*)src.message;
    (void)heap_ptr;
    return 0;
}

static inline int hako_convert_pdu2ros_JoinResponse(Hako_JoinResponse &src, hako_srv_msgs::msg::JoinResponse &dst)
{
    void* base_ptr = (void*)&src;
    void* heap_ptr = hako_get_heap_ptr_pdu(base_ptr);
    // Validate magic number and version
    if (heap_ptr == nullptr) {
        return -1; // Invalid PDU metadata
    }
    else {
        return _pdu2ros_JoinResponse((char*)heap_ptr, src, dst);
    }
}

/***************************
 *
 * ROS2 ==> PDU
 *
 ***************************/

static inline bool _ros2pdu_JoinResponse(hako_srv_msgs::msg::JoinResponse &src, Hako_JoinResponse &dst, PduDynamicMemory &dynamic_memory)
{
    try {
        // primitive convert
        hako_convert_ros2pdu(src.status_code, dst.status_code);
        // string convertor
        (void)hako_convert_ros2pdu_array(
            src.message, src.message.length(),
            dst.message, M_ARRAY_SIZE(Hako_JoinResponse, char, message));
        dst.message[src.message.length()] = '\0';
    } catch (const std::runtime_error& e) {
        std::cerr << "convertor error: " << e.what() << std::endl;
        return false;
    }
    (void)dynamic_memory;
    return true;
}

static inline int hako_convert_ros2pdu_JoinResponse(hako_srv_msgs::msg::JoinResponse &src, Hako_JoinResponse** dst)
{
    PduDynamicMemory dynamic_memory;
    Hako_JoinResponse out;
    if (!_ros2pdu_JoinResponse(src, out, dynamic_memory)) {
        return -1;
    }
    int heap_size = dynamic_memory.get_total_size();
    void* base_ptr = hako_create_empty_pdu(sizeof(Hako_JoinResponse), heap_size);
    if (base_ptr == nullptr) {
        return -1;
    }
    // Copy out on base data
    memcpy(base_ptr, (void*)&out, sizeof(Hako_JoinResponse));

    // Copy dynamic part and set offsets
    void* heap_ptr = hako_get_heap_ptr_pdu(base_ptr);
    dynamic_memory.copy_to_pdu((char*)heap_ptr);

    *dst = (Hako_JoinResponse*)base_ptr;
    return hako_get_pdu_meta_data(base_ptr)->total_size;
}

static inline Hako_JoinResponse* hako_create_empty_pdu_JoinResponse(int heap_size)
{
    // Allocate PDU memory
    char* base_ptr = (char*)hako_create_empty_pdu(sizeof(Hako_JoinResponse), heap_size);
    if (base_ptr == nullptr) {
        return nullptr;
    }
    return (Hako_JoinResponse*)base_ptr;
}
#endif /* _PDU_CTYPE_CONV_HAKO_hako_srv_msgs_JoinResponse_HPP_ */
