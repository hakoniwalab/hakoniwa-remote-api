#include "concrete_service_handler.hpp"
#include "hakoniwa/hako_capi.h"

#include "hako_srv_msgs/pdu_cpptype_conv_JoinRequestPacket.hpp"
#include "hako_srv_msgs/pdu_cpptype_conv_JoinResponsePacket.hpp"

#include "hako_srv_msgs/pdu_cpptype_conv_GetSimStateRequestPacket.hpp"
#include "hako_srv_msgs/pdu_cpptype_conv_GetSimStateResponsePacket.hpp"

#include "hakoniwa/pdu/rpc/rpc_service_helper.hpp"

namespace hakoniwa::api {

void JoinHandler::handle(
    ServerContext& service_context,
    std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> service_rpc,
    hakoniwa::pdu::rpc::RpcRequest& request)
{
    HakoRpcServiceServerTemplateType(Join) service_helper;
    std::string message = "Join request Sccessed.";
    std::cout << "Handling join request from client: "
              << request.client_name << std::endl;

    Hako_int32 result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK;

    {
        std::lock_guard<std::mutex> lock(service_context.get_mutex());
        if (service_context.get_status()
            != ServerServiceContextStatus::SERVER_SERVICE_NOT_READY)
        {
            result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_ERROR;
            message = "Server service is not ready.";
        } else {
            service_context.set_status(
                ServerServiceContextStatus::SERVER_SERVICE_STARTED);
        }
    }
    if (request.client_name != service_context.get_client_node_id()) {
        std::cerr << "WARNING: Client node ID mismatch. Expected '"
                  << service_context.get_client_node_id()
                  << "', got '" << request.client_name << "'." << std::endl;
        result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_INVALID;
        message = "Client node ID mismatch.";
    }

    auto ret = hako_asset_register_polling(service_context.get_client_node_id().c_str());
    if (!ret) {
        std::cerr << "ERROR: Failed to register asset polling for client '"
                  << service_context.get_client_node_id() << "'." << std::endl;
        result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_ERROR;
        message = "Failed to register asset polling.";
    }
    else {
        HakoCpp_JoinRequest request_body;
        ret = service_helper.get_request_body(request, request_body);
        if (!ret) {
            std::cerr << "ERROR: Failed to get join request body." << std::endl;
            result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_INVALID;
            message = "Invalid join request body.";
        }
    }
    HakoCpp_JoinResponse response_body;
    response_body.status_code = result_code;
    response_body.message = message;
    service_helper.reply(
        *service_rpc,
        request,
        hakoniwa::pdu::rpc::HAKO_SERVICE_STATUS_DONE,
        result_code,
        response_body);
}

void GetSimStateHandler::handle(
    ServerContext& service_context,
    std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> service_rpc,
    hakoniwa::pdu::rpc::RpcRequest& request)
{
    HakoRpcServiceServerTemplateType(GetSimState) service_helper;
    std::cout << "Handling get_sim_state request from client: "
              << request.client_name << std::endl;

    Hako_int32 result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK;
    if (request.client_name != service_context.get_client_node_id()) {
        std::cerr << "WARNING: Client node ID mismatch. Expected '"
                  << service_context.get_client_node_id()
                  << "', got '" << request.client_name << "'." << std::endl;
        result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_INVALID;
    }
    HakoCpp_GetSimStateRequest request_body;
    {
        auto ret = service_helper.get_request_body(request, request_body);
        if (!ret) {
            std::cerr << "ERROR: Failed to get get_sim_state request body." << std::endl;
            result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_INVALID;
        }
    }
    int state = hako_simevent_get_state();
    HakoCpp_GetSimStateResponse response_body;
    response_body.sim_state = static_cast<Hako_uint32>(state);
    service_helper.reply(
        *service_rpc,
        request,
        hakoniwa::pdu::rpc::HAKO_SERVICE_STATUS_DONE,
        result_code,
        response_body);
}

}
