#pragma once

#include <atomic>
#include <string>
#include <string_view>
#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <nlohmann/json.hpp>

#include "hakoniwa/api/iservice_handler.hpp"
#include "hakoniwa/api/server_context.hpp"

namespace hakoniwa::time_source { class ITimeSource; }
namespace hakoniwa::pdu::rpc { class RpcServicesServer; }

namespace hakoniwa::api {

class ServerCore {
public:
    ServerCore(std::string config_path, std::string node_id, bool enable_conductor = false);
    ~ServerCore(); // stop() して join する前提

    bool initialize(std::shared_ptr<hakoniwa::pdu::EndpointContainer> endpoint_container); // if needed in future
    bool initialize_rpc_services(); // separate init for rpc services
    bool start();  // non-blocking
    bool stop();   // idempotent

    bool is_running() const noexcept { return is_running_.load(); }
    std::string last_error() const noexcept;
    void set_conductor_timing(uint64_t delta_time_usec, uint64_t max_delay_time_usec) noexcept;

private:
    void serve();
    void handle();
    void conductor_loop();
    void set_last_error(std::string msg);

private:
    std::string config_path_;
    std::string node_id_;
    std::string last_error_;
    std::string rpc_config_path_;
    uint64_t poll_sleep_time_usec_{100000};
    uint64_t conductor_delta_time_usec_{1000};
    uint64_t conductor_max_delay_time_usec_{10000};

    std::atomic<bool> is_running_{false};
    std::atomic<bool> stop_requested_{false};
    std::atomic<bool> is_initialized_{false};
    bool enable_conductor_{false};

    std::mutex start_mutex_;
    mutable std::mutex err_mutex_;
    std::unique_ptr<hakoniwa::time_source::ITimeSource> time_source_;
    std::shared_ptr<hakoniwa::pdu::EndpointContainer> endpoint_container_;
    std::shared_ptr<hakoniwa::pdu::rpc::RpcServicesServer> rpc_server_;
    std::thread serve_thread_;
    std::thread conductor_thread_;

    std::thread service_handle_thread_;
    std::mutex handler_mutex_;
    std::condition_variable handler_cv_;
    // service_name, handler
    std::unordered_map<std::string, std::unique_ptr<hakoniwa::api::IServiceHandler>> handlers_;
    // service_name, pending requests
    std::unordered_map<std::string, hakoniwa::pdu::rpc::RpcRequest> pending_requests_;

    ServerContext server_context_;

};

} // namespace hakoniwa::api
