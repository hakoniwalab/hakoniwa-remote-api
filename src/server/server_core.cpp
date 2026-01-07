#include "hakoniwa/api/server_core.hpp"
#include "hakoniwa/pdu/rpc/rpc_services_server.hpp"
#include "concrete_service_handler.hpp"
#include "hakoniwa/hako_capi.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <string>
#include <filesystem>

namespace hakoniwa::api {

ServerCore::ServerCore(std::string config_path, std::string client_node_id)
    : config_path_(std::move(config_path)) {
    // Constructor initializes the config path.
    // The rest of the initialization happens in start().
    server_context_.set_client_node_id(std::move(client_node_id));
}

ServerCore::~ServerCore() {
    if (is_running()) {
        stop();
    }
}

bool ServerCore::initialize() {
    std::lock_guard<std::mutex> lock(start_mutex_);

    if (is_running()) {
        set_last_error("Server is already running.");
        return false;
    }
    if (is_initialized_.load()) {
        set_last_error("Server is already initialized.");
        return false;
    }
    bool ret = hako_asset_init();
    if (!ret) {
        set_last_error("Failed to initialize Hako asset.");
        return false;
    }

    // 1. Parse remote-api.json config and extract values
    try {
        std::ifstream ifs(config_path_);
        if (!ifs.is_open()) {
            set_last_error("Failed to open config file: " + config_path_);
            return false;
        }
        auto& config_ = server_context_.get_config();
        ifs >> config_;

        // Check for "server" and "server.nodeId"
        if (!config_.contains("server") || !config_["server"].is_object()) {
            set_last_error("Config error: 'server' object not found or not an object.");
            return false;
        }
        if (!config_["server"].contains("nodeId") || !config_["server"]["nodeId"].is_string()) {
            set_last_error("Config error: 'server.nodeId' not found or not a string.");
            return false;
        }
        node_id_ = config_["server"]["nodeId"];

        // Check for "delta_time_usec"
        if (!config_.contains("delta_time_usec") || !config_["delta_time_usec"].is_number_unsigned()) {
            set_last_error("Config error: 'delta_time_usec' not found or not an unsigned number.");
            return false;
        }
        delta_time_usec_ = config_["delta_time_usec"].get<uint64_t>();
        if (delta_time_usec_ == 0) {
            set_last_error("Config error: 'delta_time_usec' must be greater than 0.");
            return false;
        }

        // Check for "time_source_type"
        if (!config_.contains("time_source_type") || !config_["time_source_type"].is_string()) {
            set_last_error("Config error: 'time_source_type' not found or not a string.");
            return false;
        }
        std::string time_source_type = config_["time_source_type"];
        time_source_ = hakoniwa::time_source::create_time_source(time_source_type, delta_time_usec_);
        if (!time_source_) {
            set_last_error("Config error: Invalid 'time_source_type': " + time_source_type);
            return false;
        }
        
        // Check for "rpc_service_config_path"
        if (!config_.contains("rpc_service_config_path") || !config_["rpc_service_config_path"].is_string()) {
            set_last_error("Config error: 'rpc_service_config_path' not found or not a string.");
            return false;
        }
        std::filesystem::path base_path = std::filesystem::path(config_path_).parent_path();
        rpc_config_path_ = (base_path / config_["rpc_service_config_path"].get<std::string>()).string();

        // service handlers registration
        handlers_["HakoRemoteApi/Join"] = std::make_unique<JoinHandler>();

    } catch (const nlohmann::json::parse_error& e) {
        set_last_error("Failed to parse configuration file: " + std::string(e.what()));
        return false;
    }

    // 2. Initialize RPC Server
    try {
        // The queue size (1000) is hardcoded for now, as in the test.
        rpc_server_ = std::make_shared<hakoniwa::pdu::rpc::RpcServicesServer>(node_id_, "RpcServerEndpointImpl", rpc_config_path_, 1000);
        if (!rpc_server_->initialize_services()) {
            set_last_error("Failed to initialize RPC services.");
            rpc_server_.reset();
            return false;
        }
    } catch (const std::exception& e) {
        set_last_error("Failed to create RpcServicesServer: " + std::string(e.what()));
        return false;
    }
    is_initialized_ = true;
    return true;
}

bool ServerCore::start() {
    std::lock_guard<std::mutex> lock(start_mutex_);
    if (is_running()) {
        set_last_error("Server is already running.");
        return false;
    }
    if (!is_initialized_.load()) {
        set_last_error("Server is not properly initialized.");
        return false;
    }    
    // 3. Start services and serving thread
    rpc_server_->start_all_services();

    stop_requested_ = false;
    is_running_ = true;
    serve_thread_ = std::thread(&ServerCore::serve, this);
    service_handle_thread_ = std::thread(&ServerCore::handle, this);

    std::cout << "Hakoniwa Remote API Server started." << std::endl;
    return true;
}

bool ServerCore::stop() {
    std::lock_guard<std::mutex> lock(start_mutex_);
    if (!is_running()) {
        // Not an error, just idempotent.
        return true;
    }

    std::cout << "Stopping Hakoniwa Remote API Server..." << std::endl;
    stop_requested_ = true;

    if (serve_thread_.joinable()) {
        serve_thread_.join();
    }
    if (service_handle_thread_.joinable()) {
        service_handle_thread_.join();
    }

    if (rpc_server_) {
        rpc_server_->stop_all_services();
    }
    
    is_running_ = false;
    std::cout << "Hakoniwa Remote API Server stopped." << std::endl;
    return true;
}

void ServerCore::serve() {
    while (!stop_requested_) {
        if (rpc_server_) {
            hakoniwa::pdu::rpc::RpcRequest request;
            auto event = rpc_server_->poll(request);

            if (event == hakoniwa::pdu::rpc::ServerEventType::REQUEST_IN) {
                bool inserted = false;
                {
                    std::lock_guard<std::mutex> lock(handler_mutex_);
                    if (pending_requests_.count(request.header.service_name) == 0) {
                        pending_requests_[request.header.service_name] = request;
                        inserted = true;
                    } else {
                        std::cerr << "WARNING: Overwriting pending request for service: "
                                  << request.header.service_name << std::endl;
                    }
                }
                if (inserted) {
                    handler_cv_.notify_one();
                }
            }
            else if (event == hakoniwa::pdu::rpc::ServerEventType::REQUEST_CANCEL)
            {
                std::lock_guard<std::mutex> lock(handler_mutex_);
                if (pending_requests_.count(request.header.service_name) > 0) {
                    //TODO
                    auto it = handlers_.find(request.header.service_name);
                    if (it != handlers_.end()) {
                        it->second->cancel();
                    } else {
                        std::cerr << "ERROR: No handler registered for service: "
                                  << request.header.service_name << std::endl;
                    }
                } else {
                    std::cerr << "WARNING: No pending request to cancel for service: "
                              << request.header.service_name << std::endl;
                }
            }
        }
        time_source_->sleep_delta_time();
    }
    handler_cv_.notify_all(); // for stop
}

void ServerCore::handle() {
    while (!stop_requested_) {
        std::pair<std::string, hakoniwa::pdu::rpc::RpcRequest> job;
        bool has_job = false;

        {
            std::unique_lock<std::mutex> lock(handler_mutex_);
            handler_cv_.wait(lock, [&]{
                return stop_requested_ || !pending_requests_.empty();
            });

            if (stop_requested_) {
                std::cout << "Service handler thread stopping." << std::endl;
                break;
            }

            job = *pending_requests_.begin();
            has_job = true;
        }

        if (has_job) {
            const auto& service_name = job.first;
            auto it = handlers_.find(service_name);
            if (it != handlers_.end()) {
                it->second->handle(server_context_, rpc_server_, job.second);
                {
                    std::lock_guard<std::mutex> lock(handler_mutex_);
                    pending_requests_.erase(service_name);
                    if (it->second->is_canceled()) {
                        // Reset cancellation state after handling
                        it->second->reset_canceled();
                    }
                }
            } else {
                std::cerr << "ERROR: No handler registered for service: "
                          << service_name << std::endl;
            }
        }
    }
}

std::string ServerCore::last_error() const noexcept {
    std::lock_guard<std::mutex> lock(err_mutex_);
    return last_error_;
}

void ServerCore::set_last_error(std::string msg) {
    std::lock_guard<std::mutex> lock(err_mutex_);
    std::cerr << "ERROR: " << msg << std::endl;
    last_error_ = std::move(msg);
}

} // namespace hakoniwa::api
