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

ServerCore::ServerCore(std::string config_path, std::string node_id, bool enable_conductor)
    : config_path_(std::move(config_path)), node_id_(std::move(node_id)), enable_conductor_(enable_conductor) {
    // Constructor initializes the config path.
    // The rest of the initialization happens in start().
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
        if (!config_.contains("servers") || !config_["servers"].is_array()) {
            set_last_error("Config error: 'servers' array not found or not an array.");
            return false;
        }
        bool node_id_found = false;
        for (const auto& server : config_["servers"]) {
            if (server["nodeId"] == node_id_) {
                node_id_found = true;
                break;
            }
        }
        if (!node_id_found) {
            set_last_error("Config error: 'servers' does not match the provided node ID.");
            return false;
        }
        bool client_found = false;
        for (const auto& client : config_["participants"]) {
            if (client["server_nodeId"] == node_id_) {
                client_found = true;
                server_context_.set_client_node_id(client["nodeId"]);
                break;
            }
        }
        if (!client_found) {
            set_last_error("Config error: No participant found for server node ID '" + node_id_ + "'.");
            return false;
        }
    
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
        if (!config_.contains("max_delay_time_usec") || !config_["max_delay_time_usec"].is_number_unsigned()) {
            set_last_error("Config error: 'max_delay_time_usec' not found or not an unsigned number.");
            return false;
        }
        uint64_t max_delay_time_usec = config_["max_delay_time_usec"].get<uint64_t>();
        if (max_delay_time_usec < delta_time_usec_) {
            set_last_error("Config error: 'max_delay_time_usec' must be greater than or equal to 'delta_time_usec'.");
            return false;
        }
        if (enable_conductor_) {
            std::cout << "Conductor mode enabled." << std::endl;
            if (hako_master_init() == false) {
                set_last_error("Failed to initialize Hako master.");
                return false;
            }
            hako_master_set_config_simtime(max_delay_time_usec_, delta_time_usec_);
        }
        bool ret = hako_asset_init();
        if (!ret) {
            set_last_error("Failed to initialize Hako asset.");
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
        handlers_["HakoRemoteApi/GetSimState"] = std::make_unique<GetSimStateHandler>();
        handlers_["HakoRemoteApi/SimControl"] = std::make_unique<SimControlHandler>();
        handlers_["HakoRemoteApi/GetEvent"] = std::make_unique<GetEventHandler>();
        handlers_["HakoRemoteApi/AckEvent"] = std::make_unique<AckEventHandler>();

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
    if (enable_conductor_) {
        conductor_thread_ = std::thread(&ServerCore::conductor_loop, this);
    }
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

    std::cout << "Waiting for server threads to join..." << std::endl;
    if (serve_thread_.joinable()) {
        serve_thread_.join();
    }
    std::cout << "Serve thread joined." << std::endl;
    if (service_handle_thread_.joinable()) {
        service_handle_thread_.join();
    }
    std::cout << "Service handler thread joined." << std::endl;
    if (rpc_server_) {
        rpc_server_->stop_all_services();
    }
    std::cout << "RPC services stopped." << std::endl;
    if (enable_conductor_ && conductor_thread_.joinable()) {
        conductor_thread_.join();
    }
    
    is_running_ = false;
    std::cout << "Hakoniwa Remote API Server stopped." << std::endl;
    return true;
}

void ServerCore::conductor_loop() {
    while (!stop_requested_) {
        bool simulation_progressed = hako_master_execute();
        if (!simulation_progressed) {
            // If no progress, sleep for delta time
            std::this_thread::sleep_for(std::chrono::microseconds(delta_time_usec_));
        }
        else  {
            // Yield to allow other threads to run
            std::this_thread::yield();
        }
    }
}

void ServerCore::serve() {
    std::cout << "Server serving thread started." << std::endl;
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
    std::cout << "Server serving thread exit." << std::endl;
    handler_cv_.notify_all(); // for stop
}

void ServerCore::handle() {
    std::cout << "Service handler thread started." << std::endl;
    while (!stop_requested_) {
        std::pair<std::string, hakoniwa::pdu::rpc::RpcRequest> job;
        bool has_job = false;

        {
            std::unique_lock<std::mutex> lock(handler_mutex_);
            handler_cv_.wait(lock, [&]{
                return stop_requested_ || !pending_requests_.empty();
            });

            if (stop_requested_) {
                std::cout << "Service handler thread exit." << std::endl;
                return;
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
