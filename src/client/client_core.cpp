#include "hakoniwa/api/client_core.hpp"
#include "hakoniwa/pdu/rpc/rpc_services_client.hpp"
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

#include "hakoniwa/pdu/rpc/rpc_service_helper.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread> // For std::this_thread::sleep_for

namespace hakoniwa::api {


ClientCore::ClientCore(std::string client_name, std::string config_path)
    : client_name_(std::move(client_name)),
      config_path_(std::move(config_path)) {}

ClientCore::~ClientCore() {
  // RpcServiceClient is managed by shared_ptr, so it will be cleaned up
  // automatically.
}

bool ClientCore::initialize() {
  if (is_initialized_) {
    set_last_error("Client is already initialized.");
    return false;
  }

  // 1. Parse remote-api.json config and extract values
  try {
    std::ifstream ifs(config_path_);
    if (!ifs.is_open()) {
      set_last_error("Failed to open config file: " + config_path_);
      return false;
    }
    nlohmann::json config;
    ifs >> config;

    // Check for client and server node IDs
    if (!config.contains("client") || !config["client"].is_object() ||
        !config["client"].contains("nodeId") ||
        !config["client"]["nodeId"].is_string()) {
      set_last_error(
          "Config error: 'client.nodeId' not found or not a string.");
      return false;
    }
    std::string config_client_id = config["client"]["nodeId"];
    if (config_client_id != client_name_) {
        set_last_error("Config error: client name mismatch. Provided: '" + client_name_ + "', Config: '" + config_client_id + "'");
        return false;
    }


    if (!config.contains("server") || !config["server"].is_object() ||
        !config["server"].contains("nodeId") ||
        !config["server"]["nodeId"].is_string()) {
      set_last_error(
          "Config error: 'server.nodeId' not found or not a string.");
      return false;
    }
    server_node_id_ = config["server"]["nodeId"];

    // Check for RPC service config path
    if (!config.contains("rpc_service_config_path") ||
        !config["rpc_service_config_path"].is_string()) {
      set_last_error(
          "Config error: 'rpc_service_config_path' not found or not a string.");
      return false;
    }
    std::filesystem::path base_path =
        std::filesystem::path(config_path_).parent_path();
    rpc_config_path_ =
        (base_path / config["rpc_service_config_path"].get<std::string>())
            .string();

  } catch (const nlohmann::json::parse_error &e) {
    set_last_error("Failed to parse configuration file: " +
                   std::string(e.what()));
    return false;
  } catch (const std::exception &e) {
    set_last_error(
        "An unexpected error occurred during configuration parsing: " +
        std::string(e.what()));
    return false;
  }

  // 2. Initialize RPC Client
  try {
    rpc_client_ = std::make_shared<hakoniwa::pdu::rpc::RpcServicesClient>(
        client_name_, "RpcClientEndpointImpl", rpc_config_path_);
    if (!rpc_client_->initialize_services()) {
      set_last_error("Failed to initialize RPC client.");
      rpc_client_.reset();
      return false;
    }
  } catch (const std::exception &e) {
    set_last_error("Failed to create RpcServiceClient: " +
                   std::string(e.what()));
    return false;
  }

  is_initialized_ = true;
  std::cout << "Hakoniwa Remote API Client initialized." << std::endl;
  return true;
}

bool ClientCore::start() {
    if (!is_initialized_) {
        set_last_error("Client is not initialized.");
        return false;
    }
    if (!rpc_client_->start_all_services()) {
        set_last_error("Failed to start RPC client services.");
        return false;
    }
    std::cout << "Hakoniwa Remote API Client started." << std::endl;
    return true;
}
bool ClientCore::stop() {
    if (!is_initialized_) {
        set_last_error("Client is not initialized.");
        return false;
    }
    rpc_client_->stop_all_services();
    std::cout << "Hakoniwa Remote API Client stopped." << std::endl;
    return true;
}

bool ClientCore::join() {
  if (!is_initialized_) {
    set_last_error("Client is not initialized.");
    return false;
  }
  const std::string service_name = "HakoRemoteApi/Join";
  HakoRpcServiceServerTemplateType(Join) service_helper;
  HakoCpp_JoinRequest request_body;
  HakoCpp_JoinResponse response_body;

  request_body.name = client_name_;
  if (!service_helper.call(*rpc_client_, service_name, request_body, 0)) {
    set_last_error("Failed to call Join service (RPC call failed).");
    return false;
  }

  hakoniwa::pdu::rpc::RpcResponse rpc_response;
  std::string service_name_ret;
  hakoniwa::pdu::rpc::ClientEventType event = hakoniwa::pdu::rpc::ClientEventType::NONE;

  // Poll for response
  auto start_time = std::chrono::high_resolution_clock::now();
  while (event == hakoniwa::pdu::rpc::ClientEventType::NONE || service_name_ret != service_name) {
      event = rpc_client_->poll(service_name_ret, rpc_response);
      if (event == hakoniwa::pdu::rpc::ClientEventType::RESPONSE_TIMEOUT) {
          set_last_error("Join service call timed out.");
          return false;
      } else if (event == hakoniwa::pdu::rpc::ClientEventType::RESPONSE_IN) {
        if (service_name_ret == service_name) {
            std::cout << "Received response for Join service." << std::endl;
            break; // Got the response for Join service
        }
        else {
            std::cerr << "Received response for unknown service: " << service_name_ret << std::endl;
        }
      }
      std::this_thread::sleep_for(std::chrono::microseconds(100)); // Sleep briefly
  }

  if (!service_helper.get_response_body(rpc_response, response_body)) {
    set_last_error("Failed to get Join response body.");
    return false;
  }

  if (response_body.status_code !=
      hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK) {
    set_last_error("Join service returned an error: " + response_body.message);
    return false;
  }
  return true;
}
#if 0

bool ClientCore::get_sim_state(HakoSimulationState &state) {
  if (!is_initialized_) {
    set_last_error("Client is not initialized.");
    return false;
  }
  HakoRpcServiceServerTemplateType(GetSimState) service_helper;
  HakoCpp_GetSimStateRequest request_body;
  HakoCpp_GetSimStateResponse response_body;

  if (!service_helper.call(*rpc_client_, "HakoRemoteApi/GetSimState", request_body, RPC_TIMEOUT_USEC)) {
    set_last_error("Failed to call GetSimState service (RPC call failed).");
    return false;
  }

  hakoniwa::pdu::rpc::RpcResponse rpc_response;
  std::string service_name_ret;
  hakoniwa::pdu::rpc::ClientEventType event = hakoniwa::pdu::rpc::ClientEventType::NONE;

  auto start_time = std::chrono::high_resolution_clock::now();
  while (event == hakoniwa::pdu::rpc::ClientEventType::NONE || service_name_ret != "HakoRemoteApi/GetSimState") {
      event = rpc_client_->poll(service_name_ret, rpc_response);
      if (event == hakoniwa::pdu::rpc::ClientEventType::RESPONSE_TIMEOUT) {
          set_last_error("GetSimState service call timed out.");
          return false;
      } else if (event == hakoniwa::pdu::rpc::ClientEventType::ERROR) {
          set_last_error("GetSimState service call returned an error event.");
          return false;
      }
      if (service_name_ret == "HakoRemoteApi/GetSimState") {
        break; // Got the response for GetSimState service
      }
      std::this_thread::sleep_for(std::chrono::microseconds(100)); // Sleep briefly
      auto current_time = std::chrono::high_resolution_clock::now();
      if (std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time).count() > RPC_TIMEOUT_USEC && RPC_TIMEOUT_USEC != 0) {
        set_last_error("GetSimState service call polling timed out.");
        return false;
      }
  }

  if (!service_helper.get_response_body(rpc_response, response_body)) {
    set_last_error("Failed to get GetSimState response body.");
    return false;
  }

  state = static_cast<HakoSimulationState>(response_body.sim_state);
  return true;
}

bool ClientCore::sim_control(HakoSimulationControlCommand command) {
  if (!is_initialized_) {
    set_last_error("Client is not initialized.");
    return false;
  }
  HakoRpcServiceServerTemplateType(SimControl) service_helper;
  HakoCpp_SimControlRequest request_body;
  request_body.op = static_cast<Hako_int32>(command);
  HakoCpp_SimControlResponse response_body;

  if (!service_helper.call(*rpc_client_, "HakoRemoteApi/SimControl", request_body, RPC_TIMEOUT_USEC)) {
    set_last_error("Failed to call SimControl service (RPC call failed).");
    return false;
  }

  hakoniwa::pdu::rpc::RpcResponse rpc_response;
  std::string service_name_ret;
  hakoniwa::pdu::rpc::ClientEventType event = hakoniwa::pdu::rpc::ClientEventType::NONE;

  auto start_time = std::chrono::high_resolution_clock::now();
  while (event == hakoniwa::pdu::rpc::ClientEventType::NONE || service_name_ret != "HakoRemoteApi/SimControl") {
      event = rpc_client_->poll(service_name_ret, rpc_response);
      if (event == hakoniwa::pdu::rpc::ClientEventType::RESPONSE_TIMEOUT) {
          set_last_error("SimControl service call timed out.");
          return false;
      } else if (event == hakoniwa::pdu::rpc::ClientEventType::ERROR) {
          set_last_error("SimControl service call returned an error event.");
          return false;
      }
      if (service_name_ret == "HakoRemoteApi/SimControl") {
        break; // Got the response for SimControl service
      }
      std::this_thread::sleep_for(std::chrono::microseconds(100)); // Sleep briefly
      auto current_time = std::chrono::high_resolution_clock::now();
      if (std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time).count() > RPC_TIMEOUT_USEC && RPC_TIMEOUT_USEC != 0) {
        set_last_error("SimControl service call polling timed out.");
        return false;
      }
  }

  if (!service_helper.get_response_body(rpc_response, response_body)) {
    set_last_error("Failed to get SimControl response body.");
    return false;
  }

  if (response_body.status_code !=
      hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK) {
    set_last_error("SimControl service returned an error: " + response_body.message);
    return false;
  }
  return true;
}

bool ClientCore::get_event(HakoSimulationAssetEvent &event) {
  if (!is_initialized_) {
    set_last_error("Client is not initialized.");
    return false;
  }
  HakoRpcServiceServerTemplateType(GetEvent) service_helper;
  HakoCpp_GetEventRequest request_body;
  HakoCpp_GetEventResponse response_body;

  if (!service_helper.call(*rpc_client_, "HakoRemoteApi/GetEvent", request_body, RPC_TIMEOUT_USEC)) {
    set_last_error("Failed to call GetEvent service (RPC call failed).");
    return false;
  }

  hakoniwa::pdu::rpc::RpcResponse rpc_response;
  std::string service_name_ret;
  hakoniwa::pdu::rpc::ClientEventType event_type = hakoniwa::pdu::rpc::ClientEventType::NONE;

  auto start_time = std::chrono::high_resolution_clock::now();
  while (event_type == hakoniwa::pdu::rpc::ClientEventType::NONE || service_name_ret != "HakoRemoteApi/GetEvent") {
      event_type = rpc_client_->poll(service_name_ret, rpc_response);
      if (event_type == hakoniwa::pdu::rpc::ClientEventType::RESPONSE_TIMEOUT) {
          set_last_error("GetEvent service call timed out.");
          return false;
      } else if (event_type == hakoniwa::pdu::rpc::ClientEventType::ERROR) {
          set_last_error("GetEvent service call returned an error event.");
          return false;
      }
      if (service_name_ret == "HakoRemoteApi/GetEvent") {
        break; // Got the response for GetEvent service
      }
      std::this_thread::sleep_for(std::chrono::microseconds(100)); // Sleep briefly
      auto current_time = std::chrono::high_resolution_clock::now();
      if (std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time).count() > RPC_TIMEOUT_USEC && RPC_TIMEOUT_USEC != 0) {
        set_last_error("GetEvent service call polling timed out.");
        return false;
      }
  }
  
  if (!service_helper.get_response_body(rpc_response, response_body)) {
    set_last_error("Failed to get GetEvent response body.");
    return false;
  }

  event = static_cast<HakoSimulationAssetEvent>(response_body.event_code);
  return true;
}

bool ClientCore::ack_event(HakoSimulationAssetEvent event_code) {
  if (!is_initialized_) {
    set_last_error("Client is not initialized.");
    return false;
  }
  HakoRpcServiceServerTemplateType(AckEvent) service_helper;
  HakoCpp_AckEventRequest request_body;
  request_body.event_code = static_cast<Hako_uint32>(event_code);
  request_body.result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK; // Assuming success for now
  HakoCpp_AckEventResponse response_body;

  if (!service_helper.call(*rpc_client_, "HakoRemoteApi/AckEvent", request_body, RPC_TIMEOUT_USEC)) {
    set_last_error("Failed to call AckEvent service (RPC call failed).");
    return false;
  }
  
  hakoniwa::pdu::rpc::RpcResponse rpc_response;
  std::string service_name_ret;
  hakoniwa::pdu::rpc::ClientEventType event_type = hakoniwa::pdu::rpc::ClientEventType::NONE;

  auto start_time = std::chrono::high_resolution_clock::now();
  while (event_type == hakoniwa::pdu::rpc::ClientEventType::NONE || service_name_ret != "HakoRemoteApi/AckEvent") {
      event_type = rpc_client_->poll(service_name_ret, rpc_response);
      if (event_type == hakoniwa::pdu::rpc::ClientEventType::RESPONSE_TIMEOUT) {
          set_last_error("AckEvent service call timed out.");
          return false;
      } else if (event_type == hakoniwa::pdu::rpc::ClientEventType::ERROR) {
          set_last_error("AckEvent service call returned an error event.");
          return false;
      }
      if (service_name_ret == "HakoRemoteApi/AckEvent") {
        break; // Got the response for AckEvent service
      }
      std::this_thread::sleep_for(std::chrono::microseconds(100)); // Sleep briefly
      auto current_time = std::chrono::high_resolution_clock::now();
      if (std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time).count() > RPC_TIMEOUT_USEC && RPC_TIMEOUT_USEC != 0) {
        set_last_error("AckEvent service call polling timed out.");
        return false;
      }
  }

  if (!service_helper.get_response_body(rpc_response, response_body)) {
    set_last_error("Failed to get AckEvent response body.");
    return false;
  }

  // AckEvent doesn't have a status_code or message in response_body from current PDU, so just check overall success
  // If needed, the server can set the result_code in the RpcResponse header.
  if (rpc_response.header.result_code != hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK) {
      set_last_error("AckEvent service returned an error result code in RPC header.");
      return false;
  }

  return true;
}

void ClientCore::set_last_error(const std::string &msg) {
  std::cerr << "ERROR: " << msg << std::endl;
  last_error_ = msg;
}

std::string ClientCore::last_error() const { return last_error_; }
#endif
} // namespace hakoniwa::api
