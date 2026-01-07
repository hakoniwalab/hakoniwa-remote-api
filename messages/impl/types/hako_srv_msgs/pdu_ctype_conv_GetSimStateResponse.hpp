#ifndef _PDU_CTYPE_CONV_HAKO_hako_srv_msgs_GetSimStateResponse_HPP_
#define _PDU_CTYPE_CONV_HAKO_hako_srv_msgs_GetSimStateResponse_HPP_

#include "pdu_primitive_ctypes.h"
#include "ros_primitive_types.hpp"
#include "pdu_primitive_ctypes_conv.hpp"
#include "pdu_dynamic_memory.hpp"
/*
 * Dependent pdu data
 */
#include "hako_srv_msgs/pdu_ctype_GetSimStateResponse.h"
/*
 * Dependent ros data
 */
#include "hako_srv_msgs/msg/get_sim_state_response.hpp"

/*
 * Dependent Convertors
 */

/***************************
 *
 * PDU ==> ROS2
 *
 ***************************/

static inline int _pdu2ros_GetSimStateResponse(const char* heap_ptr, Hako_GetSimStateResponse &src, hako_srv_msgs::msg::GetSimStateResponse &dst)
{
    // primitive convert
    hako_convert_pdu2ros(src.sim_state, dst.sim_state);
    // primitive convert
    hako_convert_pdu2ros(src.master_time, dst.master_time);
    // primitive convert
    hako_convert_pdu2ros(src.is_pdu_created, dst.is_pdu_created);
    // primitive convert
    hako_convert_pdu2ros(src.is_simulation_mode, dst.is_simulation_mode);
    // primitive convert
    hako_convert_pdu2ros(src.is_pdu_sync_mode, dst.is_pdu_sync_mode);
    (void)heap_ptr;
    return 0;
}

static inline int hako_convert_pdu2ros_GetSimStateResponse(Hako_GetSimStateResponse &src, hako_srv_msgs::msg::GetSimStateResponse &dst)
{
    void* base_ptr = (void*)&src;
    void* heap_ptr = hako_get_heap_ptr_pdu(base_ptr);
    // Validate magic number and version
    if (heap_ptr == nullptr) {
        return -1; // Invalid PDU metadata
    }
    else {
        return _pdu2ros_GetSimStateResponse((char*)heap_ptr, src, dst);
    }
}

/***************************
 *
 * ROS2 ==> PDU
 *
 ***************************/

static inline bool _ros2pdu_GetSimStateResponse(hako_srv_msgs::msg::GetSimStateResponse &src, Hako_GetSimStateResponse &dst, PduDynamicMemory &dynamic_memory)
{
    try {
        // primitive convert
        hako_convert_ros2pdu(src.sim_state, dst.sim_state);
        // primitive convert
        hako_convert_ros2pdu(src.master_time, dst.master_time);
        // primitive convert
        hako_convert_ros2pdu(src.is_pdu_created, dst.is_pdu_created);
        // primitive convert
        hako_convert_ros2pdu(src.is_simulation_mode, dst.is_simulation_mode);
        // primitive convert
        hako_convert_ros2pdu(src.is_pdu_sync_mode, dst.is_pdu_sync_mode);
    } catch (const std::runtime_error& e) {
        std::cerr << "convertor error: " << e.what() << std::endl;
        return false;
    }
    (void)dynamic_memory;
    return true;
}

static inline int hako_convert_ros2pdu_GetSimStateResponse(hako_srv_msgs::msg::GetSimStateResponse &src, Hako_GetSimStateResponse** dst)
{
    PduDynamicMemory dynamic_memory;
    Hako_GetSimStateResponse out;
    if (!_ros2pdu_GetSimStateResponse(src, out, dynamic_memory)) {
        return -1;
    }
    int heap_size = dynamic_memory.get_total_size();
    void* base_ptr = hako_create_empty_pdu(sizeof(Hako_GetSimStateResponse), heap_size);
    if (base_ptr == nullptr) {
        return -1;
    }
    // Copy out on base data
    memcpy(base_ptr, (void*)&out, sizeof(Hako_GetSimStateResponse));

    // Copy dynamic part and set offsets
    void* heap_ptr = hako_get_heap_ptr_pdu(base_ptr);
    dynamic_memory.copy_to_pdu((char*)heap_ptr);

    *dst = (Hako_GetSimStateResponse*)base_ptr;
    return hako_get_pdu_meta_data(base_ptr)->total_size;
}

static inline Hako_GetSimStateResponse* hako_create_empty_pdu_GetSimStateResponse(int heap_size)
{
    // Allocate PDU memory
    char* base_ptr = (char*)hako_create_empty_pdu(sizeof(Hako_GetSimStateResponse), heap_size);
    if (base_ptr == nullptr) {
        return nullptr;
    }
    return (Hako_GetSimStateResponse*)base_ptr;
}
#endif /* _PDU_CTYPE_CONV_HAKO_hako_srv_msgs_GetSimStateResponse_HPP_ */
