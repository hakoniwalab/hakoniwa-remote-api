#include "concrete_service_handler.hpp"
#include "hakoniwa/hako_capi.h"
#include "hakoniwa/pdu/rpc/rpc_service_helper.hpp"

#include "hako_srv_msgs/pdu_cpptype_conv_AckEventRequestPacket.hpp"
#include "hako_srv_msgs/pdu_cpptype_conv_AckEventResponsePacket.hpp"
#include "hako_srv_msgs/pdu_cpptype_conv_GetEventRequestPacket.hpp"
#include "hako_srv_msgs/pdu_cpptype_conv_GetEventResponsePacket.hpp"
#include "hako_srv_msgs/pdu_cpptype_conv_GetSimStateRequestPacket.hpp"
#include "hako_srv_msgs/pdu_cpptype_conv_GetSimStateResponsePacket.hpp"
#include "hako_srv_msgs/pdu_cpptype_conv_JoinRequestPacket.hpp"
#include "hako_srv_msgs/pdu_cpptype_conv_JoinResponsePacket.hpp"
#include "hako_srv_msgs/pdu_cpptype_conv_SimControlRequestPacket.hpp"
#include "hako_srv_msgs/pdu_cpptype_conv_SimControlResponsePacket.hpp"

namespace hakoniwa::api {

// Helper function to validate client node ID
static bool validate_client_id(ServerContext &service_context,
                               const std::string &client_name,
                               Hako_int32 &result_code) {
  if (client_name != service_context.get_client_node_id()) {
    std::cerr << "WARNING: Client node ID mismatch. Expected '"
              << service_context.get_client_node_id() << "', got '"
              << client_name << "'." << std::endl;
    result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_INVALID;
    return false;
  }
  return true;
}

void JoinHandler::handle(ServerContext &service_context,
                         std::shared_ptr<pdu::rpc::RpcServicesServer> service_rpc,
                         pdu::rpc::RpcRequest &request) {
  HakoRpcServiceServerTemplateType(Join) service_helper;
  std::string message = "Join request Succeeded.";
  std::cout << "Handling join request from client: " << request.client_name
            << std::endl;

  Hako_int32 result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK;
  HakoCpp_JoinRequest request_body;

  // Validate state
  {
    std::lock_guard<std::mutex> lock(service_context.get_mutex());
    if (service_context.get_status() !=
        ServerServiceContextStatus::SERVER_SERVICE_NOT_READY) {
      result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_ERROR;
      message = "Server service is not ready.";
    }
  }

  // Validate client ID and request body
  if (result_code == hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK) {
    if (!validate_client_id(service_context, request.client_name,
                            result_code)) {
      message = "Client node ID mismatch.";
    } else if (!service_helper.get_request_body(request, request_body)) {
      std::cerr << "ERROR: Failed to get join request body." << std::endl;
      result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_INVALID;
      message = "Invalid join request body.";
    }
  }

  // Register asset
  if (result_code == hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK) {
    if (!hako_asset_register_polling(
            service_context.get_client_node_id().c_str())) {
      std::cerr << "ERROR: Failed to register asset polling for client '"
                << service_context.get_client_node_id() << "'." << std::endl;
      result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_ERROR;
      message = "Failed to register asset polling.";
    } else {
        service_context.set_status(ServerServiceContextStatus::SERVER_SERVICE_STARTED);
    }
  }

  HakoCpp_JoinResponse response_body;
  response_body.status_code = result_code;
  response_body.message = message;
  service_helper.reply(*service_rpc, request,
                       hakoniwa::pdu::rpc::HAKO_SERVICE_STATUS_DONE,
                       result_code, response_body);
}

void GetSimStateHandler::handle(
    ServerContext &service_context,
    std::shared_ptr<pdu::rpc::RpcServicesServer> service_rpc,
    pdu::rpc::RpcRequest &request) {
  HakoRpcServiceServerTemplateType(GetSimState) service_helper;
  std::cout << "Handling get_sim_state request from client: "
            << request.client_name << std::endl;

  Hako_int32 result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK;
  HakoCpp_GetSimStateRequest request_body;

  if (!validate_client_id(service_context, request.client_name, result_code)) {
    // Error handled in helper
  } else if (!service_helper.get_request_body(request, request_body)) {
    std::cerr << "ERROR: Failed to get get_sim_state request body."
              << std::endl;
    result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_INVALID;
  }

  HakoCpp_GetSimStateResponse response_body;
  if (result_code == hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK) {
    response_body.sim_state =
        static_cast<Hako_uint32>(hako_simevent_get_state());
    response_body.master_time = static_cast<int64_t>(hako_asset_get_worldtime());
    response_body.is_pdu_created = hako_asset_is_pdu_created();
    response_body.is_simulation_mode = hako_asset_is_simulation_mode();
    response_body.is_pdu_sync_mode = hako_asset_is_pdu_sync_mode(request.client_name.c_str());
  } else {
    response_body.sim_state = -1; // Indicate error
  }
  service_helper.reply(*service_rpc, request,
                       hakoniwa::pdu::rpc::HAKO_SERVICE_STATUS_DONE,
                       result_code, response_body);
}

void SimControlHandler::handle(
    ServerContext &service_context,
    std::shared_ptr<pdu::rpc::RpcServicesServer> service_rpc,
    pdu::rpc::RpcRequest &request) {
  HakoRpcServiceServerTemplateType(SimControl) service_helper;
  std::string message = "SimControl request Succeeded.";
  std::cout << "Handling sim_control request from client: "
            << request.client_name << std::endl;

  Hako_int32 result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK;
  HakoCpp_SimControlRequest request_body;

  if (!validate_client_id(service_context, request.client_name, result_code)) {
    message = "Client node ID mismatch.";
  } else if (!service_helper.get_request_body(request, request_body)) {
    std::cerr << "ERROR: Failed to get SimControl request body." << std::endl;
    result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_INVALID;
    message = "Invalid SimControl request body.";
  } else {
    bool ret = false;
    auto op = static_cast<HakoSimulationControlCommand>(request_body.op);
    switch (op) {
    case HakoSimulationControlCommand::HakoSimControl_Start:
      ret = hako_simevent_start();
      break;
    case HakoSimulationControlCommand::HakoSimControl_Stop:
      ret = hako_simevent_stop();
      break;
    case HakoSimulationControlCommand::HakoSimControl_Reset:
      ret = hako_simevent_reset();
      break;
    default:
      std::cerr << "ERROR: Invalid SimControl operation: " << request_body.op
                << std::endl;
      result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_INVALID;
      message = "Invalid SimControl operation.";
      break;
    }

    if (result_code == hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK && !ret) {
      std::cerr << "ERROR: Failed to execute SimControl operation: "
                << request_body.op << std::endl;
      result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_ERROR;
      message = "Failed to execute SimControl operation.";
    }
  }

  HakoCpp_SimControlResponse response_body;
  response_body.status_code = result_code;
  response_body.message = message;
  service_helper.reply(*service_rpc, request,
                       hakoniwa::pdu::rpc::HAKO_SERVICE_STATUS_DONE,
                       result_code, response_body);
}

void GetEventHandler::handle(
    ServerContext &service_context,
    std::shared_ptr<pdu::rpc::RpcServicesServer> service_rpc,
    pdu::rpc::RpcRequest &request) {
  HakoRpcServiceServerTemplateType(GetEvent) service_helper;
  std::cout << "Handling get_event request from client: " << request.client_name
            << std::endl;

  Hako_int32 result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK;

  validate_client_id(service_context, request.client_name, result_code);

  HakoCpp_GetEventResponse response_body;
  if (result_code == hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK) {
    int event_code = hako_asset_get_event(request.client_name.c_str());
    if (static_cast<HakoSimulationAssetEvent>(event_code) ==
        HakoSimulationAssetEvent::HakoSimAssetEvent_Error) {
      std::cerr << "ERROR: hako_asset_get_event() failed for client '"
                << request.client_name << "'." << std::endl;
      result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_ERROR;
    } else {
      response_body.event_code = static_cast<Hako_uint32>(event_code);
    }
  }

  service_helper.reply(*service_rpc, request,
                       hakoniwa::pdu::rpc::HAKO_SERVICE_STATUS_DONE,
                       result_code, response_body);
}

void AckEventHandler::handle(
    ServerContext &service_context,
    std::shared_ptr<pdu::rpc::RpcServicesServer> service_rpc,
    pdu::rpc::RpcRequest &request) {
  HakoRpcServiceServerTemplateType(AckEvent) service_helper;
  std::cout << "Handling ack_event request from client: " << request.client_name
            << std::endl;

  Hako_int32 result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK;
  HakoCpp_AckEventRequest request_body;

  if (!validate_client_id(service_context, request.client_name, result_code)) {
    // Error set in helper
  } else if (!service_helper.get_request_body(request, request_body)) {
    std::cerr << "ERROR: Failed to get AckEvent request body." << std::endl;
    result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_INVALID;
  } else if (request_body.result_code !=
             hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK) {
    std::cerr << "ERROR: AckEvent request contains error result code: "
              << request_body.result_code << std::endl;
    result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_INVALID;
  }

  if (result_code == hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK) {
    bool ret = false;
    auto event_code =
        static_cast<HakoSimulationAssetEvent>(request_body.event_code);
    switch (event_code) {
    case HakoSimulationAssetEvent::HakoSimAssetEvent_Start:
      ret = hako_asset_start_feedback(request.client_name.c_str(), true);
      break;
    case HakoSimulationAssetEvent::HakoSimAssetEvent_Stop:
      ret = hako_asset_stop_feedback(request.client_name.c_str(), true);
      break;
    case HakoSimulationAssetEvent::HakoSimAssetEvent_Reset:
      ret = hako_asset_reset_feedback(request.client_name.c_str(), true);
      break;
    default:
      std::cerr << "ERROR: AckEvent request contains unknown event code: "
                << request_body.event_code << std::endl;
      result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_INVALID;
      // Set ret to true to avoid falling into the error check below for this specific case
      ret = true;
      break;
    }

    if (!ret) {
        std::cerr << "ERROR: hako_asset_*_feedback failed for event " << request_body.event_code << std::endl;
        result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_ERROR;
    }
  }

  HakoCpp_AckEventResponse response_body;
  service_helper.reply(*service_rpc, request,
                       hakoniwa::pdu::rpc::HAKO_SERVICE_STATUS_DONE,
                       result_code, response_body);
}

} // namespace hakoniwa::api
