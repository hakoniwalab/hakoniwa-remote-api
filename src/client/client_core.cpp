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


ClientCore::ClientCore(std::string node_id, std::string config_path)
    : node_id_(std::move(node_id)),
      config_path_(std::move(config_path)) {}

ClientCore::~ClientCore() {
  // RpcServiceClient is managed by shared_ptr, so it will be cleaned up
  // automatically.
}

bool ClientCore::initialize(std::shared_ptr<hakoniwa::pdu::EndpointContainer> endpoint_container) {
  if (is_initialized_) {
    set_last_error("Client is already initialized.");
    return false;
  }
  endpoint_container_ = endpoint_container;

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
    if (!config.contains("participants") || !config["participants"].is_array()) {
      set_last_error(
          "Config error: 'client.nodeId' not found or not a string.");
      return false;
    }
    bool client_found = false;
    for (const auto& item : config["participants"].items()) {
        if (!item.value().is_object() ||
            !item.value().contains("name") ||
            !item.value().contains("nodeId") ||
            !item.value().contains("server_nodeId") ||
            !item.value().contains("delta_time_usec") ||
            !item.value()["nodeId"].is_string()) {
          set_last_error("Config error: 'participants' entry malformed.");
          return false;
        }
        if (item.value()["nodeId"] == node_id_) {
            client_name_ = item.value()["name"];
            server_node_id_ = item.value()["server_nodeId"];
            delta_time_usec_ = item.value()["delta_time_usec"];
            std::cout << "Client node ID: " << client_name_ << ", Server node ID: " << server_node_id_ << ", Delta time (usec): " << delta_time_usec_ << std::endl;
            client_found = true;
            break;
        }
    }
    if (!client_found) {
        set_last_error("Config error: client nodeId '" + node_id_ + "' not found in participants.");
        return false;
    }

    if (!config.contains("servers") || !config["servers"].is_array()) {
      set_last_error(
          "Config error: 'servers' array not found or not an array.");
      return false;
    }

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
        node_id_, client_name_, rpc_config_path_, "RpcClientEndpointImpl", delta_time_usec_, "real");
    if (!rpc_client_->initialize_services(endpoint_container_)) {
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
bool ClientCore::is_pdu_end_point_running() {
    return endpoint_container_ && endpoint_container_->is_running_all();
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
bool ClientCore::wait_response_for(const std::string& expected_service, hakoniwa::pdu::rpc::RpcResponse& out_resp) {
    std::string service_name_ret;
    hakoniwa::pdu::rpc::ClientEventType event = hakoniwa::pdu::rpc::ClientEventType::NONE;
    while (event == hakoniwa::pdu::rpc::ClientEventType::NONE || service_name_ret != expected_service) {
        event = rpc_client_->poll(service_name_ret, out_resp);
        if (event == hakoniwa::pdu::rpc::ClientEventType::RESPONSE_TIMEOUT) {
            set_last_error("Join service call timed out.");
            return false;
        } else if (event == hakoniwa::pdu::rpc::ClientEventType::RESPONSE_IN) {
            if (service_name_ret == expected_service) {
                //std::cout << "Received response for " << expected_service << " service." << std::endl;
                break; // Got the response for expected service
                return true;
            }
            else {
                std::cerr << "Received response for unknown service: " << service_name_ret << std::endl;
                return false;
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100)); // Sleep briefly
    }
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

  request_body.name = node_id_;
  if (!service_helper.call(*rpc_client_, service_name, request_body, 0)) {
    set_last_error("Failed to call Join service (RPC call failed).");
    return false;
  }

  // Poll for response
  hakoniwa::pdu::rpc::RpcResponse rpc_response;
  if (!wait_response_for(service_name, rpc_response)) {
      return false; // Error already set in wait_response_for
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
bool ClientCore::get_sim_state(HakoSimulationStateInfo &state) {
    if (!is_initialized_) {
        set_last_error("Client is not initialized.");
        return false;
    }
    const std::string service_name = "HakoRemoteApi/GetSimState";
    HakoRpcServiceServerTemplateType(GetSimState) service_helper;
    HakoCpp_GetSimStateRequest request_body;
    HakoCpp_GetSimStateResponse response_body;

    request_body.name = node_id_;
    if (!service_helper.call(*rpc_client_, service_name, request_body, 0)) {
        set_last_error("Failed to call GetSimState service (RPC call failed).");
        return false;
    }

    hakoniwa::pdu::rpc::RpcResponse rpc_response;
    if (!wait_response_for(service_name, rpc_response)) {
        return false; // Error already set in wait_response_for
    }
    

    if (!service_helper.get_response_body(rpc_response, response_body)) {
        set_last_error("Failed to get GetSimState response body.");
        return false;
    }

    state.sim_state = static_cast<HakoSimulationState>(response_body.sim_state);
    state.master_time = response_body.master_time;
    state.is_pdu_created = response_body.is_pdu_created;
    state.is_simulation_mode = response_body.is_simulation_mode;
    state.is_pdu_sync_mode = response_body.is_pdu_sync_mode;
    return true;
}

bool ClientCore::sim_control(HakoSimulationControlCommand command) {

    if (!is_initialized_) {
        set_last_error("Client is not initialized.");
        return false;
    }
    const std::string service_name = "HakoRemoteApi/SimControl";
    HakoRpcServiceServerTemplateType(SimControl) service_helper;
    HakoCpp_SimControlRequest request_body;
    request_body.name = node_id_;
    request_body.op = static_cast<Hako_int32>(command);
    HakoCpp_SimControlResponse response_body;

    if (!service_helper.call(*rpc_client_, service_name, request_body, 0)) {
        set_last_error("Failed to call SimControl service (RPC call failed).");
        return false;
    }

    hakoniwa::pdu::rpc::RpcResponse rpc_response;
    if (!wait_response_for(service_name, rpc_response)) {
        return false; // Error already set in wait_response_for
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

bool ClientCore::get_event(HakoSimulationAssetEvent &event_code) {
    if (!is_initialized_) {
        set_last_error("Client is not initialized.");
        return false;
    }
    const std::string service_name = "HakoRemoteApi/GetEvent";
    HakoRpcServiceServerTemplateType(GetEvent) service_helper;
    HakoCpp_GetEventRequest request_body;
    HakoCpp_GetEventResponse response_body;

    request_body.name = node_id_;
    if (!service_helper.call(*rpc_client_, service_name, request_body, 0)) {
        std::cerr << "DEBUG: service_helper.call failed in get_event()" << std::endl;
        set_last_error("Failed to call GetEvent service (RPC call failed).");
        return false;
    }

    hakoniwa::pdu::rpc::RpcResponse rpc_response;
    if (!wait_response_for(service_name, rpc_response)) {
        std::cerr << "DEBUG: wait_response_for failed in get_event()" << std::endl;
        return false; // Error already set in wait_response_for
    }

    if (!service_helper.get_response_body(rpc_response, response_body)) {
        std::cerr << "DEBUG: get_response_body failed in get_event()" << std::endl;
        set_last_error("Failed to get GetEvent response body.");
        return false;
    }
    event_code = static_cast<HakoSimulationAssetEvent>(response_body.event_code);

    return true;
}


bool ClientCore::ack_event(HakoSimulationAssetEvent event_code) {
    if (!is_initialized_) {
        set_last_error("Client is not initialized.");
        return false;
    }
    const std::string service_name = "HakoRemoteApi/AckEvent";
    HakoRpcServiceServerTemplateType(AckEvent) service_helper;
    HakoCpp_AckEventRequest request_body;
    request_body.name = node_id_;
    request_body.event_code = static_cast<Hako_uint32>(event_code);
    request_body.result_code = hakoniwa::pdu::rpc::HAKO_SERVICE_RESULT_CODE_OK; // Assuming success for now
    HakoCpp_AckEventResponse response_body;

    if (!service_helper.call(*rpc_client_, service_name, request_body, 0)) {
        set_last_error("Failed to call AckEvent service (RPC call failed).");
        return false;
    }

    hakoniwa::pdu::rpc::RpcResponse rpc_response;
    if (!wait_response_for(service_name, rpc_response)) {
        return false; // Error already set in wait_response_for
    }

    if (!service_helper.get_response_body(rpc_response, response_body)) {
        set_last_error("Failed to get AckEvent response body.");
        return false;
    }
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

} // namespace hakoniwa::api
